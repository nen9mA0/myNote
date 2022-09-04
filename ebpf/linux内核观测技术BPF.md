### BPF映射

BPF程序的运行模式是，由用户程序启动一个在内核运行的BPF程序。两边通信的机制即BPF映射

BPF映射以键值形式保存在内核中，可以被任何BPF程序访问

#### 创建BPF映射

* 使用bpf系统调用，第一个参数设置为BPF_MAP_CREATE可以创建一个新映射，返回相关的文件描述符。第二个参数为BPF映射的设置，第三个参数是设置属性的大小

* 也可以使用helper函数 bpf_create_map 创建

* 还可以使用预定义映射
  
  ```c
  struct bpf_map_def SEC("maps") my_map = {
      .type        = BPF_MAP_TYPE_HASH,
      .key_size    = sizeof(int),
      .value_size  = sizeof(int),
      .max_entries = 100,
      .map_flags   = BPF_F_NO_PREALLOC,
  };
  ```
  
  这里的 `SEC("maps")` 可以告诉内核该结构是BPF映射
  
  使用该方法创建的map，其文件描述符可以从 `map_data[n].fd` 获得，该变量是一个全局变量

#### 使用BPF映射

##### 更新

使用 `bpf_map_update_elem` 函数，内核程序和用户程序需要使用不同版本的函数

* 内核程序
  
  * 头文件 `bpf/bpf_helpers.h`
  
  * `void bpf_map_update_elem(struct bpf_map *map, void *key, void* value, __u64 flags);`
    
    * map  BPF映射指针
    
    * key  要更新的键的指针
    
    * value  要更新的值
    
    * flags  有三种值
      
      * BPF_ANY  若元素存在将更新，不存在将创建
      
      * BPF_NOEXIST  仅在元素不存在时，创建元素
      
      * BPF_EXIST  仅在元素存在时，更新元素

* 用户程序
  
  * 头文件 `tools/lib/bpf/bpf.h`
  
  * `int bpf_map_update_elem(int fd, const void *key, const void *value, __u64 flags)`
    
    除了需要用文件指针来访问外，其他与内核版一致。

##### 读取

`bpf_map_lookup_elem`

##### 删除

`bpf_map_delete_element`

##### 迭代

`bpf_map_get_next_key`

##### 查找删除

`bpf_map_lookup_and_delete_elem`

##### 并发

###### 内核

BPF提供了BPF自旋锁

* `bpf_spin_lock`

* `bpf_spin_unlock`

可以创建一个包含锁的结构体，作为map的元素

```c
struct concurrent_elem
{
    struct bpf_spin_lock semaphore;
    int count;
};
```

此后创建一个映射

```c
struct bpf_map_def SEC("map") concurrent_map = {
    .type        = BPF_MAP_TYPE_HASH,
    .key_size    = sizeof(int),
    .value_size  = sizeof(struct concurrent_element),
    .max_entries = 100,
};

BPF_ANNOTATE_KV_PAIR(concurrent_map, int, struct concurrent_element);    // 书里说这句是必要的，因为要给BTF做验证
```

这样在访问元素的时候就可以进行锁操作

```c
struct concurrent_element *read_value;

read_value = bpf_map_lookup_elem(&concurrent_map, &key);
bpf_spin_lock(&read_value->semaphore);
read_value += 100;
bpf_spin_unlock(&read_value->semaphore);
```

###### 用户

用户态提供了标志 `BPF_F_LOCK` ，作为flags传入即可实现原子操作

#### 映射类型

* 哈希表映射  `BPF_MAP_TYPE_HASH`
  
  * 内核使用 `bpf_map_update_elem` 非原子

* 数组映射  `BPF_MAP_TYPE_ARRAY`
  
  * 键即为索引，必须为4字节，且无法删除元素
  
  * 内核使用 `bpf_map_update_elem` 非原子

