## Overview

## 代码解析

### 标志位相关

#### malloc_state

##### FASTCHUNKS_BIT

用于标识当前的arena是否有可用的fastchunk

```c
#define FASTCHUNKS_BIT (1U)
```

###### have_fastchunks

```c
#define have_fastchunks(M) (((M)->flags & FASTCHUNKS_BIT) == 0)
```

###### clear_fastchunks

```c
#define clear_fastchunks(M) catomic_or(&(M)->flags, FASTCHUNKS_BIT)
```

###### set_fastchunks

```c
#define set_fastchunks(M) catomic_and(&(M)->flags, ~FASTCHUNKS_BIT)
```

##### NONCONTIGUOUS_BIT

##### ARENA_CORRUPTION_BIT

### 一些重要宏

#### 配置相关

##### 参数

###### DEFAULT_MMAP_THRESHOLD_MIN

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

#### ALIGN

###### MALLOC_ALIGNMENT

默认的大小为2*SIZE_SZ

###### ALIGN_DOWN

根据size大小，对base向下取整到size的倍数，如size为4096，base为8191，则返回值为4096

注意这个算法仅对size为2^n的适用

```c
#define ALIGN_DOWN(base, size)    ((base) & -((__typeof__ (base)) (size)))
```

算法原理很简单，假设size为2^n，则将低n位清零。这里`__typeof__ (base)`主要用于将size强制类型转换成与base相同的类型。-size其实等价于`~size+1`，或`~(size-1)`，实际上和常用的将size转化为掩码的算法相同（如MALLOC_ALIGNMENT转化为MALLOC_ALIGN_MASK的方法）

###### ALIGN_UP

根据size大小，对base向上取整到size的倍数，如size为4096，base为4097，则返回值为8192

注意这个算法仅对size为2^n的适用

```c
#define ALIGN_UP(base, size)    ALIGN_DOWN ((base) + (size) - 1, (size))
```

#### request2size

实际分配空间大小为需求的大小+SIZE_SZ，此后对齐。若小于MINSIZE则为MINSIZE，若大于则为对齐后的

```c
#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)
```

###### checked_request2size

加了个对req是否超出最大值的检测

```c
#define checked_request2size(req, sz) \
  if (REQUEST_OUT_OF_RANGE(req))      \
  {                                   \
    __set_errno(ENOMEM);              \
    return 0;                         \
  }                                   \
  (sz) = request2size(req);
```

#### arena相关

##### 标志位相关

