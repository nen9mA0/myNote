### 概念与解释

#### 数据结构

##### arena

###### arena内存布局

初始化后arena的内存布局如下

```
                高地址
 ----------------
|   top chunk    |
 ---------------- <---------\
|   pad          |           |
 ----------------            |
|   malloc_state | -- top --/
 ---------------- <------------\
|    heap_info    | -- ar_ptr --/
 ----------------    
                 低地址
```

* pad部分可能有可能没有，这段pad是为了保证top chunk的memory部分与MALLOC_ALIGNMENT对齐，即`chunk2mem(p_top_chunk)`是MALLOC_ALIGNMENT对齐的

用visio重画了一下

![](pic\arena.png)

bins初始状态下都是指向当前节点对应的malloc chunk，所以图画成这样。如bin[0]和bin[1]分别为fd和bk指针，指向的是bin_at(av, 1)  （详见[bin_at](#bin_at)）

##### chunk

chunk使用一种称为“边界标签”的方法来管理（技术细节见ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps 这篇paper，在该目录下的allocsrv有关于这篇文章的笔记）

每个空闲的chunk的大小都会在chunk的开头和末尾被记录，这使得我们将多个空闲碎片合并为一个大空间时的速度较快。此外，size成员还有一些标记位来指明当前chunk是否正在被使用。

末尾记录chunk大小的成员其实是下一个chunk的第一个成员，因此实际上从某种角度来说，一个chunk的范围是当前chunk指针的第二个成员到下一个chunk指针的第一个成员

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

###### 空闲chunk

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

###### fastbin相关

存储在malloc_state的fastbinY数组中，默认最大为`80 * SIZE_SZ / 4`（[MAX_FAST_SIZE](#MAX_FAST_SIZE)），因此默认fastbin个数为10个（[NFASTBINS](#NFASTBINS)）。

对于fastbin来说，每个数组项对应的chunk链表大小都是相同的，对于第i项，其大小是

###### smallbin相关

###### largebin相关

#### 关于锁

##### malloc

* [_libc_malloc](#关于libc_malloc的锁操作)

#### fork相关

在 [malloc.md](malloc.md) 中的atfork support一节中介绍了arena对于fork的支持，大致的流程如下：

* 首先调用 __malloc_fork_lock_parent 对所有arena上锁

* 系统进行fork，此时新进程将继承原进程地址空间的结构，包括所有的arena

* fork对地址空间的构造结束，父进程调用 __malloc_fork_unlock_parent 解锁所有的arena

* 对子进程来说，除thread_arena的数据外其他arena都是无用的，所以重新构造free_list并将所有除thread_arena的arena放入free_list中

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

选项                            默认值
编译环境选项
  HAVE_MREMAP                    0
改变默认字长
  INTERNAL_SIZE_T                size_t
配置和功能选项
  USE_PUBLIC_MALLOC_WRAPPERS    NOT defined
  USE_MALLOC_LOCK                NOT defined
  MALLOC_DEBUG                    NOT defined
  REALLOC_ZERO_BYTES_FREES        1
  TRIM_FASTBINS                    0
MORECORE设置选项
  MORECORE                        若第360行的__default_morecore未定义，则为sbrk
  MORECORE_FAILURE                -1
  MORECORE_CONTIGUOUS            1
  MORECORE_CANNOT_TRIM            NOT defined
  MORECORE_CLEARS                1
  MMAP_AS_MORECORE_SIZE            (1024*1024)
下列选项可以通过mallopt动态修改
  DEFAULT_MXFAST                64 (for 32bit)  128 (for 64bit)
  DEFAULT_TRIM_THRESHOLD        128*1024
  DEFAULT_TOP_PAD                0
  DEFAULT_MMAP_THRESHOLD        128*1024
  DEFAULT_MMAP_MAX                65536

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

* 一般来说常驻内存在进程运行的早期被分配，由于此时阈值不会被调整，因此只要大于默认的128K即会使用mmap分配。而此后的分配会影响阈值，使阈值慢慢变大
* 因为定义了最大值，因此可以满足第二点

阈值调整：若程序释放了一块mmap的内存，阈值将增加。依据是当程序开始释放mmap的内存时，该块释放的内存很有可能是程序分配的临时内存。这个优化是为了满足第三个设计目的。

##### M_MMAP_MAX

mmap可以接受的最大内存分配请求。因为部分系统在mmap分配的空间超过一定范围时会影响性能

默认值仅作为一个safeguard。设置为0则表示不使用mmap处理大的内存分配请求

##### chunk相关

这块内容因为对于理解malloc结构较重要，所以移到[chunk](#chunk)

##### 内部数据结构相关

这块内容也比较重要，所以移到[内部数据结构](#内部数据结构)