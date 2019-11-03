## ld script


### 指令

####  入口点

可以通过如下方式设置入口点

```
ENTRY(symbol)  同ld -e选项
```

默认情况下入口点指定优先级如下

* -e 选项
* ENTRY(symbol)
* 一些特定的符号，如start
* .text段的第一个字节
* 地址0

#### 文件命令

不常用

```
INCLUDE fileename		包含一个链接脚本
INPUT(file,file...)		指定一个输入文件，即ld命令行中的xxx.o可以替换成脚本里的一行INPUT
GROUP(file,file...)		类似INPUT，但一般在程序会出现 循环参考 情况时使用（ld的-(选项）
START_UP(filename)		类似INPUT，但START_UP指定的文件会被第一个链接
AS_NEEDED(file,file...)	只能出现在INPUT或GROUP指令中，只有当指定的文件真正被需要时才会被链接
						（ld的--as-needed选项）
OUTPUT(filename)		同ld -o选项
SEARCH_DIR(path)		同ld -L选项
```

#### 指定输出格式

```
OUTPUT_FORMAT(bfdname)  同ld -oformat 指定输出格式
TARGET(bfdname)			同ld -b 指定输入格式，若未指定输出格式则将其当成输出格式
```

常用的有elf32-i386、binary等

#### REGION_ALIAS

```
REGION_ALIAS(alias, region)  为一个memory region设置别名
```

##### 例子

```
MEMORY
{
	RAM : ORIGIN = 0, LENGTH = 4M
}

REGION_ALIAS("REGION_TEXT",RAM);
```

为RAM内存区设置了一个别名"REGION_TEXT"

在输出section定义中，若想将section映射到别名为xxx的内存段，可以通过 >xxx 指定

#### 输出架构

```
OUTPUT_ARCH(bfarch)
```

### 赋值

对一个符号赋值，等同于定义这个符号并且放到输出文件的符号表中

#### 简单赋值

赋值格式与C语言相同

```
symbol = expression;
symbol += expression;
symbol -= expression;
symbol *= expression;
symbol /= expression;
symbol <<= expression;
symbol >>= expression;
symbol &= expression;
symbol |= expression;
```

特殊符号 . 表示地址计数器，详见SECTION中的描述

##### 例子

```
SECTIONS
{
  .text :
    {
      *(.text)
      _etext = .;
    }
  _bdata = (. + 3) & ~ 3;
  .data : { *(.data) }
}
```

这里\_etext被定义为text段的末尾，\_bdata被定义为data段起始地址 +3 & ~3，即data段4字节对齐后的地址

#### 隐藏符号

对于ELF文件，定义一个符号（symbol）默认将被导出（export，即加入elf的符号表）

使用HIDDEN()可以使符号仅在链接时可见

```
SECTIONS
{
  .text :
    {
      *(.text)
      HIDDEN(_etext = .);
    }
  HIDDEN(_bdata = (. + 3) & ~ 3);
  .data : { *(.data) }
}
```

#### PROVIDE弱定义

使用PROVIDE定义的符号类似于C语言的weak，只有在原程序的符号表中未定义的符号才会被定义，否则被覆盖

```
SECTIONS
{
  .text :
    {
      *(.text)
      _etext = .;
      PROVIDE(etext = .);
    }
}
```

若原程序符号表中有_etext，则编译器报错，但若有etext则不报错

#### 隐藏PROVIDE定义

同隐藏符号定义

#### 在代码中引用ld script符号

##### 修饰名

首先要说明，源码中的符号和存在符号表中的符号可能名称不一致，如在C语言中引用foo变量

```c
extern int foo;
```

实际上在ld script中对应的名字可能是\_foo

接下来的讨论不考虑这种情况

##### C创建变量的过程

在C中这样定义

```c
int foo = 1000;
```

实际上编译器做了两件事：

* 为foo分配一块内存
* 在符号表中保存一个名为foo的符号，并指向其分配的内存

