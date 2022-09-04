# 服务端

## ebpf kernel端

### pr0be.safe

这里借鉴（chao）的是 [bad_bpf](bad_bpf.md) 中的pidhide程序，用于实现对某个pid进程的隐藏，这里直接隐藏了自己的进程

### pr0be.boop

#### 变量

##### event映射

```c
struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 8192);
  __type(key, int);
  __type(value, struct event_boop_t);
} event SEC(".maps");
```

用于存放抓包结果产生的事件，与 [boopkit](#boopkit) 进行通信

#### 函数

实现了两个hook

##### tcp_bad_csum

```
tp/tcp/tcp_bad_csum
```

该hook是用于拦截checksum错误的包的，boopkit使用的后门触发方式是发一个checksum错误的包，因为这种包一定会被协议栈drop掉，所以十分隐蔽

* 截获这种包后，将其源地址及包类型（这里定义为EVENT_SRC_BAD_CSUM）存放到event映射中

##### tcp_receive_reset

```
tp/tcp/tcp_receive_reset
```

该hook是用于拦截tcp的RST包

* 截获这种包后，将其源地址及包类型（这里定义为EVENT_SRC_RECEIVE_RESET）存放到event映射中

## ebpf user端

### boopkit

#### 变量

##### map_prog_array

pr0be.safe中存放用于进程隐藏的尾调用函数的映射

#### 函数

##### main

* clisetup  定义一系列环境变量及处理命令行参数

* asciiheader  打印logo（噗）

* uid_check  检测当前uid，并检测了是否为sudo，方法是判断pid和ppid是否连号

* 创建一个线程，运行 [xcap](#xcap) 抓包程序

* 下面操作用于加载并初始化pr0be.safe模块
  
  * 加载ebpf程序pr0be.safe，并初始化一系列全局变量
    
    * pid_to_hide  当前pid，用于隐藏进程
    
    * target_ppid  0，所以目标是root用户
  
  * 初始化 [map_prog_array](#map_prog_array) ，其中1号为handle_getdents_exit，2号为handle_getdents_patch，用于程序的尾调用（bpf_tail_call）
  
  * attach pr0be.safe程序，并初始化ring_buffer用于通信

* 下面操作用于加载并初始化pr0be.boop模块
  
  * 加载ebpf程序pr0be.boop
  
  * attach pr0be.boop程序，这里没有使用skeleton生成的函数，而是采用下列方式
    
    * bpf_object__next_map  初始化迭代器
    
    * bpf_object__for_each_program  迭代
      
      * bpf_program__attach  挂载ebpf程序
      
      * 此外这里通过两个helper函数获取了程序名和段名
        
        * bpf_program__name
        
        * bpf_program__section_name

* 下面进入一个消息处理的循环
  
  * 遍历pr0be.boop中的 [event映射](#event映射)
    
    * 获取每个记录的源地址
    
    * 判断源地址是否在配置中的deny列表中
      
      * 若是则忽略并删除该项
      
      * 否则
        
        * 调用 [xcaprce](#xcaprce) ，若发现当前包是一个rce包，则调用exec执行对应命令
        
        * 若配置了 reverseconn 选项，则表示是一个反向shell，调用 [recvrce](#recvrce) 连接并执行命令

##### recvrce

创建一个socket连接到远程，读入一条命令并复制到缓冲区返回，用于创建一个反向的shell进行命令执行

### dpi

该文件定义了一系列使用pcap抓包的操作，在使用ebpf程序触发后门后，boopkit使用pcap对通信包进行解析，此外在user端启动的时候同时调用了xcap函数作为线程进行抓包

#### 数据结构

这里定义了一个结构ringbuffer，有如下操作

* xcap_ring_buffer_init  初始化一个ring buffer，其中传入的参数是一个xcap_ip_packet指针数组，其大小等于XCAP_BUFFER_SIZE，所以ring buffer实际上就是这样一个由指针数组维护的结构体数组

* xcap_ring_buffer_free  释放一个xcap_ip_packet结构

* xcap_ring_buffer_dump  打印所有捕获的packet

#### 函数

##### xcap

* 进行一系列初始化，抓取所有包

* 循环将每个抓取的包放入 xcap_ring_buffer 中

##### snapshot

为 xcap_ring_buffer 创建一个快照到传入的 ring buffer 中（即复制过去）

##### xcaprce

```c
int xcaprce(char search[INET_ADDRSTRLEN], char *rce);
```



* 初始化一个ring buffer

* 调用 snapshot 获取当前捕获的包的快照（这里并非是所有的包，因为ringbuffer在缓冲区满时会覆盖之前的包）

* 对ring buffer中所有packet进行循环
  
  * 若packet的ipheader的源地址为要查找的地址search
    
    * 查找包的内容中是否含有预定义的用于标识rce指令的字符串BOOPKIT_RCE_DELIMITER（默认为`X*x.x*X`）
    
    * 调用rce_filter提取指令

# 客户端

### boopkit-boop

流程大体如下

* 第一步  用于触发后门
  
  * 打开一个 **raw socket** 连接远程
  
  * 构造并发送一个checksum错误的包触发后门，这里具体的构造阶段细节还是比较多的，比如需要构造一个TCP伪报头来计算checksum（但这里checksum最后其实就是直接传了个0）
  
  * 关闭连接

* 第二步  用于验证连接可用性
  
  * 打开一个 STREAM socket 连接远程
  
  * 关闭

* 第三步  构造一个RST连接，这是另一种比较稳定的触发后门的方式，注释里的说法是，前一种使用checksum的方式有时候会出错（可能是因为中途经过各种设备的时候因为checksum不对直接就丢了？）
  
  * 打开一个 raw socket 连接远程
  
  * 构造并发送一个 SYN 包
  
  * 接收一个 SYN-ACK 包
  
  * 解析 seq_num 和 ack_num，并构造一个 ACK-RST 包发送
  
  * 关闭

* 第四步  发送命令进行执行，若配置为反向shell则调用serverce监听反向连接的shell














