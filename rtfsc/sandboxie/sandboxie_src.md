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

* 检查证书  KphValidateCertificate

* 初始化obj模块  Obj_Init

* 初始化conf模块  Conf_Init

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

* 进程校验
  
  * 接口 KphVerifyCurrentProcess  对当前进程的可执行文件镜像验签

* 哈希/验签函数
  
  * MyInitHash  初始化哈希方法，默认调用bcrypt服务，算法为SHA256
  
  * MyHashData  对数据进行哈希
  
  * MyFinishHash  结束计算数据哈希
  
  * KphVerifySignature  对传入的哈希和签名进行校验

* 文件哈希/验签
  
  * KphHashFile  读入并哈希数据
  
  * KphVerifyFile  用KphHashFile对文件进行哈希，之后用KphVerifySignature将哈希与签名进行校验

* 读取签名
  
  * KphReadSignature  从文件读入签名
  
  * KphParseDate  解析签名中的日期信息
  
  * KphGetBuildDate  转换日期信息格式为TIME_FIELDS
  
  * KphGetDate  获取当前日期信息
  
  * KphGetDateInterval  将日期信息差值转换为100ns为单位的时间戳

* 内部函数
  
  * GetFwUuid  使用SystemFirmwareTableInformation调用ZwQuerySystemInformation获取SMBIOS的uuid

### 内存管理

源文件：mem.c

* 接口 Mem_AllocEx  底层分配函数，调用common的pool结构相关api分配内存

* 接口 Mem_AllocStringEx  使用Mem_AllocEx创建一个string

* 接口 Mem_FreeString

* 接口 Mem_GetLockResource  使用ExAllocatePoolWithTag创建一个资源

* 接口 Mem_FreeLockResource  释放资源

### obj模块

源文件：obj.c obj_flt.c obj_xp.c obj.h

该模块用于处理一些跟内核对象相关的内容，包括

*其中obj.c列出了目前内核支持的所有内核对象类型的表*

* 注册filter回调
  
  * Obj_Load_Filter  由Ipc_Init或Conf_Api_Reload调用，用于注册进程和线程的创建、复制回调函数
  
  * Obj_Init_Filter  由Obj_Init调用，获取ObRegisterCallbacks和ObUnRegisterCallbacks函数地址
  
  * Obj_Unload_Filter  卸载上述回调
  
  * Obj_PreOperationCallback  具体的回调函数实现，首先忽略了内核态创建的进程或线程，之后获取线程/进程的权限（DesiredAccess），之后调用Thread_CheckObject_CommonEx

* 获取函数地址
  
  * Obj_GetTypeObjectType  根据一些硬编码特征获取未导出函数GetTypeObjectType的地址

* 获取内核对象
  
  * Obj_GetObjectType  获取`\\ObjectTypes`下参数指定名字的对象

* 添加需要监控的内核对象类型
  
  * Obj_AddObjectType  调用Obj_GetObjectType获取特定名字的内核对象类型，若存在则加入对象类型表

* 初始化
  
  * 接口  Obj_Init  调用Obj_AddObjectType将Job Event Mutant Semaphore Section ALPC Port SymbolicLink对象加入对象类型表，并调用Obj_Init_Filter初始化Filter

* 析构
  
  * 接口  Obj_Unload  调用Obj_Unload_Filter卸载Filter

* 信息获取
  
  * 接口  Obj_GetNameOrFileName  根据传入的内核对象指针获取对应名字。调用Obj_GetName直接尝试获取参数指定的内核对象名，若获取失败则考虑为文件对象，首先解析ObjectHeader结构是否为文件，若是则调用Obj_GetParseName获取文件对象名
  
  * Obj_GetName  总体就是调用ObQueryNameString获取参数指向的内核对象名，这边根据系统不同处理了一堆case（有点shi山）
  
  * Obj_GetParseName  调用Obj_GetName获取参数指向的内核对象名，这边多了一堆去除路径名中的反斜杠的逻辑

### conf模块

源文件：conf.c conf.h conf_user.c conf_expand.c

用于配置文件相关的操作，如读取、合并等

#### 结构

##### CONF_DATA

