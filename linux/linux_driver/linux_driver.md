# 第一章 设备驱动简介

## 设备和模块的分类

### 字符设备

可以当做一个字节流来存取的设备（如同文件，因此一般映射到/dev下

至少实现**open close read write**调用

### 块设备

通过/dev目录的文件系统节点来存取，一个块设备应该可以驻有一个文件系统

和字符设备的区别仅仅在内核管理数据的方式，因此与内核接口不同，但对于用户两者没有差别

### 网络接口

网络接口不映射到文件，由系统分配名字，因此接口与前两者也完全不同

# 第二章 建立和运行模块

## hello world

### hello_world.c

```c
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int hello_init(void)
{
        printk(KERN_ALERT "HELLO WORLD\n");
        return 0;
}

static void hello_exit(void)
{
        printk(KERN_ALERT "Exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
```

### makefile

```bash
ifneq ($(KERNELRELEASE),)
        obj-m :=hello_world.o
else
        KERNELDIR ?=/lib/modules/$(shell uname -r)/build
        PWD:= $(shell pwd)
modules:
        $(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
        rm -rf *.o *~ .depend .* .
```

若有多个.c,使用`obj-m`指定生成的ko文件名,使用`module-objs`指定所有源码的.o文件
这里makefile调用了/lib/modules的makefile

## 内核模块工作示意图

```
insmod  -> init function->  blk_init_queue() ---
                        ->  add_disk()         |
                                |              |
                                |              |
        block_device ops<-  struct gendisk <-  |
        |                       |           |  |
        |   request()   <-  request_queue <-|-----
        |   _________           ________    |    |
        -->|multiple |  data   |        |   |    |
           |functions|  ->     |________|----    |
                                                 |
                                                 |
rmmod   -> cleanup      ->  del_gendisk()        |
           function     ->  blk_cleanup_queue() --
```

## 内核编程注意事项

* 代码必须可重入
* 内核空间内存占用较小(甚至只有4096B),因此尽量使用动态分配来获取大块内存
* 内核代码不能做浮点运算

## 内核加载和卸载

### insmod

用于加载一个模块,可以使用modprobe代替,但modprobe会检查模块中是否有当前内核没有定义的符号,若有则会在当前模块的路径中搜索其他模块是否有这个符号的定义,若有则将该模块一起加载.如果insmod遇到同样情况将只会报错

### rmmod

用于卸载一个模块

### lsmod

用于列出当前已加载模块,实际上是读取了`/proc/modules`也可以在`/sys/module`的sysfs文件系统中读取到

## 版本依赖

因为在编译内核时,重要的一步是链接当前内核代码树中的`vermagic.o`文件,该文件含有很多信息如编译器版本/内核版本及其他配置变量.当尝试加载一个模块时系统会先检查模块与运行内核的兼容性.
当需要编译指定版本的内核模块,需要下载该内核源码,并修改makefile的`KERNELDIR`变量制定源码树位置
若需要编写适配多版本内核的模块,需要使用`#ifdef`使代码能正确编译,此时应包含`linux/module.h`,使用以下几个变量

* `UTS_RELEASE`  描述内核树版本的字符串
* `LINUX_VERSION_CODE`  内核版本的二进制形式
* `KERNEL_VERSION`  使用这个宏可以将UTS_RELEASE转换为LINUX_VERSION_CODE

## 内核符号表

有些驱动因为需要分层而被拆分成不同的.ko,其中一些模块需要用到其他模块的符号

```c
EXPORT_SYMBOL(name);
EXPORT_SYMBOL_GPL(name);
```

可以使用这两个宏导出符号,注意必须在**全局部分调用**(即不能在函数内调用)

## 预备知识

### 一些宏与头文件

#### 头文件

* `linux/module.h`
* `linux/init.h`  指定init和exit函数

#### 宏

* `MODULE_LICENSE`  若不是几个特定的许可将会被当做私有模块
* `MODULE_AUTHOR`
* `MODULE_DESCRIPTION`
* `MODULE_VERSION`
* `MODULE_ALIAS`  模块别名
* `MODULE_DEVICE_TABLE`  模块支持的设备
  一般放在文件末尾

### init和exit

#### init

```c
static int __init initialization_function(void)
{

}
module_init(initialization_function);
```

`__init`被用来标示为一个init函数

#### exit

```c
static void __exit cleanup_function(void)
{

}
module_exit(cleanup_function);
```

`__exit`被用来标示一个exit函数,注意如果模块为built-in,标示为`__exit`的函数**不会被执行**

#### 初始化中的错误处理

