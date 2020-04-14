### 概述

vmem属于后端

### 数据结构

##### vmem_t

用于保存和记录一个arena的状态

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

##### vmem_kstat_t

```c
typedef struct vmem_kstat {
	uint64_t	vk_mem_inuse;	/* memory in use */
	uint64_t	vk_mem_import;	/* memory imported */
	uint64_t	vk_mem_total;	/* total memory in arena */
	uint32_t	vk_source_id;	/* vmem id of vmem source */
	uint64_t	vk_alloc;	/* number of allocations */
	uint64_t	vk_free;	/* number of frees */
	uint64_t	vk_wait;	/* number of allocations that waited */
	uint64_t	vk_fail;	/* number of allocations that failed */
	uint64_t	vk_lookup;	/* hash lookup count */
	uint64_t	vk_search;	/* freelist search count */
	uint64_t	vk_populate_wait;	/* populates that waited */
	uint64_t	vk_populate_fail;	/* populates that failed */
	uint64_t	vk_contains;		/* vmem_contains() calls */
	uint64_t	vk_contains_search;	/* vmem_contains() search cnt */
} vmem_kstat_t;
```

### 标志

#### arena创建标志

##### VMC_POPULATOR

表示该arena在创建时作为populator创建（）

### 全局变量

##### 上下文 vmem0

```c
static vmem_t vmem0[VMEM_INITIAL];
```

初始只有6个vmem_t，对应6个上下文，在vmem_init中被初始化

* VMEM_INITIAL = 6

##### vmem_list

```c
static vmem_t *vmem_list;
```

全局的vmem链表

##### vmem_segfree

```c
static vmem_seg_t *vmem_segfree;
```

* 保存释放后的vmem_seg_t
* 初始化时，vmem_seg0的100个元素被加入vmem_segfree（见vmem_init函数）

##### vmem_seg0

```c
static vmem_seg_t vmem_seg0[VMEM_SEG_INITIAL];
```

* VMEM_SEG_INITIAL= 100
* 初始化时被加入vmem_segfree

##### vmem_populator

```c
static vmem_t *vmem_populator[VMEM_INITIAL];
```

存放创建标志为VMC_POPULATOR的段

* VMEM_INITIAL = 6

##### vmem_populators

populator arena的数量

##### arena全局变量

在vmem_init中被初始化

###### vmem_heap

存放heap段，参数`VM_SLEEP | VMC_POPULATOR`

###### vmem_internal_arena

存放"vmem_internal"段，参数`VM_SLEEP | VMC_POPULATOR`

###### vmem_seg_arena

存放"vmem_seg"段，参数`VM_SLEEP | VMC_POPULATOR`

###### vmem_hash_arena

存放"vmem_hash"段，参数`VM_SLEEP | VMC_POPULATOR`

###### vmem_vmem_arena

存放"vmem_vmem"段，参数`VM_SLEEP | VMC_POPULATOR`

### 全局锁

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

### 重要函数

#### vmem_t操作

##### vmem_init

初始化一个新的heap，其中包括五个或六个arena

```c
vmem_t *vmem_init(const char *parent_name, size_t parent_quantum,
    vmem_alloc_t *parent_alloc, vmem_free_t *parent_free,
    const char *heap_name, void *heap_start, size_t heap_size,
    size_t heap_quantum, vmem_alloc_t *heap_alloc, vmem_free_t *heap_free);
```

###### 描述

* 将100个（VMEM_SEG_INITIAL）vmem_seg0数组元素加入vmem_segfree列表
* 若parent_name不为NULL，创建以参数parent_name为名的vmem_t，参数`VM_SLEEP | VMC_POPULATOR`
* 创建以参数heap_name为名的vmem_t，赋给vmem_heap，参数`VM_SLEEP | VMC_POPULATOR`
* 创建"vmem_internal"为名的vmem_t，赋给vmem_internal_arena，参数`VM_SLEEP | VMC_POPULATOR`
* 创建"vmem_seg"为名的vmem_t，赋给vmem_seg_arena，参数`VM_SLEEP | VMC_POPULATOR`
* 创建"vmem_hash"为名的vmem_t，赋给vmem_hash_arena，参数`VM_SLEEP`
* 创建"vmem_vmem"为名的vmem_t，赋给vmem_vmem_arena，参数`VMSLEEP`
* 

##### vmem_create

用于初始化一个vmem结构体

```c
vmem_t *vmem_create(const char *name, void *base, size_t size, size_t quantum,
	vmem_alloc_t *afunc, vmem_free_t *ffunc, vmem_t *source,
	size_t qcache_max, int vmflag)
```

###### 描述

* 若vmem_vmem_arena已分配，则调用vmem_alloc分配一个vmem_t

* 否则直接从vmem0结构体取

