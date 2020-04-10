## 一些机制和原理

### 自适应magazine大小

在umem_depot_alloc函数中需要先获取depot锁，使用mutex_trylock尝试获取，若获取不到再使用mutex_lock获取锁，并且增加umem_cache_t结构的cache_depot_contention

这样cache_depot_contention这个字段反映了depot锁的获取难度，若其大于一个阈值说明需要增加magazine大小

### 架构



### 内存结构

#### magazine

##### depot

```
MUTEX:
	umem_cache_t.cache_depot_lock

full magazine
        struct umem_maglist
	| umem_cache_t.cache_full |
	___________________________
	|           ...           |
	|          ml_list        | ---> | umem_magazine_t | -> | umem_magazine_t | -> ...
	---------------------------
	
empty magazine
        struct umem_maglist
	| umem_cache_t.cache_empty|
	___________________________
	|           ...           |
	|          ml_list        | ---> | umem_magazine_t | -> | umem_magazine_t | -> ...
	---------------------------
```

### 通用数据结构

#### vmem上下文

##### vmem_t

```c
struct vmem {
	char		vm_name[VMEM_NAMELEN];	/* arena name */
    				//30
	cond_t		vm_cv;		/* cv for blocking allocations */
	mutex_t		vm_lock;	/* arena lock */
	uint32_t	vm_id;		/* vmem id */
	uint32_t	vm_mtbf;	/* induced alloc failure rate */
	int		vm_cflags;	/* arena creation flags */
	int		vm_qshift;	/* log2(vm_quantum) */
	size_t		vm_quantum;	/* vmem quantum */
	size_t		vm_qcache_max;	/* maximum size to front by umem */
	vmem_alloc_t	*vm_source_alloc;
	vmem_free_t	*vm_source_free;
	vmem_t		*vm_source;	/* vmem source for imported memory */
	vmem_t		*vm_next;	/* next in vmem_list */
	ssize_t		vm_nsegfree;	/* number of free vmem_seg_t's */
	vmem_seg_t	*vm_segfree;	/* free vmem_seg_t list */
	vmem_seg_t	**vm_hash_table; /* allocated-segment hash table */
	size_t		vm_hash_mask;	/* hash_size - 1 */
	size_t		vm_hash_shift;	/* log2(vm_hash_mask + 1) */
	ulong_t		vm_freemap;	/* bitmap of non-empty freelists */
	vmem_seg_t	vm_seg0;	/* anchor segment */
	vmem_seg_t	vm_rotor;	/* rotor for VM_NEXTFIT allocations */
    
	vmem_seg_t	*vm_hash0[VMEM_HASH_INITIAL];	/* initial hash table */
    				//16
	void		*vm_qcache[VMEM_NQCACHE_MAX];	/* quantum caches */
    				//16
	vmem_freelist_t	vm_freelist[VMEM_FREELISTS + 1]; /* power-of-2 flists */
    				//sizeof(void*)*8+1
	vmem_kstat_t	vm_kstat;	/* kstat data */
};
```

##### vmem_seg_t

```c
struct vmem_seg {
	/*
	 * The first four fields must match vmem_freelist_t exactly.
	 */
	uintptr_t	vs_start;	/* start of segment (inclusive) */
	uintptr_t	vs_end;		/* end of segment (exclusive) */
	vmem_seg_t	*vs_knext;	/* next of kin (alloc, free, span) */
	vmem_seg_t	*vs_kprev;	/* prev of kin */

	vmem_seg_t	*vs_anext;	/* next in arena */
	vmem_seg_t	*vs_aprev;	/* prev in arena */
	uint8_t		vs_type;	/* alloc, free, span */
	uint8_t		vs_import;	/* non-zero if segment was imported */
	uint8_t		vs_depth;	/* stack depth if UMF_AUDIT active */
	/*
	 * The following fields are present only when UMF_AUDIT is set.
	 */
	thread_t	vs_thread;
	hrtime_t	vs_timestamp;
	uintptr_t	vs_stack[VMEM_STACK_DEPTH];
};
```