修改变量时，做了以下事情

* 在符号表中查找名为foo的符号
* 取得其指向的地址
* 赋新值

##### ld script创建变量的过程

在ld script中这样定义

```
foo = 1000
```

只创建了一个叫foo的符号，它存放的地址为1000，但实际上并不会分配内存给foo

因此，ld script中的变量**只能作为一个标号引用它的地址**

##### 在C中使用ld script变量

上述内容说明了ld script变量作为外部变量，**只能当右值使用**

ld script

```
start_of_ROM   = .ROM;
end_of_ROM     = .ROM + sizeof (.ROM);
start_of_FLASH = .FLASH;
```

C

```c
extern char start_of_ROM, end_of_ROM, start_of_FLASH;
memcpy (& start_of_FLASH, & start_of_ROM, & end_of_ROM - & start_of_ROM);
```

这里的start_of_flash实际上没有任何东西，但&start_of_flash是flash起始地址

同样，按照C标准可以这样引用

```c
extern char start_of_ROM[], end_of_ROM[], start_of_FLASH[];
memcpy (start_of_FLASH, start_of_ROM, end_of_ROM - start_of_ROM);
```

### SECTIONS

用于指定如何将输入的section映射到输出section，及如何将输出的section放入内存

#### 格式

```
SECTIONS
{
	sections-command
	...
}
```

sections-command:

* ENTRY命令
* 符号赋值语句
* 输出section描述语句
* ovelay描述语句

若ld script中没有SECTION语句，ld将会按照obj文件中section的顺序到复制到输出文件

#### 输出section描述语句

告诉ld在内存中如何布局输出的section

**section名之后的空格是必须的**

```
section [address] [(type)] :
    [AT(lma)]
    [ALIGN(section_align) | ALIGN_WITH_INPUT]
    [SUBALIGN(subsection_align)]
    [constraint]
    {
        output-section-command
        output-section-command
        ...
    } [>region] [AT>lma_region] [:phdr :phdr ...] [=fillexp] [,]
```

output-section-command可能会是下列的语句

* 符号赋值
* 输入section描述语句
* 输出section数据
* 输出section关键字

#### 输出section名

输出section名位于输出section描述语句的*section*，名字必须符合输出文件格式对应的规范

#### 输出section地址

地址于*address*指定，为可选项

若未指定address，则按如下规则选定地址

* 若指定了section对应的memory region，则会复制到下一个可用的地址
* 若MEMORY中定义了多个memory region，section会被复制到第一个可用的且**属性相同**的memory region
* 如果没有指定memory region，或没有适合section存放的memory region，section输出的地址将基于当前地址指针

##### 例子

```
.text . : { *(.text) }
```

和

```
.text : { *(.text) }
```

不同，第一个指定了address为. ，.text段将被放到当前地址指针

第二个没有指定address，因此.text地址将为当前地址对齐后的地址

```
.text ALIGN(0x10) : { *(.text) }
```

这里指定了按照当前地址，内存对齐为0x10，对齐后的地址（ALIGN函数返回值为当前地址按照对齐规则对齐后的地址）

#### 输入section描述语句

告诉ld如何将输入section映射到输出section

##### 基础

###### 指定某个文件某个段

```
data.o(.data)		data.o中的.data段
data.o				data.o中的所有段
```

###### 指定多个文件的某个段

通配符和EXCLUDE_FILE

```
*(.text)		所有文件的.text段
EXCLUDE_FILE (*crtend.o *otherfile.o) *(.ctors)		除了crtend.o和otherfile.o外所有文件的.ctors段
*(EXCLUDE_FILE (*crtend.o *otherfile.o) .ctors)		与上面完全相同，写法不同
```

###### 指定多个段

```
*(.text) *(.rdata)
*(.text .rdata)			两者一样，区别只在于被链接器处理时，前者的顺序是先.text后.rdata，后者两者同时处理
						最后到输出文件时可能导致两个段地址不一样
```

###### 指定多个文件多个段