* 程序数组映射  `BPF_MAP_TYPE_PROG_ARRAY`
  
  * 用于保存BPF程序的文件描述符，可以与 `bpf_tail_call` 结合实现程序间跳转，最多允许32层嵌套调用（写这本书的时候）
  
  * 键与值都是4字节

* perf事件映射  `BPF_MAP_TYPE_PERF_EVENT_ARRAY`
  
  * 将 perf_events 数据存储在环形缓冲区中，用于BPF程序与用户空间程序通信

* 单CPU哈希映射  `BPF_MAP_TYPE_PERCPU_ARRAY`
  
  * 与数组类似，但该映射与CPU绑定，可以实现高性能查找和聚合

* 栈跟踪映射  `BPF_MAP_TYPE_STACK_TRACE`
  
  * 保存运行进程的栈跟踪信息

* cgroup数组映射  `BPF_MAP_TYPE_CGROUP_ARRAY`
  
  * 类似程序数组，但存储的是指向cgroup的文件描述符

* LRU哈希映射  单CPU LRU哈希映射  `BPF_MAP_TYPE_LRU_HASH` `BPF_MAP_TYPE_LRU_PERCPU_HASH`
  
  * 哈希表基础商实现了LRU缓存
  
  * 单CPU LRU哈希使用单个哈希表来存储所有元素，但对于每个CPU都有单独的LRU缓存

* LPM Tire映射  `BPF_MAP_TYPE_LPM_TRIE`
  
  * 使用最长前缀匹配（LPM）来查找元素，该算法一般用于路由器等的流量转发表上
  
  * 键为8的倍数，8~2048

* 映射数组 映射哈希 `BPF_MAP_TYPE_ARRAY_OF_MAPS` `BPF_MAP_TYPE_HASH_OF_MAPS`
  
  * 用来保存映射的引用，注意不能保存这类映射数组本身的引用，防止循环引用
  
  * 可以用来在运行时替换映射表

* 设备映射的映射  `BPF_MAP_TYPE_DEVMAP`
  
  * 保存对网络设备的引用
  
  * 可以用来建立一个指向特定网络设备端口的虚拟映射，再使用 `bpf_redirect_map` 重定向网络数据包

* CPU映射的映射  `BPF_MAP_TYPE_CPUMAP`
  
  * 同样可以使用 `bpf_redirect_map` 重定向数据包，但是将数据包发到不同CPU上
  
  * 可以使用该映射为网络战分配特定的CPU

* 打开套接字映射  `BPF_MAP_TYPE_XSKMAP`
  
  * 打开套接字引用的映射类型，对于套接字转发数据包很有用

* 套接字数组和哈希映射  `BPF_MAP_TYPE_SOCKMAP` `BPF_MAP_TYPE_SOCKHASH`
  
  * 保存内核中打开套接字引用的专用映射
  
  * 同样可以使用 `bpf_redirect_map` 在XDP程序到其他套接字缓冲区间转发

* 重用端口套接字映射  `BPF_MAP_TYPE_REUSEPORT_SOCKARRAY`
  
  * 主要与 `BPF_PROG_TYPE_SK_REUSEPORT` 一起使用
  
  * 可以用来决定如何过滤和处理网络设备的传入数据包，如当两个套接字连接同一端口时可以决定哪个数据包传入哪个套接字

* 队列映射  `BPF_MAP_TYPE_QUEUE`
  
  * 存储，访问方式与FIFO相同，注意不能使用键来查找

* 栈映射 `BPF_MAP_TYPE_STACK`
  
  * 存储访问方式与栈相同（LIFO），也不能用键来查找

#### BPF虚拟文件系统

默认目录为 `/sys/fs/bpf`

若内核版本不支持bpf，默认不会挂载。可以通过mount挂载

```
mount -t bpf /sys/fs/bpf /sys/fs/bpf
```

可以使用BPF虚拟文件系统来保存两种类型的对象： BPF映射和BPF程序

用于写入和访问虚拟文件系统的helper函数：

* bpf_obj_pin

* bpf_obj_get

### BPF跟踪