* 接下来执行一系列操作初始化vmem_t，初始化后的vmem_t如下

```c
struct vmem {
	char		vm_name[VMEM_NAMELEN];		//name参数
    
    //锁
	cond_t		vm_cv;		//cond_init
	mutex_t		vm_lock;	//mutex_init
    
	uint32_t	vm_id;		// vmem 结构体id
	uint32_t	vm_mtbf;	/* induced alloc failure rate */
	int		vm_cflags;		//vmflag参数，用于指定Per-allocation flags和Arena creation flags
    
    //pagesize相关
	int		vm_qshift;		//log2(vm_quantum)
	size_t		vm_quantum;	//quantum参数，指定pagesize
	size_t		vm_qcache_max;	//最大的qcache大小
    void		*vm_qcache[VMEM_NQCACHE_MAX];	//指向使用umem_cache_create创建的n个cache
    					//大小为 vm_quantum 2*vm_quantum ... n*vm_quantum
    
    //当vmem分配内存时调用这两个接口，从vm_source的内存池分配
	vmem_alloc_t	*vm_source_alloc;	//参数afunc
	vmem_free_t	*vm_source_free;		//参数ffunc
	vmem_t		*vm_source;				//参数source，用于定义从当前vmem分配的内存的来源
    							//所有vmem的最终来源为sbrk_heap或mmap_heap
    
	vmem_t		*vm_next;		//vmem_list的下一个vmem_t
	ssize_t		vm_nsegfree;	//vm_segfree链表上的元素个数
	vmem_seg_t	*vm_segfree;	//vmem_t自己保留的vmem_segfree链表
    
    //哈希表相关
	vmem_seg_t	**vm_hash_table; 	//初始指向vm_hash0
	size_t		vm_hash_mask;	// hash_size - 1，用于表示hash表模数
	size_t		vm_hash_shift;	// log2(vm_hash_mask + 1)
    vmem_seg_t	*vm_hash0[VMEM_HASH_INITIAL];	//初始哈希表，被vm_hash0字段索引
    /*
    	vm_hash_table ---\
    	                 |
    	vm_hash0  <------/
    	|_______|
    	|_______|
    	|_______|
    	   ...
    */
    
    
	ulong_t		vm_freemap;	// bitmap of non-empty freelists
	vmem_seg_t	vm_seg0;	//VMEM_SPAN类型的segment，初始化双向链表都指向自己
	vmem_seg_t	vm_rotor;	//VMEM_ROTOR类型的segment，将上面的vm_seg0插入链表
  	/*
  		vm_rotor
  		|___vmem_seg___|
  		  |_vs_aprev_|
  		  |_vs_anext_| ---\
  	      ________________/
  	     /
  		vm_seg0
  		|___vmem_seg___| <-\-\-\-\
  		  |_vs_aprev_| ----/ | | |
  		  |_vs_anext_| ------/ | |
  		  |_vs_kprev_| --------/ |
  		  |_vs_knext_| ----------/
  	
  	*/
    

	vmem_freelist_t	vm_freelist[VMEM_FREELISTS + 1]; //空闲块列表
    				//数组的每个元素与前后的元素串接成一个双向链表，头尾为NULL
    				//vm_freelist按照vm_seg_t定义的地址范围大小来存放vm_seg_t链表，即
    				//vm_freelist存放元素字节数大小分别对应
    				// 0~1 2~3 4~7 8~15 16~31 32~63 64~127 128~255 ...
    /*
    	vm_freelist
    	|___vmem_freelist_t___|  <------\
    	   |____vs_kprev___|     NULL   |
    	   |____vs_knext___|     ----\  |
    	|___vmem_freelist_t___|  <---/--+--\
    	   |____vs_kprev___|     -------/  |
    	   |____vs_knext___|     ----\     |
    	|___vmem_freelist_t___|  <---/-----+-\
    	   |____vs_kprev___|     ----------/ |
    	   |____vs_knext___|     ----\       |
    	|___vmem_freelist_t___|  <---/       |
    	   |____vs_kprev___|     ------------/
    	   |____vs_knext___|     ...
    
    */
    
    
	vmem_kstat_t	vm_kstat;	/* kstat data */
};
```

* 将当前初始化的vmem_t加入vmem_list
* 若参数vmflag定义了VMC_POPULATOR，则调用vmem_populate填充该vmem_t的segfree字段，并将其加入全局数组vmem_populator中



##### vmem_destroy

析构一个vmem_t结构体

```c
void vmem_destroy(vmem_t *vmp);
```

###### 描述

* 将vmem_t结构从vmem_list链表中删除
* 回收qcache
* 判断vmem是否还有未释放的内存，若有打印信息
* vm_hash_table是否指向vm_hash0（默认哈希表），若不是则释放新分配的哈希表
* 将vmp->rotor从当前rotor链表中删除
* 将vmem_t的seg0链表的所有arena记录的kin返还给vmem_segfree
* 将vmem_t的segfree链表的所有kin返还给vmem_segfree
* 析构mutex和cond
* 回收vmem_t结构体内存



