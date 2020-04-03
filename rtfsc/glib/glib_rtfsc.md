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

### gstring

#### 数据结构

```c
typedef struct _GString GString;
struct _GString
{
  gchar  *str;
  gsize len;
  gsize allocated_len;
};
```

#### 重要函数

注意这里g_string_maybe_expand调用nearest_power时用string->len+len+1调用，即至少为1。而g_string_sized_new调用g_string_maybe_expand的最小参数是2，且调用g_string_new时若init不为空，则将以strlen(init)+2为参数调用g_string_sized_new。但若是用g_string_new_len则g_string_sized_new的参数是len

```c
static inline gsize nearest_power (gsize base, gsize num);
//以base为初始值，每次左移2位查找大于num的最小值

static void g_string_maybe_expand (GString *string, gsize len);
//增加GString的空间。测试string->len + len是否大于string->allocated_len，若大于则realloc nearest_power(1, string->len + len +1)的空间

GString *g_string_sized_new (gsize dfl_size);
//初始化GString，并分配MAX(dfl_size,2)空间给字符串。使用g_slice_new GString，并用g_string_maybe_expand分配dfl_size字节给GString结构的str成员

GString *g_string_new (const gchar *init);
//用init建立并初始化一个GString类型。若init为NULL或\0则g_string_sized_new(2)，否则g_string_sized_new(strlen(init) + 2)并将init复制过去

GString *g_string_new_len (const gchar *init, gssize len);
//分配len大小的GString并以init初始化

gchar *g_string_free (GString  *string, gboolean free_segment);
//释放GString空间。若free_segment为True则同时释放Gstring->str，否则将字符串返回

GBytes *g_string_free_to_bytes (GString *string);
//将GString转换为GBytes类型返回，并释放GString

gboolean g_string_equal (const GString *v, const GString *v2);
//比较字符串（无优化

guint g_string_hash (const GString *str);
//字符串哈希，计算公式 h = (h << 5) - h + *p h初始化为0，p每次迭代都为下一个字符

GString *g_string_assign (GString *string, const gchar *rval);
//将string改写为rval

GString *g_string_truncate (GString *string, gsize len);
//若string->len大于len，则在len处截断字符串（没有进行realloc）

GString *g_string_set_size (GString *string, gsize len);
//设置长度，若len大于string->allocated_len则重新以len分配空间

GString *g_string_insert_len (GString *string, gssize pos, const gchar *val, gssize len);
//将长度为len的val插入string的pos处。这里处理了val为string子串的情况。具体做法为，先计算并分配空间，然后将(string->str+pos)处的(string->len-pos)个字节移到(string->str+pos+len)处，再将val memcpy到(string->str+pos)处。对于为子串的情况看代码吧

GString *g_string_insert_c (GString *string, gssize pos, gchar c);
//在string的pos处插入一个字符c

GString *g_string_insert_unichar (GString *string, gssize pos, gunichar wc);
//将unicode编码为utf8并存到pos处

static gboolean is_valid (char c, const char *reserved_chars_allowed);
//判断字符c是否在可用字符列表中，可用字符包含字母数字 - ~ . _ 和reserved_chars_allowed中的字符

GString *g_string_append_uri_escaped (GString *string, const gchar *unescaped, const gchar *reserved_chars_allowed, gboolean allow_utf8);
//URI转义，若allow_utf8为true则支持utf8

static inline GString *g_string_append_c_inline (GString *gstring, gchar c);
//若string->len+1 < string->allocated_len，则直接在末尾添加c，否则调用g_string_insert_c

GString *g_string_append (GString *string, const gchar *val);
//以pos=-1和len=-1调用g_string_insert_len，直接在string尾部插入strlen(val)

GString *g_string_append_len (GString *string, const gchar *val, gssize len);
//同上，但len参数不为-1

GString *g_string_append_c (GString *string, gchar c);
//append一个字符，即以pos=-1调用g_string_insert_c

GString *g_string_append_unichar (GString *string, gunichar wc);
//以pos=-1调用g_string_insert_unichar

GString *g_string_prepend (GString *string, const gchar *val);
//以pos=0调用g_string_insert_len

GString *g_string_prepend_len (GString *string, const gchar *val, gssize len);
//以pos=0和len调用g_string_insert_len

GString *g_string_prepend_c (GString *string, gchar c);
//以pos=0调用g_string_insert_c

GString *g_string_prepend_unichar (GString *string, gunichar wc);
//以pos=0调用g_string_insert_unichar

GString *g_string_overwrite_len (GString *string, gsize pos, const gchar *val, gssize len);
//用val覆盖pos处的len个字符

GString *g_string_erase (GString *string, gssize pos, gssize len);
//擦除pos处len个字节，若pos+len<string->len则将后面字节移动到pos

GString *g_string_ascii_down (GString *string);
//全部转小写
GString *g_string_ascii_up (GString *string);
//全部转大写

void g_string_append_vprintf (GString *string, const gchar *format, va_list args);
//在string后append一个由格式化字符串控制的字符串

void g_string_vprintf (GString *string, const gchar *format, va_list args);
//替换string为一个格式化字符串控制的字符串
```

