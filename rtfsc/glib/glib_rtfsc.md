## 初始化

### glib_init

初始化debug env  g_debug_init  g_quark_init

若为win32则DllMain也在这个文件中（具体见gwin32）

* PROCESS_ATTACH
  * g_crash_handler_win32_init
  * g_clock_win32_init
  * g_thread_win32_init
  * glib_init
  * g_console_win32_init
* DLL_THREAD_DETACH
  * g_thread_win32_thread_detach
* DLL_PROCESS_DETACH
  * g_thread_win32_process_detach
  * g_crash_handler_win32_deinit

若定义了CONSTRUCTOR，则**将glib_init加入.ctor段**

#### 重要函数

```c
// 比较key与token是否一致，length为token长度，且将所有大写转为小写，将 _ 转为 -
static gboolean debug_key_matches (const gchar *key, const gchar *token, guint length);

// string为选项，keys为所有选项的键值对，nkeys为选项个数。若string为help打印所有选项，为all则选择所有选项，否则以 : ; 为分隔检测string中是否含有keys中的键
guint g_parse_debug_string (const gchar *string, const GDebugKey *keys, guint nkeys);

// 获取debug env，g_debug_init（设置一些调试参数），g_quark_init
void glib_init (void);
```

### gwin32

```c
gint g_win32_ftruncate (gint  fd, guint size);	//_chsize 为文件fd预分配空间为size
gchar *g_win32_getlocale (void);			//获取字符集代码页
gchar *g_win32_error_message (gint error);	//返回win32错误码对应的报错信息
gchar *g_win32_get_package_installation_directory_of_module (gpointer hmodule);
							//通过hmodule获取文件目录，若路径含有lib或bin则返回包含这两个文件夹的目录

static gchar *get_package_directory_from_module (const gchar *module_name);
	//查找module_dir哈希表（带mutex），若没创建则创建，若找到键则返回，否则获取module_name对应的hmodule，并调用g_win32_get_package_installation_directory_of_module，将返回的路径加入hashtable

gchar *g_win32_get_package_installation_directory (const gchar *package, const gchar *dll_name);
	//若dll_name不为NULL，则使用dll_name调用get_package_directory_from_module，若找不到或dll_name为NULL，则返回GetModuleFileNameW(NULL)的目录，并加入到哈希表，键为 ""

gchar *g_win32_get_package_installation_subdirectory (const gchar *package, const gchar *dll_name, const gchar *subdir);
	//

gboolean g_win32_check_windows_version (const gint major, const gint minor, const gint spver, const GWin32OSType os_type);
	//使用RtlGetVersion检查版本号(major, minor, spver)是否大于等于参数提供的版本号，以及os_type是否相同

guint g_win32_get_windows_version (void);		//获取windows版本，这里使用g_once_init_enter保证只调用一次
static gchar *special_wchar_to_locale_enoding (wchar_t *wstring);
	//使用WideCharToMultiByte转换字符串，返回含字符串的g_malloc0分配的空间

gchar *g_win32_locale_filename_from_utf8 (const gchar *utf8filename);
	//将文件名从utf8转为multibyte字符串

gchar **g_win32_get_command_line (void);
	//使用 CommandLineToArgvW(GetCommandLineW(), &n) 获取命令行参数，并分配n+1个元素的gchar*指针数组，放入命令行参数

```



## 类型定义

### gtypes.h

#### bswap骚操作

##### c

```c
#define GUINT16_SWAP_LE_BE_CONSTANT(val)	((guint16) ( \
    (guint16) ((guint16) (val) >> 8) |	\
    (guint16) ((guint16) (val) << 8)))

#define GUINT32_SWAP_LE_BE_CONSTANT(val)	((guint32) ( \
    (((guint32) (val) & (guint32) 0x000000ffU) << 24) | \
    (((guint32) (val) & (guint32) 0x0000ff00U) <<  8) | \
    (((guint32) (val) & (guint32) 0x00ff0000U) >>  8) | \
    (((guint32) (val) & (guint32) 0xff000000U) >> 24)))

#define GUINT64_SWAP_LE_BE_CONSTANT(val)	((guint64) ( \
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT (0x00000000000000ffU)) << 56) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT (0x000000000000ff00U)) << 40) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT (0x0000000000ff0000U)) << 24) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT (0x00000000ff000000U)) <<  8) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT (0x000000ff00000000U)) >>  8) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT (0x0000ff0000000000U)) >> 24) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT (0x00ff000000000000U)) >> 40) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT (0xff00000000000000U)) >> 56)))
```