## 平台适配与杂项

### sol_compat.h

主要用于做一些平台适配和一些对齐相关的操作

#### 有趣的片段

https://blogs.oracle.com/jwadams/macros-and-powers-of-two

```c
//举例，假设	x1=0x7fff_ffea  align=16
//		    x2=0x7fff_ffe0

#define P2PHASE(x, align)    ((x) & ((align) - 1))
	//向下获取x与最近的align对齐地址之差，与模效果相同
	//P2PHASE(x1, align) = 0x7fff_ffea & 0x0000_000f = a
	//P2PHASE(x2, align) = 0x7fff_ffe0 & 0x0000_000f = 0

#define P2ALIGN(x, align)    ((x) & -(align))
	//x向下对齐的地址，这里-align其实可以视为 (~align)+1，如align=8 -(align)=0xfffffff8，低3位为0
	//P2ALIGN(x1, align) = 0x7fff_ffea & 0xffff_fff0 = 0x7fff_ffe0
	//P2ALIGN(x2, align) = 0x7fff_ffe0 & 0xffff_fff0 = 0x7fff_ffe0

#define P2NPHASE(x, align)    (-(x) & ((align) - 1))
	//向上获取x与最近的align对齐地址之差，值等于 align-P2PHASE(x,align)
	//P2NPHASE(x1, align) = 0x8000_0016 & 0x0000_000f = 6
	//P2NPHASE(x2, align) = 0x8000_0020 & 0x0000_000f = 0

#define P2ROUNDUP(x, align)   (-(-(x) & -(align)))
	//向上对齐
	//P2ROUNDUP(x1, align) = -(0x8000_0016 & 0xffff_fff0) = -0x8000_0010 = 0x7fff_fff0
	//P2ROUNDUP(x2, align) = -(0x8000_0020 & 0xffff_fff0) = -0x8000_0020 = 0x7fff_ffe0

#define P2END(x, align)     (-(~(x) & -(align)))
	//等价于P2ALIGN(x, align) + align
	//向上一个大于当前地址的对齐地址对齐，即使当前地址是align对齐的也会向上对齐一个align
	//P2END(x1, align) = -(0x8000_0015 & 0xffff_fff0) = -0x8000_0010 = 0x7fff_fff0
	//P2END(x2, align) = -(0x8000_001f & 0xffff_fff0) = -0x8000_0010 = 0x7fff_fff0

#define P2PHASEUP(x, align, phase)  ((phase) - (((phase) - (x)) & -(align)))
	//等价于phase+P2ROUNDUP(x-phase, align)
	//相当于返回一个以align对齐的地址为base，phase为相位的地址


#define P2CROSS(x, y, align)    (((x) ^ (y)) > (align) - 1)
	//判断x和y高位是否相同（即除了小于对齐位数的部分，其他部分地址相同）
	//P2CROSS(x1, x2, align) = 0xa > 0xf = false
	//x3 = 0x7fff_fffa
	//P2CROSS(x1, x3) = 0x10 > 0xf = true

#define P2SAMEHIGHBIT(x, y)    (((x) ^ (y)) < ((x) & (y)))
	//判断x和y最高位是否相同，因为若最高位相同，假设为第n位，则x^y的结果第n位为0，x&y第n位为1

#define IS_P2ALIGNED(v, a) ((((uintptr_t)(v)) & ((uintptr_t)(a) - 1)) == 0)
	//等价于P2PHASE(v, a) == 0
	//判断当前地址是否是对齐的

#define ISP2(x)    (((x) & ((x) - 1)) == 0)
	//等价于P2PHASE(x, x) == 0
	//判断一个数是否为2^n

#define P2BOUNDARY(off, len, align) \
    (((off) ^ ((off) + (len) - 1)) > (align) - 1)
	//等价于P2CROSS(off, off+len-1)
	//判断off+len-1与off是否在一个对齐地址范围内
```