用于保存一个完整配置，一个完整配置由多个配置段构成，而配置段又由一系列配置构成。该结构同时使用了链表和映射表保存配置，因为使用链表可以保留配置的顺序，而映射表方便查找

```c
typedef struct _CONF_DATA {

    POOL *pool;
    LIST sections;      // CONF_SECTION
#ifdef USE_CONF_MAP
    HASH_MAP sections_map;
#endif
    ULONG home;         // 1 if configuration read from Driver_Home_Path
    WCHAR* path;
    ULONG encoding;     // 0 - unicode, 1 - utf8, 2 - unicode (byte swapped)
    volatile ULONG use_count;

} CONF_DATA;
```

* pool  对应的内存池

* sections  配置段链表

* sections_map  配置段映射表

* home  驱动所在目录

* path  对应配置文件目录

* encoding  配置的编码

* use_count  信号量

##### CONF_SECTION

用于保存配置段，该结构同时使用了链表和映射表保存配置，因为使用链表可以保留配置的顺序，而映射表方便查找

```c
typedef struct _CONF_SECTION {

    LIST_ELEM list_elem;
    WCHAR *name;
    LIST settings;      // CONF_SETTING
#ifdef USE_CONF_MAP
    HASH_MAP settings_map;
#endif
    BOOLEAN from_template;

} CONF_SECTION;
```

* name  配置段名

* settings  存储配置的链表

* settings_map  存储配置的映射表

* from_template  是否为template段

##### CONF_SETTING

用于保存形如`AAA = BBB`的配置信息，并转换为映射表

```c
typedef struct _CONF_SETTING {

    LIST_ELEM list_elem;
    WCHAR *name;
    WCHAR *value;
    BOOLEAN from_template;
    BOOLEAN template_handled;

} CONF_SETTING;
```

* list_elem  链表结构

* name  设置名

* value  设置值

* from_template  是否为模板文件的配置

* template_handled  该配置是否已经被merge

#### 函数

* 资源锁
  
  * Conf_AdjustUseCount

* 底层数据读取与解析
  
  * Conf_Read_Line  从文件流中读入一行，具体逻辑主要是一些去控制字符空字符等操作
  
  * Conf_Read_Settings  读取形如`AAA = BBB`的字符串，并作为键值对赋值给map
  
  * Conf_Read_Sections  读取形如`[name]`的配置块为映射表，再作为一个键值加到全局配置的映射表中

* 配置操作
  
  * 接口 Conf_Read  从文件中读取配置
  
  * 接口 Conf_Get_Section  获取配置中指定的section
  
  * 接口 Conf_Expand  将配置的值展开为完整的值（如配置值是注册表路径则补全该路径）
  
  * Conf_Merge_Templates  合并全部配置
  
  * Conf_Merge_Template  将传入的配置中指定的模板段（形如`Template_XXX`的配置段）加入传入的目标配置段
  
  * Conf_Merge_Global  将传入的全局配置段合并到完整配置中

### api模块

源文件：api.h api.c

#### 函数

* 顶层回调
  
  * Api_Irp_CREATE  IRP_MJ_CREATE回调，这边增加了Api_UseCount信号量后直接返回
  
  * Api_Irp_CLEANUP  IRP_MJ_CLEANUP回调，这边减少了Api_UseCount信号量后直接返回
  
  * Api_FastIo_DEVICE_CONTROL  IOCTL回调
  
  * Api_EnterCriticalSection  进入临界区，CREATE和CLEANUP时会先调用，该调用将IRP设置为APC_LEVEL，将会屏蔽APC中断
  
  * Api_LeaveCriticalSection  出临界区，在CREATE和CLEANUP退出时调用，会还原原来的IRP

* 顶层接口
  
  * 接口 Api_Init  创建驱动device，并设置其ioctl接口，将其分配给各个api，此后调用Api_SetFunction设置几个预设的API，最后将Api_UseCount置0，表示可以使用
  
  * 接口 Api_Unload  回收内存，删除创建的驱动device
  
  * 接口 Api_Disable  测试API是否可以停用，

* 功能函数
  
  * Api_SetFunction  设置IOCTL自定义消息对应的回调，将其加入一个大的回调函数数组

#### 注册的API