```
*(EXCLUDE_FILE (*somefile.o) .text .rdata)	除somefile.o的.text外的所有.text .rdata段
*(EXCLUDE_FILE (*somefile.o) .text EXCLUDE_FILE (*somefile.o) .rdata)
										除somefile.o的.text和somefile.o的.rdata外所有.text .rdata段
EXCLUDE_FILE (*somefile.o) *(.text .rdata)	同上，这里体现了EXCLUDE_FILE在括号内外的区别
```

###### 指定段属性

根据ELF section header给出的段属性选取段

```
SECTIONS {
  .text : { INPUT_SECTION_FLAGS (SHF_MERGE & SHF_STRINGS) *(.text) }
  .text2 :  { INPUT_SECTION_FLAGS (!SHF_WRITE) *(.text) }
}
```

* .text  所有有SHF_MERGE和SHF_STRINGS属性的段
* .text2  所有没有SHF_WRITE属性的段

###### 指定.a文件中的.o

```
libc.a:printf.o(.text)		匹配libc.a里printf.o的.text
libc.a:(.text)				匹配libc.a里所有文件的.text
:file.o(.text)				匹配所有非.a文件的.text
```

注意，':'前如果只有一个字母可能被识别为盘符

##### 通配模式

* \*  任意个字符
* ?  匹配一个字符
* [chars]  类似正则
* \ 

除了\*外其他通配符不会匹配/

模式串只会对使用INPUT或命令行指定的文件进行匹配，不会查找某个目录下文件

如果某个文件在第一个section模式中被匹配，在第二个section模式中被再次匹配，只会使用第一个的结果

```
.data : { *(.data) }
.data1 : { data.o(.data) }
```

如上，第二个规则不会被使用，因为data.o在第一个规则中就被匹配了

###### 排序

一般来说，输出段中各文件对应段的顺序会按照输入文件的顺序排列，但可以通过SORT_BY函数改变顺序

* SORT_BY_NAME
* SORT_BY_ALIGNMENT  根据对齐排序，对齐值高的在前，低的在后
* SORT_BY_INIT_PRIORITY  根据gcc的init_priority属性排序

###### 嵌套

```
SORT_BY_NAME (SORT_BY_ALIGNMENT (wildcard section pattern))	先按名字排，若有同名按alignment排
SORT_BY_ALIGNMENT (SORT_BY_NAME (wildcard section pattern))	与上面相反
SORT_BY_NAME (SORT_BY_NAME (wildcard section pattern)) 	与SORT_BY_NAME (wildcard section pattern)相同
SORT_BY_ALIGNMENT (SORT_BY_ALIGNMENT (wildcard section pattern)) 
										与SORT_BY_ALIGNMENT (wildcard section pattern)相同
其他嵌套形式都是非法的
```

###### 输出map文件

可以使用-M选项输出map文件

###### 例子

```
SECTIONS {
  .text : { *(.text) }
  .DATA : { [A-Z]*(.data) }
  .data : { *(.data) }
  .bss : { *(.bss) }
}
```

所有的.text段和.bss段对应输出文件的.text和.bss，所有大写字母开头的文件的.data对应输出文件的.DATA，所有不是大写字母开头的文件的.data对应输出文件的.data

##### COMMON段

有些输入文件不存在COMMON段，链接器会把这些文件的common符号视作存放在这些文件的COMMON段中

一般来说，输入文件的COMMON段和bss段被放入输出文件的bss段

```
.bss { *(.bss) *(COMMON) }
```

有的文件会将COMMON段分类，如MIPS ELF文件将common符号分为标准common符号和small common符号，前者被放在COMMON段，后者被放在.scommon段

在一些旧的链接脚本中*(COMMON)也被写作[COMMON]

##### KEEP垃圾回收

当链接时的垃圾回收选项（--gc-sections）被使用时，KEEP用于标记不能被忽略的段，如