*注：以P2开头的原因是这些运算仅适用于power 2的情况，即align为2^n*

### misc

#### 主要函数

```c
static void umem_log_enter(const char *error_str);
	//以循环队列的形式写log

int highbit(ulong_t i);
int lowbit(ulong_t i);
	//获取最高/最低位在第几位
```

#### 有趣的片段

```c
int highbit(ulong_t i)
{
	register int h = 1;

	if (i == 0)
		return (0);
#ifdef _LP64
	if (i & 0xffffffff00000000ul) {
		h += 32; i >>= 32;
	}
#endif
	if (i & 0xffff0000) {
		h += 16; i >>= 16;
	}
	if (i & 0xff00) {
		h += 8; i >>= 8;
	}
	if (i & 0xf0) {
		h += 4; i >>= 4;
	}
	if (i & 0xc) {
		h += 2; i >>= 2;
	}
	if (i & 0x2) {
		h += 1;
	}
	return (h);
}

int lowbit(ulong_t i)
{
	register int h = 1;

	if (i == 0)
		return (0);
#ifdef _LP64
	if (!(i & 0xffffffff)) {
		h += 32; i >>= 32;
	}
#endif
	if (!(i & 0xffff)) {
		h += 16; i >>= 16;
	}
	if (!(i & 0xff)) {
		h += 8; i >>= 8;
	}
	if (!(i & 0xf)) {
		h += 4; i >>= 4;
	}
	if (!(i & 0x3)) {
		h += 2; i >>= 2;
	}
	if (!(i & 0x1)) {
		h += 1;
	}
	return (h);
}
```

假设 i = 0x07f0 ef10，两个函数如下运行

```
highbit(i)
	0x07f0 ef10 -- [i&0xffff0000=true] --> 0x0000 07f0  h=17
	0x0000 07f0 -- [i&0x0000ff00=true] --> 0x0000 0007  h=25
	0x0000 0007 -- [i&0x000000f0=false]
	0x0000 0007 -- [i&0x0000000c=true] --> 0x0000 0001  h=27
	0x0000 0001 -- [i&0x00000002=false]
	return 27

lowbit(i)
	0x07f0 ef10 -- [!i&0x0000ffff=false]
	0x07f0 ef10 -- [!i&0x000000ff=false]
	0x07f0 ef10 -- [!i&0x0000000f=true] --> 0x007f 0ef1  h=5
	0x007f 0ef1 -- [!i&0x00000003=false]
	0x007f 0ef1 -- [!i&0x00000001=false]
	return 5
```

#### 潜在bug

**如果没有对0输入进行判断**（注意这里有，但有可能其他代码没有）

highbit输入0和1，返回值是一样的，都为1

lowbit输入0和0x80000000，返回值都为32



## 错误检查

### getpcstack

```c
int getpcstack(uintptr_t *pcstack, int pcstack_limit, int check_signal);
	//自底向上遍历获取调用栈中的所有pc指针，若指定checksignal则会检查是否调用了signal handler
	//注意这里还考虑到了遍历备用栈的情况
```

### umem_fail

这里需要明确一点，很多函数都是C库函数的替代，因为这些库函数往往用到了malloc/free，而libumem本身就是提供一套malloc/free的，若还使用原来的库函数则会导致错误发生递归

#### 全局变量

```c
static volatile int umem_exiting = 0;
	//设置退出时返回值
static mutex_t umem_exit_lock = DEFAULTMUTEX;
	//umem_exiting的锁
```



#### 主要函数

```c
static int firstexit(int type);
	//若umem_exiting为0，设置其值

static void __NORETURN umem_do_abort(void);
	//abort函数的替代品，采用raise来出发ABORT信号。因为直接调用abort()会调用free函数来释放内存
	//此外为了防止raise(SIGABRT)时错误处理函数调用malloc/free，因此替换为NULL

static void print_stacktrace(void);
	//使用getpcstack获取栈上指针并打印

void umem_panic(const char *format, ...);
	//按格式化字符串打印信息并打印栈回溯，最后abort

void umem_err_recoverable(const char *format, ...);
	//与上面类似，差别就是只有在umem_abort>0时才abort
```



