## Instruction Format

### 基本格式

![](pic/2_2_1_1.png)

#### 前缀（prefix）

##### group 1

* lock repeat
  * LOCK  F0H
  * REPNE  F2H
  * REP/REPE  F3H  两个REP前缀F2 F3可能在不能使用REP前缀的指令中表示其他意思
* BND

##### group 2

* 段寄存器前缀
  * CS  2EH
  * SS  36H
  * DS  3EH
  * ES  26H
  * FS  64H
  * GS  65H
* branch hints  用于告诉处理器某个跳转是否倾向于执行
  * 2EH  跳转未执行
  * 3EH  跳转执行

##### group 3

* Operand-size override prefix  66H  选择某个操作数应该为16b或32b

##### group 4

* Address-size override prefix  67H  选择寻址为16b或32b模式

#### Opcode

长度1 2 3字节。opcode中的各个位可能代表了不同的含义，如寄存器编码 符号扩展等。取决于opcode的类型

#### ModR/M

* Mod  可以与R/M字段一起，指定8个寄存器或24种寻址方式
* reg/opcode  可能用于指定寄存器好，也可能是其他信息，取决于指令
* R/M  可以与Mod字段合用，也可以单独用于指定一个寄存器。也是取决于指令

#### SIB

用于基址变址寻址（[eax+edx]  [eax+8*edx]）

* Scale  比例因子 1 2 4 8
* Index  变址寄存器
* Base  基址寄存器

#### Displacement

一些寻址指令在ModR/M后有Displacement字段，可能有1 2 4字节（[eax+0x12345678]）

#### Immediate Bytes

立即数，可能有1 2 4字节

#### 寻址编码

ModR/M与SIB共同编码内存地址可见P509开始的三张表。

ModR/M编码下列类型的寻址

* [reg]
* [reg+reg]
* [reg]+off

SIB编码下列类型的寻址

* [reg]
* [reg*2]
* [reg*4]
* [reg*8]

注意：

* disp32意思是后面跟着一个32位的displacement指定地址
* [eax]+disp8  意思是eax为基址，后面跟着一个8位displacement指定增加的地址
* `[--][--]+disp32`  意思是在当前ModR/M后有一个SIB，此外还有一个32位的displacement指定增加的地址。因此这种形式可以用于编码[reg + 8*reg + off]