* 默认注册（api.c）
  
  * Api_GetVersion  获取驱动的版本
  
  * Api_LogMessage  记录log，与log模块相关，具体见log模块
  
  * Api_GetMessage  从api log缓冲区中获取seq_number指定的消息
  
  * Api_GetHomePath  获取home目录，这边主要根据驱动所在的目录获取
  
  * Api_SetServicePort  设置alpc端口，注意只有签名过的sanboxie服务可以设置。这里只变更了Api_ServicePortObject变量
  
  * Api_ProcessExemptionControl  设置进程是否可以访问网络和打印机打印成文件，这里调用了process.c的接口对进程相关配置进行设置，注意只有签名过的sanboxie服务可以设置
  
  * Api_QueryDriverInfo  获取sandboxie驱动当前配置信息
  
  * Api_SetSecureParam  配置sandboxie的注册表项，注意只有签名过的sanboxie服务可以设置
  
  * Api_GetSecureParam  读取sandboxie的注册表项，这里会检查表项的签名

* driver.c注册
  
  * Driver_Api_Unload

* conf.c注册
  
  * Conf_Api_Reload  重新加载配置文件。验证程序签名，调用Conf_Read读取配置文件，并且根据配置设置WFP和obj_filter状态，最后调用Api_SendServiceMessage发送配置成功加载的消息
  
  * Conf_Api_Query  读取某项配置

### log模块

#### log_buff

源文件：log_buff.c log_buff.h

用于创建和管理log缓冲区

##### 结构

###### LOG_BUFFER

用于保存log的缓冲区

```c
typedef struct _LOG_BUFFER
{
    LOG_BUFFER_SEQ_T seq_counter;
    SIZE_T buffer_size;
    SIZE_T buffer_used;
    CHAR* buffer_start_ptr;
    CHAR buffer_data[0]; // [[SIZE 4][DATA n][SEQ 4][SITE 4]][...] // Note 2nd size tags allows to traverse the ring in both directions
} LOG_BUFFER;
```

* seq_counter  当前有几条entry

* buffer_size  缓冲区的大小

* buffer_used  缓冲区已用空间

* buffer_start_ptr  缓冲区是一块循环缓冲区，该指针指向循环缓冲区的起始位置

* buffer_data  缓冲区空间从这个指针开始

##### 函数

* 接口 log_buffer_init  创建log_buff空间。这里直接用ExAllocatePoolWithTag创建LOG_BUFFER结构体

* 接口 log_buffer_free  释放log_buff空间

* 底层字节操作函数
  
  * log_buffer_byte_at  用户在log_buff上获取下一个字节的指针。这里缓冲区是循环的，即若指针上溢则会从头开始数，下溢则会从后开始数
  
  * log_buffer_push_bytes  向缓冲区存储n个字节
  
  * log_buffer_get_bytes  从缓冲区读取n个字节

* entry操作函数
  
  * log_buffer_get_size  获取指针指向的entry的长度
  
  * log_buffer_get_seq_num  获取指针指向的entry的序列号
  
  * 接口 log_buffer_push_entry  向log_buff写入一条entry记录，该函数返回缓冲区指针给调用者写入记录的具体内容。先判断缓冲区大小，若不够大看是否设置了can_pop标志，如果设置了则把签名的entry记录pop出来腾空间，之后写入新的entry记录。一条entry记录结构如下，注意一条记录的前后都有一个size字段
    
    ```
    |   size  |
    | seq_num |
    |   内容   |
    |   size  |
    ```
  
  * 接口 log_buffer_pop_entry  从log_buff弹出一条记录，注意这里弹出的是start_ptr指向的entry，即当前最早的一条记录
  
  * 接口 log_buffer_get_next  根据传入的seq_num获取对应的记录entry

#### log

源文件：log.c log.h

* 接口 Log_Msg  用于记录log，这边会根据传入标志的不同为log添加session_id、pid等信息

* 接口 Log_Popup_Msg  以api消息的形式发送记录log消息，基本是Log_Popup_Msg_Ex函数的wrapper

* 接口 Log_Popup_Msg_Ex  以api消息的形式发送记录log消息，这里简单格式化后使用Api_AddMessage和Api_SendServiceMessage来记录消息