##### asm

```c
//16bit
__asm__ ("rorw $8, %w0"
         : "=r" (__v)
         : "0" (__x)
         : "cc");

//32bit
__asm__ ("rorw $8, %w0\n\t"		// | 4 | 3 | 2 | 1 | -> | 4 | 3 | 1 | 2 |
         "rorl $16, %0\n\t"		// | 4 | 3 | 1 | 2 | -> | 1 | 2 | 4 | 3 |
         "rorw $8, %w0"			// | 1 | 2 | 4 | 3 | -> | 1 | 2 | 3 | 4 |
         : "=r" (__v)
         : "0" (__x)
         : "cc");

//486以上有bswap
__asm__ ("bswap %0"
		: "=r" (__v)
		: "0" (__x));
//ia64   16bit
__asm__ ("shl %0 = %1, 48 ;;"	// xxxx xxxx xxxx 0102 -> 0102 0000 0000 0000
         "mux1 %0 = %0, @rev ;;"// 0102 0000 0000 0000 -> 0000 0000 0000 0201
         : "=r" (__v)
         : "r" (__x));
//ia64   32bit
__asm__ ("shl %0 = %1, 32 ;;"
         "mux1 %0 = %0, @rev ;;"
         : "=r" (__v)
         : "r" (__x));
```

## 文件

### gfileutils

```c
static gchar *g_build_path_va (const gchar  *separator, const gchar  *first_element, va_list      *args, gchar **str_array);
```



## 字符串

### gstrfuncs

#### 潜在bug

```c
gint
g_ascii_strcasecmp (const gchar *s1,
                    const gchar *s2)
{
  gint c1, c2;

  g_return_val_if_fail (s1 != NULL, 0);
  g_return_val_if_fail (s2 != NULL, 0);

  while (*s1 && *s2)
    {
      c1 = (gint)(guchar) TOLOWER (*s1);
      c2 = (gint)(guchar) TOLOWER (*s2);
      if (c1 != c2)
        return (c1 - c2);
      s1++; s2++;
    }

  return (((gint)(guchar) *s1) - ((gint)(guchar) *s2));
}
```

函数用于忽略大小写的字符串比较，并且若c1!=c2会返回两个字母的小写值相减

但若`strlen(s1)!=strlen(s2)`，最后返回值为`(((gint)(guchar) *s1) - ((gint)(guchar) *s2))`，这里的s1和s2并不是小写值相减，可能导致一些依赖于返回值判断s1和s2内容的函数错误





## 内存分配

### gmem



### gslice

是Bonwick's slab allocator的实现

#### 重要的参数和宏