#### 潜在bug与没弄懂的操作

##### nearest_power

注意这里重新实现了一个nearest_power，但只要base不为0应该不会出错

```c
static inline gsize
nearest_power (gsize base, gsize num)
{
  if (num > MY_MAXSIZE / 2)
    {
      return MY_MAXSIZE;
    }
  else
    {
      gsize n = base;
      while (n < num)
        n <<= 1;
      return n;
    }
}
```

##### g_string_insert_len

这里判断val是否为string->str子串时使用了一个C标准内未定义的方法，但至少在目前所有架构中都适用

即通过`val >= string->str && val <= string->str+string->len`判断，相当于直接用地址判断

##### g_string_append_unichar

为什么要往一个gchar数组插utf8字符啊。。。

### gbytes

#### 数据结构

```c
struct _GBytes
{
  gconstpointer data;  /* may be NULL iff (size == 0) */
  gsize size;  /* may be 0 */
  gatomicrefcount ref_count;
  GDestroyNotify free_func;
  gpointer user_data;
};
```





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

#### 数据结构

```c
struct _GMemVTable {
  gpointer (*malloc)      (gsize    n_bytes);
  gpointer (*realloc)     (gpointer mem,
			   gsize    n_bytes);
  void     (*free)        (gpointer mem);
  /* optional; set to NULL if not used ! */
  gpointer (*calloc)      (gsize    n_blocks,
			   gsize    n_block_bytes);
  gpointer (*try_malloc)  (gsize    n_bytes);
  gpointer (*try_realloc) (gpointer mem,
			   gsize    n_bytes);
};
```

标准内存分配函数的vtable，在gmem.c被定义如下

```c
static GMemVTable glib_mem_vtable = {
  malloc,
  realloc,
  free,
  calloc,
  malloc,
  realloc,
};
```

#### 宏

```c
#define g_clear_pointer(pp, destroy)  
//用于将pp赋值为NULL，调用destroy函数析构，并返回pp之前指向的内容

#define g_new(struct_type, n_structs)			_G_NEW (struct_type, n_structs, malloc)
#define g_new0(struct_type, n_structs)			_G_NEW (struct_type, n_structs, malloc0)
#define g_try_new(struct_type, n_structs)		_G_NEW (struct_type, n_structs, try_malloc)
#define g_try_new0(struct_type, n_structs)		_G_NEW (struct_type, n_structs, try_malloc0)
//上面的四个宏依赖于下面的_G_NEW，分别是使用malloc malloc0 try_malloc try_malloc0进行内存分配和初始化
#define _G_NEW(struct_type, n_structs, func)
//调用func分配n_struct个struct_type

#define g_renew(struct_type, mem, n_structs)		_G_RENEW (struct_type, mem, n_structs, realloc)
#define g_try_renew(struct_type, mem, n_structs)	_G_RENEW (struct_type, mem, n_structs, try_realloc)
#define _G_RENEW(struct_type, mem, n_structs, func)
//调用func重新分配mem处的n_struct个struct_type元素（即realloc）
```

#### 重要函数

```c
static inline gpointer g_steal_pointer (gpointer pp);
//指针移动，将pp置为NULL，返回之前pp指向的内容

void g_clear_pointer (gpointer *pp, GDestroyNotify destroy);
//将pp置零并调用destroy析构

gpointer g_malloc (gsize n_bytes);
gpointer g_malloc0 (gsize n_bytes);
gpointer g_realloc (gpointer mem, gsize n_bytes);
void g_free (gpointer mem);
gpointer g_try_malloc (gsize n_bytes);
gpointer g_try_malloc0 (gsize n_bytes);
gpointer g_try_realloc (gpointer mem, gsize n_bytes);
gpointer g_malloc_n (gsize n_blocks, gsize n_block_bytes);
gpointer g_malloc0_n (gsize n_blocks, gsize n_block_bytes);
gpointer g_realloc_n (gpointer mem, gsize n_blocks, gsize n_block_bytes);
gpointer g_try_malloc_n (gsize n_blocks, gsize n_block_bytes);
gpointer g_try_malloc0_n (gsize n_blocks, gsize n_block_bytes);
gpointer g_try_realloc_n (gpointer mem, gsize n_blocks, gsize n_block_bytes);
```