```
KEEP(*(.init))					保留.init
KEEP(SORT_BY_NAME(*)(.ctors))	保留.ctors
```

###### \_\_start\_和\_\_stop\_

当一个段名符合C语言变量名规范时（没有.的段），链接器会自动生成\_\_start\_xxx和\_\_stop\_xxx来标记xxx段的开始和结束

##### 例子

```
SECTIONS {
  outputa 0x10000 :
    {
    all.o
    foo.o (.input1)
    }
  outputb :
    {
    foo.o (.input2)
    foo1.o (.input1)
    }
  outputc :
    {
    *(.input1)
    *(.input2)
    }
}
```

outputa位于地址0x10000，内容是all.o的所有段和foo.o的.input1段

outputb内容是foo.o的.input2段和foo1.o的.input1段

outputc内容是其他文件的.input1段和.input2段

#### 输出section数据

##### 数据类型

BYTE  SHORT  LONG  QUAD  SQUAD

##### 长度

1字节  2字节  4字节  8字节  8字节有符号

当目标机为32位时，QUAD和SQUAD值为当前值的32位表示拓展到64位的值

#####  使用

 LONG(addr)		此时在当前地址存放addr的值，长度4字节，并且将地址指针+4

##### 例子

```
SECTIONS { .text : { *(.text) } LONG(1) .data : { *(.data) } }
SECTIONS { .text : { *(.text) ; LONG(1) } .data : { *(.data) } } 
```

两者结果相同

##### FILL

FILL指令用于指定内存填充值，如两个段间因内存对齐而闲置的内存值。

如想填充为0x90：FILL(0x90909090)

注意：FILL只能出现在section块内，且仅对FILL后的section起作用（对比输出section的=fillexp属性）

#### 输出section关键字

列出了输出section描述语句中的output_section_command

##### CREATE_OBJECT_SYMBOLS

告诉链接器为每个输入文件创建一个符号，符号名为文件名，符号所在的section为调用CREATE_OBJECT_SYMBOLS函数的section

##### CONSTRUCTORS

当输入文件为a.out格式或ECOFF/XCOFF这种不允许创建任意个段的格式时，C++全局构造器和析构器会被链接器识别并放在CONSTRUCTORS指定的段中

`__CTOR_LIST__`和`__CTOR_END__`标志构造器段的开始和结束，`__DTOR_LIST__`和`__DTOR_END__`标志析构器段的开始和结束

###### 手动构造全局构造器和全局析构器段

```
__CTOR_LIST__ = .;							标志构造器起始
LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)	构造器入口数
*(.ctors)									构造器入口
LONG(0)										0
__CTOR_END__ = .;							标志构造器末尾
__DTOR_LIST__ = .;							标志析构器起始
LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)	析构器入口数
*(.dtors)									析构器入口
LONG(0)										0
__DTOR_END__ = .;							标志析构器末尾
```

###### C++对于构造器和析构器的调用

一般来说全局构造器在__main中在调用main前被调用，析构器在atexit中被调用，atexit被exit调用

若要控制构造器和析构器的调用顺序，可以使用SORT_BY排列

一般说来上述操作若非指定，链接器可以自动处理

#### 输出section discarding

##### 一般规则

对于输入文件都不存在的段，ld不会创建对应的输出段，但若output-section-command中有分配空间语句则链接器将始终创建该段。对当前地址的赋值（.）也会导致链接器始终创建该段，即使语句没有改变.的值

如 . = . 将使该输出段被始终创建，因此可以用这个方法使链接器创建空段

##### /DISCARD/

/DISCARD/段对应的输出section描述语句指定的输入section将不会被放入输出section

#### 输出section属性

```
section [address] [(type)] :
  [AT(lma)]
  [ALIGN(section_align) | ALIGN_WITH_INPUT]
  [SUBALIGN(subsection_align)]
  [constraint]
  {
    output-section-command
    output-section-command
    …
  } [>region] [AT>lma_region] [:phdr :phdr …] [=fillexp]
```