```c
#define LARGEALIGNMENT          (256)
#define P2ALIGNMENT             (2 * sizeof (gsize))                            /* fits 2 pointers (assumed to be 2 * GLIB_SIZEOF_SIZE_T below) */
// 用于将size按照base内存对齐
#define ALIGN(size, base)       ((base) * (gsize) (((size) + (base) - 1) / (base)))
#define NATIVE_MALLOC_PADDING   P2ALIGNMENT                                     /* per-page padding left for native malloc(3) see [1] */
#define SLAB_INFO_SIZE          P2ALIGN (sizeof (SlabInfo) + NATIVE_MALLOC_PADDING)
#define MAX_MAGAZINE_SIZE       (256)                                           /* see [3] and allocator_get_magazine_threshold() for this */
#define MIN_MAGAZINE_SIZE       (4)
#define MAX_STAMP_COUNTER       (7)                                             /* distributes the load of gettimeofday() */
#define MAX_SLAB_CHUNK_SIZE(al) (((al)->max_page_size - SLAB_INFO_SIZE) / 8)    /* we want at last 8 chunks per page, see [4] */
#define MAX_SLAB_INDEX(al)      (SLAB_INDEX (al, MAX_SLAB_CHUNK_SIZE (al)) + 1)
#define SLAB_INDEX(al, asize)   ((asize) / P2ALIGNMENT - 1)                     /* asize must be P2ALIGNMENT aligned */
#define SLAB_CHUNK_SIZE(al, ix) (((ix) + 1) * P2ALIGNMENT)
#define SLAB_BPAGE_SIZE(al,csz) (8 * (csz) + SLAB_INFO_SIZE)

/* optimized version of ALIGN (size, P2ALIGNMENT) */
#if     GLIB_SIZEOF_SIZE_T * 2 == 8  /* P2ALIGNMENT */
// size + 0x7 & 0xfffffff8
#define P2ALIGN(size)   (((size) + 0x7) & ~(gsize) 0x7)
#elif   GLIB_SIZEOF_SIZE_T * 2 == 16 /* P2ALIGNMENT */
#define P2ALIGN(size)   (((size) + 0xf) & ~(gsize) 0xf)
#else
// 按 2*sizeof(gsize) 对齐 size
#define P2ALIGN(size)   ALIGN (size, P2ALIGNMENT)
#endif
```

#### 数据结构

```c
// slice分配器配置，默认为下面注释
typedef struct {
  gboolean always_malloc;		//FALSE
  gboolean bypass_magazines;	//FALSE
  gboolean debug_blocks;		//FALSE
  gsize    working_set_msecs;	//15*1000
  guint    color_increment;		//1
} SliceConfig;

struct _ChunkLink {
  ChunkLink *next;
  ChunkLink *data;
};

struct _SlabInfo {
  ChunkLink *chunks;
  guint n_allocated;
  SlabInfo *next, *prev;
};

typedef struct {
  ChunkLink *chunks;
  gsize      count;                     /* approximative chunks list length */
} Magazine;

typedef struct {
  Magazine   *magazine1;                /* array of MAX_SLAB_INDEX (allocator) */
  Magazine   *magazine2;                /* array of MAX_SLAB_INDEX (allocator) */
} ThreadMemory;

typedef struct {
  /* const after initialization */
  gsize         min_page_size, max_page_size;
    			/* #if HAVE_POSIX_MEMALIGN || HAVE_MEMALIGN
    				min_page_size=128
    				max_page_size=8192
    			   #else
    			   	min_page_size = max_page_size = sys_page_size
    			*/
  SliceConfig   config;
  gsize         max_slab_chunk_size_for_magazine_cache;
  /* magazine cache */
  GMutex        magazine_mutex;
  ChunkLink   **magazines;                /* array of MAX_SLAB_INDEX (allocator) */
  guint        *contention_counters;      /* array of MAX_SLAB_INDEX (allocator) */
  gint          mutex_counter;
  guint         stamp_counter;			// 时间戳计数器，直到MAX_STAMP_COUNTER(7 by default)时更新时间戳
  guint         last_stamp;				// 上个时间戳，单位毫秒
  /* slab allocator */
  GMutex        slab_mutex;
  SlabInfo    **slab_stack;                /* array of MAX_SLAB_INDEX (allocator) */
  guint        color_accu;
} Allocator;

```

#### 重要函数

```c
static inline ThreadMemory* thread_memory_from_self (void);
```



#### 有趣的片段

##### 内存对齐

通用方法（去掉强制类型转换部分）

```c
#define ALIGN(size, base)      (size + base - 1) / base
```

位运算

```c
#define ALIGN(size, base)      (size + base - 1) & (~(base - 1))
```



## 多线程/原子操作

### gthread-win32





### gatomic

#### 有趣的片段

##### 可以执行类型检测的宏

