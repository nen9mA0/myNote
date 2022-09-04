http://phrack.org/issues/58/7.html

## utils

### rip

用于修改目标程序的重定位表

* 打开第一个参数对应的文件，获取.text段的偏移

* 从.text段读取两个uint，分别为size和bss_size（这是当年.text段的布局方式么）

* 拷贝.text段偏移开始的 size-bss_size字节

* 将拷贝的这些字节保存到第二个参数对应的文件中

* 获取第一个参数文件的重定位信息，写入第二个参数对应的文件中。注意，若存在 R_386_32 类型的重定位符号，且重定位符名字为.text，则失败（不太知道为什么不支持.text使用R_386_32方式重定位，R_386_32的重定位方式是绝对地址重定位）

### bin2hex

数据格式转换

### parser

用来格式化输出

## 主要程序

### 配置与数据结构

##### config_struc

```c
struct  config_struc {
        uchar   magic[8];
        uchar   hs[32];
        uchar   pwd[32];
        uchar   home[64];
};
```

* hs  一个魔数，用来指定当前是否采用silent模式

##### idt/idtr

```c
struct idtr {
        unsigned short  limit;
        unsigned int    base;
} __attribute__ ((packed));

struct idt {
        unsigned short  off1;
        unsigned short  sel;
        unsigned char   none, flags;
        unsigned short  off2;
} __attribute__ ((packed));
```

idtr用于指示idt地址，高32位为基地址，低16位为表长度

每个idt表项结构如下：

| off2     | none flags | sel  | off1     |
| -------- | ---------- | ---- | -------- |
| 回调函数高16位 | 一些flag     | 段选择子 | 回调函数低16位 |

### 主要函数

#### kernel

底层功能，用于读写`/dev/kmem`文件

##### rkm

从kmem读取数据

```c
static inline int rkm(int fd, int offset, void *buf, int size);
```

* lseek到offset位置

* 读取size长度的内容

##### wkm

向kmem写入数据

```c
static inline int wkm(int fd, int offset, void *buf, int size);
```

- lseek到offset位置

- 写入size长度的内容

##### rkml

读取ulong长度的数据

##### wkml

写入ulong长度的数据

##### img_reloc

打印新映射的image地址

```c
int     img_reloc(void *img, ulong *reloc_tab, ulong reloc);
```

#### loc

用于获取内核的各种符号地址，包括

* 系统调用表

* 内核符号

* kamlloc

##### loc_rkm