带0的使用calloc分配，带try的加了对NULL指针的判断

#### 有趣的片段

##### g_clear_pointer

```c
#if defined(g_has_typeof) && GLIB_VERSION_MAX_ALLOWED >= GLIB_VERSION_2_58
#define g_clear_pointer(pp, destroy)                                           \
  G_STMT_START {                                                               \
    G_STATIC_ASSERT (sizeof *(pp) == sizeof (gpointer));                       \
    __typeof__((pp)) _pp = (pp);                                               \
    __typeof__(*(pp)) _ptr = *_pp;                                             \
    *_pp = NULL;                                                               \
    if (_ptr)                                                                  \
      (destroy) (_ptr);                                                        \
  } G_STMT_END                                                                 \
  GLIB_AVAILABLE_MACRO_IN_2_34
#else /* __GNUC__ */
#define g_clear_pointer(pp, destroy) \
  G_STMT_START {                                                               \
    G_STATIC_ASSERT (sizeof *(pp) == sizeof (gpointer));                       \
    /* Only one access, please; work around type aliasing */                   \
    union { char *in; gpointer *out; } _pp;                                    \
    gpointer _p;                                                               \
    /* This assignment is needed to avoid a gcc warning */                     \
    GDestroyNotify _destroy = (GDestroyNotify) (destroy);                      \
                                                                               \
    _pp.in = (char *) (pp);                                                    \
    _p = *_pp.out;                                                             \
    if (_p) 								       \
      { 								       \
        *_pp.out = NULL;                                                       \
        _destroy (_p);                                                         \
      }                                                                        \
  } G_STMT_END                                                                 \
  GLIB_AVAILABLE_MACRO_IN_2_34
```

上半部分比较好懂，下半部分为了兼容类型转换而使用了union进行类型转换

##### _G_NEW

```c
#  define _G_NEW(struct_type, n_structs, func) \
	(struct_type *) (G_GNUC_EXTENSION ({			\
	  gsize __n = (gsize) (n_structs);			\
	  gsize __s = sizeof (struct_type);			\
	  gpointer __p;						\
	  if (__s == 1)						\
	    __p = g_##func (__n);				\
	  else if (__builtin_constant_p (__n) &&		\
	           (__s == 0 || __n <= G_MAXSIZE / __s))	\
	    __p = g_##func (__n * __s);				\
	  else							\
	    __p = g_##func##_n (__n, __s);			\
	  __p;							\
	}))
```

在内存分配上进行了一些优化

若new的个数`n_structs`为1，则调用`g_##func`（若func为malloc则调用的是`g_malloc`）

若new的个数为常数且`n_struct * sizeof(struct_type)`小于等于`G_MAXSIZE`，则也调用`g_##func`

若new的个数不为常数，则调用`g_##func_n`

因为这里大多数case被预处理了，因此能优化

### gslice

是Bonwick's slab allocator的实现

#### 重要的参数和宏

##### 关于对齐

```c
//2*sizeof(size_t)
#define P2ALIGNMENT             (2 * sizeof (gsize))                            /* fits 2 pointers (assumed to be 2 * GLIB_SIZEOF_SIZE_T below) */

// 用于将size按照base内存对齐
#define ALIGN(size, base)       ((base) * (gsize) (((size) + (base) - 1) / (base)))

/* optimized version of ALIGN (size, P2ALIGNMENT) */
#if     GLIB_SIZEOF_SIZE_T * 2 == 8  /* P2ALIGNMENT */	//若sizeof(size_t) == 4，按8字节对齐
// size + 0x7 & 0xfffffff8
#define P2ALIGN(size)   (((size) + 0x7) & ~(gsize) 0x7)
#elif   GLIB_SIZEOF_SIZE_T * 2 == 16 /* P2ALIGNMENT */	//若sizeof(size_t) == 8，按16字节对齐
#define P2ALIGN(size)   (((size) + 0xf) & ~(gsize) 0xf)
#else
// 按 2*sizeof(gsize) 对齐 size
#define P2ALIGN(size)   ALIGN (size, P2ALIGNMENT)		//其他请况下按 2*sizeof(size_t)对齐（好像其实都是按这个对齐的）
#endif
```

##### magazine chain

```c
#define magazine_chain_prev(mc)         ((mc)->data)
#define magazine_chain_stamp(mc)        ((mc)->next->data)
#define magazine_chain_uint_stamp(mc)   GPOINTER_TO_UINT ((mc)->next->data)
#define magazine_chain_next(mc)         ((mc)->next->next->data)
#define magazine_chain_count(mc)        ((mc)->next->next->next->data)
```



