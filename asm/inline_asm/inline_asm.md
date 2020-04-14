### ref

https://gcc.gnu.org/onlinedocs/gcc/Using-Assembly-Language-with-C.html#Using-Assembly-Language-with-C 原文

https://www.jianshu.com/p/1782e14a0766



### basic asm内联汇编

除非有备注中的情况，一般建议将basic asm替换为extended asm

```
asm asm-qualifiers ( AssemblerInstructions )
```

#### asm-qualifiers

##### volatile

无影响，所有的内联汇编都隐含volatile

##### inline

如果使用inline关键字，为了更好地内联并提高性能，GCC将会尽量地缩小该语句占用的空间（见“asm的长度”）

#### AssemblerInstructions

GCC不会处理basic asm语句，也不会判断汇编语句是否正确，而是直接将其传给汇编器

asm内可以包含多个汇编语句，**一般用`\n\t`隔开**

#### 备注

##### 只能使用basic asm的情况

一般来说extended asm会比基本asm好，但有些情况只能使用basic asm：

* extended asm只能用在C函数内，若要单独写汇编只能用basic asm。注意，函数外的asm不能使用任何qualifiers
* `__attribute__((naked))`定义的函数，该属性作用是让编译器生成裸函数，即没有prolog和epilog以及优化的代码

##### 其他注意事项

* 如果要与C代码联用最好用extended asm
* 编译器不能保证多个连续asm标签的语句顺序，因此若有顺序要求应写入单个asm标签
* 多个asm标签间不能跳转，要跳转得写进一个asm
* 在某些情况下GCC可能在优化时复制asm标签内的代码，若在asm标签内定义了符号或标号有可能报错
* 对于basic asm，GCC不解析其中的任何东西，这意味着GCC无法获知basic asm内定义的符号以及其使用的寄存器和内存情况。GCC的做法是：认为在这过程中没有通用寄存器被修改，但可能修改任何全局可见的变量

##### basic asm的移植

https://gcc.gnu.org/wiki/ConvertBasicAsmToExtended

注意，因为GCC不处理basic asm内的任何符号，因此导致basic asm和extended asm的一些区别，比如basic asm中的`%eax`在extended asm中是`%%eax`

##### 编译参数

```
-masm
```

编译参数直接传递给汇编器

#### 示例

```c
#define DebugBreak() asm("int $3")
```

### extended asm

```
__asm__ asm-qualifiers ( AssemblerTemplate 
                 : OutputOperands 
                 [ : InputOperands
                 [ : Clobbers ] ])

__asm__ asm-qualifiers ( AssemblerTemplate 
                      : 
                      : InputOperands
                      : Clobbers
                      : GotoLabels)
```

#### qualifiers

* volatile  若汇编语句将产生除了对输入和输出寄存器的修改，需要用该选项禁用编译器优化

* inline  与basic asm相同，尽量生成最小的asm代码

* goto  语句可能产生到 *GotoLabels* 中定义的跳转

#### 参数

* AssemblerTemplate  一组汇编语句，见下面描述

* OutputOperands  一组将被修改的C变量。可以为空。见下面

* InputOperands  一组将被读取的C变量，可以为空。见下面

* Clobbers  除了OutputOperands外将被修改的寄存器和C变量，可以为空。见下面

* GotoLabels  当语句可能发生跨asm标签跳转时，需将跳转的标签列于此。见下面

**OutputOperands + InputOperands + GotoLabels个数之和应小于等于30**

#### Volatile

GCC的优化器有时会优化asm标签内的语句，可以使用volatile标签禁用其优化功能

对于没有输出的asm块（包括纯goto语句）隐式调用了volatile

有时编译器甚至可以移动asm语句的位置，如下例

```c
asm volatile("mtfsf 255, %0" : : "f" (fpenv));
sum = x + y;
```

编译器可能将其移动到sum=x+y后，即使加了volatile也没用。可以用下列方法使其保持顺序

```c
asm volatile ("mtfsf 255,%1" : "=X" (sum) : "f" (fpenv));
sum = x + y;
```

即，将sum作为输出值，但实际并不引用，以欺骗编译器认为该语句对sum值有影响