```c
int __init my_init_function(void)
{
        int err;
        err = register_this(ptr1, "skull"); /* registration takes a pointer and a name */
        if (err)
                goto fail_this;
        err = register_that(ptr2, "skull");
        if (err)
                goto fail_that;
        err = register_those(ptr3, "skull");
        if (err)
                goto fail_those;
        return 0;
        /* success */
fail_those:
        unregister_that(ptr2, "skull");
fail_that:
        unregister_this(ptr1, "skull");
fail_this:
        return err;
        /* propagate the error */
}
```

虽然goto不被推荐使用但这里使用goto可以简化流程,因为假设this和that初始化成功而those初始化失败,需要释放this和that两者的资源,而若执行到that就失败只需释放this的资源
对于较复杂的可以这样

```c
struct something *item1;
struct somethingelse *item2;
int stuff_ok;
void my_cleanup(void)
{
        if (item1)
                release_thing(item1);
        if (item2)
                release_thing2(item2);
        if (stuff_ok)
                unregister_stuff();
        return;
}

int __init my_init(void)
{
        int err = -ENOMEM;
        item1 = allocate_thing(arguments);
        item2 = allocate_thing2(arguments2);
        if (!item2 || !item2)
                goto fail;
        err = register_stuff(item1, item2);
        if (!err)
                stuff_ok = 1;
        else
                goto fail;
        return 0; /* success */
fail:
        my_cleanup();
        return err;
}
```

## 模块参数

若模块加载时需要指定参数,可以在insmod或modprobe时指定,也可以从配置文件`/etc/modprobe.conf`指定

### 声明参数

```c
static char *str = "AAAAA";
static int num = 1;
module_param(str, charp, S_IRUGO);
module_param(num, int, S_IRUGO);
```

**参数为: 变量名  类型  权限**

### 参数类型

* bool
* invbool
* charp
* int
* long
* short
* uint
* ulong
* ushort

### 数组参数

```c
module_param_array(name, type, num, perm);
```

num参数表示数组长度,超过num的参数个数是非法的

### 权限

使用`linux/stat.h`中的值,这个值控制谁可以在sysfs下存取该模块的参数

* 0  不会在sysfs显示参数
* S_IRUGO  可以被所有人读取
* S_IRUGO|S_IWUSR  允许root改变参数,最好不要指定该选项

### 调用

```
insmod hello_world.ko num=10 str="BBB"
```

# 字符驱动

## 主次设备编号

当在/dev下输入`ls -l`时,部分输出截取如下

```
crw-rw----+ 1 root    video    81,     0 Dec  2 18:20 video0
crw-rw----+ 1 root    video    81,     1 Dec  2 18:20 video1
crw-rw-rw-  1 root    root      1,     5 Dec  2 18:20 zero
```

c代表字符设备(b为块设备)
中间两个数字为主次设备编号,主编号标识设备相连的驱动,如video0和1都由驱动81管理
次编号用来决定引用哪个设备实例

### 设备编号的内部表示

由`linux/types.h`定义`dev_t`,表示设备编号,为一个32位的量(12位主,20位次)
可以使用`linux/kdev_t.h`的宏获取

```c
MAJOR(dev_t dev);
MINOR(dev_t dev);
```

若有主次编号,需转换为dev_t

```c
MKDEV(int major, int minor);
```

### 分配和释放设备编号

#### 分配

建立字符驱动的第一件事是获取一个设备编号

```c
int register_chrdev_region(dev_t first, unsigned int count, char *name);
int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count,
char *name);
```

* first  要分配的设备编号
* count  请求的连续设备编号总数
* name  连接到这个编号范围的设备的名字,将会出现在/proc/devices和sysfs中
  `alloc_chrdev_region`则用于动态分配编号,常用于不知道需要给设备驱动分配什么编号的情况.firstminor用于指定次编号从哪里开始计算,常为0

#### 释放

```c
void unregister_chrdev_region(dev_t first, unsigned int count);
```

### 主编号的动态分配

一些主编号是静态分配给指定设备的(定义在`Documentation/devices.txt`),因此一般应该使用动态分配
动态分配缺点是无法提前创建设备节点,但其实可以从`/proc/devices`中读取动态分配的编号

#### 获取设备编号

```c
if (scull_major)
{
        dev = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev, scull_nr_devs, "scull");
}
else
{
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
        scull_major = MAJOR(dev);
}
if (result < 0)
{
        printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
        return result;
}
```

## 一些重要的数据结构

### file _operations