#### api

源文件：api.c api.h

api中有几个接口是用于处理log消息的

* Api_LogMessage  为传入的msgid构造log消息，调用Log_Popup_Msg_Ex记录消息

* Api_AddMessage  获取Api_LogBuffer的一个entry，之后写入消息

* Api_SendServiceMessage  从api log获取一条消息。这里只允许服务所在的session查询log，逻辑主要就是查询、解析、复制消息，最后向sandboxie服务程序的alpc服务发送消息

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

### 数据结构

#### map

源文件：map.c map.h

映射表

##### 结构

###### map_base_t

描述映射表

```c
struct map_base_t {
  map_node_t **buckets;
  unsigned int nbuckets, nnodes;

  void* mem_pool;
  void*(*func_malloc)(void* pool, size_t size);
  void(*func_free)(void* pool, void* ptr);

  int(*func_key_size)(const void* key);
  unsigned int (*func_hash_key)(const void* key, size_t size);
  BOOLEAN (*func_match_key)(const void* key1, const void* key2);
};
```

* buckets  指向bucket数组，即哈希表的槽

* nbuckets nnodes  当前槽数和节点数

* mem_pool  指向当前内存池（若使用了Pool）

* func_malloc func_free  指向map内存分配的函数

* func_key_size  指向计算键长度的函数

* func_hash_key  指向计算哈希的函数

* func_match_key  指向判断两个键是否相同的函数，若未指定则使用memcmp

###### map_node_t

描述映射表项

```c
struct map_node_t {
  unsigned int hash;
  void *value;
  map_node_t *next;
  char key[0];
  /* char value[]; */
};
```

* hash  节点的哈希值

* value  指向节点的值

* next  指向具有同样哈希的其他节点

* key  键

###### map_iter_t

```c
typedef struct {
  unsigned int bucketIdx;
  map_node_t *node;
  int ksize;
  const void* key;
  void* value;
} map_iter_t;
```

##### 函数

* 内存管理
  
  分为两种，一种直接用malloc和free，一种用Pool分配和释放
  
  * map_alloc  分配内存
  
  * map_free  释放内存

* 功能函数
  
  * map_hash  获取键的哈希
  
  * str_map_hash  获取宽字符字符串键的哈希
  
  * map_wcssize  键的长度（字节）
  
  * map_wcsimatch  比较两个键是否相同
  
  * str_map_match  比较两个宽字符字符串键是否相同
  
  * map_bucket_idx  获取当前哈希对应的槽的索引

* 底层操作
  
  * map_new_node  为新的map_node_t结构赋值
  
  * map_add_node  将新的节点挂到链表上。首先使用map_bucket_idx获取槽下标，之后遍历槽上节点的链表，若append为true挂到最后一个，否则挂到链表头
  
  * map_resize  重新分配map的槽数。首先分配一个新的槽数组，之后将map上已有的节点以一个单独的链表保存，最后更换map的槽，遍历链表并使用map_add_node将节点加到新的槽上
  
  * map_getmatch  获取传入的键和bucket获取对应的节点
  
  * map_getref  获取键对应的值。先获取键的哈希，再拿到对应bucket，最后调用map_getmatch获取对应节点

* 初始化
  
  - 接口 map_init 初始化一个map_base_t结构

* 操作
  
  * 接口 map_add  向map添加节点。map_new_node新建节点，判断当前节点数是否大于两倍槽数，若大于则调用map_resize扩容；最后调用map_add_node添加节点
  
  * 接口 map_get  获取map对应的节点，基本就是直接包装了map_getref
  
  * 接口 map_take  获取map对应的节点，并将节点从map取出（操作后节点不在map中）
  
  * 接口 map_clear  清空map。遍历bucket中的节点，释放节点内存，释放bucket内存
  
  * 接口 map_iter  初始化普通迭代器，该迭代器从第一个槽开始迭代
  
  * 接口 map_key_iter  初始化键迭代器，用于迭代某个键对应的节点。注意该map实现似乎不要求键的唯一性，即可以为同一个键添加多个节点
  
  * 接口 map_next  获取迭代的下一个节点，其中包含普通iter和key_iter的逻辑