* [FASTCHUNKS_BIT](#FASTCHUNKS_BIT)
* [NONCONTIGUOUS_BIT](#NONCONTIGUOUS_BIT)
* [ARENA_CORRUPTION_BIT](#ARENA_CORRUPTION_BIT)

##### 其他

###### top

<span id="top"/>

```c
#define top(ar_ptr) ((ar_ptr)->top)
```

###### arena_lock

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

###### arena_get

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

* 调用[arena_lock](#arena_lock)来给当前线程的thread_arena上锁，并获取arena

###### arena_for_chunk

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

###### MALLOC_ALIGN_MASK

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

###### set_foot

设置下一个chunk块的mchunk_prev_size为当前chunk块大小，其中p指向当前chunk，s为当前chunk大小

```c
#define set_foot(p, s) (((mchunkptr)((char *)(p) + (s)))->mchunk_prev_size = (s))
```

###### chunsize_nomask

```c
#define chunksize_nomask(p) ((p)->mchunk_size)
```

###### chunksize

```c
#define chunksize(p) (chunksize_nomask(p) & ~(SIZE_BITS))
```

###### set_inuse_bit_at_offset

设置下一个chunk块的PREV_INUSE位

```c
#define set_inuse_bit_at_offset(p, s) \
  (((mchunkptr)(((char *)(p)) + (s)))->mchunk_size |= PREV_INUSE)
```

###### DUMPED_MAIN_ARENA_CHUNK

若指针落在 [dumped_main_arena_start](#dumped_main_arena_start) 与 [dumped_main_arena_end](#dumped_main_arena_end) 间则返回1

```c
#define DUMPED_MAIN_ARENA_CHUNK(p) \
  ((p) >= dumped_main_arena_start && (p) < dumped_main_arena_end)
```

##### REQUEST_OUT_OF_RANGE

```c
#define REQUEST_OUT_OF_RANGE(req) \
  ((unsigned long)(req) >=        \
   (unsigned long)(INTERNAL_SIZE_T)(-2 * MINSIZE))
```

#### chunk操作相关

##### unlink

unlink是一个宏，用于从bin上取下一块chunk。在从双链表中取出一个chunk外还执行了很多额外的检查

注意，unlink的后两个参数可以用来返回被unlink节点的前驱和后继

```c
#define unlink(AV, P, BK, FD) {                                            \
    if (__builtin_expect (chunksize(P) != prev_size (next_chunk(P)), 0))      \
      malloc_printerr ("corrupted size vs. prev_size");                  \
    FD = P->fd;                                      \
    BK = P->bk;                                      \
    if (__builtin_expect (FD->bk != P || BK->fd != P, 0))              \
      malloc_printerr ("corrupted double-linked list");                  \
    else {                                      \
        FD->bk = BK;                                  \
        BK->fd = FD;                                  \
        if (!in_smallbin_range (chunksize_nomask (P)) && __builtin_expect (P->fd_nextsize != NULL, 0))
        {                                                             \
            if (__builtin_expect (P->fd_nextsize->bk_nextsize != P, 0) || __builtin_expect (P->bk_nextsize->fd_nextsize != P, 0))    \
              malloc_printerr ("corrupted double-linked list (not small)");   \
          if (FD->fd_nextsize == NULL) {                            \
            if (P->fd_nextsize == P)                                \
                FD->fd_nextsize = FD->bk_nextsize = FD;              \
              else {                                  \
                  FD->fd_nextsize = P->fd_nextsize;                  \
                  FD->bk_nextsize = P->bk_nextsize;                  \
                  P->fd_nextsize->bk_nextsize = FD;                  \
                  P->bk_nextsize->fd_nextsize = FD;                  \
                }                                  \
            } else {                                  \
              P->fd_nextsize->bk_nextsize = P->bk_nextsize;              \
              P->bk_nextsize->fd_nextsize = P->fd_nextsize;              \
            }                                      \
          }                                      \
      }                                          \
}
```

设当前chunk指针为P，下述检查若有失败则会报错

* 检查后一个chunk（内存地址连续的下一个chunk）的prev_size是否为当前chunksize
* 检查下一个chunk（fd）的上一个chunk（bk）是否为P
* 检查上一个chunk（bk）的下一个chunk（fd）是否为P
* 将P从链表中取出（注意此时被取出的chunk中fd和bk没有被覆盖）
* 若P的大小为largebin且存在nextsize链表有后继节点（fd_nextsize）（注意，这说明该chunk在largebin中没有其他相同大小的chunk）
  * 检查下一个chunk（fd_nextsize）的上一个chunk（bk_nextsize）是否为P
  * 检查上一个chunk（bk_nextsize）的下一个chunk（fd_nextsize）是否为P
  * 判断下一个chunk在nextsize链表是否存在后继（`FD->fd_nextsize==NULL`）
    * 若不存在，说明FD应该是与P大小相同的largebin。判断当前chunk的nextsize链表后继是否是自身（`P->fd_nextsize == P`）
      * 若是，令FD的nextsize链表组成一个只含FD的环形链表（`FD->fd_nextsize = FD->bk_nextsize = FD`）
      * 若否，用FD替换原来P在nextsize链表上的位置
    * 若存在，直接将P从nextsize链表取下

##### REMOVE_FB

通过CAS原子操作将victim从链表上移除，注意这里是只使用了fd指针的单链表。其中pp是希望取得的指针，fb是&pp，victim返回取得的指针

操作若成功，`victim=pp  *fb=victim->fd`

```c
#define REMOVE_FB(fb, victim, pp) \
  do                              \
  {                               \
    victim = pp;                  \
    if (victim == NULL)           \
      break;                      \
  } while ((pp = catomic_compare_and_exchange_val_acq(fb, victim->fd, victim)) != victim);
```

在`_int_malloc`中被调用的方法是

```c
mchunkptr pp = *fb;
REMOVE_FB(fb, victim, pp);
```

#### bin相关

##### bin参数

###### NBINS

```c
#define NBINS 128
```

###### NFASTBINS

```c
#define NFASTBINS (fastbin_index(request2size(MAX_FAST_SIZE)) + 1)
```

###### NSMALLBINS

```c
#define NSMALLBINS 64
```

###### SMALLBIN_WIDTH

```c
#define SMALLBIN_WIDTH MALLOC_ALIGNMENT
```

###### SMALLBIN_CORRECTION

```c
#define SMALLBIN_CORRECTION (MALLOC_ALIGNMENT > 2 * SIZE_SZ)
```

###### MAX_FAST_SIZE

```c
#define MAX_FAST_SIZE (80 * SIZE_SZ / 4)
```

###### MIN_LARGE_SIZE

largebin的最小大小，也是smallbin的最大大小

```c
#define MIN_LARGE_SIZE ((NSMALLBINS - SMALLBIN_CORRECTION) * SMALLBIN_WIDTH)
```

##### bin范围相关

###### set_max_fast

```c
#define set_max_fast(s)                   \
  global_max_fast = (((s) == 0)           \
                         ? SMALLBIN_WIDTH \
                         : ((s + SIZE_SZ) & ~MALLOC_ALIGN_MASK))
```

###### get_max_fast

```c
#define get_max_fast() global_max_fast
```

###### in_smallbin_range

```c
#define in_smallbin_range(sz)  \
  ((unsigned long) (sz) < (unsigned long) MIN_LARGE_SIZE)
```

##### bin查找相关

###### bin_at

用于对[malloc_state](#malloc_state)结构体上的bins数组存放的各个bin链表进行寻址

```c
#define bin_at(m, i) \
  (mbinptr) (((char *) &((m)->bins[((i) - 1) * 2])) - offsetof (struct malloc_chunk, fd))
```

这里用到了一个比较令人费解的写法，可以[看这](#bin_at写法)

###### next_bin

```c
#define next_bin(b)  ((mbinptr) ((char *) (b) + (sizeof (mchunkptr) << 1)))
```

###### fastbin

```c
#define fastbin(ar_ptr, idx) ((ar_ptr)->fastbinsY[idx])
```

###### bin_index

```c
#define bin_index(sz) \
  ((in_smallbin_range (sz)) ? smallbin_index (sz) : largebin_index (sz))
```

###### fastbin_index

```c
#define fastbin_index(sz) \
  ((((unsigned int)(sz)) >> (SIZE_SZ == 8 ? 4 : 3)) - 2)
```

###### smallbin_index

```c
#define smallbin_index(sz) \
  ((SMALLBIN_WIDTH == 16 ? (((unsigned) (sz)) >> 4) : (((unsigned) (sz)) >> 3))\
   + SMALLBIN_CORRECTION)
```

###### largebin_index

```c
#define largebin_index(sz) \
  (SIZE_SZ == 8 ? largebin_index_64 (sz)                                     \
   : MALLOC_ALIGNMENT == 16 ? largebin_index_32_big (sz)                     \
   : largebin_index_32 (sz))
```

###### largebin_index_32

```c
#define largebin_index_32(sz)                                                \
  (((((unsigned long) (sz)) >> 6) <= 38) ?  56 + (((unsigned long) (sz)) >> 6) :\
   ((((unsigned long) (sz)) >> 9) <= 20) ?  91 + (((unsigned long) (sz)) >> 9) :\
   ((((unsigned long) (sz)) >> 12) <= 10) ? 110 + (((unsigned long) (sz)) >> 12) :\
   ((((unsigned long) (sz)) >> 15) <= 4) ? 119 + (((unsigned long) (sz)) >> 15) :\
   ((((unsigned long) (sz)) >> 18) <= 2) ? 124 + (((unsigned long) (sz)) >> 18) :\
   126)
```

###### largebin_index_32_big

```c
#define largebin_index_32_big(sz)                                            \
  (((((unsigned long) (sz)) >> 6) <= 45) ?  49 + (((unsigned long) (sz)) >> 6) :\
   ((((unsigned long) (sz)) >> 9) <= 20) ?  91 + (((unsigned long) (sz)) >> 9) :\
   ((((unsigned long) (sz)) >> 12) <= 10) ? 110 + (((unsigned long) (sz)) >> 12) :\
   ((((unsigned long) (sz)) >> 15) <= 4) ? 119 + (((unsigned long) (sz)) >> 15) :\
   ((((unsigned long) (sz)) >> 18) <= 2) ? 124 + (((unsigned long) (sz)) >> 18) :\
   126)
```

###### large_bin_index_64

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

##### binmap相关

###### BINMAPSHIFT

```c
#define BINMAPSHIFT 5
```

###### BITSPERMAP

每个block占用的比特位，为2^BINMAPSHIFT=32

```c
#define BITSPERMAP (1U << BINMAPSHIFT)
```

###### BINMAPSIZE

binmap的block数。binmap的每个位对应一个bin，因此每个block对应32个bin，一共就需要“总bin数除以每个block对应的bin数”个block，默认即为128/32=4

```c
#define BINMAPSIZE (NBINS / BITSPERMAP)
```

###### idx2block

i为bin的下标，这里即计算下标对应的binmap的block

```c
#define idx2block(i) ((i) >> BINMAPSHIFT)
```

###### idx2bit

i为bin的下标，这里计算bin在一个block内的偏移。低地址对应的下标较小

```c
#define idx2bit(i) ((1U << ((i) & ((1U << BINMAPSHIFT) - 1))))
```

注意这里的`((i) & ((1U << BINMAPSHIFT) - 1))`实际上就是计算`i % BITSPERMAP`，因为`(1U << BINMAPSHIFT)`即为block大小，`(1U << BINMAPSHIFT) - 1`就是对应模这个大小的掩码，后面进行&操作即取模

###### mark_bin

给binmap对应位打标

```c
#define mark_bin(m, i) ((m)->binmap[idx2block(i)] |= idx2bit(i))
```

###### unmark_bin

清除binmap对应位的标志

```c
#define unmark_bin(m, i) ((m)->binmap[idx2block(i)] &= ~(idx2bit(i)))
```

###### get_binmap

获取对应位的标志

```c
#define get_binmap(m, i) ((m)->binmap[idx2block(i)] & idx2bit(i))
```

#### 有趣的写法

##### bin_at写法

```c
#define bin_at(m, i) \
  (mbinptr) (((char *) &((m)->bins[((i) - 1) * 2])) - offsetof (struct malloc_chunk, fd))
```

为什么要减去offsetof(struct malloc_chunk, fd))。实际上是因为bin_at返回的类型是mbinptr，也就是malloc_chunk*，而实际上返回后被使用的只有fd和bk成员。换句话说，bin_at的使用场景如下

```c
mbinptr bin = bin_at(av, i);
mbinptr prev_chunk = bin->fd;
mbinptr next_chunk = bin->bk;
```

而实际上bins数组的每个元素只存储fd和bk，而非一个malloc_chunk，因此要作为malloc_chunk使用必须加上一个偏移才能正确寻找到对应的成员

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
  mstate ar_ptr; /* Arena for this heap. */            // 指向当前arena的malloc_state
  struct _heap_info *prev; /* Previous heap. */        // 指向前一个arena的heap_info
                                                //（实际上指向的就是前一个arena头）
  size_t size;   /* Current size in bytes. */        // 保存整个arena大小
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
  mfastbinptr fastbinsY[NFASTBINS];        // fastbin指针

  /* Base of the topmost chunk -- not otherwise kept in a bin */
  mchunkptr top;                        // top chunk 指针

  /* The remainder from the most recent split of a small request */
  mchunkptr last_remainder;

  /* Normal bins packed as described above */
  mchunkptr bins[NBINS * 2 - 2];        //NBINS=128

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

##### heap_trim

```c
static int internal_function heap_trim (heap_info *heap, size_t pad);
```

* #### 底层函数：arena相关

##### arena_get2

用于获取一个新的arena，根据条件可能通过`get_free_list / _int_new_arena / reused_arena`三种方法获取

size参数确定arena大小，avoid_arena仅作为参数传给[reused_arena](#reused_arena)，因为arena_get2函数仅在arena空间分配失败的情况下被调用，该指针指向上次分配失败的arena，这样在调用reused_arena时可以避免复用到之前分配失败的那个arena

```c
static mstate internal_function arena_get2(size_t size, mstate avoid_arena);
```

* 调用[get_free_list](#get_free_list)试图复用一个free_list上的arena
  * 若有直接返回新的arena
  * 若无，则首先确定arena数量的限制（static变量，只获取一次），方式如下
    * 若[mp_](#mp_)的[arena_max成员](#malloc_par)不为0，则直接以该值作为 `narenas_limit`
    * 否则，若[narenas](#narenas)大于mp_的arena_test成员
      * 调用__get_nprocs获取计算机核心数
      * 若返回值大于等于1，调用[NARENAS_FROM_NCORES](#NARENAS_FROM_NCORES)确定arena数，以该值为 `narenas_limit`
      * 否则说明该调用无法正确获取核心数，假设核心数为2，调用NARENAS_FROM_NCORES，以该值为 `narenas_limit`
  * `narenas <= narenas_limit - 1`是否成立
    * 若是，说明允许创建新的arena
      * 首先试图使用CAS操作（[catomic_compare_and_exchange_bool_acq](#catomic_compare_and_exchange_bool_acq)）为narenas加一，若CAS测试失败（即`narenas != oldval`）则跳回`narenas_limit-1`判断重新进行判断
      * 调用[_int_new_arena](#_int_new_arena)创建大小为size的arena
      * 若返回NULL，说明创建arena失败，使用原子操作[catomic_decrement](#catomic_decrement)对narenas减一
    * 若否，说明当前arena过多，不允许继续创建新的arena
      * 调用[reused_arena](#reused_arena)获取arena

##### _int_new_arena

新建一个arena，每个arena的内存布局见[arena内存布局](malloc_overview.md#arena内存布局)

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

<span id="int_new_arena_bug"/>

这里在最后加入main_arena列表和detach_arena时涉及了一些多线程操作

注释里说：最后一步给新的arena上锁是因为在这个时刻新的arena已经可以被其他线程访问了（通过main_arena.next），并且可能已经被reused_arena获取，这种情况仅在最后一个arena被创建时（arena_max-1）可能发生（因为这时候才可能既有调用`_int_new_arena`的线程又有调用`reused_arena`的线程），这时一个arena将会被挂载到两个线程上。虽然如果将这句放到获取list_lock前可以避免这种情况，但这可能造成与[__malloc_fork_lock_parent](#__malloc_fork_lock_parent)函数死锁

这里的意思应该是在在获取list_lock前将arena上锁，可能与`__malloc_fork_lock_parent`死锁，因为该函数中会先获取list_lock，再将所有的arena上锁。但这里为什么不能先获取list_lock，对arena上锁，再释放list_lock呢，这个方案似乎可以避免上述两个问题，因为这个方法会使得list_lock的临界区代码中操作了与其无关的资源么（list_lock本应只是用来对arena的链表这一资源进行控制的）

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
* 上锁[free_list_lock](#free_list_lock)，调用[detach_arena](#detach_arena)将当前的thread_arena取下
* 调用[remove_from_free_list](#remove_from_free_list)将result指向的arena取下，attached_threads成员加一，此后解锁free_list_lock
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

将原来的thread_arena解绑，并从free_list获取一个arena，赋值给thread_arena

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
    * 解锁 free_list_lock
    * 给新arena（result指向的元素）上锁
    * `thread_arena = result`

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
    
    注意这里分配了`HEAP_MAX_SIZE*2`大小的空间，令p2为p1的下一个与HEAP_MAX_SIZE对齐的地址，`ul = p2-p1`
    
    * 若ul=0，说明p1本身就是HEAP_MAX_SIZE对齐的
      
      * 会将**aligned_heap_area**的值赋为`p1+HEAP_MAX_SIZE`（这里见下面的进一步说明）。即下次调用new_heap的时候会使用第一种方式mmap，并且新创建的heap块在上一次创建的heap块的上方（高地址）。
      
      * 将p1块后多余的HEAP_MAX_SIZE释放
    
    * 若ul不等于0，说明p1不是HEAP_MAX_SIZE对齐的
      
      * 将**aligned_heap_area**的值赋为`p2+HEAP_MAX_SIZE`
      
      * 将p2块前后的空间释放
    
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

上述这种做法的问题在于，可能造成一些地址碎片，假设mmap分配的地址为 `0x00601000~0x00801000` （32位，大小2MB），则要符合HEAP_MAX_SIZE对齐（1MB对齐），则使用的地址为 `0x00700000~0x00800000` ，则会产生两片碎片（ `0x00601000~0x0060ffff` 和`0x00800000~0x00801000` ）

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
* 判断top chunk是否等于`heap+sizeof(heap_info)`，即当前的top chunk是否为heap的第一个chunk
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

将list_lock中的每个arena上锁，在父进程fork前调用

```c
void internal_function __malloc_fork_lock_parent(void);
```

* 检查[__malloc_initialized](#__malloc_initialized)，若未初始化则直接返回
* 给[list_lock](#list_lock)上锁
* 以[main_arena](#main_arena)为起点，获取arena链表，并将每个arena对应的[malloc_state](#malloc_state)上锁（mutex成员）

注意最后list_lock没有解锁

##### __malloc_fork_unlock_parent

将list_lock中的每个arena解锁，在父进程fork后调用

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
  int n_mmaps;            // 记录当前使用mmap分配的chunk个数
  int n_mmaps_max;
  int max_n_mmaps;        // 记录n_mmaps的最大值
  /* the mmap_threshold is dynamic, until the user sets
     it manually, at which point we need to disable any
     dynamic behavior. */
  int no_dyn_threshold;

  /* Statistics */
  INTERNAL_SIZE_T mmapped_mem;        // mmap分配的内存大小
  INTERNAL_SIZE_T max_mmapped_mem;    // 记录mmap分配的内存最大值

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

默认的malloc_par参数

```c
static struct malloc_par mp_ =
{
  .top_pad = DEFAULT_TOP_PAD,            //default: 0
  .n_mmaps_max = DEFAULT_MMAP_MAX,        //default: 65536
  .mmap_threshold = DEFAULT_MMAP_THRESHOLD,    //default: 128*1024
  .trim_threshold = DEFAULT_TRIM_THRESHOLD,    //default: 128*1024
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

##### global_fast_max

fastbin的最大值

```c
static INTERNAL_SIZE_T global_max_fast;
```

##### dumped_main_arena

用于支持undumping（[DUMPED_MAIN_ARENA_CHUNK](#DUMPED_MAIN_ARENA_CHUNK)），对于落在这个范围内的mmap分配的chunk我们可以放任不管，因为这类chunk标记为IS_MMAPED但不会被释放

###### dumped_main_arena_start

```c
static mchunkptr dumped_main_arena_start;
```

###### dumped_main_arena_end

```c
static mchunkptr dumped_main_arena_end;
```

#### 顶层函数

##### __libc_malloc

```c
void *__libc_malloc(size_t bytes);
```

* hook相关
  * [__libc_malloc](#HOOK__libc_malloc) 
* TCACHE相关
  * a
* 使用[arena_get](#arena_get)试图获取arena
* 使用[_int_malloc](#_int_malloc)分配内存
* 若获取失败，且之前获取到了arena
  * [arena_get_retry](#arena_get_retry)试图获取一个新的arena
  * 再次_int_malloc
* 若第一次获取的arena指针非空，则解锁arena，返回

###### 关于libc_malloc的锁操作

这里值得注意的是对`ar_ptr->mutex`的解锁与上锁，该函数中只进行了一次解锁，但实际上可以看看调用链上的各个函数对锁的处理

首先因为只对ar_ptr非空的情况解锁，因此显然锁是在获取arena时上的

调用链：

* arena_get -> arena_lock
  * 若传入的ar_ptr非空且没有corrupt则上锁
  * 否则调用arena_get2
* arena_get_retry
  * 若为main_arena则经历一次解锁-判断-上锁的过程
  * 否则调用arena_get2
* arena_get2
  * 通过三种方式获取一个arena，可以见具体说明。每种方式若非空，则返回时都会给ar_ptr->mutex上锁

出现NULL的情况

* arena_get_retry中main_arena出现corrupt
* arena_get2
  * `_int_new_arena`两次调用`new_heap`都失败，注意这里代码写的是return 0，可能出错（虽然目前所有编译器下NULL==0）
  * `reuse_arena`中没有在arena链表上找到合适的arena

##### __libc_free

```c
void __libc_free (void *mem);
```

* hook相关
  
  * [__libc_free](#HOOK_libc_free)

* 若mem为0（即NULL），直接返回

* 获取对应chunk，判断是否为mmap分配 [chunk_is_mmapped](#chunk_is_mmapped)
  
  * 若是，这里有一个动态调整mmap threshold的机制
    
    * 若 [mp_](#mp_) 的`no_dyn_threshold` 成员为0，说明开启了动态调整机制
    
    * 且chunk大小大于 [mp_](#mp_) 的 `mmap_threshold` 成员且小于 [DEFAULT_MMAP_THRESHOLD_MAX](#DEFAULT_MMAP_THRESHOLD_MAX)
    
    * 且chunk不在 [DUMPED_MAIN_ARENA_CHUNK](#DUMPED_MAIN_ARENA_CHUNK) 范围内
      
      * 更新 [mp_](#mp_) 的 `mmap_threshold` 为该chunk的大小
      
      * 更新 [mp_](#mp_) 的 `trim_threshold` 为双倍的 `mmap_threshold` 大小
  
  * 调用 [munmap_chunk](#munmap_chunk) 释放chunk
  
  * 调用 [MAYBE_INIT_TCACHE](#MAYBE_INIT_TCACHE) 初始化TCACHE（若允许TCACHE）
  
  * 调用 [arena_for_chunk](#arena_for_chunk) 获取对应的arena
  
  * 调用 [_int_free](#_int_free) 释放chunk

##### __libc_realloc

```c
void *__libc_realloc (void *oldmem, size_t bytes);
```

* hook相关
  
  * [__libc_realloc](#HOOK_libc_realloc)

* 若定义了宏 `REALLOC_ZERO_BYTES_FREES` ，则当bytes参数为0时视为调用 [__libc_free](#__libc_free)

* 若old_mem为0，则视为调用 [__libc_malloc](#__libc_malloc) ，返回其结果

* 对指针所在的地址进行简单的检查，主要是检查是否对齐、是否存在地址空间的溢出（wrap around）

* 调用 [checked_request2size](#checked_request2size) 检查bytes是否符合大小要求

* 若原来的chunk是使用mmap分配的
  
  * 若是faked mmaped chunk （[DUMPED_MAIN_ARENA_CHUNK](#DUMPED_MAIN_ARENA_CHUNK)）则不需要free原来的chunk
    
    * 调用 [__libc_malloc](#__libc_malloc) 分配内存
      
      * **若分配失败，直接返回NULL**
    
    * 将旧chunk的内容复制到新的chunk，若旧chunk比新的大，则截断复制
    
    * **返回chunk指针**
  
  * 若定义了HAVE_MREMAP，即有mremap函数
    
    * 调用 [mremap_chunk](#mremap_chunk) 分配新的chunk
    
    * 若分配成功，**返回新的chunk**
  
  * 若原来mmap的内存块大于新的请求，则 **直接返回原来的块**

* 运行到这，说明chunk不是用mmap分配的，使用 [MAYBE_INIT_TCACHE](#MAYBE_INIT_TCACHE) 初始化TCACHE并调用 [arena_for_chunk](#arena_for_chunk) 获取当前chunk的arena（其实这一步是在前面做的，但为了清晰这边放到后面来了）
  
  * 为arena上锁
  
  * 调用 [_int_realloc](#_int_realloc) 重新分配chunk
  
  * 为arena解锁

* 若新分配的chunk为NULL
  
  * 调用 [_libc_malloc](#_libc_malloc) 分配新内存
  
  * 若上面分配成功，则将旧内存复制到新内存，并调用 [_int_free](#_int_free) 释放旧的内存

##### _libc_memalign







#### 主要函数

##### _int_malloc

```c
static void *_int_malloc(mstate av, size_t bytes);
```

* 调用[checked_request2size](#checked_request2size) 转换大小
* 若传入的av为NULL
  * 调用[sysmalloc](#sysmalloc)分配内存
  * 若分配成功，调用[alloc_perturb](#alloc_perturb) 初始化
  * 返回
* 下面的流程根据bytes大小在各种bin里寻找内存块
  * fastbin：根据 [get_max_fast](#get_max_fast) 判断是否在fastbin范围内
    * 使用[fastbin_index](#fastbin_index)和[fastbin](#fastbin)根据大小获取对应的 malloc_state->fastbinsY 块指针
    * 准备获取块上的头一个指针，使用[REMOVE_FB](#REMOVE_FB)取得一块fastbin
    * 若获取的指针非NULL
      * 检查了取得的块大小是否等于对应fastbinsY数组的块的大小
      * check函数相关：
        * 调用了[do_check_remalloced_chunk](#do_check_remalloced_chunk) 检查
      * TCACHE相关：
        * TODO
      * 调用[alloc_perturb](#alloc_perturb)填充块
      * **返回chunk**
  * smallbin：根据[in_smallbin_range](#in_smallbin_range) 判断是否在smallbin范围内
    * 使用[smallbin_index](#smallbin_index)和[bin_at](#bin_at)根据大小获取对应的 malloc_state->bins 块指针
    * 若获取的元素的bk不是自身，说明该bins被初始化过，链表上有元素
      * 若指针为NULL
        * 调用[malloc_consolidate](#malloc_consolidate)
      * 否则说明获取到了对应的内存块，假设为p
        * 设置当前块在内存上相邻chunk的PREV_INUSE位（[set_inuse_bit_at_offset](#set_inuse_bit_at_offset)）
        * 更新bin链表 `bin->bk = p->bk;  p->bk->fd = bin;`
        * 若av不是main_arena，则设置p的NON_MAIN_ARENA位
        * check函数相关
          * 调用 [check_malloced_chunk](#check_malloced_chunk)
        * TCACHE相关
          * TODO
        * 调用[alloc_perturb](#alloc_perturb)填充块
        * **返回chunk**
  * 剩下的情况可能是一个largebin的请求，或是fastbin和smallbin都没有找到满足需求的块，因此进行下一步处理
    * 首先若当前arena含有fastbin（[have_fastchunks](#have_fastchunks)），先执行[malloc_consolidate](#malloc_consolidate)
  * TCACHE相关：
    * TODO
  * 下面会循环尝试（注意这个死循环囊括了下面所有的步骤）
    * unsorted_chunk：若unsorted_chunk非空，循环遍历unsorted_chunk。这里设置了单次遍历大小的上限为10000次，即如果unsorted_chunk大于10000就算没到头也不遍历了
      * 情况一：为small request，遍历到的chunk为当前unsorted_chunk的唯一一个元素且大于请求+MINSIZE（因为切分后另一块chunk的最小大小为MINSIZE）。**此时不会使用best-fit算法，而是会切分该chunk**
        * [切分chunk](#切分chunk)。这里因为chunk为unsorted chunk的唯一元素，因此多了一步：设置[malloc_state](#malloc_state)的last_remainder成员为该被切分出的chunk
        * check相关：
          * [check_malloced_chunk](#check_malloced_chunk)
        * [alloc_perturb](#alloc_perturb)填充字节
        * **返回chunk**
      * 情况二：遍历到的chunk刚好等于请求大小
        * 设置下一个chunk的PREV_INUSE（[set_inuse_bit_at_offset](#set_inuse_bit_at_offset)）
        * TCACHE相关：
          * TODO
        * 根据情况设置chunk的MAIN_ARENA位
        * check相关：
          * [check_malloced_chunk](#check_malloced_chunk)
        * [alloc_perturb](#alloc_perturb)填充字节
        * **返回chunk**
      * 情况三：说明遍历到的chunk不满足需求，则将其从unsorted chunk取出并整理到small_bin和large_bin上
        * 若chunk大小符合small_bin
          * [smallbin_index](#smallbin_index)和[bin_at](#bin_at)取对应bin
        * 否则放入large_bin，这里large_bin的nextsize链表是从大到小排序的，因此有下面的处理
          * [largebin_index](#smallbin_index)和[bin_at](#bin_at)取对应bin
          * 若bin链表非空
            * 若chunk小于当前的最后一个元素（也即bin头的bk元素），这里是一个优化，否则需要从前一直遍历到尾
              * 将chunk插入nextsize链表。注意nextsize链表不包含bin头
            * 否则从第一个元素往后遍历，直到找到不大于当前size的chunk
              * 若找到的chunk与当前chunk大小相等，则不对nextsize链表进行操作。下面的操作会将chunk插入普通链表中的下一项（fd指向的元素）
              * 否则说明找到chunk小于当前chunk，则将chunk插入nextsize链表中该chunk之前
          * 若bin链表为空
            * 将fd_nextsize和bk_nextsize都指向自身
        * 将chunk插入smallbin或largebin。并设置对应的binmap（[mark_bin](#mark_bin)）
        * TCACHE相关：
          * TODO
    * 运行到这说明没有找到合适的unsorted_chunk，下面分为几种情况
      * 如果是large request
        * 若对应largebin链表非空，且第一个元素大小大于请求
          * 遍历，直到找到一个小于当前request的元素   TODO：如果没有怎么办
          * 若该元素不是最后一个元素（防止把bin数组当做malloc chunk结构求大小，因为这无意义甚至可能导致bug），且当前chunk大小等于下一个chunk
            * 则取下一个chunk为备选的chunk。因为对于largebin来说，若有多个largebin大小相同，则除了第一个chunk作为头结点串在nextsize链表上，其他chunk只会被依次串在普通链表上。因此取第二个chunk可以避免重新调整nextsize链表的头结点
          * [unlink](#unlink)取下备选的chunk
          * 该chunk减去请求大小是否大于MINSIZE
            * 若否，不需要切分chunk，设置chunk的MAIN_ARENA和下一个chunk的PREV_INUSE
            * 若是，切分chunk
              * [切分chunk](#切分chunk)
              * check相关：
                * [check_malloced_chunk](#check_malloced_chunk)
              * [alloc_perturb](#alloc_perturb)填充字节
              * **返回chunk**
      * 到这说明与请求相符的largebin或smallbin没有找到合适的块，因此尝试从更大的bin寻找合适的块
        * 首先比对binmap来测试比请求更大的largebin有没有非空的bin，注意这里的binmap即使为1，对应的bin可能也是空的，因为在从bin中取chunk时并不会判断取走后bin是否为空。因此对binmap的修正实际上是在这个地方做的
          * 若binmap中大于请求的bin都没有内存块，直接跳到下面的use_top执行
          * 否则说明有对应的bin，则首先判断当前的bin链表是否为空
            * 若不为空，则[切分chunk](#切分chunk)。注意这里若新chunk为smallbin，会设置[malloc_state](#malloc_state)的last_remainder成员为该被切分出的chunk
            * check相关：
              * [check_malloced_chunk](#check_malloced_chunk)
            * [alloc_perturb](#alloc_perturb)填充字节
            * **返回chunk**
          * 若为空，则修正binmap（即对应该bin的项设为0），并设置将要检测的bin为下一个bin，返回到上面比对binmap的地方继续执行
      * use_top：到这说明binmap中没有对应的bin，尝试使用top chunk。top chunk实际上是arena中地址最高的一个chunk，所有其他bin的地址都要低于top chunk。top chunk理论上可以应对任意大小的内存分配，因为其可以一直向高地址扩展直到地址空间发生冲突（基本不可能）
        * 若top chunk的大小>请求大小+MINSIZE
          * 切分chunk，这里与先前的切分chunk步骤有点不一样
            * 把切分出的chunk作为新的top chunk（av->top）
            * 设置请求chunk的大小、PREV_INUSE，并根据情况设置MAIN_ARENA位（[set_head](#set_head)）
            * 设置切分出的chunk的大小和PREV_INUSE（[set_head](#set_head) ）。因为没有下一个chunk所以不用set_foot
          * check相关：
            * [check_malloced_chunk](#check_malloced_chunk)
          * [alloc_perturb](#alloc_perturb)填充字节
          * **返回chunk**
        * 若当前arena有fastchunk（[have_fastchunks](#have_fastchunks)），则执行一次[malloc_consolidate](#malloc_consolidate)，看看能不能合并内存从而在下一趟循环中满足请求。注意可能**有两种情况到达这并进行下一次循环**
          * small request，且开头寻找smallbin和fastbin没有找到对应的，且在unsorted chunk遍历到最后一个元素时刚好小于该request，且大于该request的smallbin的bin中都没有合适元素
          * large request，且且在unsorted chunk遍历到最后一个元素时刚好小于该request，且大于该request的largebin的bin中都没有合适元素，且大于top chunk
        * 否则使用最终方法，调用[sysmalloc](#sysmalloc)分配内存
          * 若分配到，[alloc_perturb](#alloc_perturb)填充字节，否则就直接为NULL
          * **返回chunk**

###### 切分chunk

因为该函数用到多次这个过程，但没有独立写一个函数，为了节省篇幅单独写在这

* 将该chunk大于请求的部分单独切分出一个新的chunk，并将其放入unsorted_chunk的头部
* 若切分出的chunk是large chunk，则清零其fd_nextsize和bk_nextsize
* 设置请求chunk的大小、PREV_INUSE，并根据情况设置MAIN_ARENA位（[set_head](#set_head)）
* 设置切分出的chunk的大小和PREV_INUSE，更新下一个chunk的prev_size（[set_head](#set_head)  [set_foot](#set_foot)）

##### _int_free

free的底层函数，注意这里的have_lock参数用于标识当前的函数调用中允不允许获取arena锁

```c
static void _int_free(mstate av, mchunkptr p, int have_lock);
```

* 进行一些简单的检查，若不满足则报错
  * 是否对齐
  * 是否存在地址溢出的情况（即p+size导致地址空间溢出，判断条件是p>-size，基本不可能出现）
  * chunk size是否小于MINSIZE
* check相关：
  * [check_inuse_chunk](#check_inuse_chunk)
* TCACHE相关：
  * TODO
* 情况一：释放到fastbin。若大小小于fastbin的阈值，这里若设置了 [TRIM_FASTBINS](#TRIM_FASTBINS) 还会要求p的下一个chunk不是top chunk
  * *检查*：下一个chunk大小是否小于等于2*SIZE_SZ，或大于malloc_state的system_mem，若是说明有错误
    * 若have_lock为1，则需要获取`av->mutex`后重新测试，若还满足上面的条件，则报错
    * 若have_lock为0，则这里试图解锁`av->mutex`
  * 调用[free_perturb](#free_perturb)覆盖要释放的内存
  * 调用[set_fastchunks](#set_fastchunks)置位fastbin标志，表示当前有fastbin可用
  * 根据chunk大小将其链入对应的fastbin数组项上，这里使用原子操作链入，因此不需要获取锁。同时也因此在其他线程调用malloc库中的函数时随时可能有fastbin链入
    * *检查*：p是否等于对应数组项的指针，若等于则说明p已经是fastbin中的第一个元素了，存在double free，报错（注意这里只检查了第一个元素）
    * 这里若have_lock为1且原fastbin指针不为NULL，则根据原fastbin指针对应chunk的size获取了对应的old_index，这里是为了之后的检查
    * 使用原子操作 [catomic_compare_and_exchange_val_rel](#catomic_compare_and_exchange_val_rel) 将fastbin数组头指针修改为p
  * *检查*：若have_lock为1且原fastbin指针不为NULL，则检查old_index是否等于p对应的fastbin index，若不等则报错
* 情况二：chunk不是由mmap分配的
  * 若
* 情况三：chunk是由mmap分配的
  * 调用[munmap_chunk](#munmap_chunk)

##### malloc_consolidate

有两个功能，其中合并碎片的功能只有在当前malloc_state被初始化后才可用：

* 初始化malloc_state结构体
* 合并内存碎片，注意这里合并的原理是**从fastbin出发**，遍历fastbin的所有块并查找每个块的**上下两个相邻块**是否可以合并

在注释里对这个函数的解释大意如下：这个函数主要是用来从fastbin合并的。这里的过程其实在free中有类似的代码，但因为free本身会把chunk放到fastbin上，因此单独用了这样一个函数。此外因为在程序第一次调用malloc时需要初始化malloc_state，在这个函数里进行这个初始化是最好的选择

```c
static void malloc_consolidate(mstate av);
```

通过[get_max_fast](#get_max_fast)是否为0判断malloc_state是否初始化过

* 不为0，说明初始化过，则进行下面的操作
  
  * 遍历fastbin数组，对每个fastbin执行下列操作
    
    * 调用[atomic_exchange_acq](#atomic_exchange_acq)，将fastbin数组的对应指针置为NULL
    
    * 若该数组项先前的指针不为NULL，即说明该fastbin含有chunk
      
      则遍历chunk，并进行下列操作
      
      * check相关：
        
        * [check_inuse_chunk](#check_inuse_chunk)
      
      * 检查当前chunk的上一个chunk是否在使用（[prev_inuse](#prev_inuse)）
        
        * 若没有在使用，则使用[unlink](#unlink)将其从链表上取出（这里的链表可以是各种bin的，因为无法保证fastbin块的相邻chunk处于哪个bin）
        * 将合并的起始指针调整到上一个chunk处，并增加合并大小（这一步即把上一个chunk合并）
      
      * 检查当前chunk的下一个chunk是否为top chunk
        
        * 若不是top chunk
          
          * 检查下一个chunk是否inuse（这里需要到下一个chunk的再下一个chunk检查PREV_INUSE标志位）
            
            * 若inuse，则简单地将其PREV_INUSE位清零（因为下一个chunk的上一个chunk，即当前chunk已经不再inuse）
              
              ​    TODO：为什么在chunk链入fastbin时没有清零下一个chunk的PREV_INUSE
            
            * 否则，将下一个chunk [unlink](#unlink)，并增加合并大小
          
          * 设置合并得到的chunk，包括判断是否在largebin范围内，若是清零nextsize，还有设置chunk大小（[set_head](#set_head) [set_foot](#set_foot)）
          
          * 并将其链入unsorted chunk
        
        * 若是top chunk
          
          * 直接将当前chunk与top chunk合并，并设置新的大小和新的top chunk地址为当前指针

* 为0，说明没初始化过（TODO：什么情况下会需要在这里初始化？）
  
  * 调用[malloc_init_state](#malloc_init_state)初始化
  * check相关：
    * [check_malloc_state](#check_malloc_state)

##### malloc_init_state

初始化malloc_state

```c
static void malloc_init_state(mstate av);
```

* 将每个`malloc_state->bin`元素链表初始化为fd和bk都指向自身
* 若define了[MORECORE_CONTIGUOUS](#MORECORE_CONTIGUOUS)且av为main_arena
  * 设置av的noncontiguous标志
* 若av为main_arena
  * 设置fastchunk大小
* av的FASTCHUNKS_BIT置位
* 将top设置为av的unsorted chunk（即bins[1]）

##### sysmalloc

使用系统调用分配空间，**此时认为当前的av->top没有足够空间处理nb字节的分配**。这里要注意，nb是已经经过 [request2size](#request2size) 转换后的字节数

```c
static void *sysmalloc(INTERNAL_SIZE_T nb, mstate av);
```

###### 描述

* 如果av为空，或`nb > mp_.mmap_threshold && mp_.n_mmaps < mp_.n_mmaps_max`，则直接使用mmap
  
  * 首先将请求的大小nb转化为页面大小，注意这里使用mmap分配chunk，比普通的chunk多了一个SIZE_SZ大小的长度，因为普通chunk在分配后，后一个chunk的prev_size域可以被使用，而mmap分配的chunk没有连续的下一个chunk，因此需要多一个SIZE_SZ分配下一个chunk的prev_size
    
    * 由于上述原因，转化调用的是`ALIGN_UP(nb + SIZE_SZ, pagesize)`，此外若自定义了MALLOC_ALIGNMENT，调用`ALIGN_UP(nb + SIZE_SZ + MALLOC_ALIGN_MASK, pagesize)`
  
  * 调用mmap前有一个有趣的处理，就是判断转化后的大小是否大于nb。这应该是为了防止整数溢出，具体见 [有趣的写法](#interesting_sysmalloc)
  
  * 使用mmap时，若分配成功
    
    * 首先会检查chunk对齐，并调整chunk头到对齐的内存
    
    * 分配结束后malloc_chunk用途如下
      
      ```c
      struct malloc_chunk {
      
        INTERNAL_SIZE_T      mchunk_prev_size;  
                          //若当前mmap后返回的内存不是MALLOC_ALIGNMENT对齐的
                          //即chunk2mem(mm)不是MALLOC_ALIGNMENT对齐的
                          //则需要调整为MALLOC_ALIGNMENT对齐，方法为不使用前n字节
                          //mchunk_prev_size保存着n。否则为0
        INTERNAL_SIZE_T      mchunk_size;
                          //保存除去不使用的字节外剩下的大小，这个大小是pagesize对齐的
                          //这里的IS_MAPPED位被设置
      
        //---------------------------------------------------------------------------
          //下面直接是用户使用的空间，malloc返回的地址即fd成员的地址
        struct malloc_chunk* fd;         /* double links -- used only if free. */
        struct malloc_chunk* bk;
      
        /* Only used for large blocks: pointer to next larger size.  */
        struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
        struct malloc_chunk* bk_nextsize;
      };
      ```
    
    * 更新[mp_](#mp_)的下列成员，这里是使用原子操作来更新值的
      
      * n_mmaps  当前mmap个数加一
      * max_n_mmaps  若当前mmap个数大于最大值，则覆盖
      * mmaped_mem  当前通过mmap分配的内存大小增加
      * max_mmaped_mem  若当前mmap的内存大于最大值，则覆盖
    
    * 直接**返回使用mmap分配的chunk**

* 若av为空，则没有arena可以分配，且无法使用mmap分配chunk，直接**返回NULL**

* 若av不为main_arena
  
  * 试图使用[grow_heap](#grow_heap)直接增加top_chunk大小以满足需求（若剩余的heap空间足够）
    * 若成功则设置对应的`av->system_mem`和`av->top.mchunk_size`
  * 试图使用[new_heap](#new_heap)分配一个新的heap块
    * 若成功则将新的堆块加入heap链表（[heap_info](#heap_info)的prev成员），设置`heap->ar_ptr`使新的heap块称为当前使用的堆块，为`av->system_mem`增加当前堆块大小，并且设置新分配heap块的top chunk相关的信息
    * 下面的操作按照注释的说法，是为了给被置换出的旧的heap创建一个fencepost，并free掉top chunk。这个fencepost至少有MINSIZE字节大小
      * 检测top chunk大小是否大于MINSIZE **TODO：补完**
        * 若大于，则最后会调用 [_int_free](#_int_free) 释放top
  * 若上述尝试都失败且未尝试过mmap，直接回到上面试图mmap。注意这里不对先前的mmap条件做检查

* 若av为main_arena

#### 其他函数

##### alloc_perturb

使用memset赋初值，初值为perturb_byte^0xff

```c
static void alloc_perturb(char *p, size_t n);
```

##### free_perturb

使用memset赋初值，初值为perturn_byte，注意与[alloc_perturb](#alloc_perturb)刚好是非的关系

```c
static void free_perturb(char *p, size_t n);
```

##### munmap_chunk

释放一个使用mmap分配的chunk

```c
static void internal_function munmap_chunk(mchunkptr p);
```

* 检查p的地址范围是不是在[DUMPED_MAIN_ARENA_CHUNK](#DUMPED_MAIN_ARENA_CHUNK) ,若是直接返回，因为这类chunk标记为IS_MMAPED但不会被释放
* 检查当前chunk的size加上prev_size是否与pagesize对齐，这里prev_size存放的是为了对齐MALLOC_ALIGNMENT而与mmap所得到的内存地址的偏移，详见[sysmalloc](#sysmalloc)
* [mp_](#mp_)的n_mmaps原子减一，mmaped_mem原子减去该chunk大小
* 调用`__munmap`释放内存

#### check函数

malloc.c中有一些check当前运行状态的函数，有助于理解运行时malloc的各个结构布局

###### do_check_chunk

```c
static void do_check_chunk(mstate av, mchunkptr p);
```

#### hook

malloc提供了一系列插入hook函数的点，插入hook的形式主要以下列代码给出

```c
  void *(*hook)(size_t, const void *) = atomic_forced_read(__malloc_hook);
  if (__builtin_expect(hook != NULL, 0))
    return (*hook)(bytes, RETURN_ADDRESS(0));
```

其中上面代码的hook函数即名为`__malloc_hook`的函数指针

下面列出了几个主要的hook函数和hook的点

##### HOOK_libc_malloc

* `__malloc_hook`  进入函数时

##### HOOK_libc_free

* `__free_hook` 进入函数时

##### HOOK_libc_realloc

* `__realloc_hook` 进入函数时

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

<span id="interesting_sysmalloc"/>

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

看起来size肯定大于nb，但一种情况例外：当`nb+SIZE_SZ >= (size_t)-pagesize`时，即nb很大，以至于加上pagesize后会导致size_t溢出，这时size输出为0。如果不对这种情况进行特殊处理可能导致安全问题; 

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

[见描述](#int_new_arena_bug)

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
