# 架构依赖

一些操作直接依赖于处理器架构，因此需要单独针对每个平台进行定义

## portmacro.h

### 数据类型

##### StackType_t

##### BaseType_t

默认整数类型

##### UBaseType_t

默认无符号整数类型

### 临界区

#### 配置

##### portSET_INTERRUPT_MASK_FROM_ISR

若定义了该函数，则使用 [portSET_INTERRUPT_MASK_FROM_ISR](#portSET_INTERRUPT_MASK_FROM_ISR) 和 [portCLEAR_INTERRUPT_MASK_FROM_ISR](#portCLEAR_INTERRUPT_MASK_FROM_ISR) 进入和离开临界区，否则使用 [portENTER_CRITICAL](#portENTER_CRITICAL) 和 [portEXIT_CRITICAL](#portEXIT_CRITICAL) 

#### 函数

##### portSET_INTERRUPT_MASK_FROM_ISR

进入临界区

```c
#define ATOMIC_ENTER_CRITICAL() \
    UBaseType_t uxCriticalSectionType = portSET_INTERRUPT_MASK_FROM_ISR()
```

##### portCLEAR_INTERRUPT_MASK_FROM_ISR

离开临界区

```c
#define ATOMIC_EXIT_CRITICAL() \
    portCLEAR_INTERRUPT_MASK_FROM_ISR( uxCriticalSectionType )
```

##### portENTER_CRITICAL

进入临界区

```c
#define ATOMIC_ENTER_CRITICAL()    portENTER_CRITICAL()
```

##### portEXIT_CRITICAL

离开临界区

```c
#define ATOMIC_EXIT_CRITICAL()     portEXIT_CRITICAL()
```

## portable.h

### Stack相关

#### pxPortInitialiseStack

根据是否有MPU和溢出检测，分为下列四种定义

```c
#if ( portUSING_MPU_WRAPPERS == 1 )
    #if ( portHAS_STACK_OVERFLOW_CHECKING == 1 )
        // 有MPU，有溢出检测
        StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                             StackType_t * pxEndOfStack,
                                             TaskFunction_t pxCode,
                                             void * pvParameters,
                                             BaseType_t xRunPrivileged ) PRIVILEGED_FUNCTION;
    #else
        // 有MPU，无溢出检测
        StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                             TaskFunction_t pxCode,
                                             void * pvParameters,
                                             BaseType_t xRunPrivileged ) PRIVILEGED_FUNCTION;
    #endif
#else /* if ( portUSING_MPU_WRAPPERS == 1 ) */
    #if ( portHAS_STACK_OVERFLOW_CHECKING == 1 )
        // 无MPU，有溢出检测
        StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                             StackType_t * pxEndOfStack,
                                             TaskFunction_t pxCode,
                                             void * pvParameters ) PRIVILEGED_FUNCTION;
    #else
        // 无MPU，无溢出检测
        StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                             TaskFunction_t pxCode,
                                             void * pvParameters ) PRIVILEGED_FUNCTION;
    #endif
#endif /* if ( portUSING_MPU_WRAPPERS == 1 ) */
```

溢出检测即多了一个标识栈底的指针，MPU则多了一个xRunPrivileged字段来描述当前特权级

# 主要代码

## 原子操作

### atomic.h

#### 宏

##### ATOMIC_COMPARE_AND_SWAP_SUCCESS

CAS操作成功

```c
#define ATOMIC_COMPARE_AND_SWAP_SUCCESS    0x1U
```

##### ATOMIC_COMPARE_AND_SWAP_FAILURE

CAS操作失败

```c
#define ATOMIC_COMPARE_AND_SWAP_FAILURE    0x0U
```

#### 函数

原子操作跟大多数其他库的一致，这里先封装了一层进入和离开临界区的函数，其他函数操作都会先进入临界区，操作完成后离开临界区

##### 临界区相关

###### ATOMIC_ENTER_CRITICAL

进入临界区

###### ATOMIC_EXIT_CRITICAL

离开临界区

##### CAS与swap

###### Atomic_CompareAndSwap_u32

CAS操作

```c
static portFORCE_INLINE uint32_t Atomic_CompareAndSwap_u32( uint32_t volatile * pulDestination,
                                                            uint32_t ulExchange,
                                                            uint32_t ulComparand );
```

* 与其他CAS一致
  
  ```c
  if(*pulDestination == ulComparand)
      *pulDestination = ulExchange;
  ```

返回值为 [ATOMIC_COMPARE_AND_SWAP_SUCCESS](#ATOMIC_COMPARE_AND_SWAP_SUCCESS) 或 [ATOMIC_COMPARE_AND_SWAP_FAILURE](#ATOMIC_COMPARE_AND_SWAP_FAILURE)

###### Atomic_SwapPointers_p32

指针交换操作

```c
static portFORCE_INLINE void * Atomic_SwapPointers_p32( void * volatile * ppvDestination,
                                                        void * pvExchange );
```

* ```c
  tmp = *ppvDestination;
  *ppvDestination = pvExchange;
  return tmp;
  ```

###### Atomic_CompareAndSwapPointers_p32

指针CAS操作，差别只是第二个参数为指针

```c
static portFORCE_INLINE uint32_t Atomic_CompareAndSwapPointers_p32( void * volatile * ppvDestination,
                                                                    void * pvExchange,
                                                                    void * pvComparand );
```

* ```c
  if(*ppvDestination == pvComparand)
      *ppvDestination = *pvExchange;
  ```

##### 算术操作

###### Atomic_Add_u32

原子加

```c
static portFORCE_INLINE uint32_t Atomic_Add_u32( uint32_t volatile * pulAddend,
                                                 uint32_t ulCount );
```

###### Atomic_Subtract_u32

原子减

```c
static portFORCE_INLINE uint32_t Atomic_Subtract_u32( uint32_t volatile * pulAddend,
                                                      uint32_t ulCount );
```

###### Atomic_Increment_u32

原子自增

```c
static portFORCE_INLINE uint32_t Atomic_Increment_u32( uint32_t volatile * pulAddend );
```

###### Atomic_Decrement_u32

原子自减

```c
static portFORCE_INLINE uint32_t Atomic_Decrement_u32( uint32_t volatile * pulAddend );
```

##### 逻辑运算

###### Atomic_OR_u32

原子或

```c
static portFORCE_INLINE uint32_t Atomic_OR_u32( uint32_t volatile * pulDestination,
                                                uint32_t ulValue );
```

###### Atomic_AND_u32

原子与

```c
static portFORCE_INLINE uint32_t Atomic_AND_u32( uint32_t volatile * pulDestination,
                                                 uint32_t ulValue );
```

###### Atomic_NAND_u32

原子与非

```c
static portFORCE_INLINE uint32_t Atomic_NAND_u32( uint32_t volatile * pulDestination,
                                                  uint32_t ulValue );
```

###### Atomic_XOR_u32

原子异或

```c
static portFORCE_INLINE uint32_t Atomic_XOR_u32( uint32_t volatile * pulDestination,
                                                 uint32_t ulValue );
```

## 数据结构

### list

**主要文件：list.h  list.c**

这个文件主要定义了一个list数据结构，主要是给scheduler用的。也因此该链表在构造时是从小到大排列的

#### 数据结构

##### List_t

列表结构

```c
typedef struct xLIST
{
    listFIRST_LIST_INTEGRITY_CHECK_VALUE      /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    volatile UBaseType_t uxNumberOfItems;
    ListItem_t * configLIST_VOLATILE pxIndex; /*< Used to walk through the list.  Points to the last item returned by a call to listGET_OWNER_OF_NEXT_ENTRY (). */
    MiniListItem_t xListEnd;                  /*< List item that contains the maximum possible item value meaning it is always at the end of the list and is therefore used as a marker. */
    listSECOND_LIST_INTEGRITY_CHECK_VALUE     /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
} List_t;
```

* 第一个和最后一个是用于检查链表元素的
* uxNumberOfItems  链表元素个数
* pxIndex  有点类似一个迭代器指针
* xListEnd  是一个item项，用于表示最后的item

##### ListItem_t

```c
struct xLIST_ITEM
{
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE           /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    configLIST_VOLATILE TickType_t xItemValue;          /*< The value being listed.  In most cases this is used to sort the list in ascending order. */
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;     /*< Pointer to the next ListItem_t in the list. */
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious; /*< Pointer to the previous ListItem_t in the list. */
    void * pvOwner;                                     /*< Pointer to the object (normally a TCB) that contains the list item.  There is therefore a two way link between the object containing the list item and the list item itself. */
    struct xLIST * configLIST_VOLATILE pxContainer;     /*< Pointer to the list in which this list item is placed (if any). */
    listSECOND_LIST_ITEM_INTEGRITY_CHECK_VALUE          /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
};
typedef struct xLIST_ITEM ListItem_t;                   /* For some reason lint wants this as two separate definitions. */
```

* pxNext  下一个节点
* pxPrevious  上一个节点
* pvOwner  指向拥有这个链表项的结构，如一个TCB结构
* pxContainer  指向包含该链表项的List_t

##### MiniListItem_t

```c
struct xMINI_LIST_ITEM
{
    listFIRST_LIST_ITEM_INTEGRITY_CHECK_VALUE /*< Set to a known value if configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES is set to 1. */
    configLIST_VOLATILE TickType_t xItemValue;
    struct xLIST_ITEM * configLIST_VOLATILE pxNext;
    struct xLIST_ITEM * configLIST_VOLATILE pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;
```

* pxNext  下一个节点
* pxPrevious  上一个节点

#### 函数

##### vListInitialise

初始化一个链表结构

```c
void vListInitialise( List_t * const pxList );
```

* pxIndex 指向xListEnd
* xListEnd的xItemValue设置为 portMAX_DELAY，即最大值，确保该链表项位于链表尾
* 设置pxNext和pxPrevious都指向xListEnd
* 设置uxNumberOfItems为0

##### vListInitialiseItem

初始化一个链表项

```c
void vListInitialiseItem( ListItem_t * const pxItem );
```

* 设置pxContainer为NULL

##### vListInsertEnd

插入一个链表项到链表迭代器（pxIndex）尾

```c
void vListInsertEnd( List_t * const pxList,
                     ListItem_t * const pxNewListItem );
```

##### vListInsert

插入一个链表项，注意这里插入的顺序是按照元素的 xItemValue 属性从小到大来的

```c
void vListInsert( List_t * const pxList,
                  ListItem_t * const pxNewListItem );
```

##### uxListRemove

移除一个链表项

```c
UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove );
```

注意这里有个处理，若 [List_t](#List_t) 的 pxIndex 项指向要移除的元素，则令 pxIndex 指向要移除元素的前一个

#### 函数宏

##### listINSERT_END

[vListInsertEnd](#vListInsertEnd) 的宏版本

```c
#define listINSERT_END( pxList, pxNewListItem )
```

##### listREMOVE_ITEM

[uxListRemove](#uxListRemove) 的宏版本

```c
#define listREMOVE_ITEM( pxItemToRemove )
```

##### listGET_OWNER_OF_NEXT_ENTRY

返回 pxList 的当前迭代器（pxIndex）的下一个元素的 pvOwner 给 pxTCB

```c
#define listGET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList )
```

###queue

**主要文件： queue.h  queue.c**

#### 数据结构

##### Queue_t

```c
/*
 * Definition of the queue used by the scheduler.
 * Items are queued by copy, not reference.  See the following link for the
 * rationale: https://www.FreeRTOS.org/Embedded-RTOS-Queues.html
 */
typedef struct QueueDefinition /* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
    int8_t * pcHead;           /*< Points to the beginning of the queue storage area. */
    int8_t * pcWriteTo;        /*< Points to the free next place in the storage area. */

    union
    {
        QueuePointers_t xQueue;     /*< Data required exclusively when this structure is used as a queue. */
        SemaphoreData_t xSemaphore; /*< Data required exclusively when this structure is used as a semaphore. */
    } u;

    List_t xTasksWaitingToSend;             /*< List of tasks that are blocked waiting to post onto this queue.  Stored in priority order. */
    List_t xTasksWaitingToReceive;          /*< List of tasks that are blocked waiting to read from this queue.  Stored in priority order. */

    volatile UBaseType_t uxMessagesWaiting; /*< The number of items currently in the queue. */
    UBaseType_t uxLength;                   /*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
    UBaseType_t uxItemSize;                 /*< The size of each items that the queue will hold. */

    volatile int8_t cRxLock;                /*< Stores the number of items received from the queue (removed from the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
    volatile int8_t cTxLock;                /*< Stores the number of items transmitted to the queue (added to the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */

    #if ( ( configSUPPORT_STATIC_ALLOCATION == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
        uint8_t ucStaticallyAllocated; /*< Set to pdTRUE if the memory used by the queue was statically allocated to ensure no attempt is made to free the memory. */
    #endif

    #if ( configUSE_QUEUE_SETS == 1 )
        struct QueueDefinition * pxQueueSetContainer;
    #endif

    #if ( configUSE_TRACE_FACILITY == 1 )
        UBaseType_t uxQueueNumber;
        uint8_t ucQueueType;
    #endif
} xQUEUE;

/* The old xQUEUE name is maintained above then typedefed to the new Queue_t
 * name below to enable the use of older kernel aware debuggers. */
typedef xQUEUE Queue_t;
```

* pcHead  指向queue存储的首地址

* pcWriteTo  指向queue存储的下一个元素地址

* u
  
  * xQueue  当该结构是作为一个queue被使用时
  * xSemaphore  当该结构是作为一个信号量被使用时

* xTasksWaitingToSend

* xTasksWaitingToReceive

* uxMessagesWaiting

* uxLength  队列长度

* uxItemSize  队列每个元素的大小

* cRxLock

* cTxLock

* ucQueueType  queue的类型，有如下几种
  
  * queueQUEUE_TYPE_BASE
  * queueQUEUE_TYPE_SET
  * queueQUEUE_TYPE_MUTEX
  * queueQUEUE_TYPE_COUNTING_SEMAPHORE
  * queueQUEUE_TYPE_BINARY_SEMAPHORE
  * queueQUEUE_TYPE_RECURSIVE_MUTEX

#### 底层函数

##### xTaskRemoveFromEventList

```c
BaseType_t xTaskRemoveFromEventList( const List_t * const pxEventList )
```

##### xQueueGenericReset

```c
BaseType_t xQueueGenericReset( QueueHandle_t xQueue,
                               BaseType_t xNewQueue );
```

#### 中间层函数

**创建：根据是否支持动态内存分配分为两种创建函数**

##### xQueueGenericCreateStatic

```c
QueueHandle_t xQueueGenericCreateStatic( const UBaseType_t uxQueueLength,
                                        const UBaseType_t uxItemSize,
                                        uint8_t * pucQueueStorage,
                                        StaticQueue_t * pxStaticQueue,
                                        const uint8_t ucQueueType );
```

##### xQueueGenericCreate

```c
QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength,
                                  const UBaseType_t uxItemSize,
                                  const uint8_t ucQueueType );
```

**发送信号**

##### xQueueGenericSend

```c
BaseType_t xQueueGenericSend( QueueHandle_t xQueue,
                              const void * const pvItemToQueue,
                              TickType_t xTicksToWait,
                              const BaseType_t xCopyPosition );
```

#### 顶层函数

**创建：根据是否支持动态内存分配分为两种创建函数**

##### xQueueCreate

```c
#define xQueueCreate( uxQueueLength, uxItemSize )    xQueueGenericCreate( ( uxQueueLength ), ( uxItemSize ), ( queueQUEUE_TYPE_BASE ) )
```

##### xQueueCreateStatic

```c
#define xQueueCreateStatic( uxQueueLength, uxItemSize, pucQueueStorage, pxQueueBuffer )    xQueueGenericCreateStatic( ( uxQueueLength ), ( uxItemSize ), ( pucQueueStorage ), ( pxQueueBuffer ), ( queueQUEUE_TYPE_BASE ) )
```

**发送信号**

##### xQueueSendToToFront

用于发送一个信号到queue的前部，注意这个函数不能在ISR中被调用

```c
#define xQueueSendToFront( xQueue, pvItemToQueue, xTicksToWait ) \
    xQueueGenericSend( ( xQueue ), ( pvItemToQueue ), ( xTicksToWait ), queueSEND_TO_FRONT )
```

##### xQueueSendToBack

用于发送一个信号到queue的尾部，注意这个函数不能在ISR中被调用

```c
#define xQueueSendToBack( xQueue, pvItemToQueue, xTicksToWait ) \
    xQueueGenericSend( ( xQueue ), ( pvItemToQueue ), ( xTicksToWait ), queueSEND_TO_BACK )
```

##### xQueueSend

与上个函数相同

## 主要模块

### task

#### 全局变量

##### uxCurrentNumberOfTasks

记录当前运行的task数量

##### pxCurrentTCB

指向当前运行task的TCB

##### pxReadyTasksLists

```c
PRIVILEGED_DATA static List_t pxReadyTasksLists[ configMAX_PRIORITIES ]; /*< Prioritised ready tasks. */
```

用于存放处于ready状态的task的链表。注意这里根据任务的优先级，task会被放进不同的链表

#### 数据结构

##### TCB_t

```c
/*
 * Task control block.  A task control block (TCB) is allocated for each task,
 * and stores task state information, including a pointer to the task's context
 * (the task's run time environment, including register values)
 */
typedef struct tskTaskControlBlock       /* The old naming convention is used to prevent breaking kernel aware debuggers. */
{
    volatile StackType_t * pxTopOfStack; /*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE TCB STRUCT. */

    #if ( portUSING_MPU_WRAPPERS == 1 )
        xMPU_SETTINGS xMPUSettings; /*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE TCB STRUCT. */
    #endif

    ListItem_t xStateListItem;                  /*< The list that the state list item of a task is reference from denotes the state of that task (Ready, Blocked, Suspended ). */
    ListItem_t xEventListItem;                  /*< Used to reference a task from an event list. */
    UBaseType_t uxPriority;                     /*< The priority of the task.  0 is the lowest priority. */
    StackType_t * pxStack;                      /*< Points to the start of the stack. */
    char pcTaskName[ configMAX_TASK_NAME_LEN ]; /*< Descriptive name given to the task when created.  Facilitates debugging only. */

    #if ( ( portSTACK_GROWTH > 0 ) || ( configRECORD_STACK_HIGH_ADDRESS == 1 ) )
        StackType_t * pxEndOfStack; /*< Points to the highest valid address for the stack. */
    #endif

    #if ( portCRITICAL_NESTING_IN_TCB == 1 )
        UBaseType_t uxCriticalNesting; /*< Holds the critical section nesting depth for ports that do not maintain their own count in the port layer. */
    #endif

    #if ( configUSE_TRACE_FACILITY == 1 )
        UBaseType_t uxTCBNumber;  /*< Stores a number that increments each time a TCB is created.  It allows debuggers to determine when a task has been deleted and then recreated. */
        UBaseType_t uxTaskNumber; /*< Stores a number specifically for use by third party trace code. */
    #endif

    #if ( configUSE_MUTEXES == 1 )
        UBaseType_t uxBasePriority; /*< The priority last assigned to the task - used by the priority inheritance mechanism. */
        UBaseType_t uxMutexesHeld;
    #endif

    #if ( configUSE_APPLICATION_TASK_TAG == 1 )
        TaskHookFunction_t pxTaskTag;
    #endif

    #if ( configNUM_THREAD_LOCAL_STORAGE_POINTERS > 0 )
        void * pvThreadLocalStoragePointers[ configNUM_THREAD_LOCAL_STORAGE_POINTERS ];
    #endif

    #if ( configGENERATE_RUN_TIME_STATS == 1 )
        configRUN_TIME_COUNTER_TYPE ulRunTimeCounter; /*< Stores the amount of time the task has spent in the Running state. */
    #endif

    #if ( configUSE_NEWLIB_REENTRANT == 1 )

        /* Allocate a Newlib reent structure that is specific to this task.
         * Note Newlib support has been included by popular demand, but is not
         * used by the FreeRTOS maintainers themselves.  FreeRTOS is not
         * responsible for resulting newlib operation.  User must be familiar with
         * newlib and must provide system-wide implementations of the necessary
         * stubs. Be warned that (at the time of writing) the current newlib design
         * implements a system-wide malloc() that must be provided with locks.
         *
         * See the third party link http://www.nadler.com/embedded/newlibAndFreeRTOS.html
         * for additional information. */
        struct  _reent xNewLib_reent;
    #endif

    #if ( configUSE_TASK_NOTIFICATIONS == 1 )
        volatile uint32_t ulNotifiedValue[ configTASK_NOTIFICATION_ARRAY_ENTRIES ];
        volatile uint8_t ucNotifyState[ configTASK_NOTIFICATION_ARRAY_ENTRIES ];
    #endif

    /* See the comments in FreeRTOS.h with the definition of
     * tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE. */
    #if ( tskSTATIC_AND_DYNAMIC_ALLOCATION_POSSIBLE != 0 )
        uint8_t ucStaticallyAllocated;                     /*< Set to pdTRUE if the task is a statically allocated to ensure no attempt is made to free the memory. */
    #endif

    #if ( INCLUDE_xTaskAbortDelay == 1 )
        uint8_t ucDelayAborted;
    #endif

    #if ( configUSE_POSIX_ERRNO == 1 )
        int iTaskErrno;
    #endif
} tskTCB;

/* The old tskTCB name is maintained above then typedefed to the new TCB_t name
 * below to enable the use of older kernel aware debuggers. */
typedef tskTCB TCB_t;
```

* pxTopOfStack  指向栈顶
* xStateListItem
* xEventListItem
* pxStack  用于保存栈空间的头指针
* pxEndOfStack  栈空间结束的指针

#### 底层函数

##### prvInitialiseNewTask

```c
static void prvInitialiseNewTask( TaskFunction_t pxTaskCode,
                                  const char * const pcName,
                                  const uint32_t ulStackDepth,
                                  void * const pvParameters,
                                  UBaseType_t uxPriority,
                                  TaskHandle_t * const pxCreatedTask,
                                  TCB_t * pxNewTCB,
                                  const MemoryRegion_t * const xRegions );
```

* 初始化 [TCB_t](#TCB_t) 字段
  * 根据栈地址的生长方向，确定pxEndOfStack字段
  * 将task的名字（pcName参数）复制给pcTaskName字段
  * 将task的优先级（uxPriority参数） 赋给uxPriority字段
  * 调用 [vListInitialiseItem](#vListInitialiseItem) 初始化 xStateListItem和xEventListItem链表项，并设置Owner为当前任务的TCB
* 调用 pxPortInitialiseStack，这个函数根据不同处理器和编译器的调用约定，在栈上布局内存，使得返回地址指向 pxTaskCode。这个处理方式使得新创建的task的栈空间布局与task被调度器暂停时的布局一致。

##### prvAddNewTaskToReadyList

```c
static void prvAddNewTaskToReadyList( TCB_t * pxNewTCB );
```

#### 顶层函数

**创建任务**：分为动态和静态

##### xTaskCreate

```c
    BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                            const char * const pcName,
                            const configSTACK_DEPTH_TYPE usStackDepth,
                            void * const pvParameters,
                            UBaseType_t uxPriority,
                            TaskHandle_t * const pxCreatedTask );
```

* 这里根据 portSTACK_GROWTH 宏判断栈的增长方向
  * 若是向下增长，则先分配栈空间再分配TCB，这样可以使得栈溢出时也不会覆盖到TCB
  * 若向上增长，则相反
* 这里的分配 TCB_t和栈 时分别使用 pvPortMalloc 和 pvPortMallocStack
* 调用 [prvInitialiseNewTask](#prvInitialiseNewTask) 初始化task
* 调用 [prvAddNewTaskToReadyList](#prvAddNewTaskToReadyList) 将task加入ready list