## 底层内存分配

### vmem_mmap

#### 全局变量

##### 上下文 mmap_heap

```c
static vmem_t *mmap_heap;
```

#### 主要函数

##### vmem_mmap_arena

```c
vmem_t *vmem_mmap_arena(vmem_alloc_t **a_out, vmem_free_t **f_out);
```

###### 描述



###### 调用





### vmem_sbrk

#### 数据结构

##### sbrk_fail

```c
typedef struct sbrk_fail {
	struct sbrk_fail *sf_next;
	struct sbrk_fail *sf_prev;
	void *sf_base;			/* == the sbrk_fail's address */
	size_t sf_size;			/* the size of this buffer */
} sbrk_fail_t;
```



#### 全局变量

##### 上下文 sbrk_heap

```c
static vmem_t *sbrk_heap;
```

用于管理分配的sbrk heap

##### real_pagesize

```c
static size_t real_pagesize;
```

保存真实的pagesize

#### 用于设置的extern变量

##### vmem_sbrk_pagesize

```c
size_t vmem_sbrk_pagesize = 0;
```

用于保存希望的pagesize，init结束后保存真实的pagesize

##### vmem_sbrk_minalloc

```c
size_t vmem_sbrk_minalloc = VMEM_SBRK_MINALLOC;
```

用于保存希望的最小分配内存，init结束后保存真实的minalloc

#### 主要函数

##### vmem_sbrk_arena

用于创建sbrk_arena

```c
vmem_t *vmem_sbrk_arena(vmem_alloc_t **a_out, vmem_free_t **f_out);
```

vmem_alloc_t和vmem_free_t都是函数指针

```c
typedef void *(vmem_alloc_t)(vmem_t *, size_t, int);
typedef void (vmem_free_t)(vmem_t *, void *, size_t);
```

###### 描述

* 若sbrk_heap为NULL说明未初始化
  * 使用`sysconf(_SC_PAGESIZE)`获取real_pagesize
  * 若geteuid==0（root），则heap_size为real_pagesize
  * 若vmem_sbrk_pagesize < real_pagesize，heap_size也为real_pagesize
  * 否则，说明vmem_sbrk_pagesize > real_pagesize
    * #预编译 MHA_MAPSIZE_BSSBRK
      * 尝试用memcntl调整页面大小，若成功heap_size = vmem_sbrk_pagesize 
  * 若vmem_sbrk_minalloc大于默认，则更新
  * vmem_init
* 若a_out为NULL初始化为vmem_alloc
* 若f_out为NULL初始化为vmem_free

###### 调用

* vmem_init

##### vmem_sbrk_alloc

```c
static void *vmem_sbrk_alloc(vmem_t *src, size_t size, int vmflags);
```

###### 描述



###### 调用



### vmem

#### 数据结构

#### 全局变量

##### vmem_internal_arena

```c
static vmem_t *vmem_internal_arena;
```

##### vmem_seg0

```c
static vmem_seg_t vmem_seg0[VMEM_SEG_INITIAL];	//VMEM_SEG_INITIAL默认值为100
```

##### vmem_segfree

```c
static vmem_seg_t *vmem_segfree;
```



#### 全局锁

##### vmem_list_lock

```c
static mutex_t vmem_list_lock = DEFAULTMUTEX;
```

##### vmem_segfree_lock

```c
static mutex_t vmem_segfree_lock = DEFAULTMUTEX;
```

用于锁全局变量 vmem_segfree

##### vmem_nosleep_lock

```c
static vmem_populate_lock_t vmem_nosleep_lock = {
  DEFAULTMUTEX,
  0
};
```



#### vmem初始化

##### vmem_getseg_global

```c
static vmem_seg_t *vmem_getseg_global(void);
```

