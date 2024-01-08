# Windows via C&C++

## 第二章 字符串和字符串处理

### ANSI版本与UNICODE版本

```c
#ifdef UNICODE

typedef WCHAR TCHAR, *PTCHAR,PTSTR;
typedef CONST WCHAR *PCTSTR;
#define __TEXT(quote) quote;

#define __TEXT(quote) L##quote;

#else

typedef CHAR TCHAR, *PTCHAR,PTSTR;
typedef CONST CHAR *PCTSTR;
#define __TEXT(quote) quote

#endif

#define TEXT(quote) __TEXT(quote)
```

对于WINAPI提供A和W两个版本，A接收ANSI字串，W接收Unicode字串，如CreateWindoEx有CreateWindowExA和CreateWindowExW

```C
#ifdef UNICODE
#define CreateWindowEx CreateWindowExW
#else
#define CreateWindowEx CreateWindowExA
#endif
```

**ANSI版本的函数会在函数调用中转为UNICODE**

### 函数

#### _tcslen

```
#ifdef _UNICODE
#define _tcslen wcslen
#else
#define _tcslen strlen
#endif
```

#### 安全的字符串函数

##### 区别

```
PTSTR _tcscpy(PTSTR strDestination, PCTSTR strSource);
error_no _tcscpy_s(PTSTR strDestination, size_t numberOfcharacters, PCTSTR strSource);

PTSTR _tcscat(PTSTR strDestination, PCTSTR strSource);
error_no _tcscat_s(PTSTR strDestination, size_t numberOfcharacters, PCTSTR strSource);
```

s版本函数指示缓冲区大小，可以通过使用**_countof**宏指定

s版本函数当发生错误时指定**errno**，并在退出程序时指定退出代码。

##### 参数的错误处理

###### 处理函数

```
void InvaildParameterHandler(PCTSTR expression, PCTSTR function, PCTSTR file, unsigned int line, uintptr_t /*pReserved*/);
```

* expression:错误信息

* function file line  源码的位置

若没有定义**DEBUG**则不会这几个参数都是NULL

###### 注册处理函数

```
_set_invaild_parameter_handler
```

###### 禁止弹出Debug Assertion Failed对话框

```
_CrtSetReportMode(_CRT_ASSERT, 0);
```

##### 扩展的字符串函数(Strsafe.h)

```
HRESULT StringCchCat(PTSTR pszDest, size_t cchDest, PCTSTR pszSrc);
HRESULT StringCchCatEx(PTSTR pszDest, size_t cchDest, PCTSTR pszSrc, PTSTR *ppszDestEnd, size_t *pcchRemaining, DWORD dwFlags);
```

类似函数还有 StringCchCopy StringCchCopyEx，StringCchPrinf StringCchPrintfEx等

其中Cch表示Count of charaters，通常用**_countof**获取（cchDest参数）

此外还有一组Cb的函数，用**sizeof**获取缓冲区大小（cbDest参数）

* pcchRemaining  指向一个变量，用于返回缓冲区内还有多少字符未用

* ppszDestEnd  指向缓冲区中字符串的末尾

* dwFlags  提供多种处理剩余缓冲区的方法，是下面几个选项的组合
  
  * STRSAFE_FILL_BEGIND_NULL  若函数成功 dwFlags低字节用于填充剩余的缓冲区（'\0'后的部分）
  * STRSAFE_IGNORE_NULLS  把值为NULL的字符串指针视为TEXT("")
  * STRSAFE_FILL_ON_FAILURE  若函数失败则用dwFlags低位填充缓冲区，但最后一字节是'\0'
  * STRSAFE_NULL_ON_FAILURE  若失败则把缓冲区第一字节改为'\0'
  * STRSAFE_NO_TRUNCATION  与上述一样
  
  一些细节看MSDN

##### Windows字符串函数

###### CompareString

```
int CompareString(
    LCID locale;
    DWORD dwCmdFlags.
    PCTSTR pString1,
    int cch1,
    PCTSTR pString2,
    int cch2);
```

LCID根据区域比较字符集

dwCmdFlags可以设置一系列语言相关的比较属性

###### CompareStringOrdinal

不考虑字符集的比较

**注意，Windows字符串比较函数返回与C标准不同，0表示失败，1小于，2等于，3大于**

### 编程规范

* 使用通用字符串类型**TCHAR/PTSTR**

