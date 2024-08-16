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

### NTSTATUS Codes

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

### Object Handles

![](pic/9.png)

object manager用于管理内核对象，内核对象一般以句柄的形式返回给用户空间来标识对象，而在API接收到句柄参数时，一般需要先进行索引来找到句柄对应的内核对象（ObReferenceObjectByHandle）

在索引时，会运行两个检查

* 检查当前的用户是否有权限获取相应的对象

* 检查当前句柄指向的对象是否与API请求的对象类型一致

### Access Masks

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