上面讲了*section* *address* 和 *output-section-command*，下面介绍其他属性

##### Type

* NOLOAD  执行时不需要被加载进内存
* DSECT
* COPY
* INFO
* OVERLAY  以上四个属性用于兼容，很少使用，它们的共同特点是标记该段为not allocatable，运行时不分配内存

默认情况下输出文件type将与输入文件相同

##### LMA

每个段都有VMA(virtual memory address)和LMA(load memory address)

VMA由*address*指定，LMA由AT/AT>lma_region指定

```
AT expression
AT> memory region
```

若没有指定AT，则判断规则如下

* 如果有指定VMA，则使用相同地址
* 若段是非allocatable的，则LMA被设置为VMA
* 若该段能找到合适大小的memory region，且该region包含至少一个段，LMA被如下设置：上一个段的LMA-VMA == 当前段的LMA-VMA
* 若ld script没有定义memory region，则默认定义一个包含整个地址空间的memory region
* 若没有合适的memory region，且未指定VMA，则设置以当前段默认的VMA作为LMA

###### 例子

```
SECTIONS
  {
  .text 0x1000 : { *(.text) _etext = . ; }
  .mdata 0x2000 :
    AT ( ADDR (.text) + SIZEOF (.text) )
    { _data = . ; *(.data); _edata = . ;  }
  .bss 0x3000 :
    { _bstart = . ;  *(.bss) *(COMMON) ; _bend = . ;}
}
```

mdata段虽然VMA为0x2000，但LMA为.text后第一个对齐的内存

可以用下面的C程序把text复制到mdata段，并清空bss

```c
extern char _etext, _data, _edata, _bstart, _bend;
char *src = &_etext;
char *dst = &_data;

/* ROM has data at end of text; copy it.  */
while (dst < &_edata)
  *dst++ = *src++;

/* Zero bss.  */
for (dst = &_bstart; dst< &_bend; dst++)
  *dst = 0;
```

##### ALIGN ALIGN_WITH_INPUT 指定输出align

可以通过**ALIGN**指定输出的alignment，**只能指定比当前alignment大的值**

可以使用**ALIGN_WITH_INPUT**保证VMA和LMA有相同的alignment，即段在VMA和LMA的内存上其处处差值相同

##### SUBALIGN 指定输入align

使用**SUBALIGN**，可以强制指定输出文件对齐，不管输入文件的对齐规则，且可以指定比当前aligment小的值

##### ONLY_IF_RO ONLY_IF_RW指定输出约束

可以使用**ONLY_IF_RO**指定只有在所有输入段为只读时才生成输出段

同理有**ONLY_IF_RW**

##### >region output section region

可以用**>region**指定section输出的memory region（注意与AT>区别）

```
MEMORY { rom : ORIGIN = 0x1000, LENGTH = 0x1000 }
SECTIONS { ROM : { *(.text) } >rom }
```

##### :phdr 输出section phdr

可以使用**:phdr**将一个section分配到一个program segment中（见PHDRS，一个是链接视图（section）一个是运行视图（segment））

如果一个section被分配到一个或多个segment中，则后面一连串没有指定segment的section都会被放到这些segment中

可以使用**:NONE**来表明一个section不需要被放到一个segment中

```
PHDRS { text PT_LOAD ; }
SECTIONS { .text : { *(.text) } :text }
```

定义一个名为text的segment，属性为PT_LOAD

输入section为`*(.text)`，输出section为`.text`，输出segment为`text`

##### =fillexp 填充

与**FILL**相同，用于将section中空余内存填充为指定值

```
SECTIONS { .text : { *(.text) } =0x90909090 }
```

#### 内存覆盖描述

```
OVERLAY [start] : [NOCROSSREFS] [AT ( ldaddr )]
  {
    secname1
      {
        output-section-command
        output-section-command
        …
      } [:phdr…] [=fill]
    secname2
      {
        output-section-command
        output-section-command
        …
      } [:phdr…] [=fill]
    …
  } [>region] [:phdr…] [=fill] [,]
```

