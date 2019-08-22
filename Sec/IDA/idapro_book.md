

## 20章 编译器变体

##### 自定义函数调用协议

Edit -> Functions -> Set function type

举例：

```
int __usercall sub_158AC<eax>( struc_1 *<eax>, unsigned __int8 index<cl>, int )
```

* __usercall  必须指定以说明使用了一个用户定义的调用协议
* 函数名后必须指定返回值通过什么返回（此处为eax）
* 使用\<xxx\>指定寄存器传参
* 若无指定寄存器，默认参数位于栈上

## 21章 模糊代码分析

### 反静态分析技巧

#### 反汇编去同步（花指令）

##### jmp xxx+1

##### 条件永远成立的条件跳转

#### 目标地址动态计算

##### 返回地址动态计算

将[esp]取出运算，再写回[esp]，最后ret

##### SEH

将要执行的代码安装进SEH并故意触发异常

##### 操作码模糊（壳）

##### 导入函数模糊（更改IAT）

### 反动态分析技巧

#### 检测虚拟化

##### 检测vmtools

##### 检测虚拟化硬件

如OUI，是网卡MAC地址前3字节，一般虚拟网卡默认会被分配指定的OUI

##### 检测虚拟机行为

```asm
mov eax,0x564D5868 ;'VMXh'
mov ecx,10
xor ebx,ebx
mov dx,0x5658	;'VX'
in eax,dx
```

若在实机中将导致异常，因为用户代码不允许使用in

但在VMware中可以用in作为与主机通信的后门

##### 检测特定指令处理器行为的不同

#### 检测检测工具

##### 扫描活动进程列表

##### 查找已知与这类软件有关的进程名

##### 扫描所有应用程序的标题栏文本

##### 扫描窗口注册类名称

#### 检测调试器

[Windows Anti-Debug Reference](http://www.symantec.com/connect/articles/windows-anti-debug-reference)

[unpackers.pdf](http://pferrie.000webhostapp.com/papers/unpackers.pdf)

#### 防止调试

##### Unix系统上的进程跟踪（即调试自己）

### 模拟器x86emu

模拟执行x86代码

#### x86emu与反调试

emu在设计时考虑到了反调试

#####  库函数

IsDebuggerPresent一类的函数返回值设为0

##### 时间戳

rdtsc获取系统时间戳的指令会返回一个足够小的间隔值

##### SEH

x86emu支持执行SEH

### 虚拟机

字节码还原，可以编写对应的IDA处理器模块，并使用扩展指令集的功能来同时显示x86代码和字节码。