* 使用通用字节类型**BYTE/PBYTE**

* 字符（串）常量**TEXT/_T**

* 使用**_countof(szBuffer)**而不是sizeof(szBuffer)，特别是分配内存时
  
  ```
  #define chmalloc(nCharacters) (TCHAR*)malloc(nCharacters*sizeof(TCHAR))
  ```

* ANSI与UNICODE转换 **WideCharToMultiByte/MultiByteToWideChar**

* 指定**UNICODE**同时指定**_UNICODE**

* **/GS** 检测缓冲区溢出

* 不用kernel32.dll的函数进行字符串处理，如lstrcat/lstrcpy

##### Unicode与ANSI转换需要注意

```
int WideCharToMultiByte(
    UINT uCodePage,
    DWORD dwFlags,
    PCWSTR pWideCharStr,
    int cchWideCHar,
    PSTR pMultiByteStr,
    int cbMultiByte,
    PCSTR pDefaultChar,
    PBOOL pfUsedDefaultChar
);
```

当遇到代码页无法转换的字符时，使用**pDefaultChar**，当这个参数为NULL时，默认一般为'?'，这在文件路径中是一个通配符，**可能导致安全问题**

若有字符不能转换会置位**pfUsedDefaultChar**

#### 一个ANSI和UNICODE版本的函数

用于反转字符串，UNICODE版本如下，实现具体功能

```
BOOL StringReverseW(PWSTR pWideCharStr, DWORD cchLength)
{
    PWSTR pEndOfStr = pWideCharStr + wcsnlen_s(pWideCharStr, cchLength) - 1;
    wchar_t cCharT;

    while (pWideCharStr < pEndOfStr)
    {
        cCharT = *pEndOfStr;
        *pEndOfStr = *pWideCharStr;
        *pWideCharStr = cCharT;
        pEndOfStr--;
        pWideCharStr++;
    }
    return(TRUE);
}
```

ANSI版本用于转换字符串

```
BOOL StringReverseA(PSTR pMultiByteStr, DWORD cchLength)
{
    PWSTR pWideCharStr;
    int nLenOfWideCharStr;
    BOOL fOk = FALSE;

    nLenOfWideCharStr = MultiByteToWideChar(CP_ACP, 0, pMultiByteStr, cchLength, NULL, 0);
    //Set cchWideChar to 0 to get length of multibyte string
    pWideCharStr = (PWSTR)HeapAlloc(GetProcessHeap(),
        0, nLenOfWideCharStr * sizeof(wchar_t));
    if (pWideCharStr == NULL)
        return(fOk);

    MultiByteToWideChar(CP_ACP, 0, pMultiByteStr, cchLength,
        pWideCharStr, nLenOfWideCharStr);
    fOk = StringReverseW(pWideCharStr, cchLength);

    if (fOk)
    {
        WideCharToMultiByte(CP_ACP, 0, pWideCharStr, -1,
            pMultiByteStr, (int)strlen(pMultiByteStr), NULL, NULL);
    }
    HeapFree(GetProcessHeap(), 0, pWideCharStr);

    return(fOk);
}
```

### 判断ANSI或UNICODE

```
BOOL IsTextUnicode(CONST PVOID pvBuffer, int cb, PINT pResult);
```

基于统计判断字符集，因此返回的结果不一定是正确的。

## 第三章 内核对象

概念：访问令牌、事件、文件、文件映射、IO、作业、邮件槽、互斥量、管道、进程、线程、信号量、计时器、线程池等对象都属于内核对象

### 基本概念

#### 引用计数

操作系统通过引用计数的方式管理和释放内核对象

#### 内核对象安全性

内核对象可以由安全描述符保护，安全描述符定义了对象创建者，允许访问的拒绝访问的组/用户。

##### 结构

```
typedef struct _SECURITY_ATTRIBUTES{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;    //此成员定义了安全描述符
    BOOL bInheritHandle;            //此成员表示允许对象继承
} SECURITY_ATTRIBUTES;
```

##### 定义与初始化

```
SECURITY_ATTRIBUTE sa;
sa.nLength = sizeof(sa);
sa.lpSecurityDescriptor = pSD;    //为NULL时表示默认安全性
sa.bInheritHandle = FLASE;

HANDLE hFileMapping = CreateFileMapping(INVAILD_HANDLE_VALUE, &sa, 
                                PAGE_READWRITE, 0, 1024, TEXT("MyFileMapping"));
```

