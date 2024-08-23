# PART I: AN OVERVIEW OF THE WINDOWS

## Setting up a powershell testing environment

### Configuring Powershell

这边设置了powershell的执行权限。windows默认为Restricted，该权限禁止所有未签名的ps脚本运行；设置成了RemoteSigned，该权限只禁止了从浏览器或邮件下载的ps脚本运行

```powershell
Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned -Force
```

安装升级相关模块

```powershell
Install-Module NtObjectManager -Scope CurrentUser
Update-Module NtObjectManager
```

导入模块

```powershell
Import-Module NtObjectManager
```

### An Overview of the Powershell Language

#### 变量 类型 表达式

##### 变量

以`$`开头，如`$var`

powershell预定义了一系列变量

```
$null
$pwd
$pid
$env
```

可以用`Get-Variable`获取所有变量

##### 类型

都对应.net下的类型

| Type      | .net Type                    | Example          |
| --------- | ---------------------------- | ---------------- |
| int       | System.Int32                 | 142 0x8e 0216    |
| long      | System.Int64                 | 142L 0x8eL 0216L |
| string    | System.String                | "Hello" ‘World’  |
| double    | System.Double                | 1.0 1e10         |
| bool      | System.Boolean               | $true            |
| array     | System.Object[]              | @(1, "ABC")      |
| hashtable | System.Collections.Hashtable | @{A=1; B="ABC"}  |

其中string可以使用`"`或`'`包裹值，其中`'`是纯字面量，`"`则可以在其中使用转义，如

```powershell
$var = 42
"The Value is $var"
```

输出为 `The Value is 42`此外，双引号还支持转义字符，注意windows使用`\`作为路径分隔符，因此转义以`开头

![](pic/3.png)

![](pic/4.png)

##### 表达式

![](pic/1.png)

![](pic/2.png)

#### 执行命令

一般powershell函数的命名方式都为`[动词]-[名词]`，如`Get-Item`

传参方式有位置参数和关键字参数两种

```powershell
Get-Item C:\Windows
Get-Item -Path C:\Windows
$ret = Get-Item -Path C:\Windows
```

#### 查询指令与获取指令帮助

```powershell
# 查询指令
Get-Command -Name [pattern]
Get-Command -Module [ModuleName] -Name [pattern]

# 获取指令帮助文档
Get-Help [Command]
Get-Help [Command] -Parameter [ParameterName]
Get-Help [Command] -Examples
Get-Help [Command] -ShowWindow        # 会显示GUI帮助窗口

# 创建指令别名
New-Alias -Name [AliasName] -Value [Command]
```

#### 定义函数

```powershell
function Get-NameValue {
    param(
        [string]$Name = "",        # 指定参数类型，非必需
        $Value
    )
    return "$Name = $Value"
}


Get-NameValue -Name "Hello" -Value "World"
Get-NameValue "Hello" "World"
```

若未指定参数，则所有传入的参数会放入`$arg`变量，并使用`$arg[0]`这种方式调用

也可以将语句块赋值给一个变量，并由`&`或`Invoke-Command`调用

```powershell
$script = {Write-Output "Hello"}
& $script                 # 调用方式1
Invoke-Command $script    # 调用方式2
```

#### 对象的显示和处理

若不使用变量获取命令的结果，则会使用默认的formatter来输出对象的内容

可以通过一系列函数来筛选输出

* Select-Object  可以选择对象中的特定项

* Format-List  打印数组

* Format-Table  打印字典

* Get-Member  获取对象的特定成员

* Out-Host  分页显示

* Write-Host  直接写入到命令行，可以控制字体颜色等

* Out-GridView  打开显示字典的GUI

#### 筛选 排序 分组

* 筛选
  
  * Where-Object  提供一系列类似sql的模式进行筛选

* 排序
  
  * Sort-Object

* 分组
  
  * Group-Object

#### 导出数据

* Out-File

* Get-Content

* Export-Csv

* Export-CliXml

* 直接重定向

## The Windows Kernel

![](pic/5.png)

可以根据内核提供的API名来判断当前API是哪个模块提供的

| Prefix  | Subsystem                  | Example                   |
| ------- | -------------------------- | ------------------------- |
| Nt / Zw | System Call Interface      | NtOpenFile ZwOpenFile     |
| Se      | Security Reference Monitor | SeAccessCheck             |
| Ob      | Object Manager             | ObReferenceObjectByHandle |
| Ps      | Process and Thread Manager | PsGetCurrentProcess       |
| Cm      | Configuration Manager      | CmRegisterCallback        |
| Mm      | Memory Manager             | MmMapIoSpace              |
| Io      | Input/Output Manager       | IoCreateFile              |
| Ci      | Code Integrity             | CiValidateFileObject      |

### Security Reference Monitor

提供了一系列控制不同用户访问不同资源的机制

![](pic/6.png)

* Access Token   在进程创建时分配，定义了与该进程关联的用户身份，并由SRM管理

* Security Descriptor  定义了资源的访问权限

* Access Check  当用户进程试图访问资源时会触发该操作，该操作会对比Access Token和Security Descriptor，并判断最终是否可以访问，并且可以产生审计事件（默认关闭，因为相关事件数量太多）

#### SID与SDDL

用户和组都以SID（security identifier）表示，字面的SID在LSASS进程（Local Security Authority Subsystem）中被转换为二进制的SID

微软定义了一套语言 SDDL（Security Descriptor Definition Language）来表示SID，可以使用Get-NtSid来获取用户的SID。具体关于SID的内容在后面章节描述

### The Object Manager

在windows系统中，万物皆对象。许多内核对象都可以分配一个安全描述符来限制用户的权限

对象管理器用来管理这些内核对象，包括内存分配 生命周期等

#### Object Type

内核对象有众多类型，可以使用Get_NtType获取所有类型

#### The Object Manager Namespace

OMNS由Directory内核对象创建，因此结构类似文件目录；每个目录项都含有security descriptor，用于定义哪些用户可以访问以及创建该对象

