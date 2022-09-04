## 具体技术与实现总结

### 文件隐藏

#### hook

##### jynxkit

jynx中hook了下述系统调用实现文件隐藏

* fstat
* fstat64
* __fxstat
* __fxstat64
* stat
* lstat64
* __lxstat
* __lxstat64
* stat
* stat64
* __xstat
* __xstat64
* open
* rmdir
* unlink
* unlinkat
* fdopendir
* opendir
* readdir
* readdir64

具体的实现方式就是当发现传入的参数中带有待隐藏的文件夹/文件，或GID为待隐藏的用户时，就不返回-1。

##### jynx2

jynx2在jynx的基础上多hook了下列函数

* accept
* access
* fopen
* fopen64
* write

#### Extended Attributes

使用文件系统额外属性实现文件隐藏

### 进程隐藏

#### GID

##### 基础知识

###### ID系列

[理解Effective UID(EUID)和Real UID(RUID) | 骏马金龙](https://www.junmajinlong.com/linux/euid_ruid/)

[Linux permissions: SUID, SGID, and sticky bit | Enable Sysadmin](https://www.redhat.com/sysadmin/suid-sgid-sticky-bit)

分为三种ID

* Effective（EUID EGID）  进程属性
  
  * EUID 有效用户ID
  
  * EGID 有效用户组ID
  
  进程在执行某些涉及权限的操作时，内核将使用进程的【有效用户/组ID】作为凭证来判断是否有权限执行对应操作

* Real （RUID RGID）  进程属性
  
  * RUID 实际用户ID
  
  * RGID 实际用户组ID
  
  RUID和RGID用于确定进程所属的用户和组，主要用于判断是否有权限向进程发送信号。此外，子进程会继承父进程的实际ID

默认情况下，用户执行程序时会将当前用户或指定的用户（如使用sudo打开）设置为该进程的有效用户ID和实际用户ID，此时它们相等

* Set  （SUID SGID）  文件属性
  
  * SUID  特殊用户ID
  
  * SGID  特殊用户组ID

当执行含有s文件属性的文件时，进程的权限总是使用拥有该文件的用户/用户组的权限。如普通用户在调用 passwd 时，因为其owner为root，且有s权限，所以实际上passwd使用root权限执行，从而有权限改写/etc/shadow。

当对一个文件夹设置s权限时，文件夹下的所有文件都拥有s权限。

此外文件属性还有一个特殊的t属性，只能在文件夹指定。只有root或文件所有者才能删除文件，典型应用如`/tmp`文件夹

###### linux ACL

[An introduction to Linux Access Control Lists (ACLs) | Enable Sysadmin](https://www.redhat.com/sysadmin/linux-access-control-lists)

##### 原理

jynx主要通过hook readdir和readdir64来隐藏进程，当发现当前遍历的文件为/proc中为某个特殊GID创建时，则认为是rootkit的文件，并跳过当前的内容返回下一个调用的内容来隐藏进程

https://raw.githubusercontent.com/mempodippy/vlany/master/README_old

说实话这里我没太看明白，大概意思似乎是它靠识别GID来确定一个进程是否是由rootkit程序打开，并且以此作为根据来决定是否隐藏进程。具体等到看实现的时候再确定他是怎么实现的

### 用户隐藏

hook getutent() getutxent() pututline()

### 网络隐藏

### LXC container

提供了一个简单的隐藏容器环境

## kernel mode

[GitHub - ivyl/rootkit: Sample Rootkit for Linux](https://github.com/ivyl/rootkit)  简单

[GitHub - deb0ch/toorkit: A simple useless rootkit for the linux kernel. It is a kernel module which hooks up the open() syscall (or potentially any syscall) to replace it with a custom function.](https://github.com/deb0ch/toorkit)  更简单

[puszek-rootkit/rootkit.c at master · Eterna1/puszek-rootkit · GitHub](https://github.com/Eterna1/puszek-rootkit/blob/master/rootkit.c)  syscall table hook

[GitHub - NoviceLive/research-rootkit: LibZeroEvil &amp; the Research Rootkit project.](https://github.com/NoviceLive/research-rootkit)  rootkit教程

[maK_it: Linux Rootkit | Development &amp; Investigation using Systemtap](https://web.archive.org/web/20190119045332/https://r00tkit.me/)

[exploits/Rootkit_tools at master · CSLDepend/exploits · GitHub](https://github.com/CSLDepend/exploits/tree/master/Rootkit_tools)