```c
struct file_operations
{
    struct module *owner;
                //指向拥有该模块的指针，用于阻止操作一半被卸载
    loff_t (*llseek) (struct file *, loff_t, int);
                //lseek
    ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
                //read
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
                //write
    ssize_t (*aio_read) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
    //异步读——在函数返回前不结束的读操作，若为NULL则默认调用read
    ssize_t (*aio_write) (struct kiocb *, const struct iovec *, unsigned long, loff_t);
    //异步写
    int (*readdir) (struct file *, void *, filldir_t);
    //对于设备应该为NULL，仅用于文件系统读目录
    unsigned int (*poll) (struct file *, struct poll_table_struct *);
    //作为poll/epoll/select调用的后端，查询对一个或多个文件描述符的读写是否会阻塞，为NULL则默认不阻塞
    int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
    //提供调用设备特定命令的方法
    long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
    long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
    int (*mmap) (struct file *, struct vm_area_struct *);
    //请求将设备内存映射到进程的地址空间
    int (*open) (struct inode *, struct file *);
    //open
    int (*flush) (struct file *, fl_owner_t id);
    //在进程关闭文件描述符时调用，用于清空缓冲区
    int (*release) (struct inode *, struct file *);
    //文件结构被释放时调用，注意这里不是close时被调用，而是文件描述符的所有拷贝都被关闭时调用，flush在close时被调用
    int (*fsync) (struct file *, struct dentry *, int datasync);
    //fsync
    int (*aio_fsync) (struct kiocb *, int datasync);
    //异步fsync
    int (*fasync) (int, struct file *, int);
    //通知设备它的FASYNC标志改变
    int (*lock) (struct file *, int, struct file_lock *);
    //文件锁，设备一般不用
    ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
    //内核调用来发送数据到对应文件，一次一页
    unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
    //在进程的地址空间找一个合适位置映射内存
    int (*check_flags)(int);
    //检查传递给fnctl(F_SETFL...)调用的标志
    int (*dir_notify)(struct file *filp, unsigned long arg);
    //使用fcntl来请求目录改变通知时调用
    int (*flock) (struct file *, int, struct file_lock *);
    ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
    ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
    int (*setlease)(struct file *, long, struct file_lock **);
};
```

### struct file

```c
struct file {
    /*
     * fu_list becomes invalid after file_free is called and queued via
     * fu_rcuhead for RCU freeing
     */
    union {
        struct list_head    fu_list;
        struct rcu_head     fu_rcuhead;
    } f_u;
    struct path        f_path;
#define f_dentry    f_path.dentry
#define f_vfsmnt    f_path.mnt
    const struct file_operations    *f_op;    //对应的file_operations
    atomic_t        f_count;
    unsigned int         f_flags; //O_RDONLY O_NONBLOCK O_SYNC
    mode_t            f_mode;    //FMODE_READ FMODE_WRITE
    loff_t            f_pos;    //当前读写位置
    struct fown_struct    f_owner;
    unsigned int        f_uid, f_gid;
    struct file_ra_state    f_ra;

    u64            f_version;
#ifdef CONFIG_SECURITY
    void            *f_security;
#endif
    /* needed for tty driver, and maybe others */
    void            *private_data;    //用于保存驱动开发者自定义的数据

#ifdef CONFIG_EPOLL
    /* Used by fs/eventpoll.c to link all the hooks to this file */
    struct list_head    f_ep_links;
    spinlock_t        f_ep_lock;
#endif /* #ifdef CONFIG_EPOLL */
    struct address_space    *f_mapping;
};
```

* file_operations只会在该结构中被记录，因此可以通过覆盖f_ops的函数指针来覆盖原函数

### inode

```c
struct inode {
    struct hlist_node    i_hash;
    struct list_head    i_list;
    struct list_head    i_sb_list;
    struct list_head    i_dentry;
    unsigned long        i_ino;
    atomic_t        i_count;
    unsigned int        i_nlink;
    uid_t            i_uid;
    gid_t            i_gid;
    dev_t            i_rdev;
    unsigned long        i_version;
    loff_t            i_size;
#ifdef __NEED_I_SIZE_ORDERED
    seqcount_t        i_size_seqcount;
#endif
    struct timespec        i_atime;
    struct timespec        i_mtime;
    struct timespec        i_ctime;
    unsigned int        i_blkbits;
    blkcnt_t        i_blocks;
    unsigned short          i_bytes;
    umode_t            i_mode;
    spinlock_t        i_lock;    /* i_blocks, i_bytes, maybe i_size */
    struct mutex        i_mutex;
    struct rw_semaphore    i_alloc_sem;
    const struct inode_operations    *i_op;
    const struct file_operations    *i_fop;    /* former ->i_op->default_file_ops */
    struct super_block    *i_sb;
    struct file_lock    *i_flock;
    struct address_space    *i_mapping;
    struct address_space    i_data;
#ifdef CONFIG_QUOTA
    struct dquot        *i_dquot[MAXQUOTAS];
#endif
    struct list_head    i_devices;
    union {
        struct pipe_inode_info    *i_pipe;
        struct block_device    *i_bdev;
        struct cdev        *i_cdev;
    };
    int            i_cindex;

    __u32            i_generation;

#ifdef CONFIG_DNOTIFY
    unsigned long        i_dnotify_mask; /* Directory notify events */
    struct dnotify_struct    *i_dnotify; /* for directory notifications */
#endif

#ifdef CONFIG_INOTIFY
    struct list_head    inotify_watches; /* watches on this inode */
    struct mutex        inotify_mutex;    /* protects the watches list */
#endif

    unsigned long        i_state;
    unsigned long        dirtied_when;    /* jiffies of first dirtying */

    unsigned int        i_flags;

    atomic_t        i_writecount;
#ifdef CONFIG_SECURITY
    void            *i_security;
#endif
    void            *i_private; /* fs or device private pointer */
};
```