### MEMORY

用于定义memory region（内存区域）

内存区域用于指定那些链接器必须分配的内存地址和不可以分配的内存地址

链接器会在MEMORY的分配情况的基础上分配各SECTION的内存

#### 格式：

```
MEMORY
{
	name [(attr)] : ORIGIN = origin, LENGTH = len
	...
}
```

#### attr

* R  read-only section
* W  read-write section
* X  executable section
* A  allocatable section
* I  initialized section
* L  与I相同
* !  对后面的属性取非（如!X = 不可执行）

#### ORIGIN

ORIGIN  org  o  指定内存起始地址

#### LENGTH

LENGTH  len  l  指定内存区大小

#### 获取起始地址/大小

```
ORIGIN(memory_region)
LENGTH(memory_region)
```

#### 链接器分配SECTION和对应MEMORY的规则

如果输入文件的section未被明确指定是否对应输出文件的section，链接器会根据section的属性寻找对应的内存，若存在属性相同的内存区域则会将该section存入

若没有为SECTION指定输出的地址，链接器会将地址指定为当前内存区域的下一个可用地址

#### 例子

```
MEMORY
  {
    rom (rx)  : ORIGIN = 0, LENGTH = 256K
    ram (!rx) : org = 0x40000000, l = 4M
  }
```


### PHDRS

ELF格式专用，用于创建合适的ELF程序头(program header)

当链接器在链接脚本中读取到PHDRS指令，除了PHDRS指定的program header外其他的header都将不会被创建

将一个section映射到多个segment是正常的，在output section语句中使用**:phdr**指定多个segment可以实现该功能

```
PHDRS
{
    name type [ FILEHDR ] [ PHDRS ] [ AT ( address ) ] [ FLAGS ( flags ) ] ;
}
```

#### name

用于指定section名，section名、文件名、符号名和program header名处于不同的名字空间中

#### type

program header的type指定了装载器将怎么把segment映射到内存中

##### PT_NULL

表示一个未使用的program header

##### PT_LOAD

表示一个需要被加载的segment

##### PT_DYNAMIC

表示在该段可以找到动态链接库的信息

##### PT_INTERP

表示在该段可能可以找到程序使用的解释器的信息

##### PT_NOTE

表示该段保存了note信息

##### PT_SHLIB

保留选项，ELF标准未定义具体意义

#####PT_PHDR

表示该段有program header

##### PT_TLS

表示该段有TLS（线程本地存储） 

#### FILEHDR

关键字，用于指定该段需包含ELF文件头

#### PHDRS

关键字，用于指定该段需包含program header

**第一个拥有PT_LOAD属性的段本身或者之前FILEHDR和PHDRS必须被包含过，见下面示例**

#### AT

这里也可以用AT指定加载位置，且将覆盖output section中的AT选项

#### FLAGS

用于设置对应program header的p_flags字段

#### 例子

```
PHDRS
{
  headers PT_PHDR PHDRS ;
  interp PT_INTERP ;
  text PT_LOAD FILEHDR PHDRS ;
  data PT_LOAD ;
  dynamic PT_DYNAMIC ;
}		/*定义5个segment，其中PHDRS放在headers和text中，FILEHDR放在text中*/

SECTIONS
{
  . = SIZEOF_HEADERS;
  .interp : { *(.interp) } :text :interp		/*指定了两个segment作为输出*/
  .text : { *(.text) } :text
  .rodata : { *(.rodata) } /* defaults to :text */
  …
  . = . + 0x1000; /* move to a new page in memory */
  .data : { *(.data) } :data
  .dynamic : { *(.dynamic) } :data :dynamic
  …
}
```

### 表达式

#### 常量

* 0开头  八进制
* o结尾  八进制
* 0x开头  16进制
* h结尾  16进制
* b结尾  二进制
* d结尾  十进制

以上均可替换为大写

