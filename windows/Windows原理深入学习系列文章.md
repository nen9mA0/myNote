* [Windows原理深入学习系列-强制完整性控制 - 信安成长计划 - 博客园](https://www.cnblogs.com/SecSource/p/15949135.html)

### 强制完整性控制

分为5个等级

| 值      | 描述               | 符号                               | SID          |
| ------ | ---------------- | -------------------------------- | ------------ |
| 0x0000 | Untrusted        | SECURITY_MANDATORY_UNTRUSTED_RID |              |
| 0x1000 | Low Integrity    | SECURITY_MANDATORY_LOW_RID       | S-1-16-4096  |
| 0x2000 | Medium Integrity | SECURITY_MANDATORY_MEDIUM_RID    | S-1-16-8192  |
| 0x3000 | High Integrity   | SECURITY_MANDATORY_HIGH_RID      | S-1-16-12288 |
| 0x4000 | System Integrity | SECURITY_MANDATORY_SYSTEM_RID    | S-1-16-16384 |

* System 最高完整性级别，在本地服务 网络服务和系统账户下运行的进程和服务使用。注意管理员进程的完整性级别为High，低于System，以此对两者进行了隔离

* High  管理员账户下运行进程的默认完整性级别，若启用了UAC则通过UAC提权的进程也会有High完整性等级

* Medium  非管理员账户下运行的进程或启用UAC的管理员账户上的进程。该完整性等级对于注册表只能修改HKRU

* Low  默认不分配，只能通过继承或父进程设置。对于注册表只能操作`HKRU\Software\AppDataLow`，文件只能写入`%USERPROFILE%\AppData\LocalLow`。但可以读取大部分数据

几个要点

* 进程无法修改自己的完整性等级

* 进程一旦开始运行，则无法修改完整性等级

* 进程只能创建小于等于当前完整性等级的进程

* 进程不能修改或写入更高完整性等级的进程和文件

但有几个例外情况

* 若有SE_DEBUG_NAME权限，且完整性等级为High，则可以修改更高完整性等级的进程

* UAC可以将完整性等级从Medium提升到High