只有两个成员对于驱动代码有用

* `dev_t i_rdev`  对于代表设备文件的节点，这个成员为设备编号
* `struct cdev *i_cdev`  代表字符设备

## scull设备驱动编写

### scull基本数据结构

#### 声明

```c
struct scull_dev
{
    struct scull_qset *data; /* Pointer to first quantum set */
    int quantum; /* the current quantum size */
    int qset; /* the current array size */
    unsigned long size; /* amount of data stored here */
    unsigned int access_key; /* used by sculluid and scullpriv */
    struct semaphore sem; /* mutual exclusion semaphore */
    struct cdev cdev; /* Char device structure */
};
```

```c
struct scull_qset
{
    void **data;
    struct scull_qset *next;
};
```

#### 结构

scull_dev.data指向scull_qset，每个scull_qset都是一个链表项，scull_qset.data指向一个指针数组，每个指针数组指向一块4KB内存

```
___________
|scull_dev|     __________      __________
|  .data  | -> |scull_qset| -> |scull_qset| -> ...
|         |    |   data   |    |   data   |
|_________|          |               |
                _____|____      _____|____
               |__________|    |__________|
       table   |__________|    |__________|  table
               |__________|    |__________|
```

### scull的设备注册

#### 老的接口

```c
int register_chrdev(unsigned int major, const char *name, struct
file_operations *fops);
int unregister_chrdev(unsigned int major, const char *name);
```

#### 新的接口及实现

##### 接口

```c
cdev_init(struct cdev* p, struct file_operations* f_ops);
cdev_add(struct cdev* p, dev_t dev, unsigned count);
```

##### 实现

```c
static void scull_setup_cdev(struct scull_dev* dev, int index)
{
    int err, devno = MKDEV(scull_major, scull_minor+index);

    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if(err)
            printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}
```

MKDEV用于将传入的major number和minor number转换为dev_t

THIS_MODULE是一个指向模块自身的`struct module*`

### open和release

#### open

函数原型

```c
int (*open)(struct inode *inode, struct file *filp);
```

使用inode获取scull_dev结构的内容：

```c
container_of(inode->i_cdev, struct scull_dev, cdev);
```

代码

```c
int scull_open(struct inode *inode, struct file *filep)
{
    struct scull_dev *dev;
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filep->private_data = dev;
    if( (filep->f_flags & O_ACCMODE) == O_WRONLY )
    {
        scull_trim(dev);
    }
    return 0;
}
```

这里当f_flags为WRONLY时会将scull的数据清零

#### release

##### release调用时机

当对应的文件描述符及其拷贝（dup fork）被全部释放时调用，实际上file有一个引用计数，每次fork/dup只会增加计数，close减少计数，当close后计数值为0时调用release

##### release任务

* 释放分配在filp->private_data的东西
* 在最后的close时关闭设备

##### 实现

scull不需要实现release，因此定义一个空函数

```c
int scull_release(struct inode *inode, struct file *filp)
{
    return 0;
}
```

### scull内存使用

#### 接口

`linux/slab.h`

```c
void *kmalloc(size_t size, int flags);
void kfree(void *ptr);
```

#### 释放内存

```c
int scull_trim(struct scull_dev *dev)
{
    struct scull_qset *next, *dptr;
    int qset = dev->qset;         /* "dev" is not-null */
    int i;
    for (dptr = dev->data; dptr; dptr = next)
    {                         /* all the list items */
        if (dptr->data)
        {
            for (i = 0; i < qset; i++)
                kfree(dptr->data[i]);
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr);
    }
    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;
    return 0;
}
```

### read和write

#### 接口

```c
ssize_t read(struct file *filp, char __user *buff, size_t count, loff_t *offp);
ssize_t write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);
```

**注意**：这里的buff是用户空间的指针，因此不能被内核代码直接解引用

因此需要`copy_to_user`和`copy_from_user`