`G_GNUC_EXTENSION`的定义为空或`__extension__`。这里有趣的地方在于第5行的`(void) (0 ? *(atomic) ^ *(atomic) : 1); `，乍一看没有任何用，实际上这句用于编译时检测atomic类型是否正确。assert检测了`*atomic`的长度是否与gint相同，而void对`*atomic`进行了异或运算，保证atomic是个指针而不是多重指针。

最后返回值有gaig_temp返回，因为被括号包裹的多个语句返回值为最右边的一句

```c
#define g_atomic_int_get(atomic) \
  (G_GNUC_EXTENSION ({                                                       \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    gint gaig_temp;                                                          \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    __atomic_load ((gint *)(atomic), &gaig_temp, __ATOMIC_SEQ_CST);          \
    (gint) gaig_temp;                                                        \
  }))
```

##### 原子操作相关

c11提供的原子操作可以看ref

当编译器提供atomic实现时

```
__atomic_load
__atomic_store
__atomic_fetch_add
__atomic_fetch_sub
__atomic_compare_exchange_n
__atomic_fetch_and
__atomic_fetch_or
__atomic_fetch_xor
```

当不支持atomic的时候，举例

###### release操作

```c
#define g_atomic_int_get(atomic) \
  (G_GNUC_EXTENSION ({                                                       \
    gint gaig_result;                                                        \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ *(atomic) : 1);                                  \
    gaig_result = (gint) *(atomic);                                          \
    __sync_synchronize ();                                                   \
    __asm__ __volatile__ ("" : : : "memory");                                \
    gaig_result;                                                             \
  }))
```

###### acquire操作

```c
#define g_atomic_int_set(atomic, newval) \
  (G_GNUC_EXTENSION ({                                                       \
    G_STATIC_ASSERT (sizeof *(atomic) == sizeof (gint));                     \
    (void) (0 ? *(atomic) ^ (newval) : 1);                                   \
    __sync_synchronize ();                                                   \
    __asm__ __volatile__ ("" : : : "memory");                                \
    *(atomic) = (newval);                                                    \
  }))
```

这里使用

    __sync_synchronize ();
    __asm__ __volatile__ ("" : : : "memory"); 
来提供内存的同步

###### 旧版本win的实现

```c
static LONG
_gInterlockedAnd (volatile guint *atomic, guint val)
{
  LONG i, j;
  j = *atomic;
  do {
    i = j;
    j = InterlockedCompareExchange(atomic, i & val, i);
  } while (i != j);
  return j;
}
```

后来版本的实现不需要循环

###### pthread的实现

可以较清楚地在这里看到所有原子操作的实际行为

库维护一个mutex

```c
static pthread_mutex_t g_atomic_lock = PTHREAD_MUTEX_INITIALIZER;
```

后面的操作就是普通mutex用法

```c
gint
(g_atomic_int_get) (const volatile gint *atomic)
{
  gint value;

  pthread_mutex_lock (&g_atomic_lock);
  value = *atomic;
  pthread_mutex_unlock (&g_atomic_lock);

  return value;
}

void
(g_atomic_int_set) (volatile gint *atomic,
                    gint           value)
{
  pthread_mutex_lock (&g_atomic_lock);
  *atomic = value;
  pthread_mutex_unlock (&g_atomic_lock);
}
```

#### 问题

为什么g_atomic_pointer_add使用的参数和返回值是gssize(signed size_t)而and/or/xor是gsize(unsigned size_t)，get/set是gpointer

## 数据结构

### garray

#### 数据结构

##### 数组

```c
struct _GRealArray
{
  guint8 *data;					//指向数据
  guint   len;					//总长
  guint   alloc;				//
  guint   elt_size;				//单元素大小
  guint   zero_terminated : 1;	//若为true则在最后应多分配一个为0的元素
  guint   clear : 1;
  gatomicrefcount ref_count;	//引用计数
  GDestroyNotify clear_func;	//回调析构函数
};
```

#### 函数

##### 分配/初始化

