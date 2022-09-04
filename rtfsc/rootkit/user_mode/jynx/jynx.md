## Overview

见上一级文件夹下的 [overview.md](../overview.md)

## jynx

https://github.com/chokepoint/jynxkit

### bc.c

一个使用pcap触发后门的程序

#### 一些宏设置

##### MAGIC_GID

默认设置为

```c
#define MAGIC_GID 90
```

这里当启动主程序[bc](#bc.c)时，会将当前进程的GID设置为MAGIC_GID

##### MAGIC_SEQ

默认为

```c
#define MAGIC_SEQ 0xbeef
```

对抓取的TCP包进行比较，确定是否是后门的触发包

##### MAGIC_ACK

默认为

```c
#define MAGIC_ACK 0xdead
```

对抓取的TCP包进行比较，确定是否是后门的触发包

##### MAGIC_DIR

默认为

```c
#define MAGIC_DIR "xochi"
```

程序的安装目录，GID需要设置为MAGIC_GID

##### CONFIG_FILE

默认放在 /etc 下，为

```c
#define CONFIG_FILE "ld.so.preload"
```

#### 函数

##### 主程序

主程序主要做的就是设置pcap抓包程序，这里只抓取tcp包

* 首先检查参数
  
  * 若有，则以第一个参数作为要捕获的设备名，使用 pcap_lookupnet 打开
  
  * 若没有，则调用 pcap_lookupdev 来查找一个默认的捕获设备

* 将当前程序的GID设置成 [MAGIC_GID](#MAGIC_GID)

* 下面是对pcap的设置
  
  * 调用 pcap_open_live 打开一个抓包设备
  
  * 调用 pcap_datalink 判断当前打开的设备是否为一个Ethernet设备
  
  * 调用 pcap_compile 编译抓包的filter，这里抓取所有的tcp包
  
  * 调用 pcap_setfilter 设置filter
  
  * 调用 pcap_loop 设置回调函数为 [got_packet](#got_packet) 
  
  * 下面开始抓包

* 下面是释放pcap的设置
  
  * 调用 pcap_freecode 释放filter句柄
  
  * 调用 pcap_close 释放设备句柄

##### got_packet

对网络包进行简单解析，若为TCP报文且seq和ack为MAGIC_SEQ和MAGIC_ACK，则视为触发后门的网络包

```c
void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
```

* 解析IP报头，若不是TCP报文直接返回

* 解析TCP头，获取seq_number和ack_number两个字段与 [MAGIC_SEQ](#MAGIC_SEQ) 和 [MAGIC_ACK](#MAGIC_ACK) 比较，若相同则说明是后门报文，调用 [backconnect](#backconnect) ，否则返回

##### backconnect

建立一个反向连接，这里后门通讯使用了SSL加密

```c
void backconnect(struct in_addr addr, u_short port);
```

* 调用 [InitCTX](#InitCTX) 初始化OpenSSL

* 与先前触发后门链接的IP和端口建立一个TCP socket连接

* 使用 SSL_connect  建立一个SSL链接

* 若链接建立成功过，则循环使用SSL_read读取输入
  
  * 使用popen将输入作为命令执行
  
  * 使用fgets获取命令的返回值并使用SSL_write回写结果

* 关闭socket和SSL上下文

##### InitCTX

初始化一个OpenSSL上下文

```c
SSL_CTX* InitCTX(void);
```

### ld_poison.c

一个污染过的ld.so，原理很简单，先获取各个linux系统调用函数的原地址，再将自己的hook过函数替换上去，执行了hook内容后再调用原来的系统调用函数

#### 初始化

##### Init

作为so程序的构造器 （`static void init (void) __attribute__ ((constructor));`），主要是用于获取各个待hook的系统调用的原函数地址

```c
void init(void);
```

获取下列要hook的函数的地址

* __fxstat

* __fxstat64

* __lxstat

* __lxstat64

* open

* rmdir

* unlink

* unlinkat

* __xstat

* __xstat64

* fdopendir

* opendir

* readdir

* readdir64

#### hook函数实现

这里主要是通过hook实现对文件和进程的隐藏，因此流程大同小异，都是先获取输入的文件属性，若是要隐藏的内容则返回-1，否则返回正常调用该函数的结果

jynx对文件hook的实现基本是采用如下的方法

* 首先在init函数内初始化一个全局函数指针

```c
old_fxstat = dlsym(RTLD_NEXT, "__fxstat");
```

* 此后在函数调用中直接返回该函数指针执行的结果

```c
return old_fxstat(ver,fildes, buf);
```

##### fstat & fstat64 & __fxstat & __fxstat64

* 先调用fxstat/fxstat64系统调用

* 查看返回值的 `st_gid` 成员是否为 MAGIC_GID
  
  * 若是则返回-1
  
  * 否则返回原函数的调用结果

##### lstat & lstat64 & __lxstat & __lxstat64

* 先调用 lxstat/lxstat64 获取文件属性

* 若输入的文件名为 [CONFIG_FILE](#CONFIG_FILE) 指定的名字，或为 [MAGIC_DIR](#MAGIC_DIR) 指定的路径，或 `st_gid` 成员为 MAGIC_GID
  
  * 返回-1
  
  * 否则返回原函数的调用结果

##### stat & stat64 & __xstat & __xstat64

* 先调用 xstat/xstat64 获取文件属性

* 若输入的文件名为 [CONFIG_FILE](#CONFIG_FILE) 指定的名字，或为 [MAGIC_DIR](#MAGIC_DIR) 指定的路径，或 `st_gid` 成员为 MAGIC_GID
  
  * 返回-1
  
  * 否则返回原函数的调用结果

##### open

* 先调用 xstat/xstat64 获取文件属性

* 若输入的文件名为 [CONFIG_FILE](#CONFIG_FILE) 指定的名字，或为 [MAGIC_DIR](#MAGIC_DIR) 指定的路径，或 `st_gid` 成员为 MAGIC_GID
  
  * 则返回-1
  
  * 否则返回原函数的调用结果

##### rmdir

- 先调用 xstat/xstat64 获取文件属性

- 若输入的文件名为 [CONFIG_FILE](#CONFIG_FILE) 指定的名字，或为 [MAGIC_DIR](#MAGIC_DIR) 指定的路径，或 `st_gid` 成员为 MAGIC_GID
  
  - 则返回-1
  
  - 否则返回原函数的调用结果

##### unlink & unlinkat

- 先调用 xstat/fxstat 获取文件属性

- 若输入的文件名为 [CONFIG_FILE](#CONFIG_FILE) 指定的名字，或为 [MAGIC_DIR](#MAGIC_DIR) 指定的路径，或 `st_gid` 成员为 MAGIC_GID
  
  - 则返回-1
  
  - 否则返回原函数的调用结果

##### fdopendir & opendir

- 先调用 fxstat/xstat 获取文件属性

- 若输入的文件名为 [CONFIG_FILE](#CONFIG_FILE) 指定的名字，或为 [MAGIC_DIR](#MAGIC_DIR) 指定的路径，或 `st_gid` 成员为 MAGIC_GID
  
  - 则返回-1
  
  - 否则返回原函数的调用结果

##### readdir & readdir64

* 循环
  
  * 调用 readdir/readdir64 获取目录下的下一个文件
  
  * 若为 `.` 或 `/` 则继续遍历下一个文件（这里有个问题，主要岂不是跟原来的行为不一样）
  
  * 试图拼接当前遍历的文件到 `/proc` 下，使用xstat获取属性
  
  * 若当前文件名为 [CONFIG_FILE](#CONFIG_FILE) 指定的名字，或为 [MAGIC_DIR](#MAGIC_DIR) 指定的路径，或上一步获取的属性 `st_gid` 成员为 MAGIC_GID，则跳过当前文件，继续循环，否则跳出循环

## jynx2

[GitHub - chokepoint/Jynx2](https://github.com/chokepoint/Jynx2)

由jynx改进而来，主要的改进是不像之前一样是有个常驻的进程运行pcap抓包来触发反弹shell，而是hook了accept系统调用来获取反弹shell，较原来更隐蔽

jynx2包含了两个文件，jynx2.c和reality.c，与第一版不同，由于第二版完全将功能迁移到了LD_PRELOAD中hook的标准函数上，所以不需要像第一版有个单独运行的bc.c常驻内存进行抓包。此外，所有带有恶意行为的hook函数均定义在jynx2.c中，而reality.c中调用的都是不带恶意行为的真实的libc函数，该so文件主要是给反弹的shell使用的

### jynx2.c

#### 配置

##### SHELL_PASSWD

```c
#define SHELL_PASSWD "DEFAULT_PASS"
```

##### SHELL_NAME

```c
#define SHELL_NAME "XxJynx"
```

##### ENV_VARIABLE

```c
#define ENV_VARIABLE "XxJynx"
```

#### 函数

##### Init

这里的init也是作为构造器，内容是dlopen一个libc

##### drop_dup_shell

该函数是hook accept后触发的第一个函数

```c
int drop_dup_shell (int sockfd, struct sockaddr *addr);
```

* 判断accept的端口号是否在范围内（[LOW_PORT](#LOW_PORT)  [HIGH_PORT](#HIGH_PORT)）
  
  * 若确实在范围内，先调用fork创建子进程
    
    * 判断是否为子进程，若为子进程
      
      * 调用fsync，写入网络接口
      
      * 调用backconnect反弹shell

##### backconnect

建立反弹shell，与前一个版本相比最大的区别是对SSL通信进行了改进

```c
void backconnect (int sock);
```

* 调用 [InitCTX](#InitCTX-1) 初始化SSL上下文

* 开启SSL监听
  
  * 若接收了一个连接，首先判断输入是否包含密码 [SHELL_PASSWD](#SHELL_PASSWD) ，若密码正确则回显，并调用 [cmd_loop](#cmd_loop)
  
  * 这里是断开连接后进行上下文释放的工作，释放socket和SSL上下文

##### InitCTX

与之前的InitCTX最大的区别就是生成了私钥并生成了一个X509证书，函数gen_cert的作用就是生成这两个内容

```c
SSL_CTX *InitCTX (void);
```

##### cmd_loop

一个反弹shell的主函数，该反弹shell比第一版的改进了不少，比如使用fork来执行一个bash shell

```c
void cmd_loop (int sock);
```

* pipe创建输入和输出的管道

* fork
  
  * 子进程使用 LD_PRELOAD 加载rootkit劫持的so并启动一个 `/bin/bash`
  * 父进程进入一个死循环
    * 使用select来读取sock和与子进程的管道
    * 若sock有输入，则调用writeloop将内容写入子进程的管道，即写入启动`/bin/bash`的子进程
    * 若子进程管道有输入，则读入输入，并将输入按512字节切分为块回写给sock（这里不知道为什么只能按照512字节来传，而且好像有个[bug](#可能的bug)）
  * 若跳出循环，则进行一些清理上下文的工作，如释放SSL和socket

##### drop_suid_shell_if_env_set

```c
void drop_suid_shell_if_env_set (void);
```

* 若euid为root，则将gid和uid设置为root

* 若环境变量中存在 [ENV_VARIABLE](#ENV_VARIABLE)

* 设置环境变量 `HISTFILE=/dev/null` （该环境变量用于设置bash的history文件）

* 使用LD_PRELOAD加载reality.so，并使用 `/bin/bash` 和 `/bin/sh` 执行 [SHELL_NAME](#SHELL_NAME) 的程序，启动一个新的shell 

##### forge_proc_net_tcp

```c
FILE * forge_proc_net_tcp (const char *filename);
```

* 通过tmpfile打开一个tmp文件

* 打开filename对应的文件（即`/proc/net/tcp`或`/proc/net/tcp6`）

* 读取文件内容，并解析出连接的一些信息，如远程端口、本地端口、uid等
  
  * 把除了rootkit相关连接外的连接配置保存到tmp文件中（具体判断条件为远程端口或本地端口在LOW_PORT到HIGH_PORT间，或uid为MAGIC_UID）

* 返回tmp文件内容来伪造真实文件

#### hook函数实现

下面主要列出了与第一版的hook有比较大区别的函数，大体如下

* 新加入的hook
  
  * accept
  
  * access
  
  * fopen / fopen64
  
  * write

* 有改变的hook
  
  * open
  
  * __xstat & __xstat64
  
  * fdopendir & opendir

与第一版不同，jynx2采用了延迟绑定的方法来绑定要调用的标准函数，如下所示

```c
if (!libc)
  libc = dlopen (LIBC_PATH, RTLD_LAZY)

if (old_fxstat == NULL)
  old_fxstat = dlsym (libc, "__fxstat");

...

return old_fxstat (ver, fildes, buf);
```

##### accept

* 执行正常accept

* 调用 [drop_dup_shell](#drop_dup_shell)

##### access

* 调用 [drop_suid_shell_if_env_set](#drop_suid_shell_if_env_set) ，试图以root执行命令

* 使用xstat获取文件属性

* 若输入的文件名为 CONFIG_FILE 指定的名字，或为 MAGIC_STRING 指定的路径，或 `st_gid` 成员为 MAGIC_GID
  
  - 则返回-1
  
  - 否则返回原函数的调用结果

##### fopen & fopen64

* 若打开的文件为 `/proc/net/tcp` 或 `/proc/net/tcp6` ，调用 [forge_proc_net_tcp](#forge_proc_net_tcp) 并**返回**
- 否则，使用xstat获取文件属性

- 若输入的文件名为 CONFIG_FILE 指定的名字，或为 MAGIC_STRING 指定的路径，或 `st_gid` 成员为 MAGIC_GID
  
  - 则返回-1
  
  - 否则返回原函数的调用结果

##### write

* 调用fxstat获取文件属性

* 若 `st_gid` 成员为 MAGIC_GID，则返回-1

* 否则返回原函数的调用结果

##### open / __xstat / __xstat64 / fdopendir / opendir

与原来的差别就是在一开始（在这之前的程序就是hook的延迟绑定部分）调用了 [drop_suid_shell_if_env_set](#drop_suid_shell_if_env_set) ，试图通过suid的属性来执行root命令

### 可能的bug

#### jynx2.c

##### cmd_loop

这里有这样一段代码

```c
if (n_r <= 512) {
    SSL_write (ssl, buf, strlen (buf));
}
else {
    char temp[512];
    char *tmp_str;
    int cnt = 0;
    while (n_r > 512) {
        ++cnt;
        bcopy (buf, &temp, 511);
        SSL_write (ssl, temp, strlen (temp));
        n_r -= 512;
        tmp_str = &buf[512];
        bcopy (tmp_str, &buf, strlen (buf));
    }
    SSL_write (ssl, buf, strlen (buf));
}
```

作用是将buf按照512字节为一块回传，这里先把buf前512字节复制到temp，回传，然后将从buf第512字节开始的512字节复制到buf开头，继续下一个循环。很显然这里传到第1024个字节就会出问题，因为buf只处理了前1024个字节（我猜这里他想写的是`&buf[512*cnt]`）