##### copy_to_user和copy_from_user

行为类似memcpy，但若当前要访问的空间不在内存中（如被交换了），系统在从交换页中获取内存时可能会让当前驱动函数sleep，所以调用这两个函数时应保证该函数可重入，且可以与其他驱动函数并行执行

执行两个函数调用时会先检查用户空间指针是否有效，可以使用`__copy_to_user`和`__copy_from_user`取消这个检测。

#### read

read函数逻辑大体如下：

dev->quantum记录了每个指针指向的内存块大小，dev->qset记录了一个scull_qset指向的指针数组的大小

因此一个scull_qset管理的内存大小为 quantum * qset

* 获取锁
* 判断f_pos和f_pos+count是否大于dev->size
* 获取当前文件指针指向的实际内存地址，方法为
  * 获取qset项下标：f_pos / itemsize
  * 获取指针在指针数组的下标：(f_pos % itemsize) / quantum
  * 获取指针指向的内存块的偏移： (f_pos % itemsize) % quantum
* 定位到对应地址
* 若count > quantum - pos，即当前内存块大小无法容纳count大小的内存，则将赋值范围调整为quantum-pos
* copy_to_user
* f_pos += count
* 释放锁

```c
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr; /* the first listitem */
    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset; /* how many bytes in the listitem */
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    /* find listitem, qset index, and offset in the quantum */
    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;
    /* follow the list up to the right position (defined elsewhere) */
    dptr = scull_follow(dev, item);
    if (dptr == NULL || !dptr->data || ! dptr->data[s_pos])
        goto out; /* don't fill holes */
    /* read only up to the end of this quantum */
    if (count > quantum - q_pos)
        count = quantum - q_pos;
    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count))
    {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;
    out:
    up(&dev->sem);
    return retval;
}
```

#### write

与read大多数类似，多了若发现地址为NULL则调用kmalloc分配的部分，`copy_to_user`变为`copy_from_user`

```c
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    /* find listitem, qset index and offset in the quantum */
    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    /* follow the list up to the right position */
    dptr = scull_follow(dev, item);
    if (dptr == NULL)
        goto out;
    if (!dptr->data)
    {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
        memset(dptr->data, 0, qset * sizeof(char *));
    }
    if (!dptr->data[s_pos])
    {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
            goto out;
    }

    /* write only up to the end of this quantum */
    if (count > quantum - q_pos)
        count = quantum - q_pos;
    if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count))
    {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;
    /* update the size */
    if (dev->size < *f_pos)
        dev->size = *f_pos;
    out:
    up(&dev->sem);
    return retval;
}
```

## 驱动安装脚本

```bash
#!/bin/sh
module="scull"
device="scull"
mode="664"
# invoke insmod with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
/sbin/insmod ./$module.ko $* || exit 1
# remove stale nodes
rm -f /dev/${device}[0-3]
major=$(awk "\\$2==\"$module\" {print \\$1}" /proc/devices)
mknod /dev/${device}0 c $major 0
mknod /dev/${device}1 c $major 1
mknod /dev/${device}2 c $major 2
mknod /dev/${device}3 c $major 3
# give appropriate group/permissions, and change the group.
# Not all distributions have staff, some have "wheel" instead.
group="staff"
grep -q '^staff:' /etc/group || group="wheel"
chgrp $group /dev/${device}[0-3]
chmod $mode /dev/${device}[0-3]
```

主要操作：加载模块，创建文件inode，更改文件用户组和权限

# 第四章 调试技术

## 内核调试支持

* CONFIG_DEBUG_KERNEL  总开关
* CONFIG_DEBUG_SLAB  分配的每一字节被设置成0xa5，释放后设为0x6b
* CONFIG_DEBUG_PAGEALLOC  页在释放时被从内核地址空间去除
* CONFIG_DEBUG_SPINLOCK  捕捉未初始化自旋锁
* CONFIG_DEBUG_SPINLOCK_SLEEP  检查持有自旋锁时sleep
* CONFIG_INIT_DEBUG  有__init的项在系统初始化或模块加载后被丢弃
* CONFIG_DEBUG_INFO  内核调试信息 for gdb
* CONFIG_MAGIC_SYSRQ
* CONFIG_DEBUG_STACKOVERFLOW
* CONFIG_DEBUG_STACK_USAGE  溢出检查
* CONFIG_KALLSYMS  内核符号信息
* CONFIG_IKCONFIG
* CONFIG_IKCONFIG_PROC  使得内核配置状态被内建到内核中
* CONFIG_ACPI_DEBUG
* CONFIG_DEBUG_DRIVER  驱动核心的调试信息
* CONFIG_SCSI_CONSTANTS
* CONFIG_INPUT_EVBUG  启用输入事件的详细日志，会保存键入的任何东西（包括密码
* CONFIG_PROFILING  性能调整

