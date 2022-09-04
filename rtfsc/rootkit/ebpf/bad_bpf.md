该仓库提供了一系列使用ebpf进行恶意操作的demo

[GitHub - pathtofile/bad-bpf: A collection of eBPF programs demonstrating bad behavior, presented at DEF CON 29](https://github.com/pathtofile/bad-bpf)

这些demo使用了下列ebpf程序类型

* tracepoint  hook点以tp开头

* trampoline  hook点以fentry/fexit/fmod_ret等开头   https://lwn.net/Articles/804112/  注意，这里说trampoline类型的程序好像可以attach到xdp等网络ebpf程序

## dos

这个demo实现对所有调用ptrace系统调用的进程的拦截，-t参数指定了调用ptrace的程序的parent pid，若为指定的pid则kill掉，若没有指定则会kill掉所有调用了ptrace的进程

### kernel端

hook点为

```
tp/syscalls/sys_enter_ptrace
```

* 调用 bpf_get_current_pid_tgid 获取当前进程的pid和tgid

* 调用 bpf_get_current_tack 获取当前进程的task_struct

* 调用 BPF_CORE_READ 读取task_struct的real_parent成员（`BPF_CORE_READ(task, real_parent, tgid)`）

* 若不等于指定的parent pid，则退出

* 否则调用bpf_send_signal发送SIG_KILL

* 发送log到user端，包括下面几步
  
  * 调用 bpf_ringbuf_reserve 获取ringbuffer
  
  * 设置event并调用 bpf_get_current_comm 和 bpf_ringbuf_submit 向ringbuffer发送数据

### user端

user端比较简单，就是调用bpftool生成的skeleton文件中的函数加载kernel端，并根据传参设置一个全局变量，此后通过ringbuffer与kernel端通信

需要注意的是生成的skeleton文件的几个函数，注意，这里的user端程序名为 `bpfdos.c` ，kernel端名为 `bpfdos.bpf.c`

* bpfdos_bpf__open  打开kernel端程序

* bfpdos_bpf__load  验证并加载kernel端程序

* bpfdos_bpf__attach  attach到kernel端

* bpfdos_bpf__destroy  卸载kernel端程序

## exechijack

该demo实现了hook execve并改写调用的执行文件为指定文件

### kernel端

hook点为

```
tp/syscalls/sys_enter_execve
```

* 与上面一样，先判断父进程以确定是否当前进程是否需要做处理

* 调用 bpf_probe_read_user 读取调用execve的参数

* 改写参数

* 调用 bpf_probe_write_user写回

## pidhide

该demo实现了hook getdent64从而隐藏进程

### kernel端

* hook点1 handle_getdents_enter，这个是getdents64函数入口的hook
  
  ```
  tp/syscalls/sys_enter_getdents64
  ```
  
  * 一样是判断父进程
  
  * 将第二个参数dirp记录到映射中，key为pid_tgid

* hook点2 handle_getdents_exit，getdents64出口的hook
  
  ```
  tp/syscalls/sys_exit_getdents64
  ```
  
  * 获取pid_tgid，查找对应的调用是否在入口时记录在map_buffs映射中，若存在则说明需要处理
  
  * 循环200次（这里我不太知道为什么是200，因为ebpf目前不支持无限循环所以没法用while么），每次读取一个返回的 `dirent64` 结构，并比较文件名是否为要隐藏的pid，若是，则尾调用handle_getdents_patch，并将当前dirent64结构指针记录在map_to_patch映射中，便于后续处理
  
  * 若循环200次后还未遍历文件夹，则尾调用handle_getdents_exit继续下一次处理

* hook点3  handle_getdents_patch
  
  ```
  tp/syscalls/sys_exit_getdents64
  ```
  
  * 查找对应pid是否在map_to_patch映射中，若是则进行后续处理
  
  * 这里隐藏pid的方法是，首先map_to_patch中记录的指针指向的是要隐藏文件的前一个dirent64结构，此时获取下一个dirent64结构（就是要隐藏的文件结构），获取其d_reclen成员（用于指明当前dirent64结构的长度），最后将前一个dirent64结构的d_reclen改写为两个结构相加的值

## sudoadd

该demo实现了对`/etc/sudoers`文件的hook，使得sudoer返回结果加入

```
<username> ALL=(ALL:ALL) NOPASSWD:ALL
```

其中username是新加入的sudoer

### kernel端

* hook点1  handle_openat_enter，openat入口的hook
  
  这里判断调用是否使用sudo且打开了/etc/sudoers
  
  ```
  tp/syscalls/sys_enter_openat
  ```
  
  * 同样先判断父进程
  
  * bpf_get_current_comm 获取当前进程调用的命令行，检查是否是在使用sudo，若不是则退出
  
  * 获取调用openat的第二个参数，若不是 `/etc/sudoers` 则退出
  
  * bpf_get_current_uid_gid  若uid不算root也不是指定的uid，则退出
  
  * 否则将当前调用加入map_fds映射，指定其在函数返回时处理

* hook点2  handle_openat_exit，openat出口的hook
  
  ```
  tp/syscalls/sys_exit_openat
  ```
  
  * 判断当前调用之前有没有被加入map_fds映射（使用pid_tgid作为key），若没有则退出
  
  * 否则将其返回的fd加入map_fds映射，等待下一步read时处理

* hook点3  handle_read_enter
  
  ```
  tp/syscalls/sys_enter_read
  ```
  
  * 判断之前有没有被加入过map_fds映射，若没有则退出
  
  * 将read调用的第一个参数加入 map_buff_addrs 映射，等待read返回时处理

* hook点4  handle_read_exit
  
  ```
  tp/syscalls/sys_exit_read
  ```
  
  * 判断之前有没有被加入过 map_buff_addrs 映射
  * 将自定义的sudoer参数写入缓冲区的开头部分（这里一般只是sudoers文件的注释），并将后面的内容覆盖为新加入的规则（这里写完规则后最后要记住加上一个注释符，因为这里的内容是覆盖上去的）

* hook点5  handle_close_exit
  
  ```
  tp/syscalls/sys_exit_close
  ```
  
  * 一样的判断父进程
  
  * 进行一些清理工作，将上述几个操作间通信需要用到的映射对应项清空

## textreplace

实现对特定文件内容的替换（但好像不能附加），实现方式类似于sudoadder

### kernel端

* hook点1  handle_openat_enter
  
  ```
  tp/syscalls/sys_enter_openat
  ```
  
  * 判断父进程
  
  * 比较第二个参数是否为目标文件，若是则将缓冲区地址存入 map_fds

* hook点2  handle_openat_exit
  
  ```
  tp/syscalls/sys_exit_openat
  ```
  
  * 若在map_fds里，则将文件描述符存入

* hook点3  handle_read_enter
  
  ```
  tp/syscalls/sys_enter_read
  ```
  
  * 判断是否在map_fds中，若是，则将read调用的缓冲区存入map_buff_addrs

* hook点4  find_possible_addrs
  
  ```
  tp/syscalls/sys_exit_read
  ```
  
  * 判断是否在map_buff_addrs中
  
  * 这里按块循环读取文件（读取64次，每次64字节），若扫描到第一个相同的字符，则存入map_name_addrs等待后续处理
  
  * 这里tail call了check_possible_addresses

* hook点5  check_possible_addresses
  
  ```
  tp/syscalls/sys_exit_read
  ```
  
  * 若在map_buff_addrs中，则遍历map_name_addrs可能的所有key（这里为行号），若与要修改的内容匹配，则将地址存入map_to_replace_addrs

* hook点6  overwrite_addresses
  
  ```
  tp/syscalls/sys_exit_read
  ```
  
  * 若在map_to_replace_addrs中，则将其替换为要替换的字符
  
  * 将对应条目从map_to_replace_addrs中删除

* hook点7  handle_close_exit
  
  ```
  tp/syscalls/sys_exit_close
  ```
  
  * 进行一些清理工作，因为在这里已经完成了替换并且要关闭文件了，因此将相关的内容从映射中移除

## textreplace2

这里的实现与textreplace基本一致，但采用了不同的hook点，分别为

```
fentry/__x64_sys_openat
fexit/__x64_sys_openat
fentry/__x64_sys_read
fexit/__x64_sys_read
fexit/__x64_sys_close
```

## writeblocker

该demo实现的功能是hook了write函数，使得目标进程无法写入文件，但以为自己成功写入

hook点是

```
fmod_ret/__x64_sys_write
```

这种hook会完全接管函数的行为，而不是执行后会返回到原函数继续执行，因此这里直接对调用者进行判断，若为目标pid则直接返回写入的数据长度而不进行实际的写入工作
