## x86

### 原始结构

```c
#define EXCEPTION_CHAIN_END ((struct _EXCEPTION_REGISTRATION_RECORD * POINTER_32)-1)
typedef enum _EXCEPTION_DISPOSITION {
    ExceptionContinueExecution,
    ExceptionContinueSearch,
    ExceptionNestedException,
    ExceptionCollidedUnwind
} EXCEPTION_DISPOSITION;

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD;

typedef EXCEPTION_RECORD *PEXCEPTION_RECORD;

typedef EXCEPTION_DISPOSITION
       (*PEXCEPTION_ROUTINE) (
            IN struct _EXCEPTION_RECORD *ExceptionRecord,
            IN PVOID EstablisherFrame,
            IN OUT struct _CONTEXT *ContextRecord,
            IN OUT PVOID DispatcherContext
            );

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    PEXCEPTION_ROUTINE Handler;
} EXCEPTION_REGISTRATION_RECORD;

typedef EXCEPTION_REGISTRATION_RECORD *PEXCEPTION_REGISTRATION_RECORD;
```

#### 枚举类型

##### EXCEPTION_DISPOSITION

* ExceptionContinueExecution  处理完SEH后继续从抛出异常的点执行
* ExceptionContinueSearch  没有处理异常，继续遍历下一个节点
* ExceptionNestedException  没有处理异常，从指定的新异常继续遍历
* ExceptionCollidedUnwind

#### 结构体

##### EXCEPTION_RECORD

记录异常抛出点的信息

##### PEXCEPTION_ROUTINE

一个函数指针，指向一个异常处理函数

* `IN struct _EXCEPTION_RECORD *ExceptionRecord` 传入EXCEPTION_RECORD

##### EXCEPTION_REGISTRATION_RECORD

`EXCEPTION_REGISTRATION_RECORD`注册一个异常处理结构

* `struct _EXCEPTION_REGISTRATION_RECORD *Next`  指向下一个SEH
* `PEXCEPTION_ROUTINE Handler`  handler指向处理函数

### 内核相关数据结构

#### 用户层

#### 内核层