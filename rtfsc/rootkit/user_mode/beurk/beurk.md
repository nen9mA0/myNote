## beurk

[GitHub - unix-thrust/beurk: BEURK Experimental Unix RootKit](https://github.com/unix-thrust/beurk)

应该是基于azazel，项目介绍说的是focus around anti-debugging and anti-detection

### init.c

beurk使用的hook方式与azazel类似，也是将函数指针保存进一个表中，在init程序中初始化

该文件的逻辑很简单，因此不赘述，分成三个函数

* init  初始化函数，若没有进行过初始化则顺序调用下面的函数

* init_hidden_literals  用于被hook的函数，异或解密__hidden_literals表中的对应函数名后使用dlsym获取地址

* init_non_hooked_symbols  用于没有被hook的函数，不需要异或解密，直接使用__hidden_literals对应表项传入dlsym获取地址

可以看到对于有hook的函数，函数名被异或加密，而对没有hook的函数直接dlsym

### cleanup_login_records.c

与azazel中的 clean_wtmp/clean_utmp 基本一致，差别只是azazel中的函数路径写死了，这里经过了一层调用

* uwtmp_clean  用于清理传入路径对应的记录文件

* cleanup_login_records  打开wtmp文件，调用uwtmp_clean清理；再打开utmp文件，调用uwtmp_clean清理

### is_procnet.c

与azazel中的is_procnet函数作用相同，用于判断当前传入的路径是否为 `/proc/net/tcp` 或 `/proc/net/tcp6`

* is_procnet

### hide_tcp_ports.c

与azazel中的hide_tcp_ports函数作用相同，打开传入的 `/proc/net/tcp`或`/proc/net/tcp6` 。若发现端口号在后门端口之间（LOW_BACKDOOR_PORT ~ HIGH_BACKDOOR_PORT）则过滤该数据，否则写入tmpfile，最后返回tmpfile的文件描述符

* hide_tcp_ports

### is_attacker.c

不赘述，与azazel一样是用环境变量来识别的

* is_attacker

### is_hidden_file.c

与azazel的is_invisable函数功能和实现都一致，判断传入的路径中是否存在rootkit要隐藏的路径，或是否是 `/proc` 下的路径来判断是否需要隐藏进程

* is_hidden_file

#### drop_shell_backdoor.c

大部分的反弹shell流程与azazel一致，只有一些细节上的差别

* beurk的转发进程也从父进程中独立出来了，在azazel中只独立了shell执行的进程，没有独立出转发进程

* beurk加入了一些对fork失败的资源释放操作，azazel似乎遗漏了这点

### hook

#### 普通hook

大多数函数hook的流程与azazel一致，为下面的模板

```c
int __lxstat(int ver, const char *path, struct stat *buf) {
    init();    // 初始化
    DEBUG(D_INFO, "called lstat(3) hook");

    if (is_attacker())  // 判断是否为攻击者
        return (REAL___LXSTAT(ver, path, buf));  // 若是直接执行函数

    if (is_hidden_file(path)) {  // 判断是否是需要隐藏的内容
        errno = ENOENT;
        return (-1);    // 若是需要隐藏的内容则返回-1
    }

    return (REAL___LXSTAT(ver, path, buf));  // 返回正常函数执行结果
}
```

采用这类模板的有下列hook函数

* access

* lstat

* lstat64

* stat

* stat64

* __lxstat

* __lxstat64

* __xstat

* __xstat64

* rmdir

* unlink

* unlinkat

#### accept()

* accept

与azazel一致，在返回前加入drop_shell_backdoor函数来反弹shell

#### fopen系列

* fopen

* fopen64

与azazel一致，除了判断shell所有者和是否存在隐藏路径外，还判断了是否打开的是 `/proc/net/tcp/` 或 `/proc/net/tcp6` （[is_procnet](#is_procnet.c)）

#### readdir系列

- readdir

- readdir64

与azazel存在的一点小区别是不跳过 `/` 路径

#### link

* link

与azazel唯一的区别在于，azazel只判断老的文件路径是否为需要隐藏的路径，beurk加上了判断新路径是否需要隐藏

#### open

* open

与azazel的区别在于，它同时处理了两种参数的hook

* `int open(const char *pathname, int flags);`

* `int open(const char *pathname, int flags, mode_t mode);`   azazel只处理了这种

其他处理大同小异，判断shell所有者和是否为隐藏路径
