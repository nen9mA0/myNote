## 概述

## 现有rootkit分析

### rooty

### SucKIT

通过读写 `/dev/kmem` 修改kernel相关的内存，从而修改各种内核信息的rootkit，但 `/dev/kmem` 文件在新版的内核中已经没有了（[Killing off /dev/kmem [LWN.net]](https://lwn.net/Articles/851531/)）

### ivyl_rootkit

很简单的一个实现，通过改写文件系统指针 `file*` 的f_ops成员，以及procfs指针 `proc_dir_entry` 的proc_fops成员（这两个成员都指向一个函数指针表，对应了对文件系统的各种操作），来实现文件系统的hook。这里hook的是readdir函数（在新版内核里好像readdir函数不在这两个成员里了）

## 相关技术点分析

### syscall hook

syscall hook的大体步骤很简单

- 获取syscall table地址

- 修改syscall table对应函数的地址为hook函数的地址

但其中有几个技术点，在不同的rootkit中有不同的实现

#### syscall table地址获取

##### 使用IDT表+指纹获取

| 使用的情况             | 使用限制               | 出现的内核版本 | 使用了该技术的rootkit | 兼容性 |
| ----------------- | ------------------ | ------- | -------------- | --- |
| 无法直接获取sys_table符号 | 系统调用是通过int 0x80进行的 | 2.6     | SucKIT         | 较差  |

因为系统调用是使用int 0x80进行的，因此syscall table的地址肯定在int 0x80对应的中断处理函数中被引用，该方式通过逆向分析该中断处理函数获取有关该地址引用的指纹，从而获取syscall table的地址。具体步骤如下

- 使用sidt读取 [idt](#idt/idtr) 表地址

- 读取调用号0x80的idt表项（linux系统调用对应的idt表）

- 获取0x80对应中断处理函数的机器码

- 判断机器码中是否存在 `ff 14 85` （`call dword ptr [eax*4+offset]`）

- 若存在，则将该机器码中对应offset部分的地址返回

这里的指纹就是 `ff 14 85` ，中断处理函数使用 `call dword ptr [eax*4+offset]` 来调用syscall，因为eax是系统调用号，显然offset就是syscall table的地址

对抗与缓解方法见 [使用IDT表+指纹获取的检测](#使用IDT表+指纹获取的检测)

### 文件隐藏

#### fops hook

ivyl rootkit / nurupo rootkit使用了这种技术

通过改写某个文件夹的文件系统指针 `file*` 的f_ops成员，就可以改写其对应的文件操作函数。

ivyl_rootkit通过hook readdir函数，改写该函数的回调函数filldir（该回调用于处理返回的每个文件名），在其中匹配要隐藏的文件名，来实现文件的隐藏  

### 进程隐藏

#### proc fops hook

ivyl_rootkit / nurupo rootkit使用了这种技术

通过改写 `/proc` 文件夹的文件系统指针 `proc_dir_entry` 的 proc_fops 成员，来实现对procfs的hook。

与文件隐藏采用的技术相同，ivyl_rootkit hook了readdir函数，改写其filldir回调

### LKM隐藏

目前看到的技术都是基本相同的。LKM在加载后的信息在procfs和sysfs中被维护

#### 从proc隐藏LKM

隐藏方式，同时在隐藏时一般会保存前一个module的结构，以便于后续恢复

```c
module_list = THIS_MODULE->list.prev;
list_del(&THIS_MODULE->list);
```

恢复模块为可见

```c
list_add(&THIS_MODULE->list, module_list);
```

注意，因为新的模块一定是被添加到kernel module的链表头，所以这里保存的是kernel module的链表头。在恢复可见时模块会被重新加载到kernel module的链表头

#### 从sysfs隐藏LKM

同样一般会保存前一个kobject结构

```c
module_kobj_previous = THIS_MODULE->mkobj.kobj.entry.prev;
kobject_del(&THIS_MODULE->mkobj.kobj);
list_del(&THIS_MODULE->mkobj.kobj.entry);
```

恢复模块为可见

```c
list_add(&THIS_MODULE->list, module_previous);
result = kobject_add(&THIS_MODULE->mkobj.kobj, THIS_MODULE->mkobj.kobj.parent, "rt");
```

### LKM防卸载

在 nurupo rootkit 中使用了这种技术，实际上就是调用

```c
try_module_get(THIS_MODULE);
```

来增加模块的引用计数，防止其直接被卸载掉。若需要卸载则可以调用

```c
module_put(THIS_MODULE);
```

## 检测与对抗

### syscall hook

#### St Michael

##### 检测

这些方法在该文档中被描述 https://kernelhacking.com/rodrigo/docs/StMichael/kernel-land-rootkits.pdf

* 保存每个syscall的地址

* 保存每个syscall函数前31字节的checksum

* 保存上述内容的checksum，防止被篡改

##### 对抗与缓解

* copy一份syscall table，再patch system_call 的代码
  
  * 缓解：这会被保存的system_call函数checksum检测到

* 复制system_call代码，修改它，并让IDT第0x80项指向这个新的system_call代码
  
  * 缓解：这会被保存的system_call函数的地址检测到

* 复制一份IDT表，进行上面的步骤（即复制system_call代码并patch）。最后通过LIDT修改IDT表
  
  * 缓解：可以通过检查IDT表地址或检查module中是否存在LIDT检测到

#### 使用IDT表+指纹获取的检测

##### 检测

被称为Tick-Tock，在加载内核模块前检测模块中是否含有sidt指令，因为大多数内核模块都不会用到这条指令。

##### 对抗

- sidt是一条ring3指令，所以。。。

- 应该可以动态生成该指令再执行，类似SMC
