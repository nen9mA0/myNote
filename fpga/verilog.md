# Verilog数字系统设计教程

## Verilog语法

### 模块

```verilog
module xxx(
	a,
    b,
    c,
    d
);			//声明模块
    input a;
    input [15:0] b;
    output c;
    inout d;

    wire wa;	//信号线
    wire wb;
    reg ra;		//寄存器
	tri ta;		//三态信号线
    
endmodule
```

### 常量和变量

#### 数字

这里的n位都是指2^n，默认为32位

* n'b00000  n位二进制数
* n'd00000  n位十进制数
* n'h00000  n位16进制数
* n'o00000  n位8进制数

x和z

* x  不定值
* z  高阻值，也可以写作?

负数，-写在最前面

* -8'd5

_ 可以分隔数字增加可读性

#### 参数

##### parameter

parameter a=1,b=2;

假设module有参数，可以在实例化时指定参数

```verilog
module test(a,b);
    parameter A=1,B=2;
...
endmodule

//实例化
test #(3,4) t1;
```

##### defparam

```verilog
module Test;
    Tpp T();
endmodule

module top;
    Block B1();
    Block B2();
endmodule

module Block;
    Parameter P=0;
endmodule

module Annotate;
    defparam
    	Test.T.B1.P = 1;
    	Test.T.B2.P = 2;
endmodule
```

### 变量

#### wire/tri

* wire 单驱动源
* tri  多驱动源

当两个信号冲突时的真值表

| wire/tri | 0    | 1    | x    | z    |
| -------- | ---- | ---- | ---- | ---- |
| 0        | 0    | x    | x    | 0    |
| 1        | x    | 1    | x    | 1    |
| x        | x    | x    | x    | x    |
| z        | 0    | 1    | x    | z    |

#### reg

注意：reg型只表示被定义的信号将用在always块内，不一定对应触发器

**注意字节顺序**

```verilog
reg [0:15] mem;
```

**这里的LSB是mem[15]，MSB是mem[0]**

#### memory

```verilog
reg [x:0] mem[y:0];	//定义
mem[n] <= x'b0000;	//赋值
```

### 端口

#### input

#### output

#### inout

inout多驱动源时的真值表

| 驱动源 | 0    | 1    | X    | Z    |
| ------ | ---- | ---- | ---- | ---- |
| 0      | 0    | X    | X    | 0    |
| 1      | X    | 1    | X    | 1    |
| X      | X    | X    | X    | X    |
| Z      | 0    | 1    | X    | Z    |

**可以看出，若要将inout端口作为输出口，应给其赋值Z**

### 运算符

#### 算术

* +
* -
* *
* /
* %

#### 位运算

* ~
* &
* |
* ^
* ^~  同或（异或非）

| ~    | res  |
| ---- | ---- |
| 1    | 0    |
| 0    | 1    |
| x    | x    |

| &    | 0    | 1    | x    |
| ---- | ---- | ---- | ---- |
| 0    | 0    | 0    | 0    |
| 1    | 0    | 1    | x    |
| x    | 0    | x    | x    |

| \|   | 0    | 1    | x    |
| ---- | ---- | ---- | ---- |
| 0    | 0    | 1    | x    |
| 1    | 1    | 1    | 1    |
| x    | x    | 1    | x    |

| ^    | 0    | 1    | x    |
| ---- | ---- | ---- | ---- |
| 0    | 0    | 1    | x    |
| 1    | 1    | 0    | x    |
| x    | x    | x    | x    |

| ^~   | 0    | 1    | x    |
| ---- | ---- | ---- | ---- |
| 0    | 1    | 0    | x    |
| 1    | 0    | 1    | x    |
| x    | x    | x    | x    |

#### 逻辑

* &&
* ||
* ！

#### 关系

* <
* \>
* <=
* \>=
* ==
* !=
* ===  对x和z也进行比较
* !==

#### 移位

* <<
* \>\>

#### 位拼接

```verilog
{a,b[3],b[2],b[1],b[0],w,1'b1,1'b0,1'b1}
//等同于
{a,b[3:0],w,3'b101}

//重复法
{4{w}}  //等同于 {w,w,w,w}
//嵌套法
{b,{3{a,b}}} //等同于{b,a,b,a,b,a,b}
```

#### 缩减运算

```verilog
reg [3:0] B;
reg c;

c = &B;	//等同于
c = B[3] & B[2] & B[1] & B[0];
```

### 赋值

#### 非阻塞

<=

#### 阻塞

=

### 块语句

#### 顺序块

语句按顺序执行

```verilog
begin:label
   ...
end
```

#### 并行块

语句同时执行

```verilog
fork:label
    ...
join
```

**注意，命名的块可以被其他模块调用，包括被disable**

#### disable

```verilog
begin:block1
    while(i<16)
    begin
        if(flag[i])
        begin
            $display("A");
            disable block1;
        end
        i = i+1
    end
end
```

### 条件语句

#### if

```verilog
if()
    ...
else if()
    ...
else
    ...
```

#### case

```verilog
case(x)
    x'b00: ...;
    x'b01: ...;
    x'b10: ...;
    x'b11: ...;
    default: ...;
endcase
```

**注意，存在casex和casez，真值表如下**

| case | 0    | 1    | x    | z    |
| ---- | ---- | ---- | ---- | ---- |
| 0    | 1    | 0    | 0    | 0    |
| 1    | 0    | 1    | 0    | 0    |
| x    | 0    | 0    | 1    | 0    |
| z    | 0    | 0    | 0    | 1    |