##### 其他

```c
#define LARGEALIGNMENT          (256)
#define NATIVE_MALLOC_PADDING   P2ALIGNMENT

//sizeof(SlabInfo+2*sizeof(size_t))
#define SLAB_INFO_SIZE          P2ALIGN (sizeof (SlabInfo) + NATIVE_MALLOC_PADDING)
#define MAX_MAGAZINE_SIZE       (256)
#define MIN_MAGAZINE_SIZE       (4)
#define MAX_STAMP_COUNTER       (7)                                             /* distributes the load of gettimeofday() */

//最大slab chunk大小，一个页面至少需要有8个chunk
#define MAX_SLAB_CHUNK_SIZE(al) (((al)->max_page_size - SLAB_INFO_SIZE) / 8)    /* we want at last 8 chunks per page, see [4] */
#define SLAB_INDEX(al, asize)   ((asize) / P2ALIGNMENT - 1)                     /* asize must be P2ALIGNMENT aligned */

//最大slab的下标
#define MAX_SLAB_INDEX(al)      (SLAB_INDEX (al, MAX_SLAB_CHUNK_SIZE (al)) + 1)

#define SLAB_CHUNK_SIZE(al, ix) (((ix) + 1) * P2ALIGNMENT)
#define SLAB_BPAGE_SIZE(al,csz) (8 * (csz) + SLAB_INFO_SIZE)

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
//用于存储当前线程上的内存信息

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

##### 初始化

```c
static void slice_config_init (SliceConfig *config);
//定义分配器的属性，默认属性见上面SliceConfig的解释。若环境变量包含G_SLICE则可以改变属性，否则忽略（除非运行了valgrind）

static void g_slice_init_nomessage (void);
//初始化allocator，主要有以下几点
//若HAVE_POSIX_MEMALIGN或HAVE_MEMALIGN定义
// max_page_size = min_page_size = sysconf(_SC_PAGESIZE)
//否则
// max_page_size = 8192  min_page_size = 128
//若always_malloc为真，则不分配contention_counters magazines slab_stack，否则分配
//初始化时间戳和用于优化的max_slab_chunk_size_for_magazine_cache


static inline guint allocator_categorize (gsize aligned_chunk_size);
//根据aligned_chunk_size大小确定分配的类型
// 0. malloc
// 1. 使用slab cache
//    当aligned_chunk_size <= allocator->max_slab_chunk_size_for_magazine_cache
//    或aligned_chunk_size <= MAX_SLAB_CHUNK_SIZE (allocator)
// 2. 使用magazine cache
//    只要定义了allocator->config.bypass_magazines

static inline void g_mutex_lock_a (GMutex *mutex, guint *contention_counter);

static inline ThreadMemory* thread_memory_from_self (void);
//在tls获取当前线程的Allocator结构体，若能获取到则直接返回，否则检查allocator是否初始化，若无调用g_slice_init_nomessage初始化。此后分配ThreadMemory和2个Magazine的空间（sizeof(Magazine)*n_magazines），并修改指针（见内存布局


gpointer g_slice_alloc (gsize mem_size);
```

##### magazine

```c
static inline ChunkLink *magazine_chain_pop_head (ChunkLink **magazine_chunks);
//
```



#### 内存布局

##### thread_memory_from_self

```
|  ThreadMemory |  Magazine1 | Magazine2 |
    .magazine1  -----|            |
    .magazine2  ------------------|
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

### grefcount

#### 类型

```c
typedef gint grefcount;
typedef volatile gint   gatomicrefcount;
```

#### 重要函数

```c
void g_ref_count_init(grefcount *rc);
// 初始化rc为-1

void g_ref_count_inc(grefcount *rc);
// rc减一，release

void g_ref_count_dec(grefcount *rc);
// rc加一，acquire

gboolean g_ref_count_compare(grefcount *rc, gint val);
// rc与-val比较

void g_atomic_ref_count_init(gatomicrefcount *arc);
// arc赋1

void g_atomic_ref_count_inc(gatomicrefcount *arc);
// g_atomic_int_inc  acquire

gboolean g_atomic_ref_count_dec(gatomicrefcount *arc);
// g_atomic_int_dec_and_test  release 即 --(*arc) == 0

gboolean g_atomic_ref_count_compare(gatomicrefcount *arc, gint val);
// g_atomic_int_get(arc) == val
```



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

## 一些知识

### 语法相关

```c
__builtin_constant_p(x)
```

gcc宏，若x在编译时为常数则返回true，否则为false

## ref

https://www.cnblogs.com/FateTHarlaown/p/8919235.html  原子操作