* K  *1024
* M  \*1024\*1024

#### 符号常量

使用**CONSTANT**引用符号常量

符号有

* MAXPAGESIZE  页最大长度
* COMMONPAGESIZE  页默认长度

```
.text ALIGN (CONSTANT (MAXPAGESIZE)) : { *(.text) }
```

#### 孤立段

孤立段（翻译其实是孤儿段 = =）表示那些存在于输入文件且没有被链接脚本指定的段，链接器将按照一定规则将这些段放入对应输出段中

* 若输入段名与输出段相同则放入对应输出段
* 若输入段名没有匹配的输出段，则会创建一个名字相同的输出段，当多个文件有同名输入段时将放入同个同名输出段
* 新的输出段将将首先放在有相同属性的输出段之后，如果没有相同属性的段，则将放在文件末尾

#### 运算符

与C一样

#### 地址计数器

`.`总是表示当前的输出段的地址，它只会出现在SECTION中

`.`表示的地址为偏移地址，当直接用在SECTION结构中，因为默认SECTION以地址0为开头，所以可以当做绝对地址。但若用在section描述语句中则偏移量是相对当前输出section的

##### 例1

```
SECTIONS
{
  output :
    {
      file1(.text)		/*file1 .text段放在output段开头*/
      . = . + 1000;		/* 1000 padding */
      file2(.text)		/* file2 .text段放在 output开头+sizeof(file1 .text)+1000
      . += 1000;		/* 1000 padding */
      file3(.text)		/* file3 .text *、
    } = 0x12345678;		/* padding的值为0x12345678 */
}
```

##### 例2

```
SECTIONS
{					/* 默认地址0 */
    . = 0x100		/* 地址0x100 */
    .text: {
      *(.text)
      . = 0x200		/* .text段末尾相对段开头为0x200，即段大小为0x200 */
    }
    . = 0x500		/* 设置段开头为0x500，这时.的相对偏移量又从0开始计算（因为在section描述语句外） */
    .data: {
      *(.data)
      . += 0x600	/* 在.data段内容结束后还有0x600字节预留空间 */
    }
}
```

##### 一个tip

```
SECTIONS
{
    start_of_text = . ;
    .text: { *(.text) }
    end_of_text = . ;

    start_of_data = . ;
    .data: { *(.data) }
    end_of_data = . ;
}
```

这里定义了用于标识text段和data段开头末尾的符号

但实际上，因为链接器无法将这些符号与对应段实际联系起来，因此极有可能在符号和段之间安插一些链接脚本中没有指定应该放在哪里的段，如.rodata。因此实际上链接脚本执行的语句可能如下

```
SECTIONS
{
    start_of_text = . ;
    .text: { *(.text) }
    end_of_text = . ;

    start_of_data = . ;
    .rodata: { *(.rodata) }
    .data: { *(.data) }
    end_of_data = . ;
}
```

即，因为链接器无法将start_of_data符号定义和.data段的分配对应起来，而是仅作为两个链接脚本的语句处理，因此可能在start_of_data和end_of_data之间安插.rodata，因为链接脚本中并没有指定.rodata的位置

因此可以用下述方法解决

```
SECTIONS
{
    start_of_text = . ;
    .text: { *(.text) }
    end_of_text = . ;

    . = . ;
    start_of_data = . ;
    .data: { *(.data) }
    end_of_data = . ;
}
```

因为链接器会将对`.`的赋值视作声明一个段的起始地址，因此不会在这条指令和下一个段声明中安插其他内容

（但end_of_data不会受影响么？）

#### 内建函数

##### ABSOLUTE(exp)

返回表达式的绝对值

##### ADDR(section)

返回section的VMA，section必须在调用函数前被创建

```
SECTIONS { …
  .output1 :
    {
    start_of_output_1 = ABSOLUTE(.);
    …
    }
  .output :
    {
    symbol_1 = ADDR(.output1);
    symbol_2 = start_of_output_1;
    }
… }
```

