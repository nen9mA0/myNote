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

### list.h/list.c

这个文件主要定义了一个list数据结构，主要是给scheduler用的

#### 数据结构

##### List_t

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



#### 函数

##### vListInitialise

```c
void vListInitialise( List_t * const pxList );
```