## 用打印调试

### printk

区别在于printk可以指定输出的类型，有如下几种

* KERN_EMERG
* KERN_ALERT
* KERN_CRIT
* KERN_ERR
* KERN_WARNING
* KERN_INFO
* KERN_DEBUG

### 重定向控制台消息

使用`ioctl(TIOCLINUX)`

```c
int main(int argc, char **argv)
{
    char bytes[2] = {11,0}; /* 11 is the TIOCLINUX cmd number */
    if (argc==2)
        bytes[1] = atoi(argv[1]); /* the chosen console */
    else
    {
        fprintf(stderr, "%s: need a single arg\n",argv[0]);
        exit(1);
    }
    if (ioctl(STDIN_FILENO, TIOCLINUX, bytes)<0)
    { /* use stdin */
        fprintf(stderr,"%s: ioctl(stdin, TIOCLINUX): %s\n", argv[0], strerror(errno));
        exit(1);
    }
    exit(0);
}
```

### 消息是如何被记录的

printk将内容打印到一个__LOG_BUF_LEN长度的环形缓冲区，尔后唤醒所有在等待消息的进程，即任何在调用`syslog`系统调用的进程或任何在读取`/proc/kmsg`的进程

当缓冲区满时printk会从头开始写

若klogd在运行，会获取内核消息并分发给syslogd，后者检查`/etc/syslog.conf`找出如何处理它们

### 速率限制

`int printk_ratelimit(void);`

用于防止以过高频率打印一堆重复数据

```c
if (printk_ratelimit())
    printk(KERN_NOTICE "The printer is still on fire\n");
```

若limit值还没到将返回false

### 打印设备编号

`linux/kdev_t.h`

```c
int print_dev_t(char *buffer, dev_t dev);
char *format_dev_t(char *buffer, dev_t dev);
```

### 使用/proc

/proc不会如printk把log同步到文件，可以实现一个接口，当应用程序需要读取时才返回数据

####在/proc里实现读取

当进程读取/proc文件时，内核分配了一页内存，由page指针指定

```c
int (*read_proc)(char *page, char **start, off_t offset, int count, int *eof, void *data);
```

* 返回值
  * 返回的字节数
  * start  用于返回大于一页的数据，下面解释
  * eof  指示不再有数据返回
* page  缓存区，指向写入数据的内存，大小是PAGE_SIZE
* offset
* count

#### scull示例

```c
int scull_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int i, j, len = 0;
    int limit = count - 80;                    /* Don't print more than this */

    for (i = 0; i < scull_nr_devs && len <= limit; i++)
    {
        struct scull_dev *d = &scull_devices[i];
        struct scull_qset *qs = d->data;
        if (down_interruptible(&d->sem))
            return -ERESTARTSYS;

        len += sprintf(buf+len,"\nDevice %i: qset %i, q %i, sz %li\n", i, d->qset, d->quantum, d->size);
        for (; qs && len <= limit; qs = qs->next)
        {                                    /* scan the list */
            len += sprintf(buf + len, " item at %p, qset at %p\n", qs, qs->data);
            if (qs->data && !qs->next)        /* dump only the last item */
                for (j = 0; j < d->qset; j++)
                {
                    if (qs->data[j])
                        len += sprintf(buf + len, " % 4i: %8p\n", j, qs->data[j]);
                }
        }
        up(&scull_devices[i].sem);
    }
    *eof = 1;
    return len;
}
```

#### 老的接口

```c
int (*get_info)(char *page, char **start, off_t offset, int count);
```

#### 创建/proc文件

```c
struct proc_dir_entry *create_proc_read_entry(const char *name,
                                              mode_t mode, 
                                              struct proc_dir_entry *base,
                                              read_proc_t *read_proc,
                                              void *data);
```

* mode  文件保护掩码（缺省填0
* base  文件目录，若为NULL表示/proc
* read_proc 实现read_proc的函数指针
* data  直接被传递给read_proc函数

#### 删除/proc文件

```c
void remove_proc_entry(const char *name, struct proc_dir_entry *parent);
```

#### 注意

* 当运行类似`sleep 100 < /proc/myfile`时，由于这条指令不会增加myfile文件的引用计数，因此在100秒内移除/proc/myfile不会导致错误，这可能使该指令执行失败
* 内核不会检查入口同名问题，因此必须避免同名的现象

### seq_file

为了简化/proc处理大文件的问题（/proc默认只给PAGE_SIZE的缓冲区，要增加得配合start使用

主要思想类似迭代器

#### 头文件

`<linux/seq_file.h>`

#### 实现函数

##### start

