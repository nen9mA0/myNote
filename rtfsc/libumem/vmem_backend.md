## 底层内存分配

### 概述

libumem提供三种内存池，一种是使用sbrk分配的，一种使用mmap，还有一种stand模式

vmem_sbrk vmem_mmap和vmem_stand是vmem的后端



### vmem_mmap

#### 概述

使用mmap来分配新内存

注意，在非WIN32中mmap时，只有**vmem_mmap_alloc**调用的mmap使用`MAP_PRIVATE | MAP_ANON`和`PROT_READ | PROT_WRITE | PROT_EXEC`作为flag分配实际内存

在**vmem_mmap_top_alloc**中使用的flag是`MAP_PRIVATE | MAP_ANON | MAP_NORESERVE`和`PROT_NONE`，只占位不映射物理内存

而在**vmem_mmap_free**中，回收的方式不是munmap而是把mmap的属性再设置成`MAP_PRIVATE | MAP_ANON | MAP_NORESERVE | MAP_FIXED`（多了MAP_FIXED）和`PROT_NONE`



在WIN32中，**vmem_mmap_alloc**不做mmap操作，若vmem_alloc无法分配直接返回NULL

**vmem_mmap_top_alloc**中调用`VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)`分配

**vmem_mmap_free**中调用`VirtualFree(addr, size, MEM_RELEASE)`直接释放

#### 全局变量

##### 上下文 mmap_heap

```c
static vmem_t *mmap_heap;
```

#### 主要函数

##### vmem_mmap_arena

创建mmap arena

```c
vmem_t *vmem_mmap_arena(vmem_alloc_t **a_out, vmem_free_t **f_out);
```

###### 描述

* 调用vmem_init创建下列几个arena
  * mmap_top
    * 分配函数 NULL
    * 释放函数 NULL
  * mmap_heap
    * 分配函数 vmem_mmap_top_alloc
    * 释放函数 vmem_free
  * vmem_internal
    * 分配函数 vmem_mmap_alloc
    * 释放函数 vmem_mmap_free
  * vmem_seg
    * 分配函数 vmem_alloc
    * 释放函数 vmem_free
  * vmem_hash
    * 分配函数 vmem_alloc
    * 释放函数 vmem_free
  * vmem_vmem
    * 分配函数 vmem_alloc
    * 释放函数 vmem_free

##### vmem_mmap_top_alloc

使用mmap为mmap_heap分配内存，注意这里的mmap使用`MAP_PRIVATE | MAP_ANON | MAP_NORESERVE`和`PROT_NONE`分配，即实际上不映射物理内存

```c
static void *vmem_mmap_top_alloc(vmem_t *src, size_t size, int vmflags);
```

###### 描述

* 先试图使用vmem_alloc分配内存
* 若为NULL说明heap空间需要增加
* 使用mmap分配size大小的内存
  * 若mmap成功调用_vmem_extend_alloc分配
    * 分配成功则**返回新空间**
    * 若分配失败则调用munmap，**返回NULL**
  * mmap失败则**返回NULL**

##### vmem_mmap_alloc

使用mmap为vmem_internal分配内存，注意这里的mmap使用`MAP_PRIVATE | MAP_ANON`

```c
static void *vmem_mmap_alloc(vmem_t *src, size_t size, int vmflags);
```

###### 描述

* 试图用vmem_alloc分配内存
  * 若为NULL直接**返回NULL**
  * 若不为NULL，则尝试直接在vmem_alloc返回的地址上使用mmap分配
    * 若成功**返回新空间**
    * 若失败则vmem_free该块内存，并调用vmem_reap

##### vmem_mmap_free

使用mmap为回收内存，注意这里的mmap使用`MAP_PRIVATE | MAP_ANON | MAP_NORESERVE | MAP_FIXED`和`PROT_NONE`分配

```c
static void vmem_mmap_free(vmem_t *src, void *addr, size_t size);
```

###### 描述

* 使用mmap修改addr的内存页属性为PROT_NONE
* 调用vmem_free