###### 描述

* 解锁vmem_segfree_lock
* 若vmem_segfree不为NULL则pop一个元素，
* 加锁，**返回元素**

##### vmem_putseg_global

```c
static void vmem_putseg_global(vmem_seg_t *vsp);
```

###### 描述

* 解锁vmem_segfree_lock
* push一个元素
* 加锁，**返回**

##### vmem_init

```c
vmem_t *vmem_init(const char *parent_name, size_t parent_quantum,
    vmem_alloc_t *parent_alloc, vmem_free_t *parent_free,
    const char *heap_name, void *heap_start, size_t heap_size,
    size_t heap_quantum, vmem_alloc_t *heap_alloc, vmem_free_t *heap_free);
```

##### 描述

* 调用vmem_putseg_global将100个vmem_seg0元素push入vmem_segfree
* 

##### 调用



#### vmem分配

##### vmem_alloc

```c
void *vmem_alloc(vmem_t *vmp, size_t size, int vmflag);
```

###### 描述



###### 调用



## umem

### 主要数据结构

###### umem_cache_t

slab主要结构

```c
typedef umem_cache umem_cache_t;
struct umem_cache {
	/*
	 * Statistics			统计信息
	 */
	uint64_t	cache_slab_create;	/* slab creates */
	uint64_t	cache_slab_destroy;	/* slab destroys */
	uint64_t	cache_slab_alloc;	/* slab layer allocations */
	uint64_t	cache_slab_free;	/* slab layer frees */
	uint64_t	cache_alloc_fail;	/* total failed allocations */
	uint64_t	cache_buftotal;		/* total buffers */
	uint64_t	cache_bufmax;		/* max buffers ever */
	uint64_t	cache_rescale;		/* # of hash table rescales */
	uint64_t	cache_lookup_depth;	/* hash lookup depth */
	uint64_t	cache_depot_contention;	/* mutex contention count */
	uint64_t	cache_depot_contention_prev; /* previous snapshot */

	/*
	 * Cache properties		cache属性
	 */
	char		cache_name[UMEM_CACHE_NAMELEN + 1];
	size_t		cache_bufsize;		/* object size */
	size_t		cache_align;		/* object alignment */
	umem_constructor_t *cache_constructor;
	umem_destructor_t *cache_destructor;
	umem_reclaim_t	*cache_reclaim;
	void		*cache_private;		/* opaque arg to callbacks */
	vmem_t		*cache_arena;		/* vmem source for slabs */
	int		cache_cflags;		/* cache creation flags */
	int		cache_flags;		/* various cache state info */
	int		cache_uflags;		/* UMU_* flags */
	uint32_t	cache_mtbf;		/* induced alloc failure rate */
	umem_cache_t	*cache_next;		/* forward cache linkage */
	umem_cache_t	*cache_prev;		/* backward cache linkage */
	umem_cache_t	*cache_unext;		/* next in update list */
	umem_cache_t	*cache_uprev;		/* prev in update list */
	uint32_t	cache_cpu_mask;		/* mask for cpu offset */

	/*
	 * Slab layer			slab层
	 */
	mutex_t		cache_lock;		/* protects slab layer */
	size_t		cache_chunksize;	/* buf + alignment [+ debug] */
	size_t		cache_slabsize;		/* size of a slab */
	size_t		cache_bufctl;		/* buf-to-bufctl distance */
	size_t		cache_buftag;		/* buf-to-buftag distance */
	size_t		cache_verify;		/* bytes to verify */
	size_t		cache_contents;		/* bytes of saved content */
	size_t		cache_color;		/* next slab color */
	size_t		cache_mincolor;		/* maximum slab color */
	size_t		cache_maxcolor;		/* maximum slab color */
	size_t		cache_hash_shift;	/* get to interesting bits */
	size_t		cache_hash_mask;	/* hash table mask */
	umem_slab_t	*cache_freelist;	/* slab free list */
	umem_slab_t	cache_nullslab;		/* end of freelist marker */
	umem_cache_t	*cache_bufctl_cache;	/* source of bufctls */
	umem_bufctl_t	**cache_hash_table;	/* hash table base */
	/*
	 * Depot layer			depot层
	 */
	mutex_t		cache_depot_lock;	/* protects depot */		//depot锁
	umem_magtype_t	*cache_magtype;		/* magazine type */		//magazine的cache
    													//注意magazine层有自己的umem_cache_t
	umem_maglist_t	cache_full;		/* full magazines */		//用于记录depot中的full magazine
	umem_maglist_t	cache_empty;		/* empty magazines */	//用于记录depot中的empty magazine

	/*
	 * Per-CPU layer		cpu层
	 */
	umem_cpu_cache_t cache_cpu[1];		/* cache_cpu_mask + 1 entries */
};
```

