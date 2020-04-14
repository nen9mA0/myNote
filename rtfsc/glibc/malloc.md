### arena

#### 数据类型

##### mchunkptr

```c
typedef struct malloc_chunk *mchunkptr;
```

##### mbinptr

```c
typedef struct malloc_chunk *mbinptr;
```

##### heap_info

```c
typedef struct _heap_info
{
  mstate ar_ptr; /* Arena for this heap. */
  struct _heap_info *prev; /* Previous heap. */
  size_t size;   /* Current size in bytes. */
  size_t mprotect_size; /* Size in bytes that has been mprotected
                           PROT_READ|PROT_WRITE.  */
  /* Make sure the following data is properly aligned, particularly
     that sizeof (heap_info) + 2 * SIZE_SZ is a multiple of
     MALLOC_ALIGNMENT. */
  char pad[-6 * SIZE_SZ & MALLOC_ALIGN_MASK];
} heap_info;
```

##### malloc_chunk

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
  mchunkptr top;

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

#### 锁

##### list_lock

```c
__libc_lock_define_initialized (static, list_lock);
```

##### free_list_lock

```c
__libc_lock_define_initialized (static, free_list_lock);
```

#### 全局变量

##### main_arena

```c
static struct malloc_state main_arena =
{
  .mutex = _LIBC_LOCK_INITIALIZER,
  .next = &main_arena,
  .attached_threads = 1
};
```

##### thread_arena

`malloc_state*`结构体指针，注意这里是TLS，每个线程都有

```c
static __thread mstate thread_arena attribute_tls_model_ie;
```

#### 主要函数

##### ptmalloc_init

主要用于malloc_state的初始化，还有对一些环境变量中设置的调试选项的处理

```c
static void ptmalloc_init(void);
```

* 将main_arena赋值给thread_arena
* 调用malloc_init_state初始化main_arena

### malloc

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
  .top_pad = DEFAULT_TOP_PAD,			//131072
  .n_mmaps_max = DEFAULT_MMAP_MAX,		//65536
  .mmap_threshold = DEFAULT_MMAP_THRESHOLD,	//128*1024
  .trim_threshold = DEFAULT_TRIM_THRESHOLD,	//128*1024
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



#### 宏

##### chunk2mem

```c
#define chunk2mem(p)   ((void*)((char*)(p) + 2*SIZE_SZ))
```

* mem  返回给用户的指针
* chunk  malloc_chunk头

##### mem2chunk

```c
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))
```

##### next_chunk

p为当前chunk指针，p+当前chunksize为nextchunk指针

```c
#define next_chunk(p) ((mchunkptr) (((char *) (p)) + chunksize (p)))
```

##### prev_chunk

p为当前chunk指针，返回前一个chunk指针

```c
#define prev_chunk(p) ((mchunkptr) (((char *) (p)) - prev_size (p)))
```

##### chunk_at_offset

将p+s指向的地址作为指针

```c
#define chunk_at_offset(p, s)  ((mchunkptr) (((char *) (p)) + (s)))
```

##### MIN_CHUNK_SIZE

MIN_CHUNK_SIZE是没对齐的最小chunk size

MINSIZE是对齐后的最小chunk size

```c
#define MIN_CHUNK_SIZE        (offsetof(struct malloc_chunk, fd_nextsize))
#define MINSIZE  \
  (unsigned long)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))
```

##### request2size

若小于MINSIZE则为MINSIZE，若大于则为对齐后的

```c
#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)
```

##### bin

###### bin_at

```c
#define bin_at(m, i) \
  (mbinptr) (((char *) &((m)->bins[((i) - 1) * 2]))			      \
             - offsetof (struct malloc_chunk, fd))
```

###### next_bin

```c
#define next_bin(b)  ((mbinptr) ((char *) (b) + (sizeof (mchunkptr) << 1)))
```

###### in_smallbin_range

```c
#define in_smallbin_range(sz)  \
  ((unsigned long) (sz) < (unsigned long) MIN_LARGE_SIZE)
```

###### smallbin_index

```c
#define smallbin_index(sz) \
  ((SMALLBIN_WIDTH == 16 ? (((unsigned) (sz)) >> 4) : (((unsigned) (sz)) >> 3))\
   + SMALLBIN_CORRECTION)
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

###### largebin_index

```c
#define largebin_index(sz) \
  (SIZE_SZ == 8 ? largebin_index_64 (sz)                                     \
   : MALLOC_ALIGNMENT == 16 ? largebin_index_32_big (sz)                     \
   : largebin_index_32 (sz))
```

###### bin_index

```c
#define bin_index(sz) \
  ((in_smallbin_range (sz)) ? smallbin_index (sz) : largebin_index (sz))
```



#### 主要函数

##### unlink

unlink是一个宏，在从双链表中取出一个chunk外还执行了很多额外的检查

设当前chunk指针为P

* 检查后一个chunk的prev_size是当前chunksize
* 检查前一个chunk（fd）的后一个chunk（bk）是P
* 检查后一个chunk（bk）的前一个chunk（fd）是P
* 若当前chunk不为smallbin且存在上一块（fd_nextsize）
  * 检查前一个chunk（fd_nextsize）的后一个chunk（bk_nextsize）是P
  * 检查后一个chunk（bk_nextsize）的前一个chunk（fd_nextsize）是P
  * 若前一个chunk不存在前置chunk（fd_nextsize==NULL）
    * 若当前chunk的下一个chunk（fd_nextsize）是自身
      * 

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