### vmem_sbrk

#### 概述

使用brk来分配新内存

这里有个有趣的sbrk_fail机制，当使用brk分配出新空间后，调用_vmem_extend_alloc初始化新空间若失败，则会把这块空间的头部作为一个sbrk_fail_t结构体，该结构体记录了空间的地址和大小，尔后放进sbrk_fails链表中

接下来若又有对vmem_sbrk_alloc的调用，会先尝试从sbrk_fails链表中分配空间，只要链表中有一块满足新分配空间大小需求的就会被直接返回

**注意**，这里只要大小需求就能返回，而sbrk_fails链表不按照大小只按照入表先后排序

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

##### sbrk_fails

```c
static sbrk_fail_t sbrk_fails = {
	&sbrk_fails,
	&sbrk_fails,
	NULL,
	0
};
```

sbrk_fail 链表

#### 全局锁

##### sbrk_faillock

```c
static mutex_t sbrk_faillock = DEFAULTMUTEX;
```

用于锁sbrk_fails链表

##### sbrk_lock

```c
static mutex_t sbrk_lock = DEFAULTMUTEX;
```

只有在获取sbrk锁时才可以调用brk增加内存

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

调用vmem_init分配下面几个arena

* sbrk_top
  * 分配函数 NULL
  * 释放函数 NULL
* sbrk_heap
  * 分配函数 vmem_sbrk_alloc
  * 释放函数 vmem_free
* vmem_internal
  * 分配函数 vmem_alloc
  * 释放函数 vmem_free
* vmem_seg
  * 分配函数 vmem_alloc
  * 释放函数 vmem_free
* vmem_hash
  * 分配函数 vmem_alloc
  * 释放函数 vmem_free
* vmem_vmem
  * 分配函数 vmem_alloc
  * 释放函数 vmem_free



**具体描述**

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

用于给sbrk_top分配内存

```c
static void *vmem_sbrk_alloc(vmem_t *src, size_t size, int vmflags);
```

###### 描述

* 试图使用vmem_alloc分配，若有则返回
* 试图用vmem_sbrk_tryfail从sbrk_tryfail链表分配，若有则返回
* 使用_sbrk_grow_aligned增加brk
  * 若失败返回NULL
  * 若成功调用vmem_sbrk_extend_alloc，返回分配的新地址

##### vmem_sbrk_tryfail

试图从sbrk_tryfail列表分配内存

```c
static void *vmem_sbrk_tryfail(vmem_t *src, size_t size, int vmflags);
```

###### 描述

* 锁sbrk_faillock
* 遍历sbrk_fails列表，试图找到sf_size大于当前size的块
* 若找到将该块从链表中删除
* 解锁
* 若刚刚取得了符合的块，则调用vmem_sbrk_extend_alloc初始化，返回新空间
* 否则返回NULL

##### _sbrk_grow_aligned

使用brk分配新的空间

```c
static void *_sbrk_grow_aligned(size_t min_size, size_t low_align, size_t high_align,
    size_t *actual_size);
```

* low_align  低地址对齐
* high_align  高地址对齐
* actual_size  返回实际分配的大小

###### 描述

* 加锁sbrk_lock
* 计算分配前和分配后地址（根据min_size和两个align参数），因为这里需要调用sbrk所以在临界区
* 判断地址是否溢出
  * 溢出则解锁，返回-1
* 使用brk增加空间
* 解锁sbrk_lock
* 若brk失败返回-1，成功则返回分配前的sbrk（对齐的地址）

##### vmem_sbrk_extend_alloc

```c
static void *vmem_sbrk_extend_alloc(vmem_t *src, void *pos, size_t size, size_t alloc,
    int vmflags);
```

* pos为新分配的内存地址

###### 描述

* 试图调用_vmem_extend_alloc初始化，成功则返回新空间
* 若失败
  * 加锁sbrk_faillock
  * 将pos对应的空间加入sbrk_fails链表
  * 解锁sbrk_faillock