### 概念与解释

#### 数据结构

##### arena

###### arena内存布局

初始化后arena的内存布局如下

```
				高地址
 ----------------
|	top chunk	|
 ---------------- <---------\
|	pad			|           |
 ----------------            |
|	malloc_state | -- top --/
 ---------------- <------------\
|	heap_info	| -- ar_ptr --/
 ----------------    
 				低地址
```

* pad部分可能有可能没有，这段pad是为了保证top chunk的memory部分与MALLOC_ALIGNMENT对齐，即`chunk2mem(p_top_chunk)`是MALLOC_ALIGNMENT对齐的

##### chunk

chunk使用一种称为“边界标签”的方法来管理（技术细节见ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps 这篇paper，在该目录下的allocsrv有关于这篇文章的笔记）

每个空闲的chunk的大小都会在chunk的开头和末尾被记录，这使得我们将多个空闲碎片合并为一个大空间时的速度较快。此外，size成员还有一些标记位来指明当前chunk是否正在被使用

```
    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of previous chunk, if unallocated (P clear)  |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of chunk, in bytes                     |A|M|P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             User data starts here...                          .
	        .                                                               .
	        .             (malloc_usable_size() bytes)                      .
	        .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             (size of chunk, but used for application data)    |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of next chunk, in bytes                |A|0|1|
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

chunk指针指向的是malloc代码里的管理结构malloc_chunk，mem指向的内容就是当用户调用malloc函数时返回的指针。nextchunk指向下一个malloc_chunk结构

chunk一般是符合一些对齐规则的，因此返回的mem也是对齐的。mem对齐至少是双字对齐（8字节）

空闲chunk被存放在一个循环双链表

```
    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of previous chunk, if unallocated (P clear)  |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                     |A|0|P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Forward pointer to next chunk in list             |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Back pointer to previous chunk in list            |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Unused space (may be 0 bytes long)                .
	        .                                                               .
	        .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of next chunk, in bytes                |A|0|0|
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