若asm块内定义了符号或标签，优化器可能会复制这段代码，使得链接时出现重复定义的错误。这种情况下可以使用`%=`来解决（见下文）

#### Assembler Template

是一组汇编语句，GCC不处理它们，但会计算它们的大小（见size of an asm）

语句分隔符根据汇编器类型。一般是`\n\t`，有时是`;`

对于多个asm标签内的语句，GCC不保证顺序。因此若要求顺序应放到一个标签

除了在input output goto几个中列举的符号外，GCC无法获知汇编语句中的任何符号

##### Special format strings

一些特殊符号

* `%% %{ %} %|`  表示符号`% { } |`
* `%=` 为每个asm实例创建一个唯一的标签。对于asm内标签被多次引用的asm块很有用。因为GCC优化器可能会为优化将asm块复制多遍放到各个引用处。若标签不唯一则可能导致重复定义错误

##### Multiple assembler dialects in `asm` templates

支持使用不同的汇编器语法。

-masm可以指定汇编器支持的语法类型 [gcc -m选项](https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html#x86-Options)

[示例](#dialect)

#### Output Operands

```
[ [asmSymbolicName] ] constraint (cvariablename)
```

##### asmSymbolicName 

可以用`%[Value]`定义符号名，同一asm中的符号名不能一样，若无定义则默认变量名为`%0 %1 ...`

##### constraint  

见下文

##### cvariablename

C语言中用于存结果的变量名

##### 注意点

* 若指定输出到寄存器，编译器不会选择**clobbered registers**作为输出寄存器（见下文）

* 当输出变量无法直接寻址，应该令其输出到寄存器，gcc将把结果放进内存

* 使用`=`约束来说明output绑定的变量将被修改

* 使用`+`约束来指定一个变量既用于输入也用于输出

* 使用`&`可以指定编译器不能让output和input使用同一个寄存器

  若asm块有a和b两个输出，其中a输出到寄存器b输出到内存，若不使用&，可能出现b寻址需要用到保存a的寄存器的情况而导致错误

#### Input Operands

```
[ [asmSymbolicName] ] constraint (cexpression)
```

大多与output类似

##### 注意点

* 不能使用`= +`作为约束
* 编译器不会选择**clobbered registers**作为输入寄存器
* 除非指定了输入输出寄存器绑定的选项，否则对于纯输入的操作数不能修改它们的内容
* 约束可以使用数字，用来表示某个输入变量与某个输出变量绑定（见示例）。注意，光是变量名一致不足以说明两个操作数存放在同一个位置，如`: "=r"(foo): "r"(foo)`不能保证最后output真正放在input的位置

#### Clobbers

Clobbers列表中声明的所有寄存器不可能作为输入输出寄存器，且**不能包含栈指针寄存器**

两个特殊选项

* "cc"  表示汇编语句将修改标志寄存器FLAGS
* "memory"  表示汇编语句将读取或修改除了input output指定地址外的内存。因为gcc编译结果中变量不一定全部被写入内存，若指定了memory则相当于一个fence，在执行asm块前gcc将生成代码将它们都写入内存

若定义了memory，则gcc会在调用asm块前将所有寄存器上的临时变量写回其对应的内存，这往往很耗时，因此内联汇编允许指定哪些寄存器不需要被刷新。见例子

#### Goto Labels

* 用来指定asm可能跳转到的所有标签
* goto语句没有输出，因此如果asm块改了任何东西，应该指定clobbers为memory
* asm goto隐式包含了volatile
* 默认使用`%ln`来引用跳转，其中n是对应label的序号

##### 关于序号

```c
asm goto (
    "btl %1, %0\n\t"
    "jc %l2"
    : /* No outputs. */
    : "r" (p1), "r" (p2) 
    : "cc" 
    : carry);

return 0;

carry:
return 1;
```

序号从0开始，%0引用p1，%1引用p2，所以carry标签的序号是2，用`%l2`引用

### 一些细节

####  Flag Output Operands

有些情况下需要标志位将作为输出，则可以用这种方法输出某个flag bit的值。

**注意**：需要定义`__GCC_ASM_FLAG_OUTPUTS__`预处理标志

语法为：在定义OutputOprand标志的时候为`=@ccCOND`，COND为与架构相关的标志位

具体标志位及其意思见6.47.2.4 https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#Extended-Asm





### 约束

注意，这里的操作数类型指在汇编语句中的类型，如

```
asm
```

#### 普通约束

* `m`  操作数类型为内存 (如[eax])
* `o`  操作数为一个常数基址+寄存器变量寻址的内存 (如[edx + 8*eax])
* `V`  属于m但不属于o的内存地址
* `> <`  操作数是一个会自动减少/增加的内存地址
* `r`  操作数类型为寄存器
* `i`  操作数为立即数，允许该数为汇编时才可见的立即数
* `n`  操作数为立即数且是一个数值，若汇编器不支持i则用n
* `I J K ... P`  操作数为立即数，在某些架构中需要用该选项来指定立即数的取值范围，具体定义可看"一些架构相关的选项"
* `E`  操作数为浮点立即数，但仅限于目标机浮点数类型与编译机相同的情况
* `F`  操作数为浮点立即数
* `G H`  操作数为浮点立即数，在某些架构中用该选项指定浮点立即数范围
* `s`  与i类似，但可以让编译器提供一些优化。比如在68000处理器上的一个-128~127的立即数，可以指定参数为Ks（K是因为上述的取值范围）
* `g`  除非通用寄存器外的所有操作数类型都允许
* `X`  所有类型的操作数都允许
* `0 1 2 ... 9`  表示对InputOprand或OutputOprand的引用，如`shl $8, %0\r\n" :"=r"(a) :"0"(a)`表示输入和输出放入一个寄存器（注意，所以这里OutputOprand的约束是=r而不能是+r）
* `p ` 操作数是个有效的内存地址

#### 修饰符

* `=`  该操作数内容将被覆盖
* `+`  该操作数将被读取并最后被覆盖
* `&`  是一个earlyclobber的输入操作数，即其在被汇编指令使用完之前会被修改（如内联汇编时直接将参数写入esi edi，而使用lodsb将修改esi edi等）
* `%`  该操作数可以跟后面的操作数互换，以使编译器优化。只适用于只读操作数

#### 一些架构相关的选项

https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html#Machine-Constraints

### 示例

#### 1

```c
int src = 1;
int dst;   

asm ("mov %1, %0\n\t"
    "add $1, %0"
    : "=r" (dst) 
    : "r" (src));

printf("%d\n", dst);
```

#### volatile

##### volatile_1

这个示例说明了一种不需要volatile的情况。若代码中不使用assert，则该函数将直接被忽略，加快执行速度

```c
void DoCheck(uint32_t dwSomeValue)
{
   uint32_t dwRes;

   // Assumes dwSomeValue is not zero.
   asm ("bsfl %1,%0"
     : "=r" (dwRes)
     : "r" (dwSomeValue)
     : "cc");

   assert(dwRes > 3);
}
```

##### volatile_2

对于这段代码编优化器可以检测出循环对于输出值没有影响，因此会将循环取消

```c
void do_print(uint32_t dwSomeValue)
{
   uint32_t dwRes;

   for (uint32_t x=0; x < 5; x++)
   {
      // Assumes dwSomeValue is not zero.
      asm ("bsfl %1,%0"
        : "=r" (dwRes)
        : "r" (dwSomeValue)
        : "cc");

      printf("%u: %u %u\n", x, dwSomeValue, dwRes);
   }
}
```

##### volatile_3

一下情况必须用volatile，因为若不用优化器会优化掉第二个rdtsc

```c
uint64_t msr;

asm volatile ( "rdtsc\n\t"    // Returns the time in EDX:EAX.
        "shl $32, %%rdx\n\t"  // Shift the upper bits left.
        "or %%rdx, %0"        // 'Or' in the lower bits.
        : "=a" (msr)
        : 
        : "rdx");

printf("msr: %llx\n", msr);

// Do other work...

// Reprint the timestamp
asm volatile ( "rdtsc\n\t"    // Returns the time in EDX:EAX.
        "shl $32, %%rdx\n\t"  // Shift the upper bits left.
        "or %%rdx, %0"        // 'Or' in the lower bits.
        : "=a" (msr)
        : 
        : "rdx");

printf("msr: %llx\n", msr);
```

#### dialects

##### dialects

<span id="dialect"></span>

假设汇编器支持两种语法 `('att', 'intel')`

```asm
bt{l %[Offset], %[Base] | %[Base], %[Offset]}  ;jc %12
```

则可以生成下列两种语句

```asm
btl %[Offset], %[Base]  ;jc %12    AT&T
bt %[Base], %[Offset]  ;jc %12     Intel
```

下列语句

```asm
xchg{l}\t{%%}ebx, %1
```

可以生成下面两种形式的语句

```asm
xchgl\t%ebx, %1
xchg\tebx, %1
```

#### extended asm output

##### extended asm output 1

* =r Index将被覆盖，结果放在寄存器
* r  输入为Mask，放在寄存器中
* %0 %1  分别引用Index和Mask

```c
uint32_t Mask = 1234;
uint32_t Index;

  asm ("bsfl %1, %0"
     : "=r" (Index)
     : "r" (Mask)
     : "cc");
```

##### extended asm output 2

这里用到了asmSymbolicName字段为变量重命名

```c
uint32_t Mask = 1234;
uint32_t Index;

  asm ("bsfl %[aMask], %[aIndex]"
     : [aIndex] "=r" (Index)
     : [aMask] "r" (Mask)
     : "cc");
```

##### extended asm output 3

这里的rm说明变量既可以在寄存器中，也可以在内存中，让GCC决定如何优化

```c
uint32_t c = 1;
uint32_t d;
uint32_t *e = &c;

asm ("mov %[e], %[d]"
   : [d] "=rm" (d)
   : [e] "rm" (*e));
```

#### extended asm input

##### extended asm input 1

0指定了input的第一个参数foo和output的第一个参数foo放在同一个内存空间

```c
asm ("combine %2, %0" 
   : "=r" (foo) 
   : "0" (foo), "g" (bar));
```

##### extened asm input 2

```c
asm ("cmoveq %1, %2, %[result]" 
   : [result] "=r"(result) 
   : "r" (test), "r" (new), "[result]" (old));
```

#### extended asm clobber

##### 基本

clobber列表指明了r0-r5不能作为输入输出寄存器，且语句可能读写某块内存

```c
asm volatile ("movc3 %0, %1, %2"
                   : /* No outputs. */
                   : "g" (from), "g" (to), "g" (count)
                   : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
```

##### 不使用clobber来指定需要回写的内存

```c
asm ("sumsq %0, %1, %2"
     : "+f" (result)
     : "r" (x), "r" (y), "m" (*x), "m" (*y));
```

一次用来指定变量用到的寄存器，一次用来指定用到的内存基址

```c
asm ("vecmul %0, %1, %2"
     : "+r" (z), "+r" (x), "+r" (y), "=m" (*z)
     : "m" (*x), "m" (*y));
```

```c
asm("repne scasb"
    : "=c" (count), "+D" (p)
    : "m" (*(const char (*)[]) p), "0" (-1), "a" (0));
```

#### extended asm goto

##### extended asm goto

```c
asm goto (
    "btl %1, %0\n\t"
    "jc %l2"
    : /* No outputs. */
    : "r" (p1), "r" (p2) 
    : "cc" 
    : carry);

return 0;

carry:
return 1;
```

注意这里的clobber有个"memory"，因为会修改y的内存

```c
int frob(int x)
{
  int y;
  asm goto ("frob %%r5, %1; jc %l[error]; mov (%2), %%r5"
            : /* No outputs. */
            : "r"(x), "r"(&y)
            : "r5", "memory" 
            : error);
  return y;
error:
  return -1;
}
```

#### flag output

获取carry flag

```c
int variable_test_bit(long n, volatile const unsigned long *addr)
{
    int oldbit;
    asm volatile("bt %[value],%[bit]"
                 : "=@ccc" (oldbit)
                 : [value] "m" (*addr), [bit] "Jr" (n));
    return oldbit;
}
```

