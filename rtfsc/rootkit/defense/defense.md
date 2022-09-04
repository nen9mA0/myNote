## HIDS

### Elkeid

字节开源HIDS，其rootkit检测方案主要在这个文件中 [Elkeid/anti_rootkit.c at main · bytedance/Elkeid · GitHub](https://github.com/bytedance/Elkeid/blob/main/driver/LKM/src/anti_rootkit.c)

此外其hook了一系列syscall，见 https://github.com/bytedance/Elkeid/blob/main/driver/LKM/src/smith_hook.c

#### rootkit检测

分为四个方面，但检测原理是一致的，也比较有意思

* 内核模块检测
  
  * 通过获取并遍历系统的符号 `module_kset` ，通过该结构的list成员获取内核模块的kobject入口，再通过kobject提供的模块地址与之前遍历/proc的地址对比来查找模块（这是函数find_hidden_module的实现原理）
    
    > 疑问：这里使用了kobject来查找隐藏模块，若删除了 `&THIS_MODULE->mkobj.kobj` 能不能正常遍历

* hook检测
  
  * 传入一个指针，如某个syscall函数的指针、中断处理函数指针等，通过内核函数 `__module_address` 来判断当前函数是否在内核模块的地址内，若是，则说明被修改过（因为LKM rootkit hook函数时，其实现一般是在内核模块中实现的，这里直接判断了地址空间）
    
    > 但感觉这种方法没法对抗inline hook

##### 检测/proc

这里获取了 /proc 的 iterate/readdir 函数，并进行上述的hook检测，因为一般LKM rootkit会hook这两个函数来隐藏模块

##### syscall table分析

获取了syscall table后，同样对每个函数进行hook检测

##### IDT分析

原理与syscall table一致

#### 可能存在的问题

这里的hook检测使用 `__module_address` 函数实现了比较通用的检测，但它只能分析各个调用表的地址是否被LKM rootkit hook到，无法分析inline hook的情况

## 检测工具

### ebpf-monitor

该工具主要的检测原理就是对bpf系统调用进行记录和设置白名单

#### 基本原理

基本原理其实就是通过对bpf系统调用行为的分析从而记录ebpf恶意软件的行为。但这里核心功能应该集中在记录方面，对于防范方面，这里采取的是白名单的方式，若开启了保护，只有在白名单内的程序才能执行bpf系统调用

#### 记录点

##### sched

这里通过对scheduler的hook对允许列表中的程序进行跟踪

```
tracepoint/sched/sched_process_exec
```

* 通过context参数获取当前调用的文件名，若不在allowed_binaries映射中，则直接返回

* 否则在allowed_cookies映射中创建/更新一个新的随机cookie

* 获取当前tgid，并在tgid_cookie映射中创建保存tgid及其对应的cookie

```
tracepoint/sched/sched_process_fork
```

* 检查当前的tgid是否在tgid_cookie中，若没有则说明不是被跟踪的程序，直接返回

* 否则说明当前进行fork的程序是被跟踪的程序，则将其fork出的子程序也加入tgid_cookie映射中，且使用与父进程相同的cookie

```
tracepoint/sched/sched_process_exit
```

* 判断当前的tgid和pid（这里包括前面都是通过bpf_get_current_pid_tgid函数获取的，其中pid在低32位，tgid在高32位）

* 若tgid与pid相等（说明当前是主线程）
  
  * 在tgid_cookie映射中查找tgid的对应项，若找到则将其删除，解除对该进程的跟踪

##### bpf系统调用相关

###### 触发顺序

下列几个hook点的调用关系如下（从 [源码](https://elixir.bootlin.com/linux/v4.20.17/source/kernel/bpf/syscall.c#L2491) 中可以看到）

* SYSCALL_KPROBE3(bpf, int, cmd, union bpf_attr *, uattr, unsigned int, size)

* kprobe/security_bpf

* 若当前调用为 BPF_MAP_CREATE 或 BPF_MAP_GET_FD_BY_ID
  
  * kprobe/security_bpf_map

* 若当前调用为 BPF_PROG_GET_FD_BY_ID
  
  * kprobe/security_bpf_prog

* 若当前调用为 BPF_PROG_LOAD
  
  * kprobe/check_helper_call
  
  * kprobe/security_bpf_prog

* SYSCALL_KRETPROBE(bpf)

###### hook点

```
SYSCALL_KPROBE3(bpf, int, cmd, union bpf_attr *, uattr, unsigned int, size)
```

* 获取tgid，若不在tgid_cookie映射中，判断当前程序是否开启了`protect_bpf`保护选项，若开启了则对于非允许列表中对bpf的调用都会返回错误

```
kprobe/security_bpf
```

* 调用 [reset_ctx](#reset_ctx) 初始化一个上下文结构

* 保存bpf调用中的attr和cmd成员（对应该函数的第一和第二个参数）

```
kprobe/security_bpf_map
```

* 获取当前map的id name和map_type属性

* 将这些属性保存到 bpf_maps 映射中

* 调用 [get_ctx](#get_ctx) ，并将id属性保存在bpf_context_t.map_id

```
kprobe/security_bpf_prog
```

* 获取当前程序的id prog_type attach_type和name属性

* 调用 [get_ctx](#get_ctx) ，并将id属性保存在bpf_context_t.prog_id中

* 获取程序使用的helper函数

* 将这些属性保存到 bpf_progs 映射中

```
kprobe/check_helper_call
```

* 调用 [get_ctx](#get_ctx) ，获取上下文

* 根据传入的helper函数类型更新上下文中对于helper的描述

```
SYSCALL_KRETPROBE(bpf)
```

- 调用 [get_ctx](#get_ctx) 一个上下文

- 获取bpf调用中的map_id和prog_id，若不是0，则调用 [save_obj_fd](#save_obj_fd) 记录

- 调用 [populate_map_id_and_prog_id](#populate_map_id_and_prog_id)

- 调用 [send_event](#send_event) 发送事件到monitor

#### 其他函数

##### get_ctx

* 以pid_tgid为key从bpf_context映射中获取对应的bpf_context_t结构体

##### reset_ctx

* 以0为key从bpf_context_gen映射中获取一个bpf_context_t结构体

* 以pid_tgid为key将该结构体赋值给bpf_context映射并返回

##### save_obj_fd

* 若当前bpf调用用于创建映射，则将其保存到tgid_fd_map_id映射

* 若当前bpf调用用于加载程序，则将其保存到tgid_fd_prog_id映射

##### populate_map_id_and_prog_id

* 主要逻辑是根据当前bpf调用的操作（cmd成员）来从tgid_fd_map_id映射或tgid_fd_prog_id映射更新prog_id或map_id成员