与 [rkm](#rkm) 一样

##### get_sct

获取系统调用表地址

```c
ulong   get_sct(ulong *i80);
```

* 打开 `/dev/kmem`

* 使用sidt读取 [idt](#idt/idtr) 表地址

* 读取调用号0x80的idt表项（linux系统调用对应的idt表）

* 获取0x80对应中断处理函数的机器码

* 判断机器码中是否存在 `ff 14 85` （`call dword ptr [eax*4+offset]`）

* 若存在，则将该机器码中对应offset部分的地址返回

这里因为从idt表找到的函数只是int 0x80对应的中断处理函数，而要获得系统调用表就需要在中断处理函数中找。作者发现了从中断处理函数执行系统调用总是使用上述的call形式，所以系统调用表基地址为offset

##### get_sym

获取某个内核符号的地址

```c
ulong   get_sym(char *n);
```

* 调用 get_kernel_syms 获取符号个数

* 若符号个数大于MAX_SYMS或小于0直接返回

* 调用 get_kernel_syms 获取全部符号

* 遍历符号表，若有同名符号则返回其地址

##### get_kma

```c
ulong   get_kma(ulong pgoff);
```

* 若可以通过 [get_sym](#get_sym) 找到kmalloc符号，则直接返回其地址

* 否则打开kmem文件进行爆破
  
  * 从pgoff+0x100000开始，到pgoff+0x1000000进行搜索
    
    * 扫描 push - push - call 序列
    
    * 若扫描到序列，获取call的地址
    
    * 比较一个记录表中是否存在同样的地址，若不存在则存入表中，若存在则将该记录的引用数加一

* 返回引用数最高的一个地址

#### gfp

##### get_gfp

通过判断内核版本来获取kmalloc的 `GFP_KERNEL` flag的值

#### client

这里是后门中带的client，该rootkit还自带了另一个client程序，应该是给攻击者使用的

##### skio

#### sk

主程序

##### main

* 比较输入命令行的最后几个字符与 [config_struc](#config_struc) 的hs
  
  * 若相等则采用silent模式，将stdin stdout stderr都导向 `/dev/null`
  
  * 调用 [fucka_is_there](#fucka_is_there) 检测当前是否已经有同样后门在运行

* 若不为silent模式，且用fucka_is_there检测不到同样后门，则返回一个 [client](#client)

* 调用 [get_sct](#get_sct) 获取系统调用表地址

* 调用 [get_kma](#get_kma) 获取kmalloc地址

* 保存原来的系统调用表内容

* 单独

### 其他程序

#### vsprintf

自己实现的一整套printf库，没啥好看的其实，顺便写一下

##### strto系列

* simple_strtol  字符串转long

* simple_strtoll  字符串转long long

* simple_strtoul  字符串转unsigned long

* simple_strtoull  字符串转unsigned long long

##### number

将传入的数字格式化为需要的类型再传出

```c
static char * number(char * buf, char * end, long long num, int base,
                     int size, int precision, int type);
```

* buf  传入的缓冲区

* end  缓冲区末尾标记

* num  要转换的数字

* base  数字的基

* size  缓冲区大小

* precision  数字的位数

* type  转换的类型

其中进制转换支持到36进制，转换类型有以下几种

* ZEROPAD  空余字符是否填充0

* SIGN  是否有符号

* PLUS  是否显示正数符号

* SPACE  若为正数，是否填充一个空格（主要是与负数的负号对齐，这种情况下不会显示正数符号）

* LEFT  是否左对齐

* SPECIAL  针对8和16进制的特殊处理

* LARGE  数中的字母是否用大写（如0xfe还是0xFE）

##### vsnprintf

```c
int vsnprintf(char *buf, unsigned int size, const char *fmt, va_list args);
```

* 首先根据buf和size计算出缓冲区末尾的地址，这里有对指针溢出的处理

* 开始扫描格式化字符串
  
  * 若当前扫描的字符不是%
    
    * 若还没到缓冲区末尾，则直接赋值给缓冲区
  
  * 死循环处理读取几个格式控制字符 `+ - # 0 空格` 并设置格式的type（对应上面number的type）
  
  * 若下一个字符为数字，则是用于指定输出长度的，读取该数字（如`%8s`）
  
  * 若下一个字符为 `*` ，则长度是由输入的参数指定的，读取一个参数来获取长度。若长度为负则令其为正，并左对齐
  
  * 若下一个字符为 `.` ，则是用于指定精度的
    
    * 若下个字符为数字则读取该数字作为精度
    
    * 若下个字符为 `*` 则精度由输入参数指定，读取一个参数作为精度
  
  * 若下一个字符为 `h l L Z` ，则作为输入参数的数据类型限定符，再处理一下 `ll` 的情况（long long）
  
  * 下面用来处理格式化字符串的数据类型说明符
    
    * c  字符
      
      * 先处理左对齐的情况，若左对齐则填充
      
      * 读取一个参数到缓冲区
      
      * 处理右对齐的情况
      
      * 继续下一轮循环
    
    * s  字符串
      
      * 若为空输出 `<NULL>`
      
      * 处理左对齐
      
      * 读取参数的字符串到缓冲区
      
      * 处理右对齐
      
      * 继续下一轮循环
    
    * p  指针
      
      * 调用number格式化参数中的指针并返回，其中基为16
      
      * 继续下一轮循环
    
    * n  当前输出的字符数量
      
      * 若数据类型限定符为 `l` ，则将指针转换为 `long *` 并赋值
      
      * 若数据类型限定符为 `Z` ，则将指针转换为 `unsigned int *` 并赋值
      
      * 正常按 `int *` 赋值
      
      * 继续下一轮循环
    
    * %  `%%`的情况，是%的转义
      
      * 直接往缓冲区写入一个 %
      
      * 继续下一轮循环
    
    * o  指定格式为8进制
      
      * 基=8
    
    * X  指定格式为16进制，且为大写
      
      * 基=16，type加上大写的属性
    
    * x  指定格式为16进制
      
      * 基=16
    
    * d i  type加上有符号的属性
    
    * u  无操作
    
    * 默认  对于不属于上述格式化输出类型的字符
      
      * 往缓冲区原样写入 `%+字符` ，这里有一个溢出的处理
      
      * 继续下一轮循环
  
  * 上面确定了输出的格式和数据类型，只有`o X x d i u` 能到达这里，下面根据数据类型限定符读入参数
    
    * 若为 `L` 按 long long读入
    
    * 若为 `l` 按 unsigned long读入，并根据处理是否为有符号数的情况
    
    * 若为 `Z` 按 unsigned int读入
    
    * 若为 `h` 按unsigned short读入，并根据处理是否为有符号数的情况
    
    * 其他情况按unsigned int读入，并根据处理是否为有符号数的情况
  
  * 调用 number 格式化数字

* 为字符串添加00截断

##### snprintf vsprintf sprintf

都是对vsnprintf的封装

##### vsscanf

```c
int vsscanf(const char * buf, const char * fmt, va_list args);
```

* 扫描格式化字符串
  
  * 若isspace则继续下一轮扫描
  
  * 若不为%
    
    * 若格式化字符串的下一个字符不等，则直接**返回当前读入的个数**
    
    * 否则继续下一轮扫描
  
  * 若为*
    
    * 循环到格式化字符串和输入中下一个空格的位置，进行下一轮扫描
  
  * 若为数字，则该数字为输入的宽度，读入该宽度
  
  * 若为 `h l L Z` ，则该字符指定了输入的数据类型，读入该类型
  
  * 进入解析格式化数据的循环
    
    * c，即读入字符
      
      * 读取一个参数，并根据输入的宽度为它赋值
      
      * 继续下一轮解析格式化数据
    
    * s，即读入字符串
      
      * 读取一个参数，跳过输入buf的空格部分，并根据输入的宽度为它赋值
      
      * 继续下一轮解析格式化数据
    
    * n，即当前已扫描的输入长度
      
      * 读取参数并赋值
      
      * 继续下一轮解析格式化数据
    
    * o，8进制数
      
      * 修改base，跳出解析循环
    
    * x X，16进制数
      
      * 修改base，跳出解析循环
    
    * d i，有符号数
      
      * 修改符号，跳出解析循环
    
    * u，无符号数
      
      * 直接跳出解析循环
    
    * %，`%%`的情况
      
      * 若下一个字符不为%，则**返回当前读入的个数**
      
      * 否则继续下一轮解析格式化数据
    
    * 默认
      
      * **返回当前读入的个数**
  
  * 首先跳过缓冲区中的空格
  
  * 下面开始处理各种数字的读入
    
    * h，short类型
      
      * 根据符号按short或unsigned short处理
    
    * l，long类型
      
      * 根据符号按long或unsigned long处理
    
    * L，long long类型
      
      * 根据符号按long long或unsigned long long处理
    
    * Z，unsigned int类型
      
      * 直接处理unsigned int
    
    * 默认
      
      * 根据符号按int或unsigned int处理
  
  * 读入个数加一

* **返回读入个数**