###### umem_cpu_cache_t

每个cpu对应cache的结构，用于描述cpu对应的magazine信息

```c
typedef struct umem_cpu_cache {
	mutex_t		cc_lock;	/* protects this cpu's local cache */
	uint_t		cc_alloc;	/* allocations from this cpu */
	uint_t		cc_free;	/* frees to this cpu */
	umem_magazine_t	*cc_loaded;	/* the currently loaded magazine */
	umem_magazine_t	*cc_ploaded;	/* the previously loaded magazine */
	int		cc_rounds;	/* number of objects in loaded mag */
	int		cc_prounds;	/* number of objects in previous mag */
	int		cc_magsize;	/* number of rounds in a full mag */
	int		cc_flags;	/* CPU-local copy of cache_flags */
#if (!defined(_LP64) || defined(UMEM_PTHREAD_MUTEX_TOO_BIG)) && !defined(_WIN32)
	/* on win32, UMEM_CPU_PAD evaluates to zero, and the MS compiler
	 * won't allow static initialization of arrays containing structures
	 * that contain zero size arrays */
	char		cc_pad[UMEM_CPU_PAD]; /* for nice alignment (32-bit) */
#endif
} umem_cpu_cache_t;
```

###### umem_slab_t

```c
typedef struct umem_slab {
	struct umem_cache	*slab_cache;	/* controlling cache */
	void			*slab_base;	/* base of allocated memory */
	struct umem_slab	*slab_next;	/* next slab on freelist */
	struct umem_slab	*slab_prev;	/* prev slab on freelist */
	struct umem_bufctl	*slab_head;	/* first free buffer */
	long			slab_refcnt;	/* outstanding allocations */
	long			slab_chunks;	/* chunks (bufs) in this slab */
} umem_slab_t;
```

###### umem_magazine_t

magazine结构

```c
typedef struct umem_magazine {
	void	*mag_next;
	void	*mag_round[1];		/* one or more rounds */
} umem_magazine_t;
```

###### umem_maglist_t

magazine链表

```c
typedef struct umem_maglist {
	umem_magazine_t	*ml_list;	/* magazine list */
	long		ml_total;	/* number of magazines */
	long		ml_min;		/* min since last update */
	long		ml_reaplimit;	/* max reapable magazines */
	uint64_t	ml_alloc;	/* allocations from this list */
} umem_maglist_t;

```

###### umem_magtype_t

```c
typedef struct umem_magtype {
	int		mt_magsize;	/* magazine size (number of rounds) */
	int		mt_align;	/* magazine alignment */
	size_t		mt_minbuf;	/* all smaller buffers qualify */
	size_t		mt_maxbuf;	/* no larger buffers qualify */
	umem_cache_t	*mt_cache;	/* magazine cache */
} umem_magtype_t;
```

###### umem_cpu_t

```c
typedef struct umem_cpu {
	uint32_t cpu_cache_offset;
	uint32_t cpu_number;
} umem_cpu_t;
```

### 全局变量

###### umem_null_cache

```c
umem_cache_t umem_null_cache;
	//默认的umem_cache_t结构
```

###### umem_alloc_table