此时如果调用OpenFileMapping，在返回有效句柄值前将检查当前用户允不允许访问，返回NULL句柄，使用GetLastError将得到ERROR_ACCRSS_DENIED。

资料：[GDI和用户对象](<https://blogs.msdn.microsoft.com/dsui_team/2013/04/23/debugging-a-gdi-resource-leak/>)

### 进程内核对象句柄表

每个进程都对应一个内核对象句柄表

#### 结构

* 指向内核对象内存块的指针
* 访问掩码
* 标志

#### 创建

调用创建内核对象的函数时，

* 为对象分配并初始化一个内存块
* 扫描内核对象句柄表，找到一个空表项
* 表项的指针指向内核对象内存块，掩码为**拥有完全访问权限**

函数一般返回一个句柄，句柄可供进程中的所有线程使用。

目前来说，创建内核对象返回句柄值是**进程内核对象句柄表的索引**（实际上是索引*4，因为考虑表项长度）

因此句柄是不可以在不同进程中使用的。

一般来说，**调用失败**返回的句柄为**NULL**,但也有例外，如返回-1（**INVALID_HANDLE_VALUE**）

#### 关闭

```
BOOL CloseHandle(HANDLE hobject);
```

* 检查主调进程的句柄表

* 验证传入的句柄值对应表项有无权访问

* 若有效则系统获取内核对象数据结构地址，递减引用计数，且当计数为0时销毁。

* 清除进程句柄表中对应表项（无论内核对象是否被销毁都会执行这步，这说明若内核对象没被销毁则其被其他进程使用）

**注意**：CloseHandle后应将参数的值置为NULL，以防止程序调用以释放的内核对象句柄。特别当该句柄对应表项填充了一个新的相同内核对象，此时bug将难以查找，并将导致程序损坏。

### 跨进程共享内核对象

目的：进程间通信及数据共享

#### 使用对象句柄继承

当两进程为**父子进程**

##### 方法

```
BOOL CreateProcessA(
  LPCSTR                lpApplicationName,
  LPSTR                 lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL                  bInheritHandles,
  DWORD                 dwCreationFlags,
  LPVOID                lpEnvironment,
  LPCSTR                lpCurrentDirectory,
  LPSTARTUPINFOA        lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
);
```

* 在create内核对象时指定**SECURITY_ATTRIBUTE**结构并将其**bInheritHandle**成员设为True

* CreateProcess时将**bInheritHandle**设为True

当CreateProcess时系统除了创建进程外将做如下操作

* 遍历内核对象句柄表

* 若对应表项是**可继承**的，则将其**复制**到子进程内核对象表的**索引相同的表项**

* 内核对象**引用计数+1**

复制完后，父进程应将可用的句柄传给子进程，可以使用 *命令行参数 进程间通信 添加环境变量* 等方式传递

[关于环境变量传递](<https://support.microsoft.com/zh-cn/help/190351/how-to-spawn-console-processes-with-redirected-standard-handles>)

##### 注意

* 内核对象被保存在内核地址空间中，对于32位系统为0x80000000到0xFFFFFFFF，64位为0x00000400 00000000到0xFFFFFFFF FFFFFFFF
* 若子进程同样适用CreateProcess创建孙进程，其同样可以继承父进程内核对象
* 在创建子进程后父进程创建新的内核对象不会被同步到子进程
* 子进程无法获取从父进程继承了哪些内核对象，因此需要由父进程传参

#### 改变句柄标志

使用情形：父进程创建了内核对象，得到一个可继承的句柄，并创建两个子进程，但只希望其中一个继承内核对象

##### 函数

```C
BOOL SetHandleInformation(
    HANDLE hObject,
    DWORD dwMask;
    DWORD dwFlags
);
```

* dwMask  想改变的标志
* dwFlags   标志的新值

```C
BOOL GetHandleInformation(
    HANDLE hObject,
    PDWORD pdwFlags
)
```

##### 方法

每个句柄默认都关联了两个标志

**HANDLE_FLAG_INHERIT**  是否可继承

**HANDLE_FLAG_PROTECT_FROM_CLOSE**  是否有句柄关闭保护

```C
SetHandleInformation(hObj,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT); //开启对象继承
SetHandleInformation(hObj,HANDLE_FLAG_INHERIT,0);    //关闭对象继承

SetHandleInformation(hObj,HANDLE_FLAG_PROTECT_FROM_CLOSE,
                     HANDLE_FLAG_PROTECT_FROM_CLOSE)    //开启句柄关闭保护
                                                    //此时若CloseHandle(hObj)将引发异常
```

第二个方法一般用于防止子进程创建孙进程后CloseHandle，使孙进程获取一个无效的内核对象

但若子进程存在以下代码，孙进程内核对象依然可能无效

```C
SetHandleInformation(hObj,HANDLE_FLAG_PROTECT_FROM_CLOSE,0);
CloseHandle(hObj);
```

#### 为对象命名

可以通过命名内核对象实现内核对象跨进程共享，但**只有Create函数有pszName选项**的内核对象可以命名，若该参数为**NULL**则创建一个**匿名内核对象**

**注意：系统中所有内核对象共享一个命名空间，且无法保证当前要命名的对象名称与已有对象不冲突**，若系统存在同名对象且类型不同，Create函数将失败并设置Errcode为ERROR_INVALID_HANDLE

##### 方法1：Create*函数

假设在ProcessA执行

```C
HANDLE hMutexProcessA = CreateMutex(NULL,FALSE,TEXT("MutexA"));
```

然后在ProcessB执行

```C
HANDLE hMutexProcessB = CreateMutex(NULL,FALSE,TEXT("MutexA"));
```

* 检查命名空间是否存在MutexA

* 存在，检查内核对象类型是否一致

* 类型都为Mutex，检查ProcessB是否有对MutexA的完全访问权限

* 有则在B的句柄表中寻找一个空白表项并填入，否则返回NULL

关于内核对象的权限详见[文档](<https://docs.microsoft.com/zh-cn/windows/desktop/Sync/synchronization-object-security-and-access-rights>)，一般需要使用Create\*Ex函数创建，并指定**dwDesiredAccess**参数

**注意：调用Create\*函数后调用GetLastError()若返回ERROR_ALREADY_EXITSTS，则表明该对象已经存在**

##### 方法2：Open*函数

原型与Create\*类似

差别在于若对象不存在Create\*会创建，而Open\*不会

**GetLastError错误代码：**

* ERROR_FILE_NOT_FOUND  未找到
* ERROR_INVALID_HANDLE  类型或访问权限不同

#### 终端服务命名空间

对于运行终端服务的计算机，其有多个命名空间

* 全局命名空间  主要由服务使用
* 本地命名空间  每个用户端会话有自己的命名空间，以防会话之间的干扰

除服务器外，**远程桌面**和**快速用户切换特性**都由终端服务实现

**注意：无用户登录时，服务从session0启动，用户应用程序则在一个新的会话启动。而由于服务运行在与应用程序不同的会话上，因此要共享对象必须在全局命名空间中创建对象**

具体关于Session 0隔离问题可见[文档](http://www.firmlogix.com/whitepapers/Impact-of-Session0_Vista.doc)

##### 方法

获知当前会话的SessionID

```
DWORD processID = GetCurrentProcessId();
DWORD sessionID;
if(ProcessIdToSessionId(processID,&sessionID))
    tprintf("Process %u runs in Terminal Services session %u\n",processID,sessionID);
```

显式创建全局命名对象或局部命名对象

```C
HANDLE h = CreateEvent(NULL,FALSE,FALSE,TEXT("Global\\Myname"));  //全局命名对象
                                                        //使用Global\\
HANDLE h = CreateEvent(NULL,FALSE,FALSE,TEXT("Local\\Myname"));      //局部命名对象
                                                        //使用Local\\
HANDLE h = CreateEvent(NULL,FALSE,FALSE,TEXT("SessionX\\Myname"));//局部命名对象
                                                        //若Session不是当前的sessionID
                                                        //则报错
```

#### 专有命名空间

虽然内核对象可以传递SECURITY_ATTRIBUTES结构指针，但在vista发布前没有机制防止共享对象名称被劫持，任何权限对象都可创建任意名字的内核对象，这会使程序误以为自身已经有实例在运行，从而导致**DoS攻击**。

为了保护名称免糟劫持，程序可以定义一个专有前缀作为专有命名空间。负责创建对象的服务器进程将定义边界描述符进行保护。