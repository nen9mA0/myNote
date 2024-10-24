### 参考链接

* [Sandboxie Documentation](https://sandboxie-plus.github.io/sandboxie-docs/)  githubio文档

* [Help Topics | Sandboxie-Plus](https://sandboxie-plus.com/sandboxie/helptopics/)  帮助向导页面

* [Getting Started | Sandboxie-Plus](https://sandboxie-plus.com/sandboxie/gettingstarted/)  Getting Start

* 技术点
  
  * [Isolation Mechanism | Sandboxie-Plus](https://sandboxie-plus.com/sandboxie/isolationmechanism/)
  
  * [Token Magic | Sandboxie-Plus](https://sandboxie-plus.com/sandboxie/tokenmagic/)
  
  * [Sandbox Hierarchy | Sandboxie-Plus](https://sandboxie-plus.com/sandboxie/sandboxhierarchy/)  沙箱隔离重定向机制

### Sandbox Hierarchy

[Sandbox Hierarchy - Sandboxie Documentation](https://sandboxie-plus.github.io/sandboxie-docs/Content/SandboxHierarchy/)

sandboxie使用一些重定向方式使得沙箱程序的操作无法影响到计算机系统本身

#### 文件

与文件重定向相关的有两个设置：FileRootPath和BoxRootPath，都是用于指定沙箱的根路径，前者优先级较高

当沙箱程序创建`C:\new.txt`，会被重定向到`{FileRootPath}\drive\C\new.txt`，此外还有一些文件夹会有独立的重定位地址，比如User目录下会被重定向到`{FileRootPath}\user\`，网络共享文件夹会被重定向到`{FileRootPath}\share`

sandboxie有一套类似写时复制的机制，即若沙箱文件夹下没有找到相应的文件，且沙箱程序以只读打开文件，则会直接以只读形式打开系统文件夹的对应文件；若试图进行写入才会复制到沙箱文件夹下。可以通过设置[OpenFilePath](https://sandboxie-plus.github.io/sandboxie-docs/Content/OpenFilePath/), [ReadFilePath](https://sandboxie-plus.github.io/sandboxie-docs/Content/ReadFilePath/)和[ClosedFilePath](https://sandboxie-plus.github.io/sandboxie-docs/Content/ClosedFilePath/)改变上述行为

#### 注册表

注册表存储在一个registry hive中，可以作为一个单独的文件来保存沙箱的注册表

设置KeyRootPath用于指定沙箱注册表根路径，默认为`*HKEY_USERS\Sandbox_(user name)_(sandbox name)`，路径下的注册表结构如下

```
KeyRootPath
 . HKEY_USERS
 . . KeyRootPath
 . . . machine
 . . . user
 . . . . current
```

当沙箱程序试图创建`HKEY_LOCAL_MACHINE\Software\NewKey` ，实际会创建到`{KeyRootPath}\machine\Software\NewKey`

注意，沙箱内的`{KeyRootPath}\user\current\software\classes`软链接到主机的`user\current_classes`

注册表也有与文件类似的写时复制机制，可以通过设置[OpenKeyPath](https://sandboxie-plus.github.io/sandboxie-docs/Content/OpenKeyPath/), [ReadKeyPath](https://sandboxie-plus.github.io/sandboxie-docs/Content/ReadKeyPath/)和[ClosedKeyPath](https://sandboxie-plus.github.io/sandboxie-docs/Content/ClosedKeyPath/)改变上述行为

#### 跨进程对象

这部分沙箱化主要有两个目标

* 可以在不同沙箱里同时运行一个程序

* 可以与外部程序交互

设置IpcRootPath可以为沙箱指定一个NT命名空间，默认为`\Sandbox\(user name)\(sandbox name)\Session_(session number)`

```
IpcRootPath
 . BaseNamedObjects
 . . Global
 . . Local
 . . Session
 . RPC Control
```

注意程序可以创建匿名对象，并且其他程序可以访问这些匿名对象（比如通过句柄），sandboxie通过禁止这类访问防止沙箱程序访问外部的匿名对象

默认情况下沙箱程序不允许与外部程序进行IPC通信，可以通过设置[OpenIpcPath](https://sandboxie-plus.github.io/sandboxie-docs/Content/OpenIpcPath/)和[ClosedIpcPath](https://sandboxie-plus.github.io/sandboxie-docs/Content/ClosedIpcPath/)指定可访问的IPC

### 隐私问题

主要是关于windows系统可能会记录一些与沙箱程序相关的内容导致隐私泄露的问题。虽然sandboxie通过沙箱可以将大多数程序的副作用保留在沙箱内，但仍有一些机制可能导致外部系统被影响

#### Prefetch SuperFetch

该机制会对常用程序进行缓存，一般在`C:\Windows\Prefetch`

#### MUI Cache

`HKEY_CURRENT_USER\Software\Microsoft\Windows\ShellNoRoam\MUICache`注册表会记录最近启动的程序

#### Windows任务栏

explorer中任务栏会记录与文件关联的常用启动软件，保存在`%Appdata%\Microsoft\Internet Explorer\Quick Launch\User Pinned\ImplicitAppShortcuts`

#### Windows页面文件

Windows页面文件包含了内存快照，可能泄露一些信息

#### Windows Hibernate File

与页面文件类似，是休眠时保存的一些内存页面和状态

#### 系统还原

系统还原快照可能会包含沙箱的文件夹，从而还原那些已删除沙箱的文件。可以在还原选项中指定排除项

#### 事件日志

事件日志可能记录沙箱程序的一些信息，特别是安全审计事件

#### 系统托盘图标

沙箱程序若需要申请系统托盘权限，sandboxie会允许其将图标文件写入系统托盘文件夹，因此会泄露图标信息

#### IP地址隐私

sandboxie在访问网络时使用与本机相同的IP，因此可能存在IP泄露的问题

#### DNS缓存

sandboxie访问网页时可能产生DNS缓存，该缓存在`C:\Windows\System32\drivers\etc`

### 服务程序

[Service Programs | Sandboxie-Plus](https://sandboxie-plus.com/sandboxie/serviceprograms/)

用于沙箱程序与一些系统服务的交互

#### RPC

* 服务名：rpcss

* 程序名：SandboxieRpcSs.exe

用于沙箱程序的RPC通信

#### DCOM Server Process Launcher

* 服务名：dcomlaunch

* 程序名：SandboxDcomLaunch.exe

该服务结合RPC服务可以使得一个服务程序在沙箱中被启动

#### Cryptographic Services

* 服务名：cryptsvc

* 程序名：SandboxieCrypto.exe

用于管理和验证沙箱程序的证书，该程序会连接到mscrl.microsoft.com验证证书

#### BITS（Background Intelligent Transfer Service）

* 服务名：bits

* 程序名：SandboxieBITS.exe

用于后台下载，一些程序依赖该服务进行升级

#### 自动升级

* 服务名：wuauserv

* 程序名：SandboxieWUAU.exe

使用上述BITS服务下载windows更新，并试图在sandboxie中安装

#### Windows Installer

* 服务名：msiserver

* 程序名：msiexec.exe

使用windows的安装服务安装沙箱程序

### 技术细节

#### 隔离机制

##### 基本原理

[Isolation Mechanism - Sandboxie Documentation](https://sandboxie-plus.github.io/sandboxie-docs/Content/IsolationMechanism/)

沙箱内运行的进程主令牌会被替换成一个低权限令牌，该令牌基本被禁止访问一切资源（该令牌细节见下面章节）

sandboxie hook了ntdll.dll中的大多数系统调用，并且将其重定向到SbieDrv驱动，此后驱动根据设置的规则对系统调用进行审计，之后执行相应系统调用

采用这种方式，即使恶意软件卸载了ntdll的hook，也会因为令牌权限不够基本做不了任何事

除了ntdll外，sandboxie还hook了一些windows的标准dll，以提供如BITS等服务

注册表和文件重定向是通过SbieDll实现的用户态hook

##### Token Magic

[Token Magic | Sandboxie-Plus](https://sandboxie-plus.com/sandboxie/tokenmagic/)

沙箱进程的低权限令牌是一个未文档化（undocumented）的令牌

当沙箱进程执行一个系统调用时，流程如下：

* 通过ntdll的hook重定向到SbieDrv

* SbieDrv判断当前调用是否符合沙箱规则

* 若符合则一个调用线程会模拟沙箱进程原本该有的令牌，运行对应系统调用，最后将线程的模拟令牌还原

* 返回调用结果

sandboxie通过一些未文档化的操作保证上述机制运行

* 使用一个未导出的函数SepFilterToken创建令牌。该调用其实可以替换为CreateToken或CreateTokenEx，但这些函数照样没导出。若后续考虑取消对未导出函数的依赖，可以用ZwCreateTokenEx替代

* 为了正确处理每个hook的系统调用，驱动必须知道每个系统调用号对应的函数地址和参数。目前通过分析一个未导出函数KeAddSystemServiceTable来获取调用表。若后续考虑取消对未导出函数的依赖，可以使用KeServiceDescriptorTableShadow

* 由于PsImpersonateClient函数的限制，需要先使用SecurityIdentification的模拟等级调用该函数，然后在线程中将其改为SecurityImpersonation

* 为了将沙箱进程的主令牌替换，需要在EPROCESS结构体中将其PrimaryTokenFrozen位清零，上述行为是由PsSetLoadImageNotifyRoutine触发的回调

* sandboxie会从windows station对象中获取剪切板对象，以便调整其完整性级别，让沙箱程序访问
