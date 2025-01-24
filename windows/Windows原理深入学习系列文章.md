* [Windows原理深入学习系列-强制完整性控制 - 信安成长计划 - 博客园](https://www.cnblogs.com/SecSource/p/15949135.html)

## 文章内容

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

## Windbg调试

### 通用

#### 获取某个进程信息及地址

```
!process [pid] [flag]
```

其中pid需要十六进制指定，flags有5个bit（见help文档），一般用7就好

#### EPROCESS结构

通过!process获取进程结构地址（在返回内容的第一行PROCESS后显示）

```
dt _EPROCESS [addr]
```

### 完整性等级

#### Token结构

TOKEN结构在偏移0x4b8处，类型为_EX_FAST_REF，该结构为内核态的智能指针，注意其**低4位（x64）或低3位（x86）为RefCnt字段**，因此在获取对象指针的时候应对其低4位清零

如获取的指针为`0xffffac07cd530068`，则实际的对象地址为`0xffffac07cd530060`

以上述地址为例，获取TOKEN结构

```
dt _TOKEN 0xffffac07cd530060
```

偏移0x0d0处有一个字段名为IntegrityLevelIndex，但其不是完整性等级本身，而是一个索引，查找的逻辑在SeQueryTokenIntegrity中

```
lkd> uf nt!SeQueryTokenIntegrity
nt!SeQueryTokenIntegrity:
fffff802`78b5d964 4883ec28        sub     rsp,28h
fffff802`78b5d968 e8a34ff0ff      call    nt!SepCopyTokenIntegrity (fffff802`78a62910)
fffff802`78b5d96d 4883c428        add     rsp,28h
fffff802`78b5d971 c3              ret

lkd> uf nt!SepCopyTokenIntegrity
nt!SepCopyTokenIntegrity:
fffff802`78a62910 8b81d0000000    mov     eax,dword ptr [rcx+0D0h]
fffff802`78a62916 83f8ff          cmp     eax,0FFFFFFFFh
fffff802`78a62919 741b            je      nt!SepCopyTokenIntegrity+0x26 (fffff802`78a62936)  Branch

nt!SepCopyTokenIntegrity+0xb:
fffff802`78a6291b 48c1e004        shl     rax,4
fffff802`78a6291f 48038198000000  add     rax,qword ptr [rcx+98h]
fffff802`78a62926 740e            je      nt!SepCopyTokenIntegrity+0x26 (fffff802`78a62936)  Branch

nt!SepCopyTokenIntegrity+0x18:
fffff802`78a62928 488b08          mov     rcx,qword ptr [rax]
fffff802`78a6292b 48890a          mov     qword ptr [rdx],rcx
fffff802`78a6292e 8b4808          mov     ecx,dword ptr [rax+8]
fffff802`78a62931 894a08          mov     dword ptr [rdx+8],ecx
fffff802`78a62934 c3              ret

nt!SepCopyTokenIntegrity+0x26:
fffff802`78a62936 488b05334d9d00  mov     rax,qword ptr [nt!SeUntrustedMandatorySid (fffff802`79437670)]
fffff802`78a6293d 488902          mov     qword ptr [rdx],rax
fffff802`78a62940 c7420860000000  mov     dword ptr [rdx+8],60h
fffff802`78a62947 c3              ret


```