在初始化时被调用

```c
void *start(struct seq_file *sfile, loff_t *pos);
```

sfile指向seq_file文件

##### next

```c
void *next(struct seq_file *sfile, void *v, loff_t *pos);
```

* v  前一个对start或者next调用返回的iterator
* pos  文件当前位置

##### stop

销毁前被调用

```c
void stop(struct seq_file *sfile, void *v);
```

##### show

真正返回数据的函数

```c
int show(struct seq_file *sfile, void *v);
```

一些常用的用于seq_file输出的函数

```c
int seq_printf(struct seq_file *sfile, const char *fmt, ...);
int seq_putc(struct seq_file *sfile, char c);
int seq_puts(struct seq_file *sfile, const char *s);
int seq_escape(struct seq_file *m, const char *s, const char *esc);   //除了以八进制格式打印esc中出现的字符外，其他和seq_puts一样
int seq_path(struct seq_file *sfile, struct vfsmount *m, struct dentry *dentry,
char *esc);
```

##### scull的操作

```c
static void *scull_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= scull_nr_devs)
        return NULL; /* No more to read */
    return scull_devices + *pos;
}

static void *scull_seq_next(struct seq_file *sfile, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= scull_nr_devs)
        return NULL;
    return scull_device + *pos;
}

static int scull_seq_show(struct seq_file *s, void *v)
{
    struct scull_dev *dev = (struct scull_dev *) v;
    struct scull_qset *d;
    int i;
    if (down_interruptible (&dev->sem))
        return -ERESTARTSYS;
    seq_printf(s, "\nDevice %i: qset %i, q %i, sz %li\n", 
               (int) (dev - scull_devices), dev->qset, 
               dev->quantum, dev->size);
    for (d = dev->data; d; d = d->next)
    {                                     /* scan the list */
        seq_printf(s, " item at %p, qset at %p\n", d, d->data);
        if (d->data && !d->next) /* dump only the last item */
            for (i = 0; i < dev->qset; i++)
            {
                if (d->data[i])
                    seq_printf(s, " % 4i: %8p\n", i, d->data[i]);
            }
    }
    up(&dev->sem);
    return 0;
}
```

设计的调用对应关系如下

| 应用程序  | 驱动                             |
| ----- | ------------------------------ |
| open  | scull_seq_start                |
| read  | scull_seq_next, scull_seq_show |
| close | scull_seq_stop                 |

###### 注册

```c
static struct seq_operations scull_seq_ops = {
    .start = scull_seq_start,
    .next = scull_seq_next,
    .stop = scull_seq_stop,
    .show = scull_seq_show
};
```

此后，需要自己实现一个seq_open函数，以便使用自定义的seq_operations

```c
static int scull_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &scull_seq_ops);
}
```

最后，注册file_operations

```c
static struct file_operations scull_proc_ops = {
    .owner = THIS_MODULE,
    .open = scull_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};
```

注意这里除了open其他都是默认的函数

###### 创建/proc文件

```c
entry = create_proc_entry("scullseq", 0, NULL);
if (entry)
    entry->proc_fops = &scull_proc_ops;
```

这里的create_proc_entry是比上面create_proc_read_entry更底层的接口

## 使用监视程序来调试

### strace

查看系统调用

* -t  每个调用执行时间
* -T  调用中花费的时间
* -e  限制被跟踪调用的类型

## 调试系统故障

### oops

缓冲区溢出、读写0地址等。可以通过看调用栈和寄存器来定位

### 死循环

若驱动有死循环，系统将彻底陷入死循环

#### schedule

可以用通过在循环中插入`schedule()`来避免系统死机，但要保证schedule调用前锁全部被释放

#### sysrq

`alt+sysrq+某个键`可以以特殊模式处理键盘输入

* r   关闭键盘原始模式，当键盘被不正确地设置时使用
* k  杀死当前终端的所有进程
* s  紧急同步磁盘文件
* u  重新以只读模式加载磁盘，通常用在s后
* b  重启，通常用在s+u后
* p  打印处理器信息
* t  打印当前任务列表
* m  打印内存信息

**需要手动使能sysrq功能**

```
echo 0 > /proc/sys/kernel/sysrq
```

#### profile

readprofile和oprofile工具

## 调试器

### gdb

#### 加载内核镜像

```
gdb /usr/src/linux/vmlinux /proc/kcore
```

vmlinux为内核镜像，kcore为一个代表内核可执行文件的/proc文件

#### 加载模块

```
add-symbol-file .../scull.ko 0xd0832000 \
-s .bss 0xd0837100 \
-s .data 0xd0836be0
```

其中第一个参数指定了.text地址，可以在`/sys/module/"dev_name"/sections`下看到