```c
static umem_cache_t *umem_alloc_table[UMEM_MAXBUF >> UMEM_ALIGN_SHIFT];
	//默认有16384个元素，初始化时全部填充为umem_null_cache的指针
```

###### umem_cpus

umem_startup_cpu定义了当前cpu在umem_cpu_cache_t结构体中的偏移量和cpu编号

```c
static umem_cpu_t umem_startup_cpu = {  /* initial, single, cpu */
        UMEM_CACHE_SIZE(0),
        0 };
static umem_cpu_t *umem_cpus = &umem_startup_cpu;
```

### 主要函数与关键逻辑

#### 顶层接口

##### _umem_alloc

```c
void *_umem_alloc(size_t size, int umflag);
```

###### 描述

根据size选择分配方式

* 若size小于等于UMEM_MAXBUF（131072） 则
  * 获取umem_alloc_table[size>>3]作为对应自己块的链表头
  * 调用_umem_cache_alloc创建内存
    * 若成功获取则处理cache_flag
    * 若失败则调用umem_alloc_retry
* 若size为0，**返回NULL**
* 大于的情况
  * 若umem_oversize_arena为NULL，调用umem_init初始化新块
  * 用vmem_alloc分配
    * 若分配失败，调用umem_alloc_retry
* **返回buf**

###### 调用

* _umem_cache_alloc
* umem_alloc_retry
* umem_init
* vmem_alloc
* umem_log_event

##### _umem_zalloc

```c
_umem_zalloc(size_t size, int umflag)
```

###### 描述

* 若size小于等于UMEM_MAXBUF（131072） 则
  * 获取umem_alloc_table[size>>3]作为对应自己块的链表头
  * 调用_umem_cache_alloc创建内存
    * 若成功
      * 若定义了REDZONE，则对内存块buftag编码
      * 调用bzero清零，**返回buf**
    * 若失败则调用umem_alloc_retry，跳回开头重试
* 若size大于UMEM_MAXBUF
  * 调用_umem_alloc分配buf
    * 若不为NULL调用bzero
* **返回buf**

###### 调用

* _umem_cache_alloc
* umem_alloc_retry
* _umem_alloc

##### _umem_free

```c
void _umem_free(void *buf, size_t size);
```

###### 描述

* 若size小于等于UMEM_MAXBUF（131072） 则
  * 获取umem_alloc_table[size>>3]作为对应自己块的链表头
  * 若定义了REDZONE，进行内存检查（REDZONE检查原理下面单独写）
  * 调用_umem_cache_free
* 若size大于UMEM_MAXBUF且不为0，buf不为NULL
  * 调用vmem_free释放

###### 调用

* _umem_cache_free
* vmem_free

#### magazine操作

##### umem_cpu_reload

```c
static void umem_cpu_reload(umem_cpu_cache_t *ccp, umem_magazine_t *mp, int rounds);
```

###### 描述

* 将previous magazine和round替换为loaded
* 用新的magazine mp和rounds赋值loaded

##### umem_depot_alloc

```c
static umem_magazine_t *umem_depot_alloc(umem_cache_t *cp, umem_maglist_t *mlp);
```

从depot获取的magazine

###### 描述

* 根据是否能马上申请到depot锁处理cache_depot_contention（见自适应magazine大小）
* 如果mlp的ml_list链表非NULL
  * 取出最上面的元素
  * 修改ml_total（当前链表上元素个数）ml_min（链表上最少一次有几个magazine）和ml_alloc（链表分配了几个magezine）

#### magazine分配释放

##### _umem_cache_alloc

```c
void *_umem_cache_alloc(umem_cache_t *cp, int umflag);
```

###### 描述

这个函数即论文里从cpu magazine分配内存的函数实现

