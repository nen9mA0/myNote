## rooty

### rooty.c

该文件主要是LKM的初始化和析构

#### 模块初始化

##### rooty_init

```c
int rooty_init(void);
```

* 使用两种隐藏LKM的方式分别将LKM从proc和sysfs中隐藏（见 [research_rootkit](../research_rootkit/research_rootkit.md)）
* 调用 [find_sys_call_table](#find_sys_call_table) 找到syscall table的地址
* 调用 [root_me](#root_me) 提权
* 调用 [get_vfs_iterate](#get_vfs_iterate) 和 [hijack_start](#hijack_start) 对 `/proc` 和 `/` 进行隐藏
* 调用 [get_tcp_seq_show](#get_tcp_seq_show) 和 [hijack_start](#hijack_start) 对 `/proc/net/tcp` 和 `/proc/net/tcp6` 进行隐藏
* 调用 [get_udp_seq_show](#get_udp_seq_show) 和 [hijack_start](#hijack_start) 对 `/proc/net/udp` 和 `/proc/net/udp6` 进行隐藏
* 调用 [get_inet_ioctl](#get_inet_ioctl) 和 [hijack_start](#hijack_start) hook inet_ioctl函数来对rootkit进行控制
* 调用 [init_keylogger](#init_keylogger) 初始化键盘记录
* 调用 [init_ssh](#init_ssh)
* 调用 [hide_file](#hide_file) 隐藏 `sshd vncd rooty.ko rooty` 这几个rootkit相关文件
* 调用 [hide_tcp4_port](#hide_tcp4_port) [hide_udp4_port](#hide_udp4_port) [hide_tcp6_port](#hide_tcp6_port) [hide_udp6_port](#hide_udp6_port) 来隐藏sshd端口（22）和vncd端口（5900）

#### 模块析构

##### rooty_exit

```c
void rooty_exit(void);
```

* 调用 [stop_ssh](#stop_ssh) 停止ssh服务

* 调用 [stop_keylogger](#stop_keylogger) 停止键盘记录

* 调用 [hijack_stop](#hijack_stop) 释放前面的程序hook

### syscall_table.h

#### 函数

##### find_sys_call_table

```c
unsigned long *find_sys_call_table ( void );
```

* 使用sidt指令获取idt表地址

* 