**技巧：**print *(address)  address为一个16进制地址，如此可以获取该地址的代码文件名和行号

### kdb/kgdb

前者是非官方补丁，后者似乎用于远程调试

### user mode linux

有点类似于内核支持的虚拟机

### Linux Trace Toolkit

[链接](https://www.opersys.com/LTT/)

### Dynamic Probes

探针

[dprobes](http://dprobes.sourceforge.net/)

# 第五章 并发和竞争情况

## 信号量和锁

### 临界区

一段只能被一个线程执行的代码

### 信号量

信号量是一个整型值（假设为x），对于该值有两个操作P和V

#### 原理

##### P

进入临界区时调用，行为类似下面代码

若信号量>0，则信号量自减，否则等待信号被释放

```c
int P(int x)
{
    if( x>0 )
        x--;
    else
    {
        while(x<1)
            schedule();
    }
    return x;
}
```

##### V

出临界区时使用

信号量自增

```c
int V(int x)
{
    x++;
    wakeup_waitproc();
}
```

##### 互斥锁

信号量最常用于互斥锁，即同一时间只有一个线程可以占有资源

把信号量的值初始化为1即可实现互斥锁

#### linux实现

##### 头文件

`<asm/semaphore.h>`

##### 初始化

###### 信号量初始化

```c
void sema_init(struct semaphore *sem, int val);
```

val为信号量初始值

###### 使用宏初始化互斥锁

linux定义了几个宏方便声明互斥锁

```c
DECLARE_MUTEX(name);        //初始化为1
DECLARE_MUTEX_LOCKED(name);    //初始化为0
```

###### 动态分配互斥锁初始化

上面两种都是静态声明后初始化信号量的变量，下面函数用于初始化**动态分配**的信号量

```c
void init_MUTEX(struct semaphore *sem);
void init_MUTEX_LOCKED(struct semaphore *sem);
```

##### P

```c
void down(struct semaphore *sem);
int down_interruptible(struct semaphore *sem);
int down_trylock(struct semaphore *sem);
```

* down  不可被用户中断，即若调用down但总是获取不到锁则进程会卡死

* down_interruptible  最常用，可被中断，但中断意味着没有获取到锁，程序种应当正确处理该错误

* down_trylock  非阻塞，即使不能获取锁也立马返回

##### V

```c
void up(struct semaphore *sem);
```

#### scull示例

##### 声明

scull的semaphore定义在scull_dev结构中

##### 初始化

scull加载时执行初始化

```c
for (i = 0; i < scull_nr_devs; i++) {
    scull_devices[i].quantum = scull_quantum;
    scull_devices[i].qset = scull_qset;
    init_MUTEX(&scull_devices[i].sem);
    scull_setup_cdev(&scull_devices[i], i);
}
```

每个设备都对应一个semaphore

##### write

以这段代码开始，获取信号量

```c
if( down_interrupt(&dev->sem) )
    return -ERESTARTSYS
```

释放信号量

```c
out:
    up(&dev->sem);
    return retval;
```

### reader/writer semaphore

对于一些只会读而不会写的用户，应该允许其读取行为以提高性能

允许**一个writer**或**任意个reader**获取信号量

**writer有优先权**，当writer获取了信号量时，reader不能获取直到其被释放

#### 头文件

<linux/rwsem.h>

#### 初始化

```c
void init_rwsem(struct rw_semaphore *sem);
```

#### reader P

```c
void down_read(struct rw_semaphore *sem);
int down_read_trylock(struct rw_semaphore *sem);
```

#### reader V

```c
void up_read(struct rw_semaphore *sem);
```

#### writer P

```c
void down_write(struct rw_semaphore *sem);
int down_write_trylock(struct rw_semaphore *sem);
void downgrade_write(struct rw_semaphore *sem);
```

* downgrade_write  可以把writer降级为reader，这可以使得等待的reader立即访问到修改后的值

#### writer V

```c
void up_write(struct rw_semaphore *sem);
```

### completions

内核一个经常发生的场景就是一个函数调用另一个线程的函数（可能用于硬件初始化等），并等待函数返回，一个实现可能如下

```c
struct semaphore sem;
init_MUTEX_LOCKED(&sem);
start_external_task(&sem);
down(&sem);
```

start_external_task调用up(&sem)即可通知主线程函数返回

但对于这个常用场景，没必要用信号量（也因为信号量的一些限制

因此有了一种轻量化的通知一个线程另一个线程完成任务的机制：completions

#### 头文件

```c
<linux/completion.h>
```

#### 初始化

##### 静态

```c
DECLARE_COMPLETION(my_completion);
```

##### 动态

```c
struct completion my_completion;
init_completion(&my_completion);
```

#### 等待