P（PREV_INUSE）位表示前一个chunk正在被使用，若该位被清零（即前一个chunk没有被使用），那么[malloc_chunk](#malloc_chunk)的mchunk_prev_size记录了前一个chunk的大小。对于top chunk，该位总是被设置。若P被设置，则无法直接通过mchunk_prev_size获取前一个chunk的大小

A（NON_MAIN_ARENA）位表示当前chunk没有在main_arena上。在程序刚开始运行时，内存池只有一个main_arena。当其他线程被创建时，每个线程都有自己的thread_arena，因此这些使用thread_arena的chunk的A位会被设置。若想获取这些不在main_arena上的chunk，应该在[heap_info](#heap_info)的ar_ptr成员上调用[heap_for_ptr](#heap_for_ptr)

注意，每个malloc_chunk的第一个成员是prev_size，该成员也在上一个chunk的末尾，因此对于一个空闲chunk来说，chunk末尾后的第一个元素也是该chunk的大小（即有两个位置记录了chunk大小）

此外，还有下列三种情况较为例外

* top chunk是一个特殊chunk，因为top chunk没有下一个连续的块需要进行索引，所以top chunk不会使用尾随的大小字段。malloc初始化时top chunk默认存在。当top chunk长度小于MINSIZE时，会自动扩充其大小
* 倒数第二个表示位M（IS_MMAP）表示当前chunk是否是直接mmap得到的。若该位被设置，则其他两个标志位自动被忽略，因为mmap分配的chunk既不与其他chunk相邻，也不在main_arena上。此外M位会在hooks.c的malloc_set_state被设置
* fastbin中的chunk被分配器视为已分配的块，他们只会在自身体积过大的时候被合并

##### 内部数据结构

用于保存算法内部状态的结构体主要就是malloc_state，除此之外基本没有其他用于保存内部状态的变量，除了下面两个情况

* 若定义了USE_MALLOC_LOCK，则应有一个额外的malloc_mutex定义
* 若mmap不支持MAP_ANONYMOUS参数，则应额外为mmap定义一个文件描述符

在使用了许多用于减少内部数据结构大小的技巧后，最终大约使用1K来保存malloc的内部状态（当size_t为4字节时）

##### bins

指向一系列空闲chunk的bin头的指针数组，每个bin都在一个双链表中。这些bin的大小大约符合log的关系，且这些bin的数量很多（128）。虽然看起来很浪费空间，但实际上运用得很好。这些bin的大小虽然可能并不符合大多数malloc请求的空间大小，但对于合并一系列chunk



###### bin索引

对于小于512字节的bin，





### 注释

#### malloc.c

```
这是由Doug Lea写的一个版本的malloc/free/realloc（又名ptmalloc2），并且由Wolfram Gloger做了多线程适配
在整合进glibc后，代码在很多地方做了修改，因此跟原版的ptmalloc2差别较大
```

##### Quick Start

```
为了编译这版代码，ptmalloc2提供了一个makefile，其中含有很多预定义的target，包含了一些常用的系统（如使用make posix可以以posix的线程调用来编译）。对于编译器选项来说，唯一要做的事就是通过定义USE_PTHREADS USE_THR或USE_SPROC中的一个来选择一个线程包。可以在thread-m.h文件中知道这些标志位的影响。很多系统需要额外要求定义USE_TSD_DATA_HACK，因此这个定义作为make posix的默认选项。
```

##### 为什么使用这个malloc算法

```
这个算法并非最快的、空间利用率最高的、最具可移植性的、可调性最好的malloc算法。但它是介于这几者中综合性能较为平衡的，使其适合作为一个通用的分配器。

该算法的主要特点如下
* 对于大空间（>=512B）的内存分配需求，这是一个通常通过FIFO（如使用LRU算法的FIFO）来实现的best-fit allocator。
* 对于小空间（<=64B）的内存分配需求，这是一个caching allocator，其维护了一个可以快速复用的内存块池
* 对于介于大空间和小空间的需求，它尽可能满足上述两种分配器的优点
* 对于极大空间（>=128KB）的需求，分配器直接使用系统的内存分配函数

关于更多（但可能有点过时）ptmalloc的描述可以见
http://gee.cs.oswego.edu/dl/html/malloc.html

虽然你可能已经使用过含有某个版本malloc函数的C库（比如在linux中）。但你可能也希望能使用该文件中的malloc来自定义一些设置或避免使用库函数的额外开销。
```

##### 文件内容（主要是函数列表）

```
标准函数（ANSI/SVID/...）:
    malloc(size_t n);
    calloc(size_t n_elements, size_t element_size);
    free(void* p);
    realloc(void* p, size_t n);
    memalign(size_t alignment, size_t n);
    valloc(size_t n);
    mallinfo()
    mallopt(int parameter_number, int parameter_value)
    
额外的函数:
    independent_calloc(size_t n_elements, size_t size, void* chunks[]);
    independent_comalloc(size_t n_elements, size_t sizes[], void* chunks[]);
    pvalloc(size_t n);
    malloc_trim(size_t pad);
    malloc_usable_size(void* p);
    malloc_stats();
```

##### 重要的参数

```
支持的指针长度:  4/8
支持的size_t:  4/8
    注意即使指针长度为8，size_t长度可能为4
    可以使用INTERNAL_SIZE_T宏来指定size_t长度

对齐:  2 * sizeof(size_t)
    如对于4B的size_t，对齐长度为8B。对于大多数编译器和架构来说都是合理的，但你也可以使用MALLOC_ALIGNMENT宏来指定

每个块头部的最小长度:  4/8
    每个被分配的块都有一段隐藏的存放块信息的头部

分配块的最小长度:  对于4B的指针，长度为16B（包含4B的头部）
                  对于8B的指针，长度为24/32B（包含4/8B的头部）
    当一个块被free后，12B（对于4B的指针，4B的size_t）或20B（对于8B指针，4B的size_t）或24B（对于8B的指针，8B的size_t）的额外空间将被使用，其中4B(8B)的空间用于存放当前chunk大小，8B(16B)空间用于存放free链表的指针。因此分配块最小长度为16/24/32B
    malloc(0)也会返回一个指向长度最小的分配块。
    最大的额外空间开销（例如，实际分配的空间大小减去请求的空间大小）小于等于分配块的最小长度。除了当请求大于mmap_threshold的情况（这种情况下直接使用mmap分配），使用mmap时最坏情形下的额外空间开销为 2 * sizeof(size_t) + page_size - mmap_threshold

分配块的最大长度:  对于4B的size_t: 2^32 - 2*page_size
                 对于8B的size_t: 2^64 - 2*page_size
    这里假设size_t（可能是一个unsigned类型）用于表示块的大小。虽然ISO C规定了size_t必须是unsigned，但还是有部分系统的size_t为signed。此外，即使size_t是unsigned类型，sbrk系统调用接收一个signed参数，因此可能无法接受所有的size_t值。通常来说，只有mmap能完全支持size_t的所有值。
    若试图分配一个大于允许分配长度的内存，或者系统无法分配更多内存时，malloc将返回NULL。
    
线程安全性:  线程安全
```

##### 编译选项

```
很多人在不同的系统中编译过这份代码，如各版本的unix，有时下述的各个宏会被调整。
该malloc的实现是ANSI C，并定义了很多宏。该代码应该使用优化选项进行编译，因为其可以简化宏中的表达式（FAQ：一些变量被定义为参数的形式，因为如果不这样做可能会使调试器无法正确解析）

选项							默认值
编译环境选项
  HAVE_MREMAP					0
改变默认字长
  INTERNAL_SIZE_T				size_t
配置和功能选项
  USE_PUBLIC_MALLOC_WRAPPERS	NOT defined
  USE_MALLOC_LOCK				NOT defined
  MALLOC_DEBUG					NOT defined
  REALLOC_ZERO_BYTES_FREES		1
  TRIM_FASTBINS					0
MORECORE设置选项
  MORECORE						sbrk
  MORECORE_FAILURE				-1
  MORECORE_CONTIGUOUS			1
  MORECORE_CANNOT_TRIM			NOT defined
  MORECORE_CLEARS				1
  MMAP_AS_MORECORE_SIZE			(1024*1024)
下列选项可以通过mallopt动态修改
  DEFAULT_MXFAST				64 (for 32bit)  128 (for 64bit)
  DEFAULT_TRIM_THRESHOLD		128*1024
  DEFAULT_TOP_PAD				0
  DEFAULT_MMAP_THRESHOLD		128*1024
  DEFAULT_MMAP_MAX				65536
  
还有一些其他宏，但除非你要自己修改或扩充malloc功能，否则不需要调整
```

##### 调试

因为chunk被free后，指针指向的位置会被malloc写入数据来维护释放的chunk（即[malloc_chunk](#malloc_chunk)结构体的fd_nextsize和bk_nextsize成员），因此如果用户在某块内存free后继续往该内存写数据（Use After Free），则会导致错误。

若使用`-DMALLOC_DEBUG`编译，一系列的assert会被插入，来获取更多内存错误。虽然你可能并不了解导致assert的具体原因，但这可以帮你定位错误点。这些测试覆盖面很广，并且会让程序速度显著变慢。在定义了MALLOC_DEBUG标志的情况下调用[malloc_stats](#malloc_stats)或[mallinfo](#mallinfo)将检查每个非mmap直接分配的块和free chunk。

设置MALLOC_DEBUG标志也会对你更改malloc代码有所帮助。这些assert体现了更多关于这个算法中的固定关系和假设。

设置MALLOC_DEBUG标志并不会提供一个自动化的机制，用于检查所有对malloc的内存访问是否越界。但对于这个版本或其他版本的malloc，可以有一系列的附加机制来保证这一点

##### TRIM_FASTBINS

TRIM_FASTBINS用于控制当free一个非常小的chunk时是否会马上进行trim。为true时会减少内存占用，但当程序使用很多非常小的chunk时会减慢速度

如果你希望在使用很多small chunk时，牺牲速度来节省一些内存，你可以设置该选项为true。还有一个类似的选择是把MXFAST设为0，但这将对速度造成更大影响。TRIM_FASTBINS是一个较为折中的选项，它可以禁止将那些与内存边界接壤的chunk放入fastbin中

##### MORECORE相关

###### MORECORE

MORECORE定义为从系统分配更多内存的函数的别名

###### MORECORE_FAILURE

定义为试图从系统获取内存返回错误时的值

###### MORECORE_CONTIGUOUS

若该选项为true，则假设当我们多次调用MORECORE获取内存时，返回的内存地址是连续增长的。unix的sbrk函数是满足该条件的。即使没有定义该选项，当多个区域是连续的时，malloc也允许分配跨越这些连续区域的内存。但若定义了这个选项，可以提供更好的一致性检测和空间利用率

###### MORECORE_CANNOT_TRIM

若当前系统的MORECORE函数无法处理负数参数（一般负数参数代表释放内存），定义这个标志。

###### MORECORE_CLEARS

对于MORECORE分配的内存，应如何清零

* 0 从不
* 1 只给为新分配的内存
* 2 总是

1和2的区别在于：在一些系统中，若程序先减少break值（sbrk），再增加，这段内存的内容是未定义的。

##### MMAP相关

###### MMAP_AS_MORECORE_SIZE

若sbrk失败，则使用mmap分配系统内存。该值即为使用mmap分配时最小的参数大小。该值必须是页面大小的倍数。

一般malloc仅在地址空间出现“空洞”的时候才会调用mmap，此时因为连续的地址被占用，break值无法继续增加，但系统还有空闲空间。在大多数系统中这种情况仅在程序分配非常大空间的时候才会出现。

为了防止调用过多mmap，该值应该设置得较大（默认为1024*1024）

###### HAVE_MREMAP

系统是否含有mremap函数

##### 顶层函数

###### malloc

```c
malloc(size_t n);
```

返回一个大小至少为n字节的空间，或如果空间不够时返回NULL。此外，若分配失败，errno设为ENOMEM

若n=0，则返回一个最小的chunk（32位系统16B，64位系统24或32B）。由于size_t一般是unsigned，因此若使用负数调用会作为一个超大的正整数传入

###### free

```c
free(void* p);
```

释放p指向的一个chunk，且p指向的地址是先前malloc/realloc返回的地址。若p为NULL，则不会执行任何动作。当对一个free过的chunk再次执行free会导致意想不到的结果（double free）

除非使用mallopt函数禁用该功能，否则当一块大空间被释放时，会触发自动回收机制将该空间释放回系统。

###### calloc

```c
calloc(size_t n_elements, size_t element_size);
```

返回一个`n_elements*element_size`的空间，且空间内所有值被清零

###### realloc

```c
realloc(void* p, size_t n);
```

返回一个指针，指向一个长度为n的chunk，且这块chunk的前`min(p->size, n)`字节含有与p完全相同的内容。若空间不够则返回NULL

返回值可能是p，也可能不是。算法趋向于扩展p指向的chunk的大小，若计算得到扩展后空间仍无法满足要求，则执行malloc memcpy free三个操作

若p为NULL，realloc即为malloc

若空间不够，返回NULL，errno设置为ENOMEM，p不会被free

若n比p->size小，则多出来的空间会尽量被释放并重复使用，除非定义了[REALLOC_ZERO_BYTES_FREES](#REALLOC_ZERO_BYTES_FREES)标志。若n为0，则p被调整为最小的chunk

使用mmap分配的大chunk会始终通过malloc memcpy free三个操作执行，且空间会一直增加，除非系统支持mremap函数

老的unix系统中允许最后被free的chunk被当做一个参数来调用realloc，但现在不支持了

###### memalign

```c
memalign(size_t alignment, size_t n);
```

返回一个指向大小为n的新chunk的指针，且其是根据alignment参数对齐的

alignment参数必须是2的幂。若该参数不是2的幂，则会使用大于该参数的最小的2的幂（nearest greater power）。

在普通的malloc调用中就可以保证返回的地址是8字节对齐的，因此若alignment小于等于8则不需要使用这个函数。

过度调用memalign函数会导致空间被碎片化

###### valloc

```c
valloc(size_t n);
```

等价于`memalign(pagesize, n)`，若pagesize未指定则默认为4096

###### mallopt

```c
mallopt(int parameter_number, int parameter_value);
```

用于设置一些可调整的参数。若成功返回1，否则为0

SVID/XPG/ANSI 定义了4个mallopt的标准参数（`M_MXFAST M_NLBLKS M_GRAIN M_KEEP`），但只有M_MXFAST在该版malloc中被使用

此外该版malloc还提供了4个额外参数，见下表

| Symbol           | param | default  | allowed param values          |
| ---------------- | ----- | -------- | ----------------------------- |
| M_MXFAST         | 1     | 64       | 0-80 (0 dinssables fastbins)  |
| M_TRIM_THRESHOLD | -1    | 128*1024 | any (-1U disables trimming)   |
| M_TOP_PAD        | -2    | 0        | any                           |
| M_MMAP_THRESHOLD | -3    | 128*1024 | any (or 0 if no MMAP support) |
| M_MMAP_MAX       | -4    | 65536    | any (0 disables use of mmap)  |

###### mallinfo

```c
mallinfo();
```

返回一个结构体，其中包含一系列统计信息

* arena  当前所有从系统中申请的（非mmap申请）内存大小
* ordblks  free chunk的数量
* smblks  fastbin的数量
* hblks  当前mmap区域的数量
* hblkhd  当前mmap区域的大小
* usmblks  0
* fsmblks  fastbin的大小
* uordblks  当前分配空间的总大小（包括正常分配的和mmap的）
* fordblks  总的空闲空间
* keepcost  可以使用malloc_trim释放的系统内存的最大值

由于上述所有成员都是int，但实际上一些参数是由long保存的，因此返回值可能会是溢出后的值

###### pvalloc

```c
void *__libc_pvalloc(size_t);
```

与valloc类似，只不过这里的n是向上对齐到最近的pagesize的值

###### malloc_trim

```c
malloc_trim(size_t pad);
```

若在malloc的内存池的高地址，与break边界相邻区域有未使用的内存，则试图将内存释放回系统（使用负数参数调用sbrk）。这个函数可以在free一块大内存后调用，从而减少系统内存的使用。但该调用并不能保证减少内存使用。在一些情况下，这块被free的大内存地址空间在两块被使用的chunk之间，所以不能被释放回系统。

pad参数代表在释放时需要在末尾保留的空间（即若有n字节可以释放，最终释放的应该是n-pad字节）。若这个参数是0，只有那些保存了内部数据结构的内存会被保留（大小是一个页面或更小）。这个参数主要是用来预留足够的分配空间，使得这次释放内存后，如果下次还需要申请空间时不需要再向系统申请

返回值：若返回1表示确实释放了空间，否则为0。在sbrk调用参数不能为负的系统中总是返回0

###### malloc_usable_size

```c
malloc_usable_size(void* p);
```

返回malloc分配来的chunk的实际可用空间。由于内存对齐等原因，这个空间一般大于等于调用malloc时的size参数。这块多余的内存也可以被直接使用。

这个函数在debug或者断言中比较常用，如：

```c
p = malloc(n);
assert(malloc_usable_size(p) >= 256);
```

###### malloc_stats

```c
malloc_stats();
```

在stderr中打印下述内容

* 使用mmap和sbrk分配的内存数
* 最大内存值（如果malloc_trim或munmap被调用，当前内存使用量可能比这个值少）
* 当前使用malloc/realloc分配的内存大小（未释放）。由于对齐和内部数据结构的空间开销，这个值比用户申请的内存大，且即使用户没有申请任何内存，这个值仍可能大于0

若用户程序自身通过sbrk或mmap分配了内存，这部分不会被统计进上述函数，因此要直接用这个函数来反应用户程序的内存使用量可能不准确。

malloc_stat只打印一些最常用的统计数据，更多数据可以使用mallinfo获取

###### malloc_get_state

```c
void *__malloc_get_state(void);
```

获取当前状态下malloc的变量，这里返回的数据结构是不透明的，应该只能用于malloc_set_state函数来使malloc回到某一时刻的状态

###### malloc_set_state

```c
int __malloc_set_state(void *);
```

重新设置malloc变量，与[malloc_get_state](#malloc_get_state)配合使用

##### M_MXFAST

fastbin的最大长度（小于这个大小的bin可以被加入fastbin）。其中fastbin是一系列特殊的bin，用于保存一些刚释放的chunk，在释放后不合并空间。这可以加速程序多次分配释放同样大小的块。但可能导致碎片化。

这个malloc算法在管理fastbin方面较为保守，但也高效，所以碎片化并不是一个很严重的问题。MXFAST的最大值为80。fastbin主要针对的场景是多个小内存的分配。对于大的内存请求，fastbin非但不会提高速度，反而会导致碎片化。

当M_MXFAST被设为0时fastbin将不会被使用。这将导致malloc算法更加接近于一个fifo-best-fit算法，并且会导致速度变慢

##### M_TRIM_THRESHOLD

用于指定当与break边界相邻的高地址空闲空间达到多少的时候调用free时会触发[malloc_trim](#malloc_trim)。

自动内存回收对于一个长期驻留的程序是必要的，但因为使用sbrk回收内存可能运行较为缓慢，所以有时候这个操作是很浪费时间的（如一个程序在trim后又分配了一堆大内存）。因此这个阈值应该足够大，使得其在节省内存开销的同时也不会拖慢运行速度

这个参数与[M_MMAP_THRESHOLD](#M_MMAP_THRESHOLD)两个参数应该相互权衡。trim和mmap是将空闲内存归还给系统的两种方法。正确配置这两个参数可以使得常驻程序的系统资源需求保持最低。例如在一个测试中，linux的X server使用128K作为trim threshold，192K作为mmap threshold可以使得程序开销最低

若你需要在一个常驻程序使用这个版本的malloc，有必要对这两个参数进行一些实验。粗略建议可以先设置为程序的平均内存占用大小，这样一来当程序释放这么多内存时将触发一次回收。此外，当一个程序连续分配或释放多个大内存，并多次重用这些内存时，应着重于调整trim threshold。在一个长期驻留的程序中，相对于使用mmap释放大内存，使用sbrk通常更快。

但在大多数程序中，这些参数仅仅用于防止程序占用过多无用内存。且由于sbrk、mmap或munmap通常会降低效率，因此默认阈值都会设置得比较高

trim值应该大于pagesize才能起作用。若想禁用trim，可以设置为(unsigned long)(-1)

trim与MXFAST的关系：除非TRIM_FASTBINS被设置，否则在释放一个小于等于MXFAST的chunk时不会自动trimming，直到之后有其他大内存被释放。但总是可以通过显式调用malloc_trim来回收内存

此外，在main_arena使用mmap分配的情况下，trimming并不一定可用

注意，一些人会用一些trick来预留内存，如在程序开头先malloc一块大内存再free。这种trick在开启自动trimming的情况下是无效的。

###### M_TOP_PAD

当sbrk被调用时，该参数用于指定额外需要分配的padding空间。一般在下列两种情况中被使用

* 当malloc需要调用sbrk来增加arena空间以满足新的分配请求时，这个padding将额外加在需要分配的空间上。
* 当free函数中调用malloc_trim时，该值将作为malloc_trim的pad参数传入

无论哪种情况，padding最终都会与pagesize对齐

使用padding参数的主要原因是防止过多调用sbrk函数。即使padding使用一个很小的值，它都可以极大地降低sbrk的调用概率，特别是每个程序在启动后第一次malloc时几乎都需要调用一次sbrk，padding可以显著减少这种情况的发生。

由于一般在调用sbrk时会自动对齐到pagesize，一般来说pagesize的预留空间足以降低开销，所以M_TOP_PAD的默认参数为0。但若系统调用sbrk速度极慢，应该适当增加该值。

##### M_MMAP_THRESHOLD

当一个分配请求大于当前已有空闲空间，且大于该值时，会使用mmap直接分配内存来满足该请求

当使用mmap直接分配内存时，地址空间与使用large chunk分配的空间不同，因此它们可以独立地被分配与释放。使用mmap分配的内存空间释放后将不会被其他分配请求重用（至少不会直接被重用，但有些系统在释放内存后重新分配可能趋向于分配一块与上次释放的内存相同的内存）

这种情况下分配的内存使用独立的地址空间有下列好处

* mmap分配的内存可以独立地释放回内存，可以使得内存占用率维持在较低水平
* 不会像malloc_trim中遇到的空闲块被两个正在使用块夹住导致无法释放的情况
* 对于一些有地址空间缺失的系统（即，在地址空间的中间一段被占用导致使用sbrk无法继续分配出连续空间的情况），mmap可以获取sbrk无法获取的内存

但也有下列不足

* 空间不能像正常的chunk一样被回收、合并、重用
* 由于mmap的地址对齐，可能导致很多的空间浪费
* 使得malloc的性能取决于系统内存分配的实现

从上面这几点看，对于大内存分配使用mmap的优点大于缺点。但“大”的概念可能对于不同系统不一样。这里的默认值使用了一个经验值

**Update in 2006**

上面表述是在2001年写的。现在世界变化了很多，内存变大了，程序也变大了。32位linux的内存布局也有了变化。

现在，brk和mmap的地址空间是共享的，且使用brk分配的内存不会被内核限制了。且现在程序也经常分配大于128K的内存（2001年这是很少见的）

mmap的开销依旧很大，每次glibc调用mmap，内核都需要先将内存清零再返回。清零操作会占用大量cache和内存带宽。

在2001年，brk在32位x86处理器上最高可以分配800M，此后brk将与第一个共享库的地址空间冲突。但在2.6内核中，虚拟内存布局不同了，brk和mmap都可以获取整个堆空间。

现在我们使用一个动态的mmap/brk阈值来替代之前的静态阈值，目的仍是防止地址空间碎片化。此外下列设计目的仍被保留

* 使用mmap分配常驻的大内存块
* 使用mmap分配非常大的请求

现在加上了一个新的设计目的

* 对于一个非常驻内存的分配请求，应该使用brk而不是mmap，从而避免内核花费大量时间清零内存

最终的实现是使用了一个介于最大值和最小值间的动态阈值，最大值和最小值默认为128K和32M（在64位系统中为64M）。这使得设计要求可以满足：

*  一般来说常驻内存在进程运行的早期被分配，由于此时阈值不会被调整，因此只要大于默认的128K即会使用mmap分配。而此后的分配会影响阈值，使阈值慢慢变大
* 因为定义了最大值，因此可以满足第二点

阈值调整：若程序释放了一块mmap的内存，阈值将增加。依据是当程序开始释放mmap的内存时，该块释放的内存很有可能是程序分配的临时内存。这个优化是为了满足第三个设计目的。

##### M_MMAP_MAX

mmap可以接受的最大内存分配请求。因为部分系统在mmap分配的空间超过一定范围时会影响性能

默认值仅作为一个safeguard。设置为0则表示不使用mmap处理大的内存分配请求

##### chunk相关

这块内容因为对于理解malloc结构较重要，所以移到[chunk](#chunk)

##### 内部数据结构相关

这块内容也比较重要，所以移到[内部数据结构](#内部数据结构)



### 一些外部引用的宏和函数

#### 宏

##### ALIGN_DOWN

根据size大小，对base向下取整到size的倍数，如size为4096，base为8191，则返回值为4096

注意这个算法仅对size为2^n的适用

```c
#define ALIGN_DOWN(base, size)	((base) & -((__typeof__ (base)) (size)))
```

算法原理很简单，假设size为2^n，则将低n位清零。这里`__typeof__ (base)`主要用于将size强制类型转换成与base相同的类型。-size其实等价于`~size+1`，或`~(size-1)`，实际上和常用的将size转化为掩码的算法相同（如MALLOC_ALIGNMENT转化为MALLOC_ALIGN_MASK的方法）

##### ALIGN_UP

根据size大小，对base向上取整到size的倍数，如size为4096，base为4097，则返回值为8192

注意这个算法仅对size为2^n的适用

```c
#define ALIGN_UP(base, size)	ALIGN_DOWN ((base) + (size) - 1, (size))
```



### 一些重要宏

#### 配置相关

##### 参数

######  DEFAULT_MMAP_THRESHOLD_MIN

<span id="DEFAULT_MMAP_THRESHOLD_MIN"/>

```c
#ifndef DEFAULT_MMAP_THRESHOLD_MIN
#define DEFAULT_MMAP_THRESHOLD_MIN (128 * 1024)
#endif 
```

###### DEFAULT_MMAP_THRESHOLD_MAX

<span id="DEFAULT_MMAP_THRESHOLD_MAX"/>

```c
#if __WORDSIZE == 32
#define DEFAULT_MMAP_THRESHOLD_MAX (512 * 1024)
#else
#define DEFAULT_MMAP_THRESHOLD_MAX (4 * 1024 * 1024 * sizeof(long))
#endif
```

###### DEFAULT_MMAP_THRESHOLD

<span id="DEFAULT_MMAP_THRESHOLD"/>

```c
#ifndef DEFAULT_MMAP_THRESHOLD
#define DEFAULT_MMAP_THRESHOLD DEFAULT_MMAP_THRESHOLD_MIN
#endif
```

##### 函数

###### NARENAS_FROM_NCORES

根据CPU核数计算arena的数量

```c
#define NARENAS_FROM_NCORES(n) ((n) * (sizeof(long) == 4 ? 2 : 8))
```

#### 原子操作

##### catomic_compare_and_exchange_bool_acq

CAS操作，若`*mem == oldval`，则`*mem = newval`。若发生了上述交换，返回值为False，否则为True

```c
catomic_compare_and_exchange_bool_acq(mem, newval, oldval);
```

##### catomic_decrement

原子减一操作



#### arena相关

##### top

<span id="top"/>

```c
#define top(ar_ptr) ((ar_ptr)->top)
```

##### arena_lock

<span id="arena_lock"/>

用于给传入的arena指针上锁

```c
#define arena_lock(ptr, size)          \
  do                                   \
  {                                    \
    if (ptr && !arena_is_corrupt(ptr)) \
      __libc_lock_lock(ptr->mutex);    \
    else                               \
      ptr = arena_get2((size), NULL);  \
  } while (0)
```

* 判断arena指针是否为空，或是否[corrupt](#arena_is_corrupt)
  * 若判断条件为真，则代表当前arena可用，将其上锁
  * 若判断条件有假，则代表当前arena不可用，调用[arena_get2](#arena_get2)获取新的arena

##### arena_get

<span id="arena_get"/>

用于获取thread_arena指针

```c
#define arena_get(ptr, size) \
  do                         \
  {                          \
    ptr = thread_arena;      \
    arena_lock(ptr, size);   \
  } while (0)
```

* 调用[arena_lock](#arena_lock)来给当前线程的thread_arena上锁

##### arena_for_chunk

<span id="arena_for_chunk"/>

返回chunk对应的arena

```c
#define arena_for_chunk(ptr) \
  (chunk_main_arena(ptr) ? &main_arena : heap_for_ptr(ptr)->ar_ptr)
```

若程序含有main_arena，则直接返回main_arena指针，否则返回当前所在heap的heap_info结构体中的ar_ptr（指向当前heap所属的arena）

#### heap相关

##### 参数

###### HEAP_MIN_SIZE

<span id="HEAP_MIN_SIZE"/>

创建heap时最小的heap大小

```c
#define HEAP_MIN_SIZE (32 * 1024)
```

###### HEAP_MAX_SIZE

<span id="HEAP_MAX_SIZE"/>

创建heap时最大的heap大小

```c
#define HEAP_MAX_SIZE (2 * DEFAULT_MMAP_THRESHOLD_MAX)
```

为了快速找到chunk对应的heap，该大小应该是2的n次方。并且应该比mmap阈值大得多，这样对于刚好小于mmap阈值的请求来说，可以很方便地满足其要求，以避免创建过多的heap

引用的值：[DEFAULT_MMAP_THRESHOLD_MAX](#DEFAULT_MMAP_THRESHOLD_MAX)

##### 函数

###### heap_for_ptr

<span id="heap_for_ptr"/>

用于获取一个指针对应的heap头

```c
#define heap_for_ptr(ptr) \
  ((heap_info *)((unsigned long)(ptr) & ~(HEAP_MAX_SIZE - 1)))
```

原理：因为heap以HEAP_MAX_SIZE对齐，因此与其对齐的地址即为heap头的地址

###### delete_heap

删除一个heap块

```c
#define delete_heap(heap)                                    \
  do                                                         \
  {                                                          \
    if ((char *)(heap) + HEAP_MAX_SIZE == aligned_heap_area) \
      aligned_heap_area = NULL;                              \
    __munmap((char *)(heap), HEAP_MAX_SIZE);                 \
  } while (0)
```

使用munmap直接删除内存映射。注意这里如果[aligned_heap_area](#aligned_heap_area)指向待删除heap块的下一个块，则将其置为NULL

#### chunk相关

##### 参数

######  MALLOC_ALIGN_MASK

```c
#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)
```

###### MIN_CHUNK_SIZE

MIN_CHUNK_SIZE是没与MALLOC_ALIGNMENT对齐的最小chunk size，大小为`4*sizeof(size_t)`

```c
#define MIN_CHUNK_SIZE        (offsetof(struct malloc_chunk, fd_nextsize))
```

###### MINSIZE

MINSIZE是对齐后的最小chunk size

```c
#define MINSIZE  \
  (unsigned long)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))
```



##### 函数

###### chunk2mem

```c
#define chunk2mem(p)   ((void*)((char*)(p) + 2*SIZE_SZ))
```

* mem  返回给用户的指针
* chunk  malloc_chunk头

###### mem2chunk

```c
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))
```

###### next_chunk

p为当前chunk指针，p+当前chunksize为nextchunk指针

```c
#define next_chunk(p) ((mchunkptr) (((char *) (p)) + chunksize (p)))
```

###### prev_chunk

p为当前chunk指针，返回前一个chunk指针。p-前一个chunksize为prev_chunk指针

```c
#define prev_chunk(p) ((mchunkptr) (((char *) (p)) - prev_size (p)))
```

###### chunk_at_offset

将p+s指向的地址作为指针

```c
#define chunk_at_offset(p, s)  ((mchunkptr) (((char *) (p)) + (s)))
```

###### set_head

```c
#define set_head(p, s) ((p)->mchunk_size = (s))
```

###### chunsize_nomask

```c
#define chunksize_nomask(p) ((p)->mchunk_size)
```

###### chunksize

```c
#define chunksize(p) (chunksize_nomask(p) & ~(SIZE_BITS))
```



#### request2size

若小于MINSIZE则为MINSIZE，若大于则为对齐后的

```c
#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)
```

#### bin相关

##### bin_at

```c
#define bin_at(m, i) \
  (mbinptr) (((char *) &((m)->bins[((i) - 1) * 2]))			      \
             - offsetof (struct malloc_chunk, fd))
```

##### next_bin

```c
#define next_bin(b)  ((mbinptr) ((char *) (b) + (sizeof (mchunkptr) << 1)))
```

##### in_smallbin_range

```c
#define in_smallbin_range(sz)  \
  ((unsigned long) (sz) < (unsigned long) MIN_LARGE_SIZE)
```

##### smallbin_index

```c
#define smallbin_index(sz) \
  ((SMALLBIN_WIDTH == 16 ? (((unsigned) (sz)) >> 4) : (((unsigned) (sz)) >> 3))\
   + SMALLBIN_CORRECTION)
```

##### largebin_index_32

```c
#define largebin_index_32(sz)                                                \
  (((((unsigned long) (sz)) >> 6) <= 38) ?  56 + (((unsigned long) (sz)) >> 6) :\
   ((((unsigned long) (sz)) >> 9) <= 20) ?  91 + (((unsigned long) (sz)) >> 9) :\
   ((((unsigned long) (sz)) >> 12) <= 10) ? 110 + (((unsigned long) (sz)) >> 12) :\
   ((((unsigned long) (sz)) >> 15) <= 4) ? 119 + (((unsigned long) (sz)) >> 15) :\
   ((((unsigned long) (sz)) >> 18) <= 2) ? 124 + (((unsigned long) (sz)) >> 18) :\
   126)
```

##### largebin_index_32_big

```c
#define largebin_index_32_big(sz)                                            \
  (((((unsigned long) (sz)) >> 6) <= 45) ?  49 + (((unsigned long) (sz)) >> 6) :\
   ((((unsigned long) (sz)) >> 9) <= 20) ?  91 + (((unsigned long) (sz)) >> 9) :\
   ((((unsigned long) (sz)) >> 12) <= 10) ? 110 + (((unsigned long) (sz)) >> 12) :\
   ((((unsigned long) (sz)) >> 15) <= 4) ? 119 + (((unsigned long) (sz)) >> 15) :\
   ((((unsigned long) (sz)) >> 18) <= 2) ? 124 + (((unsigned long) (sz)) >> 18) :\
   126)
```

##### large_bin_index_64

```c
// XXX It remains to be seen whether it is good to keep the widths of
// XXX the buckets the same or whether it should be scaled by a factor
// XXX of two as well.
#define largebin_index_64(sz)                                                \
  (((((unsigned long) (sz)) >> 6) <= 48) ?  48 + (((unsigned long) (sz)) >> 6) :\
   ((((unsigned long) (sz)) >> 9) <= 20) ?  91 + (((unsigned long) (sz)) >> 9) :\
   ((((unsigned long) (sz)) >> 12) <= 10) ? 110 + (((unsigned long) (sz)) >> 12) :\
   ((((unsigned long) (sz)) >> 15) <= 4) ? 119 + (((unsigned long) (sz)) >> 15) :\
   ((((unsigned long) (sz)) >> 18) <= 2) ? 124 + (((unsigned long) (sz)) >> 18) :\
   126)
```

##### largebin_index

```c
#define largebin_index(sz) \
  (SIZE_SZ == 8 ? largebin_index_64 (sz)                                     \
   : MALLOC_ALIGNMENT == 16 ? largebin_index_32_big (sz)                     \
   : largebin_index_32 (sz))
```

##### bin_index

```c
#define bin_index(sz) \
  ((in_smallbin_range (sz)) ? smallbin_index (sz) : largebin_index (sz))
```



### arena.c

arena中存放着多个heap，每个heap都是一个连续的内存块，用于存放各个malloc_chunk

#### 数据类型

##### mchunkptr

<span id="mchunkptr"/>

```c
typedef struct malloc_chunk *mchunkptr;
```

##### mbinptr

<span id="mbinptr"/>

```c
typedef struct malloc_chunk *mbinptr;
```

##### heap_info

<span id="heap_info"/>

```c
typedef struct _heap_info
{
  mstate ar_ptr; /* Arena for this heap. */			// 指向当前arena的malloc_state
  struct _heap_info *prev; /* Previous heap. */		// 指向前一个arena的heap_info
    											//（实际上指向的就是前一个arena头）
  size_t size;   /* Current size in bytes. */		// 保存整个arena大小
  size_t mprotect_size; /* Size in bytes that has been mprotected
                           PROT_READ|PROT_WRITE.  */
  /* Make sure the following data is properly aligned, particularly
     that sizeof (heap_info) + 2 * SIZE_SZ is a multiple of
     MALLOC_ALIGNMENT. */
  char pad[-6 * SIZE_SZ & MALLOC_ALIGN_MASK];
} heap_info;
```

* ar_ptr  当前heap的arena指针
* prev  前一个heap
* size  当前heap的大小
* mprotect_size  mprotect标志位被设置为PROT_READ|PROT_WRITE的大小
* pad  用于对齐，至于为什么是这个数见该节下有趣的写法

##### malloc_chunk

<span id="malloc_chunk"/>

```c
struct malloc_chunk {

  INTERNAL_SIZE_T      mchunk_prev_size;  /* Size of previous chunk (if free).  */
  INTERNAL_SIZE_T      mchunk_size;       /* Size in bytes, including overhead. */

  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;

  /* Only used for large blocks: pointer to next larger size.  */
  struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
  struct malloc_chunk* bk_nextsize;
};
```

##### malloc_state

<span id="malloc_state"/>

每个arena都有一个对应的malloc_state

```c
struct malloc_state
{
  /* Serialize access.  */
  __libc_lock_define (, mutex);

  /* Flags (formerly in max_fast).  */
  int flags;

  /* Set if the fastbin chunks contain recently inserted free blocks.  */
  /* Note this is a bool but not all targets support atomics on booleans.  */
  int have_fastchunks;

  /* Fastbins */
  mfastbinptr fastbinsY[NFASTBINS];

  /* Base of the topmost chunk -- not otherwise kept in a bin */
  mchunkptr top;						// top chunk 指针

  /* The remainder from the most recent split of a small request */
  mchunkptr last_remainder;

  /* Normal bins packed as described above */
  mchunkptr bins[NBINS * 2 - 2];		//NBINS=128

  /* Bitmap of bins */
  unsigned int binmap[BINMAPSIZE];

  /* Linked list */
  struct malloc_state *next;

  /* Linked list for free arenas.  Access to this field is serialized
     by free_list_lock in arena.c.  */
  struct malloc_state *next_free;

  /* Number of threads attached to this arena.  0 if the arena is on
     the free list.  Access to this field is serialized by
     free_list_lock in arena.c.  */
  INTERNAL_SIZE_T attached_threads;

  /* Memory allocated from the system in this arena.  */
  INTERNAL_SIZE_T system_mem;
  INTERNAL_SIZE_T max_system_mem;
};
```

###### arena corrupt

<span id="arena_is_corrupt"/> <span id="set_arena_corrupt"/>

程序是否发生了arena corrupt

```c
#define ARENA_CORRUPTION_BIT (4U)
#define arena_is_corrupt(A) (((A)->flags & ARENA_CORRUPTION_BIT))
#define set_arena_corrupt(A) ((A)->flags |= ARENA_CORRUPTION_BIT)
```

###### chunk main arena

<span id="chunk_main_arena"/> <span id="set_non_main_arena"/>

程序是否含有main_arena

```c
#define NON_MAIN_ARENA 0x4

/* Check for chunk from main arena.  */
#define chunk_main_arena(p) (((p)->mchunk_size & NON_MAIN_ARENA) == 0)

/* Mark a chunk as not being on the main arena.  */
#define set_non_main_arena(p) ((p)->mchunk_size |= NON_MAIN_ARENA)
```



#### 锁

##### list_lock

<span id="list_lock"/>

```c
__libc_lock_define_initialized (static, list_lock);
```

##### free_list_lock

<span id="free_list_lock"/>

用于锁 [free_list](#free_list) 全局变量和 [malloc_state](#malloc_state) 结构体中的next_free和attached_threads成员

```c
__libc_lock_define_initialized (static, free_list_lock);
```

#### 全局变量

##### narenas

用于记录当前arena的数量

```c
static size_t narenas = 1;
```

##### main_arena

<span id="main_arena"/>

```c
static struct malloc_state main_arena =
{
  .mutex = _LIBC_LOCK_INITIALIZER,
  .next = &main_arena,
  .attached_threads = 1
};
```

##### thread_arena

<span id="thread_arena"/>

`malloc_state*`结构体指针，注意这里是TLS，每个线程都有

```c
static __thread mstate thread_arena attribute_tls_model_ie;
```

##### free_list

记录被释放的arena的链表，为单链表

```c
static mstate free_list;
```

##### aligned_heap_area

仅在[new_heap](#new_heap)和[delete_heap](#delete_heap)中被使用

如果多个`mmap(0, HEAP_MAX_SIZE << 1, ...)`被调用，并且每次返回的地址都是递减的（而不是递增），则new_heap函数会导致空间碎片化。

```c
static char *aligned_heap_area;
```

##### __malloc_initialized

用于标记是否执行过初始化（[ptmalloc_init](#ptmalloc_init)）

```c
int __malloc_initialized = -1;
```



#### 顶层函数

##### ptmalloc_init

主要用于malloc_state的初始化，还有对一些环境变量中设置的调试选项的处理

```c
static void ptmalloc_init(void);
```

* 将main_arena赋值给thread_arena
* 调用[next_env_entry](#next_env_entry)一些malloc选项并调用[libc_mallopt](#libc_mallopt)处理，主要涉及一些malloc默认值的修改
  * MALLOC_CHECK_x  其中x是一个0-9间的数字
  * MALLOC_TOP_PAD_n  其中n是一个数字
  * MALLOC_PERTURB_n
  * MALLOC_MMAP_MAX_n
  * MALLOC_ARENA_MAX_n
  * MALLOC_ARENA_TESTn
  * MALLOC_TRIM_THRESHOLD_n
  * MALLOC_MMAP_THRESHOLD_n

##### arena_get_retry

获取一个新的arena

```c
static mstate arena_get_retry(mstate ar_ptr, size_t bytes);
```

* 判断`ar_ptr != &main_arena`
  * 若是，说明当前arena不是main_arena，试图从main_arena获取空间：
    * 解锁`ar_ptr->mutex`
    * 判断[arena_is_corrupt](#arena_is_corrupt)，若corrupt返回NULL
    * 否则将`ar_ptr->mutex`上锁，并返回main_arena指针
  * 若否，说明当前arena是main_arena，即main_arena没空余空间，给其分配新的arena空间：
    * 解锁`ar_ptr->mutex`
    * 调用[arena_get2](#arena_get2)获取arena并返回指针

#### 底层函数：arena相关

#####arena_get2

用于获取一个新的arena，根据条件可能通过`get_free_list / _int_new_arena / reused_arena`三种方法获取

size参数确定arena大小，avoid_arena仅作为参数传给[reused_arena](#reused_arena)，因为arena_get2函数仅在arena空间分配失败的情况下被调用，该指针指向上次分配失败的arena，这样在调用reused_arena时可以避免复用到之前分配失败的那个arena

```c
static mstate internal_function arena_get2(size_t size, mstate avoid_arena);
```

* 调用[get_free_list](#get_free_list)试图复用一个free_list上的arena
  * 若有直接返回新的arena
  * 若无，则首先确定arena数量的限制（static变量，只获取一次），方式如下
    * 若[mp_](#mp_)的[arena_max成员](#malloc_par)不为0，则直接使用该值
    * 若[narenas](#narenas)大于mp_的arena_test成员
      * 调用__get_nprocs获取计算机核心数
      * 若返回值大于等于1，调用[NARENAS_FROM_NCORES](#NARENAS_FROM_NCORES)确定arena数
      * 否则说明该调用无法正确获取核心数，假设核心数为2，调用NARENAS_FROM_NCORES
  * `narenas <= narenas_limit - 1`是否成立
    * 若是
      * 首先试图使用CAS操作（[catomic_compare_and_exchange_bool_acq](#catomic_compare_and_exchange_bool_acq)）为narenas加一，若CAS测试失败（即`narenas != oldval`）则跳回`narenas_limit-1`判断重新进行判断
      * 调用[_int_new_arena](#_int_new_arena)创建大小为size的arena
      * 若返回NULL，说明创建arena失败，使用原子操作[catomic_decrement](#catomic_decrement)对narenas减一
    * 若否，说明当前arena过多，不允许继续创建新的arena
      * 调用[reused_arena](#reused_arena)获取arena



##### _int_new_arena

新建一个arena，每个arena的内存布局见[arena内存布局](#arena内存布局)

```c
static mstate _int_new_arena(size_t size);
```

* 使用[new_heap](#new_heap)分配一块大小为`size + (sizeof(*h) + sizeof(*a) + MALLOC_ALIGNMENT)`的空间，top_pad为[mp_](#mp_)的top_pad成员
  * 若没有申请成功，使用new_heap分配一块`sizeof(*h) + sizeof(*a) + MALLOC_ALIGNMENT`的空间，作为最小的arena块
    * 若还是没成功，返回0
* 调用[malloc_init_state](#malloc_init_state)对当前arena属性（[malloc_state](#malloc_state)）进行初始化
* 设置malloc_state的
  * attached_threads = 1
  * system_mem和max_system_mem都为新的heap_info的size属性
  * top chunk指针。其中top chunk需要满足`chunk2mem(p_top_chunk)`与MALLOC_ALIGNMENT对齐，即top chunk的memory部分与MALLOC_ALIGNMENT对齐（见[arena内存布局](#arena内存布局)）
* 设置top_chunk的mchunk_size，其中设置了`PREV_INUSE`标志位
* 使用新的arena替换thread_arena，并初始化新arena的mutex
* 获取[list_lock](#list_lock)并将新arena加入全局arena链（main_arena.next）
* 获取[free_list_lock](#free_list_lock)并对先前的thread_arena调用[detach_arena](#detach_arena)。但是注意当前的detach_arena实现并没有将detach的arena放入free_list
* 给新的arena上锁

###### 进一步的说明

这里在最后加入main_arena列表和detach_arena时涉及了一些多线程操作

注释里说：最后一步给新的arena上锁是因为在这个时刻新的arena已经可以被其他线程访问了（通过main_arena.next），并且可能已经被reused_arena获取，这种情况仅在最后一个arena被创建时（arena_max-1）可能发生，这时一个arena将会被挂载到两个线程上。虽然如果将这句放到获取list_lock前可以避免这种情况，但这可能造成与[__malloc_fork_lock_parent](#__malloc_fork_lock_parent)函数死锁

##### reused_arena

```c
static mstate reused_arena(mstate avoid_arena);
```

* 定义一个static变量next_to_use，若未初始化，则初始化为&main_arena
* 初始化`result=next_to_use`循环检查result指向的arena是否[corrupt](#arena_is_corrupt)，并试图获取锁。若上述条件不满足，则遍历下一个arena（`result->next`）
  * 若遍历结束都没有找到合适的arena，此时result又指向了next_to_use
  * 若result指向avoid_arena，则令result指向下一个arena
  * 第二次循环检查arena，试图找到不是avoid_arena且不corrupt的arena
    * 若遍历一遍找不到，则**返回NULL**
  * 执行到这里说明找到了可以复用的arena，将arena上锁
* （第一次循环若找到了arena，也跳到该处执行，此时result指向可用arena且已上锁）
* 调用[detach_arena](#detach_arena)将当前的thread_arena取下
* 上锁[free_list_lock](#free_list_lock)，调用[remove_from_free_list](#remove_from_free_list)将result指向的arena取下，attached_threads成员加一，此后解锁
* 将result赋给thread_arena，且令`next_to_use=result->next`

##### detach_arena

使replaced_arena脱离当前线程，实际上这个函数当前只是对[malloc_state](#malloc_state)结构的attached_threads成员-1

```c
static void detach_arena(mstate replaced_arena);
```

##### arena_thread_freeres

用于将一个arena放入free_list，且令当前thread_arena为NULL

```c
static void __attribute__((section("__libc_thread_freeres_fn")))
arena_thread_freeres(void);
```

* free_list_lock上锁
* 若thread_arena不为NULL，且attach_threads成员减一为0（即当前线程是该arena唯一一个attach的线程）
  * 将该arena加入free_list
* free_list_lock解锁
* `thread_arena=NULL`（其实源码中这句在函数开头）

##### get_free_list

从free_list获取一个arena，赋值给thread_arena

```c
static mstate get_free_list(void);
```

* 若当前free_list指向的不为NULL
  * 获取free_list_lock，重复上述检查（防止多线程导致free_list数据被修改）
  * 若不为NULL
    * `result = freelist; free_list = free_list->next_free`
    * （若attached_threads不为0触发assert，理论上必为0，因为在加入free_list时会进行检查）
    * `result->attach_threads`设为1
    * 调用[detach_arena](#detach_arena)解绑thread_arena
    * `thread_arena = result`
    * 给新arena（result指向的元素）上锁

##### remove_from_free_list

从free_list取得一个arena，调用者在调用该函数前必须先给free_list上锁

```c
static void remove_from_free_list(mstate arena);
```

* 循环遍历free_list，若当前指向的arena与arena参数相同，则将该arena从链表中取出（即修改前一个链表的next指针为该arena的next），注意被取出的arena的next指针没有被修改



#### 底层函数：heap相关

##### new_heap

创建一个新的heap

```c
static heap_info *internal_function new_heap(size_t size, size_t top_pad);
```

* 判断size+top_pad大小

  * 若在HEAP_MIN_SIZE到HEAP_MAX_SIZE之间，则分配大小即为size+top_pad
  * 若小于HEAP_MIN_SIZE则设为HEAP_MIN_SIZE
  * 若大于HEAP_MAX_SIZE直接返回

* size+top_pad按照pagesize值向上取倍数（[ALIGN_UP](#ALIGN_UP)）

* 下面使用mmap创建空间，有三种方式。这些方式下面还会进一步说明。三种方式mmap后都会检查与HEAP_MAX_SIZE的对齐，若不对齐则释放

  * 在[aligned_heap_area](#aligned_heap_area)不为NULL时

    ```c
    p2 = (char *)MMAP(aligned_heap_area, HEAP_MAX_SIZE, PROT_NONE, MAP_NORESERVE);
    ```

  * 一般情况下使用这个

    ```c
    p1 = (char *)MMAP(0, HEAP_MAX_SIZE << 1, PROT_NONE, MAP_NORESERVE);
    ```

    注意这里分配了`HEAP_MAX_SIZE*2`大小的空间，并且如果ul为0，会将**aligned_heap_area**的值赋为`p1+HEAP_MAX_SIZE`（这里见下面的进一步说明）。即下次调用new_heap的时候会使用第一种方式mmap，并且新创建的heap块在上一次创建的heap块的上方（高地址）。

    调用完成后会把第二块HEAP_MAX_SIZE大小的块munmap掉。

  * 前两种方法失效的情况下使用这个

    ```c
    p2 = (char *)MMAP(0, HEAP_MAX_SIZE, PROT_NONE, MAP_NORESERVE);
    ```

* 使用mprotect改变页面属性为RW，注意改变的大小为size+top_pad，而不是mmap出来的HEAP_MAX_SIZE

* 将mmap的空间的开头作为[heap_info](#heap_info)结构体，并赋值

  * heap_info.size为size+top_pad
  * heap_info.mprotect_size为size+top_pad

###### 进一步的说明

<span id="arena_new_heap_explain" />

这段代码本身功能比较简单，就是使用mmap分配一段大小为HEAP_MAX_SIZE的空间来构造新的heap，但这里用了三种不同方式的mmap，主要是优化考虑。

因为每次分配的大小是HEAP_MAX_SIZE，并且要求地址与[HEAP_MAX_SIZE](#HEAP_MAX_SIZE)（64位系统默认是64MB，32位系统默认是1MB）对齐，而mmap分配的地址只保证是pagesize对齐的，因此为了满足HEAP_MAX_SIZE对齐的要求，就申请2*HEAP_MAX_SIZE的大小，这样空间内一定有HEAP_MAX_SIZE大小的满足对齐要求的空间。此后将没有用到的其余空间unmap掉。

上述这种做法的问题在于，可能造成一些地址碎片，假设mmap分配的地址为0x00601000~0x00801000（32位，大小2MB），则要符合HEAP_MAX_SIZE对齐（1MB对齐），则使用的地址为0x00700000~0x00800000，则会产生两片碎片（0x00601000~0x0060ffff和0x00800000~0x00801000）

这里解决这个问题的方法就是引入[aligned_heap_area](#aligned_heap_area)，做法是在获取一个对齐的地址p后，在aligned_heap_area中保存p+HEAP_MAX_SIZE，下次分配的时候首先尝试这个地址。但是这里的实现在只有当mmap返回的地址符合HEAP_MAX_SIZE对齐时才会触发这个机制，疑似有[bug](#arena_bug_1)

##### grow_heap

增加heap的大小，注意这里不会重新mmap空间，而只是使用mprotect改变页面属性而已

```c
static int grow_heap(heap_info *h, long diff)
```

* `new_size = heap_info.size + diff`，判断new_size是否大于HEAP_MAX_SIZE，若大于返回-1
* 若new_size大于heap_info.mprotect_size，则调用mprotect将多的页面属性也改为RW

##### shrink_heap

减少heap的大小

```c
static int shrink_heap(heap_info *h, long diff);
```

* `new_size = heap_info.size - diff`
* 若new_size小于sizeof(heap_info)，返回-1
* 调用[check_may_shrink_heap](#check_may_shrink_heap)检查系统的overcommit标志位
  * 若标志不为0，则使用`mmap(..., PROT_NONE, ...)`来释放页面
  * 若标志为0，则使用`madvise(..., MADV_DONTNEED)`来释放页面

<span id="shrink_heap_bug"/>

这里调用mmap和madvise的时候都使用`h+new_size`作为地址，没有进行对齐，感觉如果mmap和madvise没有做页对齐检查的话可能出bug（[见这里](#arena_bug_2)）

##### heap_trim



```c
static int internal_function heap_trim(heap_info *heap, size_t pad);
```

* 使用[top](#top)宏获取heap指向的arena的top_chunk
* 判断top chunk是否等于`heap+sizeof(heap_info)`
  * 



#### 其他函数

##### next_env_entry

用于获取下一个带MALLOC_前缀的环境变量

##### check_may_shrink_heap

这个函数定义在sysdeps的malloc_sysdep.h中，用于获取并检查系统的overcommit选项（[选项细节](http://blog.itpub.net/15498/viewspace-2638288/)）配置（linux kernel中默认为true）

* 0  表示内核将检查是否有足够的可用内存供应用进程使用；如果有足够的可用内存，内存申请允许；否则，内存申请失败，并把错误返回给应用进程。
* 1  表示内核允许分配所有的物理内存，而不管当前的内存状态如何。
* 2  表示内核允许分配超过所有物理内存和交换空间总和的内存。

```c
static inline bool check_may_shrink_heap (void);
```

* 检查overcommit标志

  若已经获取过该标志则直接返回

* 获取overcommit标志：

  检查是否定义了__libc_enable_secure符号，若没定义，打开`/proc/sys/vm/overcommit_memory`读取该标志

#### atfork support

下面3个函数用于在多线程程序中在fork函数的前后被调用（详见[pthread_atfork](https://linux.die.net/man/3/pthread_atfork)）。这里我们不需要使用fork handler机制来保证我们的handler是最后被调用的handler，因此对于有其他fork handler的函数，malloc依然适用

其中parent函数在fork前被调用，child函数在fork后被调用

##### __malloc_fork_lock_parent

将list_lock中的每个arena上锁

```c
void internal_function __malloc_fork_lock_parent(void);
```

* 检查[__malloc_initialized](#__malloc_initialized)，若未初始化则直接返回
* 给[list_lock](#list_lock)上锁
* 以[main_arena](#main_arena)为起点，获取arena链表，并将每个arena对应的[malloc_state](#malloc_state)上锁（mutex成员）

注意最后list_lock没有解锁

##### __malloc_fork_unlock_parent

将list_lock中的每个arena解锁

```c
void internal_function __malloc_fork_unlock_parent(void);
```

* 检查[__malloc_initialized](#__malloc_initialized)，若未初始化则直接返回
* 以[main_arena](#main_arena)为起点遍历arena，并将每个arena对应的[malloc_state](#malloc_state)解锁（mutex成员）
* [list_lock](#list_lock)解锁

##### __malloc_fork_unlock_child

重新构造free_list，并将所有除了thread_arena之外的arena全部放进[free_list](#free_list)

```c
void internal_function __malloc_fork_unlock_child(void);
```

* 检查[__malloc_initialized](#__malloc_initialized)，若未初始化则直接返回
* 初始化[free_list_lock](#free_list_lock)
* 从main_arena开始遍历arena。
  * 初始化每个arena的锁
  * 若当前arena不是[thread_arena](#thread_arena)
    * 加入free_list
* 初始化[list_lock](#list_lock)

#### 有趣的写法

##### 自动计算对齐

在heap_info结构中有这样一段代码

```c
char pad[-6 * SIZE_SZ & MALLOC_ALIGN_MASK];  // 其中MALLOC_ALIGN_MASK为MALLOC_ALIGNMENT-1
										//  SIZE_SZ=sizeof(size_t)
```

注释中说这段代码是为了保证`sizeof(heap_info)+2*sizeof(size_t)`长度整除`MALLOC_ALIGNMENT`

原理应该是这样，首先因为sizeof(heap_info)为`4*sizeof(size_t)`，所以其实要保证的是`6*sizeof(size_t)`整除`MALLOC_ALIGNMENT`

所以`6*sizeof(size_t) & MALLOC_ALIGN_MASK`应该表示的是`6*sizeof(size_t)`超出对齐的字节数，因此pad的字节数应该为

```
MALLOC_ALIGNMENT - (6*sizeof(size_t) & MALLOC_ALIGN_MASK)
```

显然，在模MALLOC_ALIGNMENT的数中，该值与

```
-6*sizeof(size_t) & MALLOC_ALIGN_MASK
```

相等

##### 编译时检查参数

```c
extern int sanity_check_heap_info_alignment[(sizeof(heap_info) + 2 * SIZE_SZ) % MALLOC_ALIGNMENT
                                                ? -1
                                                : 1];
```

这段用于检查heap_info结构体是否正确对齐，如果对齐的话，模MALLOC_ALIGNMENT结果为0，因此该数组长度为1，不报错。若不对齐，则该数组长度为-1，报错

### malloc.c

#### 数据类型

##### malloc_par

```c
struct malloc_par
{
  /* Tunable parameters */
  unsigned long trim_threshold;
  INTERNAL_SIZE_T top_pad;
  INTERNAL_SIZE_T mmap_threshold;
  INTERNAL_SIZE_T arena_test;
  INTERNAL_SIZE_T arena_max;

  /* Memory map support */
  int n_mmaps;
  int n_mmaps_max;
  int max_n_mmaps;
  /* the mmap_threshold is dynamic, until the user sets
     it manually, at which point we need to disable any
     dynamic behavior. */
  int no_dyn_threshold;

  /* Statistics */
  INTERNAL_SIZE_T mmapped_mem;
  INTERNAL_SIZE_T max_mmapped_mem;

  /* First address handed out by MORECORE/sbrk.  */
  char *sbrk_base;

#if USE_TCACHE
  /* Maximum number of buckets to use.  */
  size_t tcache_bins;
  size_t tcache_max_bytes;
  /* Maximum number of chunks in each bucket.  */
  size_t tcache_count;
  /* Maximum number of chunks to remove from the unsorted list, which
     aren't used to prefill the cache.  */
  size_t tcache_unsorted_limit;
#endif
};
```



#### 全局变量

##### mp_

```c
static struct malloc_par mp_ =
{
  .top_pad = DEFAULT_TOP_PAD,			//default: 0
  .n_mmaps_max = DEFAULT_MMAP_MAX,		//default: 65536
  .mmap_threshold = DEFAULT_MMAP_THRESHOLD,	//default: 128*1024
  .trim_threshold = DEFAULT_TRIM_THRESHOLD,	//default: 128*1024
#define NARENAS_FROM_NCORES(n) ((n) * (sizeof (long) == 4 ? 2 : 8))
  .arena_test = NARENAS_FROM_NCORES (1)
#if USE_TCACHE
  ,
  .tcache_count = TCACHE_FILL_COUNT,
  .tcache_bins = TCACHE_MAX_BINS,
  .tcache_max_bytes = tidx2usize (TCACHE_MAX_BINS-1),
  .tcache_unsorted_limit = 0 /* No limit.  */
#endif
};
```



#### 主要函数

##### __libc_malloc

顶层函数

```c
void *__libc_malloc(size_t bytes);
```

* 使用arena_get试图从当前arena中获取元素
* 使用_int_malloc分配内存
* 若获取失败
  * arena_get_retry试图重新获取arena
  * _int_malloc
* 解锁arena，返回

##### unlink

unlink是一个宏，在从双链表中取出一个chunk外还执行了很多额外的检查

```c
#define unlink(AV, P, BK, FD) {                                            \
    if (__builtin_expect (chunksize(P) != prev_size (next_chunk(P)), 0))      \
      malloc_printerr ("corrupted size vs. prev_size");			      \
    FD = P->fd;								      \
    BK = P->bk;								      \
    if (__builtin_expect (FD->bk != P || BK->fd != P, 0))		      \
      malloc_printerr ("corrupted double-linked list");			      \
    else {								      \
        FD->bk = BK;							      \
        BK->fd = FD;							      \
        if (!in_smallbin_range (chunksize_nomask (P)) && __builtin_expect (P->fd_nextsize != NULL, 0))
        {                                                             \
	        if (__builtin_expect (P->fd_nextsize->bk_nextsize != P, 0) || __builtin_expect (P->bk_nextsize->fd_nextsize != P, 0))    \
	          malloc_printerr ("corrupted double-linked list (not small)");   \
          if (FD->fd_nextsize == NULL) {				            \
            if (P->fd_nextsize == P)				                \
                FD->fd_nextsize = FD->bk_nextsize = FD;		      \
              else {							      \
                  FD->fd_nextsize = P->fd_nextsize;			      \
                  FD->bk_nextsize = P->bk_nextsize;			      \
                  P->fd_nextsize->bk_nextsize = FD;			      \
                  P->bk_nextsize->fd_nextsize = FD;			      \
                }							      \
            } else {							      \
              P->fd_nextsize->bk_nextsize = P->bk_nextsize;		      \
              P->bk_nextsize->fd_nextsize = P->fd_nextsize;		      \
            }								      \
          }								      \
      }									      \
}
```

设当前chunk指针为P，下述检查若有失败则会报错

* 检查后一个chunk的prev_size是否为当前chunksize
* 检查前一个chunk（fd）的后一个chunk（bk）是否为P
* 检查后一个chunk（bk）的前一个chunk（fd）是否为P
* 将当前chunk从链表中取出（注意此时被取出的chunk中fd和bk没有被覆盖）
* 若当前chunk不为smallbin且存在上一块（fd_nextsize）
  * 检查前一个chunk（fd_nextsize）的后一个chunk（bk_nextsize）是否为P
  * 检查后一个chunk（bk_nextsize）的前一个chunk（fd_nextsize）是否为P
  * 判断前一个chunk是否存在前置chunk（`FD->fd_nextsize==NULL`）
    * 若不存在，判断当前chunk的前置chunk是否是自身（`P->fd_nextsize == P`）
      * 若是，令FD的nextsize链表组成一个只含FD的环形链表（`FD->fd_nextsize = FD->bk_nextsize = FD`）
      * 若否，用FD替换原来在nextsize链表上的位置
    * 若存在，直接将P从nextsize链表取下

##### malloc_init_state

初始化malloc_state

```c
static void malloc_init_state(mstate av);
```

* 将每个`malloc_state->bin`元素链表初始化为fd和bk都指向自身
* 设置fastchunk大小
* 将top设置为unsorted chunk

##### sysmalloc

使用系统调用分配空间

```c
static void *sysmalloc(INTERNAL_SIZE_T nb, mstate av);
```

###### 描述

* 如果av为空，或`nb > mp_.mmap_threshold && mp_.n_mmaps < mp_.n_mmaps_max`，则直接使用mmap

  * 首先将请求的大小nb转化为页面大小，注意这里使用mmap分配chunk，比普通的chunk少了一个SIZE_SZ大小的长度，因为普通chunk在分配后，后一个chunk的prev_size域可以被使用，而mmap分配的chunk没有连续的下一个chunk

    * 由于上述原因，转化调用的是`ALIGN_UP(nb + SIZE_SZ, pagesize)`，此外若自定义了MALLOC_ALIGNMENT，调用`ALIGN_UP(nb + SIZE_SZ + MALLOC_ALIGN_MASK, pagesize)`

  * 调用mmap前有一个有趣的处理，就是判断转化后的大小是否大于nb。这应该是为了防止整数溢出，具体见有趣的写法

  * 使用mmap时，若分配成功，分配结束后malloc_chunk用途如下

    ```c
    struct malloc_chunk {
    
      INTERNAL_SIZE_T      mchunk_prev_size;  
        				//若当前mmap后返回的内存不是MALLOC_ALIGNMENT对齐的
        				//即chunk2mem(mm)不是MALLOC_ALIGNMENT对齐的
        				//则需要调整为MALLOC_ALIGNMENT对齐，方法为不使用前n字节
        				//mchunk_prev_size保存着n
      INTERNAL_SIZE_T      mchunk_size;
        				//保存除去不使用的字节外剩下的大小，这个大小是pagesize对齐的
    
      //---------------------------------------------------------------------------
        //下面直接是用户使用的空间，malloc返回的地址即fd成员的地址
      struct malloc_chunk* fd;         /* double links -- used only if free. */
      struct malloc_chunk* bk;
    
      /* Only used for large blocks: pointer to next larger size.  */
      struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
      struct malloc_chunk* bk_nextsize;
    };
    ```

* 若av为空，则没有arena可以分配，直接返回NULL

* 



#### 有趣的写法

##### next_env_entry

```c
static char *
next_env_entry(char ***position)
{
  char **current = *position;
  char *result = NULL;

  while (*current != NULL)
  {
    if (__builtin_expect((*current)[0] == 'M', 0) && (*current)[1] == 'A' && (*current)[2] == 'L' && (*current)[3] == 'L' && (*current)[4] == 'O' && (*current)[5] == 'C' && (*current)[6] == '_')
    {
      result = &(*current)[7];
      /* Save current position for next visit.  */
      *position = ++current;
      break;
    }
    ++current;
  }
  return result;
}
```

position是一个指向字符串数组（`char** or char*[]`）的指针（一般是argv[]或envp[]），因此current为字符串数组头指针。`++current`使current指向字符串数组的下一个元素（实际上的操作是`current += sizeof(char**)`）

##### sysmalloc

在将请求大小转化为mmap申请页面大小时有下列代码

```c
if (MALLOC_ALIGNMENT == 2 * SIZE_SZ)
    size = ALIGN_UP(nb + SIZE_SZ, pagesize);
else
    size = ALIGN_UP(nb + SIZE_SZ + MALLOC_ALIGN_MASK, pagesize);
tried_mmap = true;

/* Don't try if size wraps around 0 */
if ((unsigned long)(size) > (unsigned long)(nb))
{
    ...
}
```

看起来size肯定大于nb，但一种情况例外：当`nb+SIZE_SZ >= (size_t)-pagesize`时，即nb很大，以至于加上pagesize后会导致size_t溢出，这时size输出为0。如果不对这种情况进行特殊处理可能导致安全问题

### 一些可能的bug

#### arena.c

##### new_heap

<span id="arena_bug_1"/>

bug所在代码功能的[描述](#arena_new_heap_explain))

```c
p1 = (char *)MMAP(0, HEAP_MAX_SIZE << 1, PROT_NONE, MAP_NORESERVE);
if (p1 != MAP_FAILED)
{
    p2 = (char *)(((unsigned long)p1 + (HEAP_MAX_SIZE - 1)) & ~(HEAP_MAX_SIZE - 1));
    ul = p2 - p1;
    if (ul)
        __munmap(p1, ul);
    else
        aligned_heap_area = p2 + HEAP_MAX_SIZE;
    __munmap(p2 + HEAP_MAX_SIZE, HEAP_MAX_SIZE - ul);
}
```

这里只有当ul为0时，即p1为HEAP_MAX_SIZE的地址时，才对aligned_heap_area进行赋值，实际上触发优化的概率很低（依赖于mmap实现）

http://sourceware-org.1504.n7.nabble.com/malloc-performance-improvements-and-bugfixes-td374269.html

##### shrink_heap

<span id="arena_bug_2"/>

```c
static int shrink_heap(heap_info *h, long diff)
{
  long new_size;

  new_size = (long)h->size - diff;
  if (new_size < (long)sizeof(*h))
    return -1;

  /* Try to re-map the extra heap space freshly to save memory, and make it
     inaccessible.  See malloc-sysdep.h to know when this is true.  */
  if (__glibc_unlikely(check_may_shrink_heap()))
  {
    if ((char *)MMAP((char *)h + new_size, diff, PROT_NONE,
                     MAP_FIXED) == (char *)MAP_FAILED)
      return -2;

    h->mprotect_size = new_size;
  }
  else
    __madvise((char *)h + new_size, diff, MADV_DONTNEED);
  /*fprintf(stderr, "shrink %p %08lx\n", h, new_size);*/

  h->size = new_size;
  LIBC_PROBE(memory_heap_less, 2, h, h->size);
  return 0;
}
```

功能描述见[这里](#shrink_heap)。从上述代码可以看出调用mmap和madvise使用的地址参数为`h+new_size`，没有经过地址对齐

##### reused_arena

第一次循环取arena时，若取得符合条件的arena（非corrupt，且可以获取锁），不会对其是否为avoid_arena进行检查

```c
result = next_to_use;
do
{
    if (!arena_is_corrupt(result) && !__libc_lock_trylock(result->mutex))
        goto out;

    /* FIXME: This is a data race, see _int_new_arena.  */
    result = result->next;
} while (result != next_to_use);

/* Avoid AVOID_ARENA as we have already failed to allocate memory
     in that arena and it is currently locked.   */
if (result == avoid_arena)
    result = result->next;
```

这里可以看到第一个循环中，若result不corrupt且可以获取锁，直接跳到out执行，而不会执行下面的判断是否为avoid_arena

##### _int_new_arena



##### remove_from_free_list

arena被从链表中取出后，其next指针没有置为NULL（泄露下一个arena地址？）

```c
static void
remove_from_free_list(mstate arena)
{
  mstate *previous = &free_list;
  for (mstate p = free_list; p != NULL; p = p->next_free)
  {
    assert(p->attached_threads == 0);
    if (p == arena)
    {
      /* Remove the requested arena from the list.  */
      *previous = p->next_free;
      break;
    }
    else
      previous = &p->next_free;
  }
}
```

可以看到break前的语句修改前一个arena的next_free指针为当前arena的next_free指针，但没有修改当前arena的next_free指针。这里可能泄露下一个arena的指针

##### arena_get2

get_free_list不管size参数？

```c
static mstate
    internal_function
    arena_get2(size_t size, mstate avoid_arena)
{
  mstate a;

  static size_t narenas_limit;

  a = get_free_list();
  if (a == NULL)
  {
      ...
  }
  return a;
}
```

这里首先使用的获取arena的方式就是调用get_free_list，但这种方法并不使用size参数（下面另一种获取arena的方式：reused_arena也不检查）