可以使用`ls NtObject:\` 列出所有的OMNS对象

![](pic/7.png)

其中，Directory表示包含其他目录，SymbolicLink表示软链接到其他文件。可以查看其链接到的路径

```powershell
ls NtObject:\Dfs | Select-Object SymbolicLinkTarget
```

windows系统预定义了一系列OMNS目录

| Path              | 描述                               |
| ----------------- | -------------------------------- |
| \BaseNamedObjects | 用户对象的全局目录                        |
| \Device           | 设备的目录                            |
| \GLOBAL??         | 符号链接的全局目录，包括驱动器映射（drive mapping） |
| \KnownDlls        | knowndll映射                       |
| \ObjectTypes      | 命名对象类型（named object type）        |
| \Sessions         | 控制台会话                            |
| \Windows          | 与窗口管理器相关的对象                      |
| \RPC Control      | RPC端点（Remote Procedure Call）     |

#### System Calls

用户程序通过调用系统API间接在OMNS中创建对应的对象。一般相关的系统API由Nt或Zw开头。对于用户程序来说两者相同，对于内核程序来说，存在的一个区别是Zw前缀会更改安全检查的过程

相关系统调用一般是由 [操作动词][对象] 组成。操作包括

* Create

* Open

* QueryInformation

* SetInformation

##### System call中的安全属性

以NtCreateMutex为例

```c
NTSTATUS NtCreateMutant (
    HANDLE* FileHandle,
    ACCESS_MASK DesiredAccess,
    OBJECT_ATTRIBUTES* ObjectAttributes,
    BOOLEAN InitialOwner
);
```

其中第三个参数定义了对象的一些安全属性，OBJECT_ATTRIBUTES结构定义如下

```c
typedef struct _OBJECT_ATTRIBUTES {
  ULONG           Length;
  HANDLE          RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG           Attributes;
  PVOID           SecurityDescriptor;
  PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES;
```

* Length  长度，用于检查

* RootDirectory  是一个打开的内核对象句柄，该句柄用于查找ObjectName

* ObjectName  一个Unicode字符串，与RootDirectory共同组成内核对象路径
  
  ```
  如 \BaseNamedObjects\ABC，可以是
  RootDirectory = "\BaseNamedObjects", ObjectName = "ABC" 或
  RootDirectory = "", ObjectName = "\BaseNamedObjects\ABC"
  ```

* Attributes  对象的属性
  
  [_OBJECT_ATTRIBUTES (ntdef.h) - Win32 apps | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/win32/api/ntdef/ns-ntdef-_object_attributes)
  
  | Powershell Name             | 描述                                                                     |
  | --------------------------- | ---------------------------------------------------------------------- |
  | Inherit                     | 句柄可由当前进程的子进程继承                                                         |
  | Permanent                   | 仅适用于在对象管理器中命名的对象。若不指定该标志，关闭所有打开的句柄时会自动析构该内核对象；若指定该标志则不会。使用该标志可以永久化一个对象 |
  | Exclusive                   | 以独占方式访问对象                                                              |
  | CaseInsensitive             | 查找内核对象时忽略大小写                                                           |
  | OpenIf                      | 若存在对应内核对象，调用Create类API时优先打开现有对象                                        |
  | OpenLink                    | 仅被Configuration Manager使用。若当前对象是一个链接，则打开链接对象本身（而非打开链接引用的对象）            |
  | KernelHandle                | 句柄在系统进程上下文创建，只能在内核模式访问                                                 |
  | ForceAccessCheck            | 打开句柄时强制进行AccessCheck                                                   |
  | IgnoreImpersonatedDeviceMap | 解析DOS名称时使用，设备映射是DOS设备名与系统中设备间的映射                                       |
  | DontReparse                 | 分析关联对象的名称时不会遵循任何重新分析点                                                  |

```c
struct UNICODE_STRING {
    USHORT Length;            // 当前字符串长度
    USHORT MaximumLength;     // 总长度
    WCHAR* Buffer;
};
```

#### NTSTATUS Codes

![](pic/8.png)

* Severity  表示返回值的严重性，若以32位有符号整数解析，错误相关的最高位都为1，因此相应的值为负数
  
  * STATUS_SEVERITY_SUCCESS  0
  
  * STATUS_SEVERITY_INFORMATIONAL  1
  
  * STATUS_SEVERITY_WARNING 2
  
  * STATUS_SEVERITY_ERROR 3

* CC  表示当前值是否由microsoft定义，为1表示是第三方自行定义的

* R  保留位，必须为0

* Facility  与错误相关的设备或子系统，微软预定义了50个相关的值
  
  * FACILITY_DEFAULT  0
  
  * FACILITY_DEBUGGER  1
  
  * FACILITY_NTWIN32  7

* Status Code  状态码

可以使用`Get-NtStatus`获取当前定义的状态码

#### Object Handles

![](pic/9.png)

object manager用于管理内核对象，内核对象一般以句柄的形式返回给用户空间来标识对象，而在API接收到句柄参数时，一般需要先进行索引来找到句柄对应的内核对象（ObReferenceObjectByHandle）

在索引时，会运行两个检查

* 检查当前的用户是否有权限获取相应的对象

* 检查当前句柄指向的对象是否与API请求的对象类型一致

#### Access Masks

句柄表中存放了一个access mask，用于标识相应内核对象的访问权限。这个结构跟API中的DesiredAccess参数是一致的

![](pic/10.png)

* Generic access  当使用DesiredAccess参数调用系统调用时会用到，包含四个访问权限
  
  * GenericRead
  
  * GenericWrite
  
  * GenericExecute
  
  * GenericAll

* Special access  大部分是保留的，但有下述特殊的属性
  
  * AccessSystemSecurity  读写对象的审计信息
  
  * MaximumAllowed  当进行access check的时候请求对象的最大权限

* Standard access  定义了标准的访问权限，包含
  
  * Delete  删除
  
  * ReadControl  读取对象的安全标识符
  
  * WriteDac  读取对象安全标识符的DAC
  
  * WriteOwner  写入对象的owner信息
  
  * Synchronize  等待内核对象，如该对象是个锁的情况

* Type-specific access  为不同的内核对象定义了相应的权限

注意，Access Mask对于用户是不可见的，内核对象都预定义了几种访问控制符号，这些符号对应了特定的Access Mask

```powershell
Get-NtType | Select-Object Name, GenericMapping
```

![](pic/11.png)

其中，R 读取  W 写入  E 执行  A 追加

书的84页进一步描述了怎么在powershell工具中获取不同对象预定义的Access Type，如何获取在SDK中对应的名字，以及如何查找名字对应的值

在WinAPI中，可以使用NtQuerySystemInformation获取所有handle，powershell中可以使用Get-NtHandle，书的88页说明了怎么使用powershell管理句柄

当内核对象不再被引用，其内存会被内核回收，若OMNS中存在相关内容也会被删除，但内核也允许创建永久对象，如在创建对象时指定Permanent或使用NtMakePermanentObject创建对象，此时需要有SeCreatePermanentPrivilege

在所有的内核对象中，File和Key是永久性存储的，因此其默认拥有永久的名字（路径），且不被OMNS管理。要删除的话必须显式使用相应的系统调用

#### Handle Duplication

可以使用NtDuplicateObject复制句柄，一般用于以下几个场景

* 增加句柄引用计数

* 将句柄传入其他进程

* 将权限受限的句柄传入其他进程，如本进程有读写权限，而传入其他进程的是只有只读权限的句柄。但不该依赖这种方法进行授权，因为若其他进程有权限访问这个资源，则它可以直接重新打开这个句柄对应的资源来获取写权限

第三种用法也说明同一个句柄在不同进程中可以有不同的Access Mask

在创建句柄时采用Inherit属性也类似句柄复制，可以使得子进程继承该句柄

此外若指定了ProtectFromClose，则进程中所有试图关闭句柄的操作都会失败

#### Query and Set Information System Calls

```c
NTSTATUS NtQueryInformationProcess(
    HANDLE Handle,
    PROCESS_INFORMATION_CLASS InformationClass,
    PVOID Information,
    ULONG InformationLength,
    PULONG ReturnLength
);

NTSTATUS NtSetInformationProcess(
    HANDLE Handle,
    PROCESS_INFORMATION_CLASS InformationClass,
    PVOID Information,
    ULONG InformationLength,
);
```

书94页说明了如何使用powershell调用这两个函数

### The Input/Output Manager

IO管理器主要就是管理与文件交互的驱动，而其他设备驱动本质上也都是文件驱动

一个driver（驱动）可以使用IoCreateDevice创建Device。一个驱动可以有0个（若不需要交互）到多个device

驱动都挂载在Device目录下，可以在powershell中使用下列指令列出驱动

```powershell
ls NtObject:\Device
```

实际上文件系统也是挂载在该目录下的，如notepad.exe实际上路径为

```
\Device\HarddiskVolume3\Windows\notepad.exe
```

其中`\Device\HarddiskVolume3`对应了文件系统的驱动，因此后面的路径`Windows\notepad.exe`作为参数被传给文件系统驱动

windows并不把网络协议栈作为系统调用实现，而是使用了一套称为AFD（Ancillary Function Driver）的驱动

### The Process and Thread Manager

进程和线程只能通过PID和TID进行打开和访问，而不能直接通过名字

PID为0的进程为Idle，是计算机空闲时运行的等待进程；PID为4的是System进程，该进程运行在内核态，当驱动或内核创建一个后台线程时就挂在该进程下

进程和线程的安全状态是分离的：即使没有权限访问进程，也有可能访问进程下对应的线程

### The Memory Manager

#### NtVirtualMemory Commands

基本内存操作及其需要的权限

| 操作     | 权限                      | 说明                      |
| ------ | ----------------------- | ----------------------- |
| 获取内存信息 | QueryLimitedInformation | NtQueryVirtualMemory    |
| 读取内存   | VmRead                  | NtReadVirtualMemory     |
| 写入内存   | VmWrite                 | NtWriteVirtualMemory    |
| 分配内存   | VmOpertion              | NtAllocateVirtualMemory |
| 释放内存   | VmOpertion              | NtFreeVirtualMemory     |
| 改变内存属性 | VmOpertion              | NtProtectVirtualMemory  |

内核中的内存页有三种状态：

* Commit  可用

* Reserve  保留

* Free  已释放

其中使用Reserve和Free内存都会导致crash，Reserve与Free的区别在于，Reserve相当于占了内存的位，之后还可以将该区域变为Commit

#### Section Objects

该对象是一种虚拟内存对象，一般用于

* 文件内存映射

* 共享内存

关键函数：NtMapViewOfSection

若在创建Section时指定OMNS路径，则可以被其他进程访问并用于共享内存，否则为匿名Section

Section的权限包含两个部分：创建Section时指定的权限与创建Section的内存映射时指定的权限，映射时的权限小于等于创建Section时的权限

此外，若知道其他进程的PID，可以将内存映射到这些进程

内存映射有三种类型

* Mapped  指定了文件的映射

* Anonymous  匿名映射

* Image  可执行文件的段映射

### Code Integrity

Windows提供了该子系统专门用于代码完整性校验（验签等）

一般签名有两种提供方式：嵌入在文件中或使用Catalog文件（.cat）

### Advanced Local Procedure Call

ALPC子系统提供了一套单独的LPC机制，为S/C架构，使用NtCreateAlpcPort创建服务端，NtConnectAlpcPort创建客户端连接

### The Configuration Manager

在OMNS为`\REGISTRY`

```powershell
ls NtObject:\REGISTRY
```

## User-Mode Applications

### Win32 and the User-Mode Windows APIS

![](pic/12.png)

#### Searching for DLLs

在NT3.1中，查找顺序如下

* 可执行文件目录

* 当前工作目录

* System32

* Windows

* 环境变量

为了防止DLL劫持，在Vista改成了

* 可执行文件目录

* System32

* Windows

* 当前工作目录

* 环境变量

注意，可执行文件目录仍可能被劫持，因此对于特权进程，应该确保其目录只有特权用户可以写入。此外，还有一个潜在的安全问题：若传入LoadLibrary的名字不带`.dll`，函数会自动加上，如果传入的名字带了一个`.`，函数只会把`.`去掉；这里假设主程序在加载前校验了dll，但未加上扩展名（如LIB），而文件夹下存在`LIB.dll`，则会导致最终加载的文件与校验的文件不符

为了加快程序加载，内核在OMNS中为常用系统库设置了一个section `KnownDlls`，该段的dll都是加载在共享内存中的，由此也避免了系统库被劫持的问题

### The Win32 GUI

![](pic/13.png)

注意，这里GUI主要是由Win32K的驱动实现的。win10的时候kernel中的system call分发的代码嵌入到Win32U中以提高性能

此外，GUI API还需要跟特权进程CSRSSS交互（Client Server Runtime Subsystem）

#### GUI Kernel Resources

* Windows station  用于表示连接的屏幕与用户接口，比如鼠标键盘

* Window  与用户交互的GUI元素

* Desktop  用于表示桌面的对象

* Drawing resource  具体的GUI控件对象

在程序开始运行时，每个Process都会被分配一个Windows station（或使用NtUserSetProcessWindowStation指定），每个Thread都会被分配一个Desktop（或使用NtUserSetThreadDesktop）。一般来说所有的应用程序都在同一个WorkStation和Desktop下

Window则表示了一个应用程序的GUI单元，每个Desktop上有多个window，所有的窗口消息都在一个window中进行交互。window有被称为线程亲和性的特性，即只有创建窗口的线程可以处理消息，其他线程只能发送消息

#### Console Sessions

当用户登录时，Session Manager会为用户新建一个Console Session，该session会用于组织用户的Windows Station和Desktop对象。内核会在OMNS路径中创建该Session对象。Session对象由一个数字标记，从0开始

Session Manager在还未有用户登录时就会启动若干个进程，如CSRSS和WinLogon，来显示登录界面以及认证用户凭据

一般来说Windows只能拥有一个物理控制台（physical console），即连接了键盘鼠标的控制台；但通过RDP可以连接其他session。此外也可以切换物理控制台的登录用户，切换时先前用户的程序仍会在后台执行

每个Console Session都有独立的内存空间，因此不同Console Session可能存在多个同样的资源。

Session 0是给特权服务和系统管理使用的，因此该session一般不可能使用GUI

> SHATTER ATTACK
> 
> Vista前，服务和物理控制台都在session 0上，由于同一个session下的进程都可以向其他进程发送窗口消息，因此会导致称为SHATTER ATTACK的攻击：低特权的程序可以向高特权级程序发送窗口消息，如发送WM_TIMER可能触发高特权级进程调用消息提供的回调实现提权
> 
> Vista采用两种方法解决该问题：
> 
> * 将物理控制台从Session 0移除，使得用户程序无法直接与特权级程序交互
> 
> * 用户接口特权隔离（User Interface Privilege Isolation, UIPI），阻止低特权级程序与直接与高特权级程序交互

Console Session还有一个重要特性：当多个用户同时登录时，必会导致名字冲突。因此Windows给每个Session创建了单独的BNO `\Sessions\<N>\BaseNamedObjects`和单独的Window `\Session\<N>\Windows`，其中`<N>`为session id。注意session 0没有单独的BNO，它用的是全局BNO

### Comparing Win32 APIs and System Calls

win32 API很多时候并不会直接wrap一个系统调用，而是会有一些区别

```c
HANDLE CreateMutexEx( 
    SECURITY_ATTRIBUTES* lpMutexAttributes, 
    const WCHAR*         lpName, 
    DWORD                dwFlags, 
    DWORD                dwDesiredAccess
);

NTSTATUS NtCreateMutant( 
    HANDLE*              MutantHandle, 
    ACCESS_MASK          DesiredAccess, 
    OBJECT_ATTRIBUTES*   ObjectAttributes, 
    BOOLEAN              InitialOwner 
); 
```

* Win32 API返回NTSTATUS，CreateMutexEx则直接返回handle。关于错误传播，内核函数通过NTSTATUS传播，可以调用`RtlNtStatusToDosError`，而Win32 API通过一个全局变量传播，可以使用`GetLastError`获取错误码

* Win32 API不接收OBJECT_ATTRIBUTES参数，而是将OBJECT_ATTRIBUTES的相关内容分成了两个参数进行接收
  
  * lpMutexAttributes  一个指向SECURITY_ATTRIBUTES
    
    ```c
    struct SECURITY_ATTRIBUTES { 
        DWORD  nLength; 
        VOID*  lpSecurityDescriptor; 
        BOOL   bInheritHandle; 
    }; 
    ```
  
  * lpName  对象名

* Win32 API中，名字参数lpName不是一个完整的OMNS路径，而是会自动地被放置到对应BNO文件夹下。如指定名字为`ABC`，则会放到`\Sessions\<N>\BaseNamedObjects\ABC`文件夹下。若想创建全局对象，则可以使用`Global\ABC`

* dwDesiredAccess与内核的DesiredAcce是直接映射，InitialOwner则由dwflags的一个枚举指定

#### Win32 Registry Paths

几个常用主键的OMNS路径

| 主键名                 | OMNS路径                                                                     |
| ------------------- | -------------------------------------------------------------------------- |
| HKEY_LOCAL_MACHINE  | \REGISTRY\MACHINE                                                          |
| HKEY_USERS          | \REGISTRY\USER                                                             |
| HKEY_CURRENT_CONFIG | \REGISTRY\MACHINE\SYSTEM\CurrentControlSet\Hardware Profiles\Current       |
| HKEY_CURRENT_USER   | \REGISTRY\USER\<SDDL SID>                                                  |
| HKEY_CLASSES_ROOT   | \REGISTRY\MACHINE\SOFTWARE\Classes  <br/>\REGISTRY\USER\<SDDL SID>_Classes |

> 注意：Win32API使用NUL结尾的字符串作为参数，而内核函数使用标记了长度的字符串作为参数，因此内核函数的路径可以包含NUL，这可能导致Win32API访问不到对应注册表，而内核函数可以情况，导致安全问题

#### DOS Devoce Paths

内核函数调用的是NT path，而Win32 API使用的是DOS path（即带盘符的路径）。NTDLL使用RtlDosPathNameToNtPathName做转换，例如`C:\Windows`会被转换成`\??\C:\Windows`，其中`\??`前缀称为DOS设备映射前缀（DOS device map prefix），它用于通知对象管理器查找盘符应分为两步：

* 查找用户的DOS设备映射目录 `Sessions\0\DosDevices\<AUTHID>`，其中AUTHID与session的token有关。注意这里所有的DOS设备映射都会放在session 0的目录下

* 若上一步没有找到，则会检查`\Global??`目录

由于会优先查找用户DOS目录，因此可以在这边创建磁盘的映射，且该映射可以将磁盘映射到某个目录下

##### Path Types

下面是几种路径的类型

![](pic/14.png)

由于DOS路径存在很多种不同写法，因此必须通过一个规范化（canonicalize）的过程进行转换。

> 像Linux这样的系统规范化主要在内核里做，但Windows将该过程前置到用户空间。这是因为Windows的subsystem设计初衷是可以兼容POSIX路径等，因此可以通过前置该规范化过程使得用户态的subsystem dll就可以处理这些路径问题

规范化包含下列处理过程：

* 处理`/`和`\`  Native路径只使用`\`作为分隔符，而DOS路径两者皆可，因此转换的一个主要操作就是将所有的正斜杠和反斜杠都转换为反斜杠

* 处理`.`和`..`  规范化过程会将所有的`.`删除，将`..`转换为上一级目录

* 若DOS路径以`\\?\`或`\??\`开头，则不会经过规范化过程，而是把其当作Native路径

> 文章提到在一些情况下`\??\`的写法可能会对Win32 API造成混淆，比如将其当作根驱动器路径下的文件而打开诸如`\??\C:\??\Path`的路径

默认状态下，DOS路径最长只能有260个字符，而Native路径长度为32767。但该限制可以通过一个注册表项修改，此外还需要程序的manifest文件支持LongPathAware选项，否则很多软件在编写时默认将260作为路径缓冲区的长度

### Process Creation

#### Command Line Parsing

最简单的一种创建进程的方式是将命令行传入CreateProcess

当传入命令行`notepad test.txt`时，操作如下：

* 按空格分割命令行字串（除非被双引号括起来），以第一个元素作为可执行文件名，若没有.exe后缀则加上

* 搜索可执行文件，在命令行中搜索可执行文件的顺序为：
  
  * 当前进程可执行文件所在目录
  
  * 当前工作目录
  
  * System32
  
  * Windows
  
  * 环境变量

* 若无法找到notepad.exe，则会将`"notepad test.txt"`整体作为文件进行查找，重新运行一遍上面查找可执行文件的逻辑。这里由于已经有`.txt`后缀，不会补上exe后缀。若notepad.exe被双引号括住则不会执行这一步

> 注意，这边可能有两类安全问题：
> 
> * 与DLL劫持类似的路径搜索问题
> 
> * 若第一个元素包含路径分隔符且无双引号，则会有一些额外的操作，如`C:\Program Files\abc.exe`（注意这里包含了一个空格）。程序会依次尝试搜索下列的文件
>   
>   * `C:\Program`
>   
>   * `C:\Program.exe`
>   
>   * `C:\Program Files\abc.exe`
>   
>   * `C:\Program Files\abc.exe.exe`
> 
> 这类安全问题的缓解方式为：在调用CreateProcess时对ApplicationName参数指定一个可执行文件路径

#### Shell APIs

```c
HINSTANCE ShellExecuteA(
  [in, optional] HWND   hwnd,
  [in, optional] LPCSTR lpOperation,
  [in]           LPCSTR lpFile,
  [in, optional] LPCSTR lpParameters,
  [in, optional] LPCSTR lpDirectory,
  [in]           INT    nShowCmd
);

BOOL ShellExecuteExA(
  [in, out] SHELLEXECUTEINFOA *pExecInfo
);

BOOL ShellExecuteExW(
  [in, out] SHELLEXECUTEINFOW *pExecInfo
);
```

使用ShellExecute函数与CreateProcess的最大区别在于，前者会为不同后缀名的文件查找其对应的处理软件，如传入某个txt文件，ShellExecute会拉起默认文本处理软件打开，而CreateProcess只会报文件不是可执行文件的错

后缀名对应的默认处理软件保存在注册表的HKEY_CLASSES_ROOT键中，ShellExecute执行时会搜索该注册表中对应的键。注册表中定义了多种行为，对应了对文件不同的操作，这些操作也可以作为参数传入ShellExecute

| 操作        | 描述                  |
| --------- | ------------------- |
| open      | 打开文件，是双击默认行为        |
| edit      | 编辑文件                |
| print     | 打印文件                |
| printto   | 使用指定的打印机打印          |
| explore   | 在文件管理器打开文件          |
| runas     | 以管理员身份打开，只适用于可执行文件  |
| runasuser | 以其他用户身份打开，只适用于可执行文件 |

### System Process

相当一部分系统服务是在用户空间运行的，这是因为内核代码难以编写，且出问题的话会导致crash

#### The Session Manager

Session Manager Subsystem（SMSS）是第一个用户态进程，其在系统启动时就开始运行，作用包含

* 加载known DLLs，创建section对象

* 启动其他子系统进程，如CSRSS

* 初始化基础的DOS设备，如串口

* 自动运行磁盘完整性校验

#### The Windows Logon Process

* 设置新的console session

* 显示用户登录界面

* 运行用户态字体驱动（user-mode font driver, UMFD）

* 运行桌面窗口管理器（desktop window manager, DWM）

#### The Local Security Authority Subsystem

LSASS，用于管理用户登录与认证

#### The Service Control Manaer

SCM，用于管理系统的服务。其中下列服务是一些系统关键服务

* Remote Procedure Call Subsystem（RPCSS）  用于管理 注册RPC端点，将RPC端口暴露到本地或网络

* DCOM Server Process Launcher  是RPCSS的一个关联项，曾经是RPCSS的一部分，用于启动本地以及远程的COM服务进程

* Task Scheduler  用于管理计划任务

* Windows Installer  用于安装程序和新特性

* Windows Update  用于升级

* Application Information  用于UAC控制（User Account Control）

# PART II: THE WINDOWS SECURITY REFERENCE MONITOR

ref: 

* [【windows 访问控制】一、访问令牌 - 小林野夫 - 博客园 (cnblogs.com)](https://www.cnblogs.com/cdaniu/p/15630161.html)

* [Windows 访问控制模型（一） | MYZXCG](https://myzxcg.com/2021/08/Windows-%E8%AE%BF%E9%97%AE%E6%8E%A7%E5%88%B6%E6%A8%A1%E5%9E%8B%E4%B8%80/)

## SECURITY ACCESS TOKENS

### Primary Tokens

每个进程都会分配一个primary token（主令牌），该令牌用于描述进程的权限。当SRM进行access check时使用该token进行比较

可以使用`NtOpenProcessToken`获取当前进程的token，该操作需要QueryLimitedInformation权限

打开一个token对象后，可以请求下列权限

* AssignPrimary  将该token当作当前进程的primary token

* Duplicate  复制token对象

* Impersonate  模拟token对象

* Query  查询token的属性

* QuerySource  查询token对象的源（source）

* AdjustPrivileges  调整token对象的权限列表

* AdjustGroups  调整token对象的组列表

* AdjustDefault  调整未被其他权限覆盖的token对象属性

* AdjustSessionId  调整token对象的session ID

可以使用

```powershell
Show-NtToken -All
```

列出所有Token

Token包含一些重要属性

![](pic/15.png)

* User SID  用于标识用户，token中只保存了SID，上图中通过SID索引了用户名

* Token ID  Token对象的唯一标识符

* Authentication ID  标识了Token所属的logon session

* Origin Login ID  父logon session的身份验证标识符

* Modified ID  当Token值被修改时，该ID会变化

* Integrity Level  完整性级别，用于实现强制访问控制机制

* session ID  创建进程的session ID

当用户登录时，LSASS会为用户创建一个logon session（登录会话），该session会跟踪所有与该用户认证相关的资源（比如它会保存一份用户的credential）。logon session在创建时会分配一个独立的ID，这个ID就是每个进程中的Authentication ID，因此某个用户所有的进程都使用同一个ID（若某个用户在同一台机器上认证了两次，则SRM会分配新的Authentication ID）

origin login ID标明了哪个logon session创建了token，若在计算机上登录了另一个账号，则该属性将用于调用该token的身份验证标识符（意思应该就是登录了另一个账号会更换session，但仍然可以使用这个域来调用该token）

SRM预定义了四个Authentication ID

| Authentication ID | User SID | Logon session username       |
| ----------------- | -------- | ---------------------------- |
| 00000000-000003E4 | S-1-5-20 | NT AUTHORITY\NETWORK SERVICE |
| 00000000-000003E5 | S-1-5-19 | NT AUTHORITY\LOCAL SERVICE   |
| 00000000-000003E6 | S-1-5-7  | NT AUTHORITY\ANONYMOUS LOGON |
| 00000000-000003E7 | S-1-5-18 | NT AUTHORITY\SYSTEM          |

> **LOCALLY UNIQUE IDENTIFIERS (LUID)**
> 
> 所有的token ID都是64位唯一值，可以调用NtAllocateLocallyUniqueId获取此类ID

### Impersonation Tokens

Impersonation Token（模拟令牌），对于系统服务来说很重要，因为它允许一个进程暂时模拟其他令牌来获取对资源的访问权限

模拟token被分配到线程粒度而非进程，一共有三种方法为一个线程分配模拟令牌

* 显式获取一个令牌对象，赋予Impersonate权限并使用SetThreadToken为该线程设置token

* 显式从一个含有DirectImpersonation权限的线程对象获取模拟令牌对象

* 隐式模拟一个RPC请求

第三种是最常见的形式，一个例子是，某个服务创建了一共命名管道服务器，程序就可以模拟一个客户端通过ImpersonateNamedPipe连接到该服务；当客户端在命名管道发出一个请求，内核会根据要调用的线程和进程捕获一个模拟上下文（Impersonation context），该模拟上下文会将模拟令牌分配给调用ImpersonateNamedPipe的线程。模拟上下文的内容可以基于线程上已有的模拟令牌，也可以是基于进程的主令牌

##### Security Quality of Service

相关文档 [客户端/服务器访问控制 - Win32 apps | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/win32/secauthz/client-server-access-control)

SQoS，用于控制其他服务是否能够模拟当前线程的令牌，场景主要是服务端为了安全，希望使用客户端的安全上下文而不是服务端本身的安全上下文来访问资源

当打开命名管道服务器 DDE服务器等，可以在OBJECT_ATTRIBUTES结构的SecurityQualityOfService域传入SECURITY_QUALITY_OF_SERVICE结构来指定一些访问控制属性，该结构包含3类属性

* 模拟等级（Impersonation Level），用于控制当前线程令牌是否可以被模拟
  
  * Anonymous  服务无法模拟或标识客户端
    
    最小的访问权限，只允许有限的几个服务访问线程令牌，其他服务打开或查询令牌的操作都会被拒绝
  
  * Identification  服务可以获取客户端的标识和特权，但不能模拟客户端
    
    允许服务打开 查找令牌 用户ID 用户组和权限，但模拟令牌无法访问任何安全资源
  
  * Impersonation  服务可以在本地系统上模拟客户端的安全上下文
    
    允许服务打开和操作用户的令牌，可以访问本地用户的所有远程资源，但若用户是一个远程认证的用户，则无法访问其资源（如SMB）
  
  * Delegation  服务可以在远程系统上模拟客户端的安全上下文
    
    最高权限，可以访问远程认证用户的资源（但还需要远程服务器有相应配置）

* 上下文跟踪模式（Context Tracking Mode）
  
  开启该模式时，服务会静态地获取用户的令牌（即调用者进程的主令牌），否则动态获取，在动态获取的情况下调用者可以传入一个模拟令牌（只有在Impersonation或Delegation等级下才能传入）

* 有效令牌模式（effective token mode）
  
  若不启用，服务可以在其中更改传入令牌的权限和组，之后再应用这个令牌

> 模拟等级为Anonymous与使用ANONYMOUS LOGON令牌登录不一样，前者无论资源的安全等级设置为上面都无法通过access check
> 
> Kernel实现了一个函数NtImpersonateAnonymousToken使程序可以获取ANONYMOUS用户的token

默认情况下若打开IPC时不设置SQoS结构，调用者默认以Impersonation、上下文跟踪开启、有效令牌模式关闭 的方式启动。当一个线程尝试获取另一个线程的模拟上下文时，该线程的模拟等级必须大于等于被捕获线程的模拟等级，这可以防止一个低模拟等级的用户伪装成一个高模拟等级的用户调用RPC

```powershell
# 获取当前线程主令牌
$token = Get-NtToken
# 创建模拟等级Impersonation的模拟令牌，并试图获取OMNS根目录
# 该操作可以成功
Invoke-NtToken $token {
    Get-NtDirectory -Path "\"
} -ImpersonationLevel Impersonation

# 以Identification等级访问，该操作会失败
Invoke-NtToken $token {
    Get-NtDirectory -Path "\"
} -ImpersonationLevel Identification
```

##### Explicit Token Impersonation

有两种显式获取模拟令牌的方法

* 若当前能获取一个含有Impersonate权限的模拟令牌，则可以调用NtSetInformationThread将该令牌赋予给某个线程

* 若要模拟的线程有Direct Impersonation权限，则可以调用NtImpersonateThread将该线程的模拟令牌分配给其他线程。该函数调用后内核会获取被模拟线程的模拟上下文执行目标线程，就像隐式模拟中通过命名管道进行调用一样

### Converting Between Token Type

可以通过NtDuplicateToken复制（duplication）的方式在两种token类型（即主令牌和模拟令牌）间转换，该操作会对token进行深拷贝，区别于线程句柄的复制操作（该操作只引用先前的token）。复制token需要Duplicate权限

```powershell
# 将一个主令牌以Delegation模拟等级复制为一个模拟令牌
$imp_token = Copy-NtToken -Token $token -ImpersonationLevel Delegation

# 将该模拟令牌复制为主令牌
$pri_token = Copy-NtToken -Token $imp_token -Primary
```

复制操作与模拟等级相关，无法复制一个Anonymous或Identification模拟等级的令牌，这样可以防止程序通过将一个低模拟等级的模拟令牌复制为主令牌来绕过SQoS检查

```powershell
# 将一个主令牌以Identification模拟等级复制为一个模拟令牌
$imp_token = Copy-NtToken -Token $token -ImpersonationLevel Identification

# 下面操作会报错
$pri_token = Copy-NtToken -Token $imp_token -Primary
```

### Pseudo Token Handles

token有三种伪句柄（pseudo handle），以此让程序可以在不打开自身token句柄的情况下查询token属性，括号里是句柄值。该特性主要是方便进程或线程查询自身的token属性

* Primary (-4)  当前进程主令牌

* Impersonation (-5)  当前线程模拟令牌，若当前线程没有进行模拟则会返回错误

* Effective (-6)  若当前线程在模拟，则返回模拟令牌，否则返回主令牌

```powershell
# 使用模拟等级为Anonymous的令牌以伪句柄方式获取主令牌。该操作将返回正常的用户
Invoke-NtToken -Anonymous {Get-NtToken -Pseudo -Primary | Get-NtTokenSid}

# 使用模拟等级为Anonymous的令牌以伪句柄方式获取模拟令牌。该操作将返回NT ANONYMOUS
Invoke-NtToken -Anonymous {Get-NtToken -Pseudo -Impersonation | Get-NtTokenSid}
```

### Token Groups

ref:

* [TOKEN_GROUPS (winnt.h) - Win32 apps | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/win32/api/winnt/ns-winnt-token_groups)

若管理员需要为每一个用户设置可以访问的资源，则一般会使用分组的功能进行管理

从SRM的视角来看，一个组也由一个SID标识，并可以定义对各个资源访问权限

令牌组定义如下

```c
typedef struct _TOKEN_GROUPS {
  DWORD              GroupCount;
#if ...
  SID_AND_ATTRIBUTES *Groups[];
#else
  SID_AND_ATTRIBUTES Groups[ANYSIZE_ARRAY];
#endif
} TOKEN_GROUPS, *PTOKEN_GROUPS;
```

SID_AND_ATTRIBUTES结构包含一个SID和对应的属性，属性描述了对应SID的特性，包括

* Enabled EnabledByDefault  该组SID启用（默认启用）
  
  当设置Enabled时，表示令牌组SID被启用，当对token进行access check时SRM会考虑令牌组权限。拥有EnabledByDefault属性的令牌组SID默认被启用

* Mandatory  无法禁用该组SID
  
  当拥有AdjustGroups权限时，可以通过NtAdjuestGroupsToken禁用令牌组SID；但无法禁用包含Mandatory属性的令牌组SID。所有正常的用户令牌组都设置了该属性，但一些系统令牌组没有设置该属性
  
  在传递模拟令牌时，若某个令牌组失能，且传递模拟令牌时没有打开[有效令牌模式](#Security Quality of Service)，则在传递的令牌中会删除这些失能令牌组的信息

* LogonId  SID是Logon SID
  
  标记Logon SID。例如使用runas作为不同用户运行一个进程时，新进程的token中Logon ID项与调用runas的进程是相同的，这个行为使得SRM可以授予该进程某个会话特定资源的访问权限

* Owner  SID是令牌的所有者，或可以将SID分配为令牌所有者
  
  系统上所有的安全资源都有一个所属的用户SID或组SID，token在创建资源时会默认将拥有该属性的SID设为owner

* UseForDenyOnly  SID是受限令牌中的仅拒绝SID
  
  考虑下列情况，一个文档希望A组的用户可以访问，B组的不能访问，如果一个用户c既属于A又属于B，那么它可以禁用自己的B组属性来访问文档，而若为该用户添加UseForDenyOnly属性可以防止其访问文档，从而解决这个问题

* Integrity IntegrityEnabled  SID是强制完整性SID / 强制完整性启用SID
  
  该SID用于标识token的完整性等级（integrity level）。系统预定义了7个默认的完整性等级
  
  ![](pic/16.png)
  
  ![](pic/17.png)

* Resource 表示组SID是一个domain local SID

token还可以拥有设备组（device groups），当用户登录远程服务器或资源时这些SID被加入到组中

### Privileges

组是系统管理员用来控制用户访问资源权限的工具，而特权则是让用户可以暂时通过安全检查来访问大多数资源的方法

![](pic/18.png)

当拥有AdjustPrivileges权限时，可以使用NtAdjustPrivileges修改token权限

在用户程序中，使用NtPrivilegeCheck检查特权，内核程序中则使用SePrivilegeCheck

下列是一些系统可用的特权

* SeChangeNotifyPrivilege  允许用户接收文件/注册表变化的通知，此外可以用于通过traversal检查

* SeAssignPrimaryTokenPrivilege / SeImpersonatePrivilege  允许用户直接通过对主令牌 / 模拟令牌赋值的检查。该权限必须在当前进程主令牌上启用才生效

* SeBackupPrivilege / SeRestorePrivilege  允许用户在打开注册表/文件时直接通过检查，主要用于备份/恢复程序，避免操作时无法获取对应文件权限

* SeSecurityPrivilege / SeAuditPrivilege  前者允许用户获取AccessSystemSecurity权限，该权限可以修改资源的审计配置；后者则允许用户程序产生审计事件

* SeCreateTokenPrivilege  该权限允许用户使用NtCreateToken生成任意令牌

* SeDebugPrivilege  允许用户在打开一个进程或线程对象时直接通过检查

* SeTcbPrivilege  包含了一系列内核相关操作

* SeLoadDriverPrivilege  允许程序通过NtLoadDriver加载驱动（该特权无法绕过诸如驱动签名检查等机制）

* SeTakeOwnershipPrivilege / SeRelabelPrivilege  首先该特权允许用户获取资源的WriteOwner权限，其中SeTakeOwnershipPrivilege允许用户成为资源的owner，SeRelabelPrivilege可以直接通过对资源的强制标签（mandatory label）检查

### Sandbox Tokens

windows通过三类token提供了一套沙箱机制来限制进程对资源的访问

#### Restricted Tokens

使用NtFilterTolen或CreateRestrictedToken创建，上述API生成一系列指定的限制令牌来限制对资源的访问。在进行access check的时候，该过程不仅可以接收正常的SID组，也可以接收限制令牌列表

NtFilterToken函数还可以将进程的SID组转为UseForDenyOnly属性，且删除其特权。可以通过创建限制令牌，结合删除原令牌的一些权限来实现沙箱功能

```powershell
$token = Get-NtToken -Filtered -RestrictedSids RC -SidsToDisable WD -Flags DisableMaxPrivileges
# 
Get-NtTokenGroup $token -Attributes UseForDenyOnly
# Name     Attributes
# ----     ----------
# Everyone UseForDenyOnly
Get-NtTokenGroup $token -Restricted
# Name                    Attributes
# ----                    ----------
# NT AUTHORITY\RESTRICTED Mandatory, EnabledByDefault, Enabled
Get-NtTokenPrivilege $token
# Name                    Luid              Enabled
# ----                    ----              -------
# SeChangeNotifyPrivilege 00000000-00000017 True
$token.Restricted
# True
```

创建了一个限制令牌，该令牌被映射到RC（NT AUTHORITY\RESTRICTED），并指定了WD（Everyone组）的属性为UseForDenyOnly，最后指定了一个选项来尽量取消该token的特权。后面四行代码都是展示创建的令牌的属性

#### Write-Restricted Tokens

也属于限制令牌的一种，但只限制了写权限。可以通过在NtFilterToken中传入WRITE_RESTRICTED创建。但这种限制令牌实用性比较低，因为它无法阻止应用读取敏感信息

```powershell
$token = Get-NtToken -Filtered -RestrictedSids WR -Flags WriteRestricted
```

使用WR，表示该令牌会被映射到NT AUTHORITY\WRITE RESTRICTED

#### AppContainer and Lowbox Tokens

使用NtCreateLowBoxToken创建，当创建该类token时，需要指定一个Package SID和一系列capability SID，前者类似正常令牌中的User SID，后者则类似于限制令牌的SID

capability SID分为两类

* Legacy  包含了一系列简单的预定义SID来限制程序可使用的资源

* Named  该类SID的RID由给定字串派生，如上面的WR代表`NT AUTHORITY\WRITE RESTRICTED`，这类别名见书中附录B

下表展示了Legacy中预定义的SID

![](pic/19.png)

![](pic/20.png)

### What Makes an Administrator User

在linux中，UID为0的root用户拥有最高权限，可以做任意操作。但在windows中Administrator并不可以执行任意操作，相反是可以限制Administrator的各种权限的。windows中与root拥有类似权限的用户是system

Administrator有三个主要特性

* 当某个用户被配置为Administrator，默认会将其加入`BUILTIN\Administrators`组

* Administrator会被赋予一些额外的权限，如SeDebugPrivilege

* Administrator运行的完整性等级为High，系统服务则运行在System等级

书中提供了一些方法判断一个token是否为Administrator，包括Elevated属性、一些可用的特权以及一些特殊的RID

完整性等级为High的token不一定是Administrator，反过来Administrator也可能运行在低完整性等级，但某些特权要求进程运行在高完整性等级

### User Account Control

Windows默认在安装时第一个创建的账户具有Administrator权限，但在Vista之前没有对相关行为进行限制，导致用户可能默认使用的都是Administrator账号。Vista之后引入了UAC机制，该机制下默认用户仍是Administrator，但默认会禁用Administrator组相关权限，当需要运行Administrator程序时系统会弹出UAC窗口，若通过则提权运行该程序

若一个程序默认需要管理员权限执行，则会调用RPC函数RAiLaunchAdminProcess，另一种显式采用管理员权限运行进程的方式是使用runas执行ShellExecute

#### Linked Tokens and Elevation Type

对于Administrator用户，windows采用一种称为split-token的方式，该token分为两部分

* Limited  非特权的token，用于运行大多数程序

* Full 为一个Administrator token，只有在提权后才能用

split-token有一个域将上述两部分token链接，称为linked token

#### UI Access

Vista的另一个安全机制是用户接口权限隔离（User Interface Privilege Isolation，UIPI），可以防止低权限用户与高权限用户程序进行交互。但这里有一个问题，就是程序需要跟键盘 鼠标等进行交互，因此token可以设置一个叫UI access的标志，若设置了该标志则与桌面环境进行交互时不需要经过UIPI

需要通过ShellExecute函数创建有UI access权限的进程

#### Virtualization



# reference

* [授权 - Win32 apps | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/win32/secauthz/authorization-portal)
- [【windows 访问控制】一、访问令牌 - 小林野夫 - 博客园 (cnblogs.com)](https://www.cnblogs.com/cdaniu/p/15630161.html)

- [Windows 访问控制模型（一） | MYZXCG](https://myzxcg.com/2021/08/Windows-%E8%AE%BF%E9%97%AE%E6%8E%A7%E5%88%B6%E6%A8%A1%E5%9E%8B%E4%B8%80/)
* [TOKEN_GROUPS (winnt.h) - Win32 apps | Microsoft Learn](https://learn.microsoft.com/zh-cn/windows/win32/api/winnt/ns-winnt-token_groups)
