## 概述

vmem属于后端

#### 全局变量

##### 上下文 vmem0

```c
static vmem_t vmem0[VMEM_INITIAL];
```

初始只有6个vmem_t，对应6个上下文，在vmem_init中被初始化

##### vmem_segfree

##### vmem_seg0



#### 重要函数

##### vmem_init

```c
vmem_t *vmem_init(const char *parent_name, size_t parent_quantum,
    vmem_alloc_t *parent_alloc, vmem_free_t *parent_free,
    const char *heap_name, void *heap_start, size_t heap_size,
    size_t heap_quantum, vmem_alloc_t *heap_alloc, vmem_free_t *heap_free);
```

###### 描述

* 将100个（VMEM_SEG_INITIAL）vmem_seg0数组元素加入vmem_segfree列表
* 若parent_name不为NULL，创建parent_name为名的vmem_t
* 创建heap_name为名的vmem_t，赋给vmem_heap
* 创建"vmem_internal"为名的vmem_t，赋给vmem_internal_arena
* 创建"vmem_seg"为名的vmem_t，赋给vmem_seg_arena
* 创建"vmem_hash"为名的vmem_t，赋给vmem_hash_arena
* 创建"vmem_vmem"为名的vmem_t，赋给vmem_vmem_arena
* 

##### vmem_create

```c
vmem_t *vmem_create(const char *name, void *base, size_t size, size_t quantum,
	vmem_alloc_t *afunc, vmem_free_t *ffunc, vmem_t *source,
	size_t qcache_max, int vmflag)
```

用于初始化一个vmem结构体

###### 描述

* 若vmem_vmem_arena被分配，则分配一个vmem_t

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
	size_t		vm_qcache_max;	/* maximum size to front by umem */
    
    //当vmem分配内存时调用这两个接口，从vm_source的内存池分配
	vmem_alloc_t	*vm_source_alloc;	//参数afunc
	vmem_free_t	*vm_source_free;		//参数ffunc
	vmem_t		*vm_source;				//参数source，用于定义从当前vmem分配的内存的来源
    							//所有vmem的最终来源为sbrk_heap或mmap_heap
    
	vmem_t		*vm_next;	/* next in vmem_list */
	ssize_t		vm_nsegfree;	/* number of free vmem_seg_t's */
	vmem_seg_t	*vm_segfree;	/* free vmem_seg_t list */
    
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
    
    
	void		*vm_qcache[VMEM_NQCACHE_MAX];	/* quantum caches */

	vmem_freelist_t	vm_freelist[VMEM_FREELISTS + 1]; //空闲块列表
    				//数组的每个元素与前后的元素串接成一个双向链表，头尾为NULL
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