* 获取当前cpu（通过umem_startup_cpu），上锁
* 若loaded_rounds>0
  * buf = loaded_magazine[--loaded_round]指向的buf
  * alloc字段+1
  * 解锁
  * 若定义了RED_ZONE检查（UMF_BUFTAG），调用umem_cache_alloc_debug
    * 若返回-1，调用umem_alloc_retry并跳回开头重试
    * 若retry失败**返回NULL**
  * **返回buf**
* 若loaded_rounds==0，则检查previous magazine，若大于0
  * umem_cpu_reload交换loaded magazine和previous magazine
  * 跳回开头重试
* 若上述的分支都没进入，说明两个magazine都没有空间了
  * 检查magsize是否为0，若为0说明depot层被disable，直接跳出，使用slab分配
  * 若存在magazine layer，调用umem_depot_alloc从depot分配一个full magazine
  * 将previous magazine还回depot，将当前loaded magazine换成新分配的magazine，旧loaded magazine给previous magazine
  * 跳回开头重试
* 上述操作结束后，如还未返回说明是magazine满且没有depot层的情况
* 调用umem_slab_alloc从slab层分配内存
  * 若分配未成功
    * 若返回umem_null_cache，则说明无法分配slab，**返回NULL**
    * 否则尝试umem_alloc_retry，跳回开头重试
  * 若定义了RED_ZONE检查，调用umem_cache_alloc_debug
    * 若返回-1，调用umem_alloc_retry并跳回开头重试
    * 若retry失败**返回NULL**
* 分配slab后使用umem_cache_t的cache_constructor字段构造
  * 若构造失败调用umem_slab_free，并umem_alloc_retry重试
  * 若retry失败**返回NULL**

###### 调用

* umem_cache_alloc_debug
* umem_alloc_retry
* umem_cpu_reload
* umem_depot_alloc
* umem_depot_free
* umem_slab_alloc
* umem_slab_free

##### _umem_cache_free

```c
void _umem_cache_free(umem_cache_t *cp, void *buf);
```

###### 描述

论文中free内存到cpu magazine的实现

* 若定义了REDZONE，则检查内存标志是否正确，**不正确直接返回**
* 获取当前CPU，上锁
* 若loaded_round<magsize，说明loaded magazine未满
  * 直接将buf放回magazine，loaded_round++，free字段++
  * 解锁，**返回**
* 若previous_round==0，说明loaded满previous未满
  * 调用umem_cpu_reload交换，跳到开头重试
* 若magsize==0，说明depot层未启用
  * 直接跳到下面slab层分配
* 运行至此说明previous和loaded均满，需交换一个empty页面上来
  * 调用umem_depot_alloc从empty magazine分配一个空页面
  * 若分配成功
    * umem_depot_free将previous页面换到depot中
    * umem_cpu_reload将loaded页面交换到previous并将loaded改为新分配的页面
    * 跳到开头重试
  * 若分配失败，说明depot中也没有空magazine，需要分配（**注意这里的操作比较有趣**）
    * 从umem_cache_t结构的cache_magtype字段取得指向umem_magtype_t结构的指针，解锁，调用_umem_cache_alloc为magazine分配内存，加锁。
    * 若分配成功，判断当前umem_cache_t结构的magsize是否等于新分配的magazine的magsize
      * 若不等于说明在解锁期间umem_magtype_t被改过
        * 解锁，调用_umem_cache_free释放刚刚分配的magazine，加锁
        * 跳到开头重试
      * magazine分配成功，调用umem_depot_free为cpu换上新的magazine
* 运行到这说明magazine层无法分配出空的magazine来接收free的内存，于是直接还给slab层
  * 调用cache_destructor析构
  * 调用umem_slab_free归还内存

###### 调用

* umem_cache_free_debug
* umem_cpu_reload
* umem_depot_alloc
* umem_depot_free
* _umem_cache_alloc
* _umem_cache_free  **注意这里有个递归调用**
* umem_slab_free

#### umem初始化

##### umem_init

```c
int umem_init(void);
```

###### 描述



###### 调用





#### 潜在bug

_umem_free(void *buf, size_t size) 的buf参数似乎可以传入NULL