# JTAG

## IEEE 1149.1

### 边界扫描

![](pic\jtag_1.png)

在芯片输入输出加入一组移位寄存器单元，因为分布在芯片周围引脚因此被称为**边界扫描寄存器**

一般这些移位寄存器会被串联为多个**边界扫描链**，提供对不同引脚、寄存器的访问

这组寄存器在运行时对于芯片是透明的，而在调试时可以通过这些寄存器加载或读取芯片中的引脚、寄存器值

![](pic\jtag_2.png)

### TAP(Test Access Port)

* TCK(Test Clock Input)
* TMS(Test Mode Selection Input)
* TDI(Test Data Input)
* TDO(Test Data Output)
* TRST(Test Reset Input)

JTAG标准把寄存器分为两大类，DR(Data Register)和IR(Instruction Register)，一般来说访问一个DR的过程如下

* 通过IR选定一个需要访问的DR
* 把选定的寄存器连接到TDI和TDO之间
* 由TCK驱动，将数据写入寄存器

### 状态机

![](pic\jtag_3.png)

#### Reset

上电复位模式，可以通过TRST触发，也可以通过连续5个TMS=1触发（注意，这里无论处于状态机的什么地方都可以通过5个TMS=1触发复位，所以TRST不是必要的信号）

#### Run-Test/Idle

空闲

#### Select-DR-Scan

选择扫描DR

#### Select-IR-Scan

选择扫描IR

#### Capture-DR

TCK上升沿将寄存器、引脚当前数据传入到移位寄存器

#### Capture-IR

类似

#### Shift-DR

移位寄存器移位一位

#### Shift-IR

类似

#### Update-DR

移位寄存器的数据加载入对应寄存器

#### Update-IR

类似