#### getseg/putseg

##### vmem_getseg_global

从vmem_segfree取一个vmem_seg_t

```c
static vmem_seg_t *vmem_getseg_global(void);
```

###### 描述

* 解锁vmem_segfree_lock
* 若vmem_segfree不为NULL则pop一个元素，
* 加锁，**返回元素**

##### vmem_putseg_global

push一个vmem_seg_t到vmem_segfree

```c
static void vmem_putseg_global(vmem_seg_t *vsp);
```

###### 描述

* 解锁vmem_segfree_lock
* push一个元素
* 加锁，**返回**

##### vmem_getseg

从vmem_t的vm_segfree取一个vmem_seg_t

```c
static vmem_seg_t *vmem_getseg(vmem_t *vmp);
```

##### vmem_putseg

push一个vmem_seg_t到vmem_t的vm_segfree

```c
static void vmem_putseg(vmem_t *vmp, vmem_seg_t *vsp);
```

#### hash表

##### VMEM_HASH_INDEX

哈希函数

```c
#define	VMEM_HASH_INDEX(a, s, q, m)					\
	((((a) + ((a) >> (s)) + ((a) >> ((s) << 1))) >> (q)) & (m))
```

##### VMEM_HASH

计算哈希并且返回对应hash table的下标

```c
#define	VMEM_HASH(vmp, addr)						\
	(&(vmp)->vm_hash_table[VMEM_HASH_INDEX(addr,			\
	(vmp)->vm_hash_shift, (vmp)->vm_qshift, (vmp)->vm_hash_mask)])
```

##### vmem_hash_insert

```c
static void vmem_hash_insert(vmem_t *vmp, vmem_seg_t *vsp);
```

###### 描述

* 根据`vsp->vs_start`计算哈希，获取对应`vmem_t->vm_hash_table`地址
* 将vsp加入链表中
* 更新`vmem_t->vm_kstat.vk_alloc`和`vmem_t->vm_kstat.vk_mem_inuse`

##### vmem_hash_delete

```c
static vmem_seg_t *vmem_hash_delete(vmem_t *vmp, uintptr_t addr, size_t size);
```

###### 描述

* 根据addr计算哈希表地址
* 若addr与哈希表中记录的`vmem_seg_t->vs_start`不同
  * 则遍历链表直到找到
  * 且每次查找都递增`vmem_t->vm_kstat.vk_lookup`
* 检查有没有找到对应链表项和对应size是否相等
* 更新`vmem_t->vm_kstat.vk_free`和`vmem_t->vm_kstat.vk_mem_inuse`

#### freelist

##### vmem_freelist_insert

```c
static void vmem_freelist_insert(vmem_t *vmp, vmem_seg_t *vsp);
```

往vmem_t的vm_freelist元素插入一个vmem_seg_t

###### 描述

* 检查vsp的哈希，确定其没有被存放过

* 将其存入对应的vmem_t的vm_freelist链表，具体来说，存放到vm_freelist下标为

  `highbit(vsp->end - vsp->start)-1`的元素中

* 更新vm_freemap参数

##### vmem_freelist_delete

```c
static void vmem_freelist_delete(vmem_t *vmp, vmem_seg_t *vsp);
```

从vmem_t的vm_freelist元素删除一个vmem_seg_t

###### 描述

* 若vsp的前面和后面都是freelist头（判据start=0），则将对应freemap置0
* 将其从链表中删除

#### span

##### vmem_span_create

```c
static vmem_seg_t *vmem_span_create(vmem_t *vmp, void *vaddr, size_t size, uint8_t import);
```

###### 描述



#### seg初始化

##### vmem_populate

若vmem_t的vm_segfree内元素个数小于VMEM_MINFREE，则填充新的空vmem_seg_t元素

```c
static int vmem_populate(vmem_t *vmp, int vmflag);
```

**注意**：vmem_populate进入前后`vmp->vm_lock`都是锁着的



##### vmem_seg_create

```c
static vmem_seg_t *vmem_seg_create(vmem_t *vmp, vmem_seg_t *vprev, uintptr_t start, uintptr_t end);
```

在vmem_t的vm_segfree取一个元素（vmem_getseg）并进行初始化

###### 描述

* vs_start = start
* vs_end = end
* vs_type = 0
* vs_import = 0
* 链表项插入vprev之后

##### vmem_seg_destroy

```c
static void vmem_seg_destroy(vmem_t *vmp, vmem_seg_t *vsp);
```

从vsp所在链表中删除该项，并放回vm_segfree（vmem_putseg）



