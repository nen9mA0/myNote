## 生成程序消息

使用Parse和SboxMsg，其中Parse负责解析消息的txt文件，生成解析后的txt文件，并且调用mc（Message Compiler）编译这些消息，生成.rc和.h文件

SboxMsg则会将上述.rc和.h文件编译为dll

## 图形化程序

### start

主界面

#### 启动

* 调用SbieApi_QueryConf查找是否存在DefaultBox

* 获取当前程序token是否为特权token

* 检查一些启动设置，如是否为窗口模式、按键状态判断是否为强制执行模式等

* 调用SetCurrentProcessExplicitAppUserModelID设置任务栏User Model ID（为了将不同路径执行的沙箱程序合并到任务栏中）

* 处理命令行

* 

## Sboxdll

### 信息获取

#### 函数

##### SbieDll_GetTokenElevationType

获取令牌是否为提权的令牌

### 代码注入模块

[Code Injection - Sandboxie Documentation](https://sandboxie-plus.github.io/sandboxie-docs/Content/CodeInjection/)

#### 远程注入

由函数`SbieDll_InjectLow`实现

## SboxDrv

### 入口

* 检查系统版本

* 创建内存池

* 初始化两个安全描述符Driver_PublicSd和Driver_LowLabelSd

* 保存驱动的注册表路径

* 初始化dyn_data  Dyndata_Init

* 通过注册表路径获取sandboxie安装路径

* 检查证书

* 初始化obj模块

* 初始化conf模块

* 初始化dll模块  Dll_Init

* 初始化syscall模块

* 初始化session模块

* 检查系统中是否导出了ZwCreateToken或ZwCreateTokenEx函数（调用Hook_GetService）

* 初始化token模块

* 初始化process模块

* 初始化thread模块

* 初始化file模块

* 初始化key模块

* 初始化ipc模块

* 初始化gui模块

* 初始化api

### dll模块

源文件：dll.c dll.h

该模块将dll文件加载到内核的内存中，并通过解析PE文件头获取对应的导出函数信息

* DLL加载/卸载
  
  * Dll_Load  从system32中读取dll文件到内存中，并解析PE头，最后插入dll链表中
  
  * Dll_Unload  将dll链表中所有加载的dll文件释放

* DLL获取函数地址
  
  * 接口 Dll_GetProc  调用Dll_GetNextProc函数从加载的dll中的export表获取对应函数地址

* 内部函数
  
  * Dll_GetNextProc
  
  * Dll_RvaToAddr

* 初始化
  
  * 接口 Dll_Init  加载NTDLL，若支持xp则额外加载USER32

### hook模块

源文件：hook.c hook.h hook_32.c hook_64.c hook_inst.c

* hook.c 接口
  
  * Hook_GetService  查找某个ntdll函数对应的Zw/Nt内核系统调用，依次调用Hook_GetServiceIndex、Hook_GetNtServiceInternal和Hook_GetZwServiceInternal

* hook.c 分析系统调用号
  
  * Hook_GetServiceIndex  调用Hook_FindSyscall2分析传入的函数对应的系统调用号
  
  * Hook_FindSyscall2  接收一个传入的dll导出函数，对其指令进行解析，通过一些编码分析其调用syscall时eax的值

* hook_32.h hook_64.h通过一些方法搜索SSDT（这边好像只有x86/x64）
  
  * Hook_GetNtServiceInternal  通过syscall模块的Syscall_GetServiceTable获取ShadowTable地址
  
  * Hook_GetZwServiceInternal  通过查找ntoskernl.exe中的ZwWaitForSingleObject和ZwUnloadKey的方法搜索SSDT地址，并根据找到指令的硬编码获取服务地址

* hook_inst.c  一个简化的解析hook指令的反汇编器，但重点解析跳转指令，其他指令基本只要能判断长度即可
  
  * Hook_Analyze  解析传入地址的对应指令，返回HOOK_INST结构

### dyn_data模块

源文件：dyn_data.h dyn_data.c

该模块用于存储一些与系统相关的，hook中需要使用的偏移量

* 接口 Dyndata_Init  初始化当前系统对应的偏移量配置。首先会在注册表中查找相关配置，若没找到则在代码中预置的配置表中查找

### verify模块

源文件：verify.c verify.h

用于校验证书，是sandboxie颁发的证书，公钥也在该模块中硬编码

* 证书校验
  
  * 接口 KphValidateCertificate  用于解析和验证sandboxie文件夹下的证书信息

* 哈希/验签函数
  
  * MyInitHash  初始化哈希方法，默认调用bcrypt服务，算法为SHA256
  
  * MyHashData  对数据进行哈希
  
  * MyFinishHash  结束计算数据哈希

* 文件哈希/验签
  
  * KphHashFile  读入并哈希数据
  
  * KphVerifyFile  

* 内部函数
  
  * GetFwUuid  使用SystemFirmwareTableInformation调用ZwQuerySystemInformation获取SMBIOS的uuid





### 内存管理

源文件：mem.c

* 接口 Mem_AllocEx  底层分配函数，调用common的pool结构相关api分配内存

* 接口 Mem_AllocStringEx  使用Mem_AllocEx创建一个string

* 接口 Mem_FreeString

* 接口 Mem_GetLockResource  使用ExAllocatePoolWithTag创建一个资源

* 接口 Mem_FreeLockResource  释放资源

## common

SboxDll和SboxDrv共用的一些组件

### 内存管理

#### pool

#### 结构

##### PAGE

```c
struct PAGE {
    LIST_ELEM list_elem;
    PAGE *next;
    POOL *pool;
    ULONG eyecatcher;
    USHORT num_free;                    // estimated, not accurate
};
```

内存页管理，在该结构后还有一个bitmap用于记录页内存的使用情况

* next  指向下一个页

* pool  指向当前页所属的池

* num_free  空闲的内存块

每个页以cell为单位进行管理，一个cell默认大小为128字节，一个页默认大小为65536字节

##### POOL

```c
struct POOL {

    ULONG eyecatcher;

#ifdef POOL_USE_CUSTOM_LOCK

    LOCK pages_lock;
    LOCK large_chunks_lock;

#elif defined(KERNEL_MODE)

    PERESOURCE lock;

#else /* ! KERNEL_MODE */

    volatile LONG thread;
    CRITICAL_SECTION lock;

#endif

    LIST pages;                         // pages searched during allocation
    LIST full_pages;                    // full pages that are not searched
    LIST large_chunks;

    UCHAR initial_bitmap[PAGE_BITMAP_SIZE];
};
```

* initial_bitmap  用于初始化页的bitmap

##### 函数

* 底层内存分配
  
  * Pool_Alloc_Mem  分配内存，若为内核模式调用ExAllocatePool2（ExAllocatePoolWithTag），若为用户模式则调用NtAllocateVirtualMemory
  
  * Pool_Free_Mem  释放内存，若为内核模式调用ExFreePoolWithTag，若为用户模式调用VirtualFree

* 内存页管理
  
  * Pool_Alloc_Page  调用Pool_Alloc_Mem分配内存页，并且在内存页的头部初始化页面结构体。默认内存页大小65536B

* 内存池管理
  
  * 接口 Pool_Create  直接调用Pool_CreateTagged创建池
  
  * Pool_CreateTagged  先调用Pool_Alloc_Page分配一个页，初始化Page结构体与initial_bitmap（计算Page中有多少Cell，并对每个Cell对应的bit置1），在Page结构体后初始化Pool结构体
  
  * 接口 Pool_Delete  删除pool，依次删除large_chunk pages和full_pages

* 内存Cell管理
  
  * Pool_Find_Cells
  
  * Pool_Get_Cells
  
  * Pool_Free_Cells

* 内存large_chunk
  
  * Pool_Get_Large_Chunk
  
  * Pool_Free_Large_Chunk

* 内存分配释放
  
  * 接口 Pool_Alloc
  
  * 接口 Pool_Free

### 文件流式读取

源文件：stream.h stream.c

用于流式读取文件，其中包含读取UTF编码格式的函数（Stream_Read_BOM、Stream_Read_Wchar）