```c
GArray* g_array_new (gboolean zero_terminated, gboolean clear, guint  elt_size);
GArray* g_array_sized_new (gboolean zero_terminated, gboolean clear, guint elt_size, guint reserved_size);  // g_array_new的底层调用，reserved_size指定在init时预分配给data的空间
static void g_array_maybe_expand (GRealArray *array, guint len);
  //各种内存分配函数的底层调用，分配len的g_nearest_pow大小，若小于16则分配16。若定义了g_mem_gc_friendly则清零，否则不清

```

##### 清理/设置

```c
gpointer g_array_steal (GArray *array, gsize *len); //返回值为array->data，len返回array大小，并清零array
g_array_set_clear_func (GArray *array, GDestroyNotify  clear_func); //设置回调
GArray *g_array_ref (GArray *array);  //增加引用计数
void g_array_unref (GArray *array);  //减少引用计数，若为0调用array_free
gchar* g_array_free (GArray   *farray, gboolean  free_segment);  //
```

##### 获取属性

```c
guint g_array_get_element_size (GArray *array);  //返回elt_size
```





#### 有趣的片段

返回最接近n的大于n的2的指数

原理类似快速幂，目的是把n的最高位1后的所有位数都变成1，最后+1则为最接近n的2的指数

如（这里演示为16位，程序实际是给32位用的）

```
n = 0100 0000 0000 0000
n = 0110 0000 0000 0000  (n|=n>>1)
n = 0111 1000 0000 0000  (n|=n>>2)
n = 0111 1111 1000 0000  (n|=n>>4)
n = 0111 1111 1111 1111  (n|=n>>8)
```

```c
static guint
g_nearest_pow (guint num)
{
  guint n = num - 1;

  g_assert (num > 0);

  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
#if SIZEOF_INT == 8
  n |= n >> 32;
#endif

  return n + 1;
}
```

#### 潜在bug

一样是上面的g_nearest_pow，**这段代码有个潜在风险，当num>0x1000 0000 0000 0000 0000 0000 0000 0000时，return值为0**



## 异常捕获

### gmessages





## 一些宏配置

### gmacros

检查gcc版本

检查是否支持inline关键字

宏定义下列attribute

* `__pure__`  表明函数输出只与输入或全局变量有关，可以提供优化（合并调用等）  https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-pure-function-attribute
* `__malloc__`  表明函数返回的内存块不包含任何其他指针或函数返回的指针没有被其他变量引用  https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-functions-that-behave-like-malloc
* `noinline`  表明函数不能inline
* `__sentinel__`  表明函数的参数是NULL，仅用于参数可变的函数  https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-sentinel-function-attribute
* `__alloc_size`  表明函数返回的是个指向内存的指针和内存大小  https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-alloc_005fsize-function-attribute
* `__noreturn__`  表明函数永远不返回
* `__const__`  表明函数除了参数 局部变量和返回值，不会有其他副作用
* `__unused__`  表明函数可能不会被调用
* `no_instrument_function`  表明函数不会被优化  https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#index-no_005finstrument_005ffunction-function-attribute
* `fallthrough`  https://gcc.gnu.org/onlinedocs/gcc/Statement-Attributes.html#index-fallthrough-statement-attribute
* `__deprecated__`  如果有该选项的函数被编译会报warning
* `may_alias`  https://gcc.gnu.org/onlinedocs/gcc/Common-Type-Attributes.html#index-may_005falias-type-attribute
* `warn_unused_result`  若返回值没被使用则报warning

#### 有趣的片段

```c
#define G_SIZEOF_MEMBER(struct_type, member) \
    GLIB_AVAILABLE_MACRO_IN_2_64 \
    sizeof (((struct_type *) 0)->member)
```

获取type->member的字段大小

```c
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#define G_APPROX_VALUE(a, b, epsilon) \
  (((a) > (b) ? (a) - (b) : (b) - (a)) < (epsilon))
```

CLAMP 将x限制在low~high间

G_APPROX_VALUE 判断浮点数是否相等



### ref

https://www.cnblogs.com/FateTHarlaown/p/8919235.html  原子操作