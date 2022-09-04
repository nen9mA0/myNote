#### 初始化

##### rootkit_init

* 调用 [procfs_init](#procfs_init) 创建一个proc文件

* 调用 [fs_init](#fs_init) 获取 `/etc` 目录下的文件指针，并修改其`file*->f_op`
  
  * 若创建失败则调用 [procfs_clean](#procfs_clean) 和 [fs_clean](#fs_clean) 清理，并返回

* 调用 [module_hide](#module_hide) 隐藏模块

#### 析构

##### rootkit_exit

* 调用 [procfs_clean](#procfs_clean) 和 [fs_clean](#fs_clean) 清理

#### proc后门

##### procfs_init

* 在proc下创建rtkit文件

* 注册其读写函数分别为 [rtkit_read](#rtkit_read) 和 [rtkit_write](#rtkit_write)

* 获取当前文件的父文件夹（即`/proc`），并获取其 `proc_fops`

* 修改其readdir操作为rootkit定义的hook函数

##### rtkit_read

proc文件的读函数，返回当前rootkit的一些统计信息，包括当前隐藏的pid、是否开启隐藏文件功能和是否开启隐藏模块功能

##### rtkit_write

proc文件的写函数，主要实现下列功能

* mypenislong  作为密码，当读取到该输入时切换当前用户权限为root

* hp  要隐藏的pid，会将其放入 pids_to_hide 数组

* up  解出除上一个pid的隐藏

* thf  开启/关闭文件隐藏功能

* mh 调用 [module_hide](#module_hide) 隐藏模块

* ms 调用 [module_show](#module_show) 显示模块

#### 模块隐藏/显示

##### module_hide

* 记录当前模块的前一个模块（`THIS_MODULE->list.prev`）

* 记录当前模块的前一个kobject （`THIS_MODULE->mkobj.kobj.entry.prev`）

* list_del 删除 `THIS_MODULE->list`

* list_del 删除 `THIS_MODULE->mkobj.kobj.entry`

* kobject_del 删除 `THIS_MODULE->mkobj.kobj`

##### module_show

* list_add 将 `THIS_MODULE->list` 加入链表（先前记录了前一个模块的链表块地址 `THIS_MODULE->list.prev`）

* kobject_add 将 `THIS_MODULE->mkobj.kobj` 加入链表（`THIS_MODULE->mkobj.kobj.parent`）

**问题**：这里kobject被hide删除过了之后又被show这里访问是不是uaf了

#### hook

这里hook的方式是通过访问并替换文件指针的fops成员来进行的。注意这里在替换时需要为页面加上可写属性，这里用到的两个函数为 set_addr_rw 和 set_addr_ro

hook的方式类似下面

```c
fs_readdir_orig = fs_fops->readdir;
set_addr_rw(fs_fops);
fs_fops->readdir = fs_readdir_new;
set_addr_ro(fs_fops);
```

##### set_addr_rw

```c
static void set_addr_rw(void *addr);
```

* 使用lookup_address找到addr对应的pte（page table entry）

* 若页面没有rw属性则加上rw属性

##### set_addr_ro

```c
static void set_addr_ro(void *addr);
```

- 使用lookup_address找到addr对应的pte（page table entry）

- 若页面有rw属性则去掉rw属性

##### fs_init

* 打开 `/etc` 文件夹

* 获取file*的f_op成员

* hook readdir函数

##### proc_init

proc_init的最后一步也是在hook readdir函数

##### procfs_clean

* 删除`/proc/rkkit`文件

* 解除对 `/proc` 的readdir函数的hook

##### fs_clean

* 解除对 `/etc` 的readdir函数的hook

#### hook函数

对readdir的hook主要是hook了readdir的第三个参数filldir函数，该回调函数会对readdir时返回的每个文件名都执行一次，来处理返回的文件名

这里hook该函数来实现文件和进程隐藏，主要就是改写了filldir函数，在函数体中对比了当前的文件名是否为要隐藏的文件，具体为

* 普通文件系统
  
  * `__rt` 或 `10-__rt` 开头的文件

* proc文件系统
  
  * rootkit本身创建的 rtkit 文件
  
  * 保存在pid_to_hide数组中的数字，用来隐藏进程