#### 探针

探针（probe），有四种不同类型的探针

* 内核探针

* 跟踪点

* 用户空间探针

* 用户静态定义跟踪点

##### 内核探针

内核探针需要注意的一点是：因为没有稳定的应用程序二进制接口（ABI），所以如果同一个内核探针附加到不同内核上可能无法正常工作

###### kprobes

允许载任何内核指令前插入BPF程序，需要知道插入点的函数签名

###### kretprobes

在内核有返回时插入BPF程序，配合kprobes可以实现函数进入前和退出时的hook

##### 跟踪点

跟踪点是静态存在的，由内核开发人员在内核中编写和修改，所以比ABI更稳定。但可能不会涵盖所有的内核子系统

可以在 `/sys/kernel/debug/tracing/events` 目录下查看系统中所有可用的跟踪点，在 `/sys/kernel/debug/tracing/events/bpf` 下查看BPF可用的所有跟踪点。

此外还有两个文件

* enable  允许启用和禁用BPF子系统的所有跟踪点

* filter  用来编写表达式定义内核跟踪子系统的过滤事件

##### 用户空间探针

###### uprobes

可以跟踪到用户程序，需要设置要跟踪程序的路径和要跟踪的符号（可以使用nm查看符号）

###### uretprobes

与kprobes和kretprobes的关系相同

##### 用户静态定义跟踪点

USDT，灵感从DTrace工具而来

###### USDT使用

这里的跟踪点是由用户程序自己定义的，如下例

```c
#include <sys/sdt.h>

int main()
{
    DTRACE_PROBE("hello-usdt", "probe-main");
}
```

将该程序编译为 `hello-usdt` ，该程序就为自身设置了一个跟踪点 `probe-main`

可以使用`readelf -n`查看一个二进制文件的跟踪点信息（段 `.note.stapsdt`）

或者使用bcc的`tplist`工具查看

###### USDT对其他语言的绑定

书里介绍了USDT的其他更高级的用法。USDT这套系统是独立于BPF存在的，因此有很多自己的特性，而eBPF提供了一系列helper函数来于USDT进行交互

#### 跟踪数据可视化

##### perf事件

这里介绍了一个使用perf事件映射将结果发送到用户空间进行进一步处理的例子

### BPF工具

#### BPFTool

```shell
# === bpf信息 ===
# 获取系统的bpf信息，如是否开启JIT
bpftool feature

# === bpf程序 ===
# 查看所有bpf程序，可以使用--json参数打印json
# 若开启了bpf统计信息 sysctl -w kernel.bpf_stats_enabled=1，会额外显示每个bpf程序的CPU时间
bpftool prog show
# 查看某个id的bpf程序
bpftool prog show id xxx
# 查看某个id bpf程序的字节码
bpftool prog dump xlated id xxx
# 此外还可以输出字节码的dot，从而构建bpf程序的CFG
bpftool prog dump xlated id xxx visual

# === 加载bpf程序到文件系统 ===
bpftool prog load xxx.o /sys/fs/bpf/xxx

# === bpf映射 ===
# 查看所有bpf映射
bpftool map show
# 创建映射并持久化到文件系统
bpftool map create /sys/fs/bpf/xxx
# 更新现有映射
bpftool map update id xxx key xxxx value xxxxx
# 查看映射中所有的值
bpftool map dump id xxx
# 将预创建的映射附加到新程序，可以让bpf程序内部即使没有从文件系统读取映射的程序，也可以通过map_data全局变量访问
bpftool prog load xxx.o /sys/fs/bpf/xxx map name xxxx /sys/fs/bpf/xxxx

# === 查看附加到特定接口的程序 ===
# 查看cgroup上的程序
bpftool cgroup tree

# === 批处理 ===
bpftool batch file xxx

# == 显示BTF信息 ===
# BTF是BPF类型格式信息，用于帮助调试程序
bpftool btf dump id xxx
```

### Linux网络和BPF

#### BPF和数据包过滤
