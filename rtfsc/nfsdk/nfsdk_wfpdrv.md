[ComodoSecurity/openedr: Open EDR public repository](https://github.com/ComodoSecurity/openedr)

[zengjunlong/JsqSdk: This is a private project](https://github.com/zengjunlong/JsqSdk)

[AngusLkc/nfsdk_v1.6.1.7](https://github.com/AngusLkc/nfsdk_v1.6.1.7)

[netfiltersdk.com/help/nfsdk2/](https://netfiltersdk.com/help/nfsdk2/)

[[转载]使用 Lookaside List 分配内存 - Acg!Check - 博客园](https://www.cnblogs.com/Acg-Check/p/4268469.html)  这一篇有图，但是排版太难看了

[Lookaside List 详解-CSDN博客](https://blog.csdn.net/yangdazhi/article/details/13745257)  这篇排版稍微好一些，但是没图（两个都属于是抄都抄不明白）

## 通用组件

### hashtable.c

说是哈希表，其实就是个单链表，用索引作为哈希

* hash_table_new  新建哈希表

* hash_table_free  释放哈希表

* ht_add_entry  添加元素

* ht_find_entry  查找元素

* ht_remove_entry  删除元素

## wfp框架相关代码

### callouts.c

#### 结构

##### NF_CALLOUT

用于记录callout函数列表及其layer

```c
struct NF_CALLOUT
{
   FWPS_CALLOUT_CLASSIFY_FN classifyFunction;
   FWPS_CALLOUT_NOTIFY_FN notifyFunction;
   FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN flowDeleteFunction;
   GUID const* calloutKey;
   UINT32 flags;
   UINT32* calloutId;
}
```

## 设备相关

### driver.c

#### 函数

##### DriverEntry

* 将所有DriverObject的回调设为devctrl_dispatch

* 初始化工作
  
  * wfplink_init
  
  * devctrl_init
  
  * rules_init
  
  * flowctl_init
  
  * tcpctx_init
  
  * udpctx_init
  
  * ipctx_init
  
  * FwpmBfeStateGet0获取WFP引擎状态
    
    * 若正在运行则调用callouts_init打开设备
    
    * 否则调用FwpmBfeStateSubscribeChanges0注册引擎状态回调（bfeStateCallback），当WFP引擎开启时打开设备

##### driverUnload

* FwpmBfeStateUnsubscribeChanges0  卸载WFP引擎回调

* cleanup  清理各个模块

##### cleanup

* devctrl_setShutdown

* flowctl_free

* callouts_free

* rules_free

* devctrl_free

* tcpctx_free

* udpctx_free

* ipctx_free

### dynlink.c

用于从内存解析PE文件，并获取导出函数的地址

#### 函数

##### dynlink_getModuleBase

获取模块基址并加载到内存

##### dynlink_getProcAddress

从模块解析对应的导出函数地址

### wfplink.c

主要用于导入wfp函数，以及对wfp进行patch等操作（默认不使用）

#### 函数

##### wfplink_init

调用wfplink_resolve获取wfp各个函数指针

##### wfplink_resolve

* 获取fwpkclnt.sys模块基址，并解析其导出表，将各个wfp函数的指针放入全局指针

* wfplink_applyPatches对netio.sys的FeAcquireWritableLayerDataPointer函数进行patch，以泄露其上下文。**默认不使用该功能以适配杀毒软件**

### devctrl.c

#### 函数

##### devctrl_init

* 若`\\Registry\\Machine\\SYSTEM\\ControlSet001\\services\\webssx`存在，使用UDP兼容模式
* 初始化g_ioQueue和g_pendedIoRequests链表
* 初始化g_slIoQueue自旋锁
* 初始化g_ioThreadEvent事件
* 以函数devctrl_ioThread创建线程，并将该线程设为高优先级，主线程通过g_ioThreadEvent事件通信
* 初始化g_slTcpInjectQueue自旋锁
* 初始化g_tcpInjectQueue链表
* 初始化g_threadIoEvent事件
* 以函数devctrl_injectThread创建线程，并将该线程设为高优先级，主线程通过g_threadIoEvent事件通信
* 初始化三个LookasideList：
  * g_ioQueueList  NF_QUEUE_ENTRY结构
  * g_udpInjectContextLAList  NF_UDP_INJECT_CONTEXT结构
  * g_ipInjectContextLAList  NF_IP_INJECT_CONTEXT结构
* 下面是与wfp相关的操作
  * NdisAllocateGenericObject分配NDIS对象
  * NdisAllocateNetBufferListPool分配NET_BUFFER_LIST对象池
  * FwpsInjectionHandleCreate0对下列网络流进行注入
    * IPV4地址的TCP和UDP
    * IPV6地址的TCP和UDP
    * 其他地址的TCP和UDP
  * 获取驱动名，假设为[drvname]，创建设备`\\Device\\CtrlSM\\[drvname]`及其符号链接`\DosDevices\CtrlSM\[drvname]`
  * 从驱动对应的注册表读入seclevel项，并调用setDeviceDacl根据该项设置当前DeviceObject的DACL

## 网络相关

### ipctx.c

#### ref

[基于WFP的windows驱动对TCP数据的抓取,修改以及注意事项_wfp拦截tcp-CSDN博客](https://blog.csdn.net/Mr_oOo_/article/details/122112496)  NdisRetreatNetBufferDataStart函数的作用

#### 结构体

##### IPCTX

保存IP记录的上下文

```c
typedef struct _IPCTX
{
    LIST_ENTRY    pendedPackets;    // List of NF_IP_PACKET

    ULONG        pendedSendBytes;    // Number of bytes in outbound packets from pendedSendPackets
    ULONG        pendedRecvBytes;    // Number of bytes in inbound packets from pendedRecvPackets

    KSPIN_LOCK    lock;                // Context spinlock
} IPCTX, *PIPCTX;
```

* pendedPacket  记录IP数据包的链表

* pendedSendBytes  发送的字节数

* pendedRecvBytes  接收的字节数

* lock  链表锁

##### NF_IP_PACKET

记录单个捕获的IP数据包

```c
typedef struct _NF_IP_PACKET
{    
    LIST_ENTRY            entry;
    NF_IP_PACKET_OPTIONS options;
    BOOLEAN                isSend;
    char *                dataBuffer;
    ULONG                dataLength;
} NF_IP_PACKET, *PNF_IP_PACKET;
```

#### 函数

##### ipctx_init

* 初始化lookaside链表g_ipPacketsLAList

* 初始化链表g_ipCtx

##### ipctx_cleanup

* 遍历g_ipCtx的包链表，并释放所有NF_IP_PACKET结构

##### ipctx_free

* ipctx_cleanup释放所有包

* 释放lookaside链表g_ipPacketsLAList

##### ipctx_pushPacket

* 获取包大小，判断是否超出包拦截允许的大小范围（若超出则退出），更新发送和接收包的长度

* ipctx_allocPacket分配IP数据包空间

* 若当前请求是接收数据包的请求，调用NdisRetreatNetBufferDataStart多分配一个IP数据头的空间

* NdisGetDataBuffer获取数据包访问权限，并复制到dataBuffer指向的缓冲区

* ipctx_getIPHeaderLengthAndProtocol获取IP头长度和协议

* 若当前请求是接收数据包的请求，调用NdisAdvanceNetBufferDataStart将多分配的IP数据头空间归还

* 将解析出的IP头信息根据协议等填入NF_IP_PACKET结构体中

* 将NF_IP_PACKET结构放入g_ipCtx的链表中

##### ipctx_getIPHeaderLengthAndProtocol

* NET_BUFFER_DATA_LENGTH获取包长度

* NdisGetDataBuffer获取IP头

* 判断IP头版本
  
  * 若为IPv4，则直接返回IP头长度以及协议字段
  
  * 若为IPv6，则需要解析IPv6头以及扩展头，以获取IP头长度与协议字段

##### ipctx_allocPacket

为新的IP数据包分配缓冲区，具体先在IP的全局Lookaside List分配一个NF_IP_PACKET结构，之后在FILD池上分配空间存储数据包（dataBuffer成员）

##### ipctx_freePacket

释放IP数据包缓冲区，第一步释放FILD池空间，第二步释放Lookaside List空间

### udpctx.c

#### 结构体

##### UDPCTX

```c
typedef struct _UDPCTX
{
    LIST_ENTRY entry;

    HASH_ID    id;
    PHASH_TABLE_ENTRY id_next;

    UINT64                transportEndpointHandle;
    PHASH_TABLE_ENTRY    transportEndpointHandle_next;

    BOOLEAN        closed;            // TRUE if the context is disassociated from WFP flow

    ULONG        filteringFlag;    // Values from NF_FILTERING_FLAG
    ULONG        processId;        // Process identifier
    USHORT        ip_family;        // AF_INET or AF_INET6
    UCHAR        localAddr[NF_MAX_ADDRESS_LENGTH];    // Local address
    UCHAR        remoteAddr[NF_MAX_ADDRESS_LENGTH];    // Remote address
    USHORT      ipProto;        // protocol

    UINT16        layerId;        // WFP layer id
    UINT32        calloutId;        // WFP callout id

    LIST_ENTRY    pendedSendPackets;    // List of outbound packets
    LIST_ENTRY    pendedRecvPackets;    // List of inbound packets

    ULONG        pendedSendBytes;    // Number of bytes in outbound packets from pendedSendPackets
    ULONG        pendedRecvBytes;    // Number of bytes in inbound packets from pendedRecvPackets

    ULONG        injectedSendBytes;    // Number of bytes in injected outbound packets
    ULONG        injectedRecvBytes;    // Number of bytes in injected inbound packets

    BOOLEAN        sendInProgress;        // TRUE if the number of injected outbound bytes reaches a limit
    BOOLEAN        recvInProgress;        // TRUE if the number of injected inbound bytes reaches a limit

    UDP_REDIRECT_INFO    redirectInfo;
    BOOLEAN        seenPackets;
    BOOLEAN        redirected;

    tFlowControlHandle    fcHandle;
    uint64_t        inLastTS;
    uint64_t        outLastTS;

    uint64_t        inCounter;
    uint64_t        outCounter;

    uint64_t        inCounterTotal;
    uint64_t        outCounterTotal;

    BOOLEAN            filteringDisabled;

    wchar_t            processName[MAX_PATH];

    PISID            pPackageSid;

    ULONG        refCount;            // Reference counter

    LIST_ENTRY    auxEntry;            // List entry for adding context to temp lists

    KSPIN_LOCK    lock;                // Context spinlock
} UDPCTX, *PUDPCTX;
```

#### 函数

##### udpctx_init

* hash_table_new建立两个查找链表
  
  * g_phtUdpCtxById  用于通过ID查找对应的UDPCTX结构
  
  * g_phtUdpCtxByHandle  用于通过句柄查找对应的UDPCTX结构

* ExInitializeNPagedLookasideList建立两个lookaside链表
  
  * g_udpCtxLAList  用于存放UDPCTX结构
  
  * g_udpPacketsLAList  用于存放NF_UDP_PACKET结构

* 初始化g_lUdpCtx链表  用于记录UDPCTX结构

##### udpctx_alloc

* g_udpCtxLAList分配新UDPCTX结构 

* 初始化发送和接收数据链表
  
  * pendedSendPackets成员
  
  * pendedRecvPackets成员

* 设置UdpCtx的引用计数为2

* 若传入了transportEndpointHandle参数
  
  * 将句柄和id挂到对应链表上
    
    * g_phtUdpCtxByHandle
    
    * g_phtUdpCtxById
  
  * 将新的UDPCTX插入g_lUdpCtx链表

* 否则UdpCtx不会被挂到handle和id链表上，引用计数减一

##### udpctx_free

- udpctx_releaseFlows释放所有数据包

- 释放两个查找链表
  
  - g_phtUdpCtxById
  
  - g_phtUdpCtxByHandle

- 释放两个lookaside链表
  
  - g_udpCtxLAList
  
  - g_udpPacketsLAList

##### udpctx_allocPacket

* 从g_udpPacketsLAList中分配一个新的NF_UDP_PACKET结构

* 从内存池中分配dataBuffer成员的缓冲区

##### udpctx_freePacket

- 释放dataBuffer缓冲区

- 释放controlData的缓冲区

- 释放g_udpPacketsLAList对应的项

##### udpctx_releaseFlows

* 遍历g_lUdpCtx链表，并对每个项调用udpctx_release释放数据包

##### udpctx_cleanupFlows

* 遍历g_lUdpCtx链表
  
  * 遍历pendedSendPackets链表并调用udpctx_freePacket释放数据包
  
  * 遍历pendedRecvPackets链表并调用udpctx_freePacket释放数据包

##### udpctx_addRef

* UDPCTX的引用计数自增

##### udpctx_postCreateEvents

* 遍历g_lUdpCtx上的所有UDPCTX

* 调用devctrl_pushUdpData将NF_UDP_CREATED作为事件发送，并插入g_ioQueue

* udpctx_release释放当前UDPCTX

##### udpctx_release

释放一个UDPCTX

* 若引用计数不为1，返回

* 若有设置transportEndpointHandle，将其从句柄和ID链表中删除，并从g_lUdpCtx链表中删除

* 释放UDPCTX的所有pendedSendPackets和pendedRecvPackets表项

* udpctx_purgeRedirectInfo删除重定向数据

* 若UDPCTX有pPackageSid字段，则释放

* 将UDPCTX从g_udpCtxLAList链表中删除

##### udpctx_find

根据传入的id在g_phtUdpCtxById表中查找对应元素，返回并将引用计数+1

##### udpctx_findByHandle

根据传入的id在g_phtUdpCtxByHandle表中查找对应元素，返回并将引用计数+1

##### udpctx_purgeRedirectInfo

* 若UDPCTX中设置了redirectInfo.classifyHandle
  
  * 若redirectInfo.isPended为True
    * 调用FwpsCompleteClassify0以异步形式完成一次ALE请求，之后清空isPended属性
  * FwpsReleaseClassifyHandle0释放ALE Flow

### rules_init

### flowctl_init

### tcpctx_init