| casez | 0    | 1    | x    | z    |
| ----- | ---- | ---- | ---- | ---- |
| 0     | 1    | 0    | 0    | 1    |
| 1     | 0    | 1    | 0    | 1    |
| x     | 0    | 0    | 1    | 1    |
| z     | 1    | 1    | 1    | 1    |

| casex | 0    | 1    | x    | z    |
| ----- | ---- | ---- | ---- | ---- |
| 0     | 1    | 0    | 1    | 1    |
| 1     | 0    | 1    | 1    | 1    |
| x     | 1    | 1    | 1    | 1    |
| z     | 1    | 1    | 1    | 1    |

### 循环语句

#### forever

用于产生testbench波形，**必须写在initial块中**

#### repeat

计数循环

```verilog
repeat(n)
begin
    ...
end
```

#### while

条件循环

```verilog
while(expr)
begin
    ...
end
```

#### for

与C语言for相同

```verilog
for(;;)
```

### 生成块

#### 循环生成

```verilog
module bitwise_xor(out,i0,i1);
parameter N=32;
    
    output [N-1:0] out;
    input [N-1:0] i0,i1;
    
    genvar j;		//该变量只用于生成块的循环计算，不会被实例化
    
    generate
        for(j=0;j<N;j=j+1)
            begin
                xor g1(out[j],i0[j],i1[j]);
            end
    endgenerate
    
//或
    generate
        for(j=0;j<N;j=j+1)
            begin
                always@(i0[j] or i1[j]) out[j] = i0[j]^i1[j];
            end
    endgenerate
```

#### 条件生成

使用case和if生成，具体见书86页

### 结构语句

#### initial

仿真开始时对各变量初始化，不需要仿真时间

注意，若initial块中有延时且修改了下面always块信号列表中的信号，一样会触发always块执行

#### always

```verilog
always@(信号列表)
    begin
        ...
    end
```

##### 信号列表

* 信号列表可以用**posedge**和**negedge**指定上升沿和下降沿
* 可以用or或,表示或的关系
* @*表示对后面语句块中所有输入变量的变化都敏感
* 仿真时除了使用always+信号列表检测信号，还可以用wait(信号名)来监测信号变化

**注意：若使用诸如always clk=~clk;将导致仿真器死锁，应使用always #1 clk=~clk**

#### task

```verilog
//定义，类似于模块，有输入输出信号
task my_task;
    input a,b
    output c;
    
    ...
    c = 1'b1;
endtask
//实例化
my_task(port1,port2,port3);
```

**注意：嵌套的task只有在所有task执行完后才会返回**

##### 延时tip

```verilog
repeat(tics)
    @(posedge clock);
```

#### function

```verilog
function [7:0] getbyte;
    input [15:0] address;
    begin
        ...
        getbyte = result;
    end
endfunction
```

* 函数必须有返回值，并且返回值在函数内名称与函数名相同

##### 函数的规则

* 不能包含时间控制语句，即# @ wait
* 不能启动任务
* 至少有一个输入
* 必须有输出

##### 关于递归

声明时使用automatic关键字指定的函数可以递归

### 系统任务

```verilog
//输出
$display
$write

//文件
$fopen
$fdisplay
$fmonitor
$fwrite
$fstrobe
$fclose

//转储
$dumpfile
$dumpvars
$dumpoff

//监控
$monitor(列表)  //每次值发生变化时都会记录
$monitoron
$monitoroff

//时间
$time		//当前仿真时刻
$realtime

//控制
$finish		//退出仿真
$stop		//暂停仿真

//读取
readmemb
readmemh	//从文件中读取数据到存储器（例子见104）

//随机数
$random

//条件执行
$test$plusargs
$value$plusargs
```

### 预编译

* `define`
* include "filename"  这个操作会使被包含文件整个复制到include处
* \`timescale 单位/精度    \`timescale 1ns/1ps   时间单位1ns，时间可打印为3位小数的实型，因为精度为1ps
* `ifdef
* `else
* `endif

## testbench

#### event

用于定义事件，可以作为敏感信号

在仿真时使用

```verilog
# time -> event_name
```

触发

## 阻塞赋值与非阻塞赋值

### 要点

* 建模**时序电路**使用非阻塞
* 建模**锁存器电路**使用非阻塞
* 建模**组合逻辑电路**采用阻塞
* 同一个always块建模**时序和组合逻辑电路**用非阻塞
* 同一个always不要同时使用两种
* 不要在不同always块中给同一个变量赋值
* 赋值时不要使用#0延迟

注意，可以用$strobe显示非阻塞赋值的变量值

## 可综合的语句

* module  endmodule
* input output inout
* wire reg tri
* parameter
* 除===和!==以外的运算符
* if...else  case(casex,casez)...default...endcase
* assign  ?:
* always
* begin end
* task...endtask
* for
* =  <=

## 复位设计

### 异步复位与同步复位

#### 异步复位

复位信号与clk独立

```verilog
always @(posedge clk or negedge rst_n)
    if(!rst_n) b <= 1'b0;
	else b <= a;
```

#### 同步复位

```verilog
always @(posedge clk)
    if(!rst_n) b <= 1'b0;
	else b <= a;
```

#### 差别

异步复位综合后，rst_n会被接入锁存器的rst脚。同步复位综合后，锁存器的rst脚不会被连接，rst_n作为输入脚的使能端，即rst_n与输入相与后接入锁存器

### 亚稳态

* setup time  建立时间，指触发器时钟信号上升沿**到来前**数据稳定不变的时间。若建立时间不够数据将不能在这个上升沿稳定输入锁存器
* hold time  保持时间，是指在触发器的时钟信号上升沿**到来后**，数据稳定不变的时间，如果保持时间不够，数据同样不能被稳定的输入触发器