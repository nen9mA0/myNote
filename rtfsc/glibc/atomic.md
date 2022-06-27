### 原语

所有的atomic都是基于原语实现的，原语主要就是CAS操作

即对一个内存值mem进行比较，分别比较newval和oldval，并做如下操作

```c
bool CompareAndSwap(int *mem, int oldval, int newval)
{
    if(*mem == oldval)
    {
        mem = newval;
        return true;
    }
    else
        return false;
}
```

#### 构造宏

这里定义的宏是为了拼接成其他宏，因此暂时称为构造宏

#####__atomic_val_bysize

```c
#define __atomic_val_bysize(pre, post, mem, ...)
```

将传入的参数拼接为 `pre_n_post`的形式，其中n为8 16 32 64。

*举例*：如调用形式如下

```
__atomic_val_bysize (__arch_compare_and_exchange_val,acq, mem, newval, oldval)
```

则宏根据mem指针指向元素的大小，会推断出下列的宏

* `__arch_compare_and_exchange_val_8_acq(mem, newval, oldval)`
* `__arch_compare_and_exchange_val_16_acq(mem, newval, oldval)`
* `__arch_compare_and_exchange_val_32_acq(mem, newval, oldval)`
* `__arch_compare_and_exchange_val_64_acq(mem, newval, oldval)`

##### __atomic_bool_bysize

与上个宏一样，只不过这里新建了个宏以便于使用中区分类型

#### 函数宏

这里的函数宏主要就是为了定义CAS原语，基本调用格式是 `CAS(mem, newval, oldval)`。

主要有6种：

* 其中3种是catomic前缀的，另外3种是atomic前缀的（暂时不清楚区别，如果catomic没有单独的操作函数则会直接转成对atomic的调用）
* 上述两类分别都有两种返回格式，一种是bool，一种是val。其中bool的返回值就是CAS操作是否成功，val的返回值则是mem当前值。因此其实bool型CAS等同于 `CAS_val(mem, newval, oldval) != oldval`
* val型的除了acquire操作还有release，但实际操作直接转换为acquire

几乎所有的原语都是由`atomic_compare_and_exchange_val_acq`派生，而这个宏默认又是由下列四个函数定义的，因此这里应该是外部引入的原子操作定义。或由外部定义了`atomic_compare_and_exchange_val_acq`

[外部引用](#外部引用与依赖) 详细说明了头文件中对外部函数的调用情况

* `__arch_compare_and_exchange_val_8_acq`
* `__arch_compare_and_exchange_val_16_acq`
* `__arch_compare_and_exchange_val_32_acq`
* `__arch_compare_and_exchange_val_64_acq`

##### atomic_compare_and_exchange_val_acq

acquire 操作，这个定义是其他操作的底层依赖之一

```c
#if !defined atomic_compare_and_exchange_val_acq \
    && defined __arch_compare_and_exchange_val_32_acq
# define atomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __atomic_val_bysize (__arch_compare_and_exchange_val,acq,              \
               mem, newval, oldval)
#endif
```

* 若之前没有定义过`atomic_compare_and_exchange_val_acq`，且定义了`__arch_compare_and_exchange_val_32_acq`
* 则将`atomic_compare_and_exchange_val_acq`定义为`__atomic_val_bysize (__arch_compare_and_exchange_val,acq, ...)`

##### catomic_compare_and_exchange_val_acq

acquire 操作

```c
#ifndef catomic_compare_and_exchange_val_acq
# ifdef __arch_c_compare_and_exchange_val_32_acq
#  define catomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __atomic_val_bysize (__arch_c_compare_and_exchange_val,acq,              \        // 使用__atomic_val_bysize定义
               mem, newval, oldval)
# else
#  define catomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  atomic_compare_and_exchange_val_acq (mem, newval, oldval)            // 使用atomic_compare_and_exchange_val_acq
# endif
#endif
```

* 若定义了`__arch_c_compare_and_exchange_val_32_acq`，则使用`__atomic_val_bysize(__arch_c_compare_and_exchange_val, acq)`宏来定义
* 否则直接使用`atomic_compare_and_exchange_val_acq`函数作为CAS操作原语

##### catomic_compare_and_exchange_val_rel

release操作

```c
#ifndef catomic_compare_and_exchange_val_rel
# ifndef atomic_compare_and_exchange_val_rel
#  define catomic_compare_and_exchange_val_rel(mem, newval, oldval)          \
  catomic_compare_and_exchange_val_acq (mem, newval, oldval)        // 使用catomic_compare_and_exchange_val_acq定义
# else
#  define catomic_compare_and_exchange_val_rel(mem, newval, oldval)          \
  atomic_compare_and_exchange_val_rel (mem, newval, oldval)            // 使用atomic_compare_and_exchange_val_rel定义
# endif
#endif
```

* 若定义了 `atomic_compare_and_exchange_val_rel`，则使用 `atomic_compare_and_exchange_val_rel` 宏来定义
* 否则使用 `catomic_compare_and_exchange_val_acq` 来定义（也就是上一个函数的acquire操作）

##### atomic_compare_and_exchange_val_rel

若无定义过，则直接定义为`atomic_compare_and_exchange_val_acq`

##### atomic_compare_and_exchange_bool_acq

* 若定义了 `__arch_compare_and_exchange_bool_32_acq`，则使用`__atomic_bool_bysize (__arch_compare_and_exchange_bool,acq)`来定义

* 否则只使用`atomic_compare_and_exchange_val_acq`函数，实现如下
  
  ```c
  #  define atomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
    ({ /* Cannot use __oldval here, because macros later in this file might     \
      call this macro with __oldval argument.     */                  \
       __typeof (oldval) __atg3_old = (oldval);                      \
       atomic_compare_and_exchange_val_acq (mem, newval, __atg3_old)          \
         != __atg3_old;                                  \
    })
  ```

很简单，因为oldval和newval都是bool值

##### catomic_compare_and_exchange_bool_acq

跟上面的其他操作类似

* 若定义了`__arch_c_compare_and_exchange_bool_32_acq`， 则使用`__atomic_bool_bysize (__arch_c_compare_and_exchange_bool,acq)`定义

* 否则实现几乎跟`atomic_compare_and_exchange_bool_acq`一模一样
  
  ```c
  #  define catomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
    ({ /* Cannot use __oldval here, because macros later in this file might     \
      call this macro with __oldval argument.     */                  \
       __typeof (oldval) __atg4_old = (oldval);                      \
       catomic_compare_and_exchange_val_acq (mem, newval, __atg4_old)          \
         != __atg4_old;                                  \
    })
  ```

### 派生原子操作

#### 派生原子操作的代码模板

派生原子操作是从原语派生来的原子操作，原语主要定义了CAS操作

派生原子操作的大多数代码是用下面的模板构成的，这里统一说明，下面在描述各个派生操作时就不直接贴代码了

以交换加法为例，完整定义及注释如下

```c
#ifndef atomic_exchange_and_add_acq
# ifdef atomic_exchange_and_add            // 这里是针对有单独定义atomic_exchange_and_add的情况（如编译器定义了或自定义）
#  define atomic_exchange_and_add_acq(mem, value) \
  atomic_exchange_and_add (mem, value)
# else
#  define atomic_exchange_and_add_acq(mem, value) \
  ({ __typeof (*(mem)) __atg6_oldval;                          \    // 声明类型为mem指向的对象，用来暂存旧值
     __typeof (mem) __atg6_memp = (mem);                      \    // 一个中间变量，用来暂存mem
     __typeof (*(mem)) __atg6_value = (value);                      \    // 暂存新值
                                          \
     do                                          \
       __atg6_oldval = *__atg6_memp;                          \        // 循环体内将当前mem指向的值作为旧值存入
     while (__builtin_expect                              \
        (atomic_compare_and_exchange_bool_acq (__atg6_memp,    \ // CAS操作，比较mem指向的值是否为旧值，若是则将运算结果赋给它
                           __atg6_oldval          \
                           + __atg6_value,          \
                           __atg6_oldval), 0));          \
                                          \
     __atg6_oldval; })        // 返回值为旧值
# endif
#endif
```

#### 交换

原子交换，`*mem = new_val`，并返回旧值

##### atomic_exchange_acq

```c
# define atomic_exchange_acq(mem, newvalue)
```

主要操作就是下面这段代码（去掉了一些不必要的修饰）

```c
     do                                          \
       __atg5_oldval = *__atg5_memp;                          \
     while (atomic_compare_and_exchange_bool_acq (__atg5_memp, __atg5_value,__atg5_oldval));      
```

##### atomic_exchange_rel

直接调用上述的atomic_exchange_acq

#### 交换加法

原子加，`*mem = new_val + old_val`，并返回旧值

##### atomic_exchange_and_add

若有外部定义，直接引用。否则使用下面定义的`atomic_exchange_and_add_acq`

##### atomic_exchange_and_add_acq

```c
#  define atomic_exchange_and_add_acq(mem, value)
```

若定义过`atomic_exchange_and_add`则直接调用，否则为下述操作

主要操作

```c
     do                                          \
       __atg6_oldval = *__atg6_memp;                          \
     while (atomic_compare_and_exchange_bool_acq (__atg6_memp, __atg6_oldval + __atg6_value, __atg6_oldval));
```

##### atomic_exchange_and_add_rel

若没有外部定义，则直接调用`atomic_exchange_and_add_acq`

##### catomic_exchange_and_add

区别就在于这里调用的原子操作是`catomic_compare_and_exchange_bool_acq`

#### 加法

##### 类型1 无返回值

直接使用的是交换加法的操作，差别在于**没有返回值**。不赘述

函数：

* `atomic_add`
* `catomic_add`

##### 类型2 带返回值测试

###### atomic_add_negative

`*mem + value 返回结果是否为负`

```c
# define atomic_add_negative(mem, value)
```

主要操作就是这句

```c
atomic_exchange_and_add (mem, value) < -value;
```

因为交换加法返回旧值，因此若旧值小于 `-value`，则结果小于0。（对负数同样成立）

###### atomic_add_zero

`*mem + value 返回结果是否为0`

```c
# define atomic_add_zero(mem, value)
```

主要操作是这句

```c
atomic_exchange_and_add (mem, value) == -value;
```

原理类似上面，不赘述

#### 加一减一

##### 类型1 无返回值

无返回值。

函数：

* atomic_increment
* catomic_increment
* atomic_decrement
* catomic_decrement

##### 类型2 有返回值

返回值是自加后的值，操作即

```c
# define atomic_increment_val(mem) (atomic_exchange_and_add ((mem), 1) + 1)
```

函数：

* atomic_increment_val
* catomic_increment_val
* atomic_decrement_val
* catomic_decrement_val

##### 类型3 test

测试返回值是不是0

函数：

* atomic_increment_and_test
* atomic_decrement_and_test

##### 类型4 为正减一

若为正则减一，返回旧值

函数：

* atomic_decrement_if_positive

#### 原子与

##### atomic_and

`*mem & mask`，无返回值

```c
# define atomic_and(mem, mask)
```

##### atomic_and_val

`*mem & mask`，返回old_val

```c
# define atomic_and_val(mem, mask)
```

##### catomic_and

一样，使用catomic系列原语

##### catomic_and_val

同上

#### 或

##### atomic_or

`*mem | mask`，无返回值

```c
# define atomic_or(mem, mask)
```

##### atomic_or_val

同上，返回old_val

##### catomic_or

使用catomic系列

##### catomic_or_val

同上

#### 最大值

原子最大值，`*mem = new_val > old_val ? new_val : old_val`

##### atomic_max

```c
# define atomic_max(mem, value)
```

主要操作如下

```c
    do {                                      \
      __atg8_oldval = *__atg8_memp;                          \
      if (__atg8_oldval >= __atg8_value)                      \
    break;                                      \
    } while (__builtin_expect                              \
         (atomic_compare_and_exchange_bool_acq (__atg8_memp, __atg8_value,\
                            __atg8_oldval), 0));  
```

##### catomic_max

一样

#### 最小值

原子最小值，`*mem = new_val < old_val ? new_val : old_val`

与最大值原理一样，不赘述，函数：

* `atomic_min`

#### bit_set

##### atomic_bit_test_set

置位`*mem`的第bit位，返回旧值是否置位（非bool值，而是返回`old_val & (1<<bit)`）

```c
atomic_bit_test_set(mem, bit)
```

##### atomic_bit_set

直接调用atomic_bit_test，区别仅在于无返回值

#### barrier

内存屏障，用于同步内存

##### atomic_full_barrier

这里的写法很有趣

```c
# define atomic_full_barrier() __asm ("" ::: "memory")
```

##### atomic_write_barrier

直接调用上述内存屏障

##### atomic_read_barrier

同上

##### atomic_forced_read

用于强制将某个变量写入寄存器

这里写法也很有趣，具体解析可以见最后一节

```c
# define atomic_forced_read(x) \
  ({ __typeof (x) __x; __asm ("" : "=r" (__x) : "0" (x)); __x; })
```

### C11 原子操作

这部分是C11编译器原生提供的原子操作，下面会记录其对应的函数名

此外，对于调用了C11原生的原子操作，但使用了非C11编译器的程序，这里提供了一套使用上面定义的原子操作实现的，与C11原生原子操作命名相同的函数

#### 原语

原语的官方文档在这 https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html

##### __atomic_thread_fence

https://en.cppreference.com/w/cpp/atomic/atomic_thread_fence

或者中文版

https://zh.cppreference.com/w/cpp/atomic/atomic_thread_fence

##### 读

__atomic_load_n

##### 写

__atomic_store_n

#####CAS

__atomic_compare_exchange_n

##### 交换

__atomic_exchange_n

#####加法 fetch add

__atomic_fetch_add

##### 与 fetch and

__atomic_fetch_and

##### 或 fetch or

__atomic_fetch_or

##### 异或 fetch xor

__atomic_fetch_xor

#### 派生操作

##### atomic_thread_fence相关

下列三个都是使用不同的参数调用 [atomic_thread_fence](#__atomic_thread_fence)

* atomic_thread_fence_acquire
* atomic_thread_fence_release
* atomic_thread_fence_seq_cst

没有定义的情况下，分别对应

* [atomic_read_barrier](#atomic_read_barrier)
* [atomic_write_barrier](#atomic_write_barrier)
* [atomic_full_barrier](#atomic_full_barrier)

##### __atomic_load_n相关

* atomic_load_relaxed
* atomic_load_acquire

都是用**不同的memory order参数**调用__atomic_load_n，下同，不赘述

没有定义的情况下：

* atomic_load_relaxed
  
  ```c
  #  define atomic_load_relaxed(mem) \
     ({ __typeof ((__typeof (*(mem))) *(mem)) __atg100_val;              \
     __asm ("" : "=r" (__atg100_val) : "0" (*(mem)));                  \
     __atg100_val; })
  ```
  
  这条内联汇编将一个mem指向的内容传给一个寄存器，然后返回寄存器的值

* atomic_load_acquire
  
  ```c
  #  define atomic_load_acquire(mem) \
     ({ __typeof (*(mem)) __atg101_val = atomic_load_relaxed (mem);          \
     atomic_thread_fence_acquire ();                          \
     __atg101_val; })
  ```
  
  在atomic_load_relaxed后加了个acquire fence

##### __atomic_store_n相关

* atomic_store_relaxed
* atomic_store_release

没有定义的情况下：

* atomic_store_relaxed
  
  ```c
  #  define atomic_store_relaxed(mem, val) do { *(mem) = (val); } while (0)
  ```
  
  注意这里直接用了条C的赋值语句，可能非原子

* atomic_store_release
  
  ```c
  #  define atomic_store_release(mem, val) \
     do {                                          \
       atomic_thread_fence_release ();                          \
       atomic_store_relaxed ((mem), (val));                      \
     } while (0)
  ```
  
  在atomic_store_relaxed前加了个release fence

##### __atomic_compare_exchange_n相关

* atomic_compare_exchange_weak_relaxed
* atomic_compare_exchange_weak_acquire
* atomic_compare_exchange_weak_release

没有定义的情况下：

* atomic_compare_exchange_weak_relaxed
  
  直接调用了下述的atomic_compare_exchange_weak_acquire

* atomic_compare_exchange_weak_acquire
  
  ```c
  #  define atomic_compare_exchange_weak_acquire(mem, expected, desired) \
     ({ typeof (*(expected)) __atg102_expected = *(expected);              \
     *(expected) =                                  \
       atomic_compare_and_exchange_val_acq ((mem), (desired), *(expected));     \
     *(expected) == __atg102_expected; })
  ```
  
  使用先前定义的[atomic_compare_and_exchange_val_acq](#atomic_compare_and_exchange_val_acq)实现

* atomic_compare_exchange_weak_release
  
  ```c
  #  define atomic_compare_exchange_weak_release(mem, expected, desired) \
     ({ typeof (*(expected)) __atg103_expected = *(expected);              \
     *(expected) =                                  \
       atomic_compare_and_exchange_val_rel ((mem), (desired), *(expected));     \
     *(expected) == __atg103_expected; })
  ```
  
  使用先前定义的[atomic_compare_and_exchange_val_rel](#atomic_compare_and_exchange_val_rel)实现

##### __atomic_exchange_n相关

* atomic_exchange_relaxed
* atomic_exchange_acquire
* atomic_exchange_release

没有定义的情况下：

* atomic_exchange_relaxed
  
  直接调用先前定义的[atomic_exchange_acq](#atomic_exchange_acq)

* atomic_exchange_acquire
  
  同上

* atomic_exchange_release
  
  直接调用先前定义的[atomic_exchange_rel](#atomic_exchange_rel)

##### __atomic_fetch_add相关

* atomic_fetch_add_relaxed
* atomic_fetch_add_acquire
* atomic_fetch_add_release
* atomic_fetch_add_acq_rel

没有定义的情况下：

* atomic_fetch_add_relaxed
  
  直接调用[atomic_fetch_add_acquire](#atomic_fetch_add_acquire)

* atomic_fetch_add_acquire
  
  直接调用[atomic_exchange_and_add_acq](#atomic_exchange_and_add_acq)，注意与上面的区别，一个是fetch_add，一个是exchange_and_add，下同

* atomic_fetch_add_release
  
  直接调用[atomic_exchange_and_add_rel](#atomic_exchange_and_add_rel)

* atomic_fetch_add_acq_rel
  
  ```c
  #  define atomic_fetch_add_acq_rel(mem, operand) \
     ({ atomic_thread_fence_release ();                          \
     atomic_exchange_and_add_acq ((mem), (operand)); })
  ```
  
  在read-modify-write操作前添加了一个fence

##### __atomic_fetch_and相关

* atomic_fetch_and_relaxed
* atomic_fetch_and_acquire
* atomic_fetch_and_release

没有定义的情况下：

* atomic_fetch_and_relaxed
  
  直接调用下述atomic_fetch_and_acquire

* atomic_fetch_and_acquire
  
  直接调用[atomic_and_val](#atomic_and_val)

* atomic_fetch_and_release
  
  ```c
  #  define atomic_fetch_and_release(mem, operand) \
     ({ atomic_thread_fence_release ();                          \
     atomic_and_val ((mem), (operand)); })
  ```
  
  与其他release操作类似，添加了一个fence

##### __atomic_fetch_or相关

* atomic_fetch_or_relaxed
* atomic_fetch_or_acquire
* atomic_fetch_or_release

没有定义的情况下：

* atomic_fetch_or_relaxed
  
  直接调用下述atomic_fetch_or_acquire

* atomic_fetch_or_acquire
  
  直接调用[atomic_or_val](#atomic_or_val)

* atomic_fetch_or_release
  
  ```c
  #  define atomic_fetch_or_release(mem, operand) \
     ({ atomic_thread_fence_release ();                          \
     atomic_fetch_or_acquire ((mem), (operand)); })
  ```
  
  与其他release操作类似，添加了一个fence

##### __atomic_fetch_xor相关

* atomic_fetch_xor_release

没有定义的情况下：

* atomic_fetch_xor_release
  
  ```c
  # define atomic_fetch_xor_release(mem, operand) \
    ({ __typeof (mem) __atg104_memp = (mem);                      \
       __typeof (*(mem)) __atg104_expected = (*__atg104_memp);              \
       __typeof (*(mem)) __atg104_desired;                      \
       __typeof (*(mem)) __atg104_op = (operand);                      \
                                            \
       do                                          \
         __atg104_desired = __atg104_expected ^ __atg104_op;              \
       while (__glibc_unlikely                              \
          (atomic_compare_exchange_weak_release (                  \
             __atg104_memp, &__atg104_expected, __atg104_desired)          \
           == 0));                                  \
       __atg104_expected; })
  ```
  
  这段代码其实与[派生原子操作的代码模板](#派生原子操作的代码模板)描述一致，因此不赘述

### 外部引用与依赖

由于原子操作是由平台提供的操作，与架构直接相关，因此必然有一些外部引用的函数。

atomic.h主要提供了对原语操作的高层封装，从原语派生出一系列原子操作

需要注意的是，[C11派生原子操作](#C11 派生原子操作) 中描述了一系列由符合C11规范的编译器引入的原子操作

#### 原语

##### atomic_compare_and_exchange_val_acq

几乎所有的原语都是由`atomic_compare_and_exchange_val_acq`派生，而这个宏默认又是由下列四个函数定义的，因此这里应该是外部引入的原子操作定义。

* `__arch_compare_and_exchange_val_8_acq`
* `__arch_compare_and_exchange_val_16_acq`
* `__arch_compare_and_exchange_val_32_acq`
* `__arch_compare_and_exchange_val_64_acq`

##### catomic_compare_and_exchange_val_acq

在`catomic_compare_and_exchange_val_acq`中若定义了`__arch_c_compare_and_exchange_val_32_acq`标志，则会从外部引入下列函数。否则直接引用`atomic_compare_and_exchange_val_acq`

* `__arch_c_compare_and_exchange_val_8_acq`
* `__arch_c_compare_and_exchange_val_16_acq`
* `__arch_c_compare_and_exchange_val_32_acq`
* `__arch_c_compare_and_exchange_val_64_acq`

##### atomic_compare_and_exchange_bool_acq

对于bool的acq，也与上面类似，因此可能从外部引入下列4个函数

* `__arch_compare_and_exchange_bool_8_acq`
* `__arch_compare_and_exchange_bool_16_acq`
* `__arch_compare_and_exchange_bool_32_acq`
* `__arch_compare_and_exchange_bool_64_acq`

##### catomic_compare_and_exchange_bool_acq

对于有c前缀的，若定义了`__arch_c_compare_and_exchange_bool_32_acq`则引入下列函数，否则直接使用`atomic_compare_and_exchange_bool_acq`

* `__arch_c_compare_and_exchange_bool_8_acq`
* `__arch_c_compare_and_exchange_bool_16_acq`
* `__arch_c_compare_and_exchange_bool_32_acq`
* `__arch_c_compare_and_exchange_bool_64_acq`

#### 派生操作

##### atomic_exchange_and_add

原子交换加法有可能通过外部引用

### 一些有趣（或者匪夷所思）的写法

##### typeof

匪夷所思

```c
__typeof ((__typeof (*(mem))) *(mem)) __atg5_oldval;
```

假设mem为`int *`，代入则为`__typeof ((int) *(mem))`，即`__typeof ((int) *(mem))`，相当于将`*(mem)`进行了强制类型转换，但转换的类型又是`*(mem)`本身的类型。所以为什么不直接用`__typeof(*(mem))`呢

原因在glibc邮件列表找到了 [click here](http://sourceware.org/legacy-ml/libc-alpha/2017-02/msg00158.html)

大体说来就是一个typeof的写法在指针为volatile类型时会造成多余的压栈和出栈操作（因为保存volatile值），所以使用两个typeof来去掉volatile属性

真申必，特别是下面这种写法跟单个typeof混用的时候

全部当typeof看就完事了 （

##### 内存屏障

glibc是这样实现屏障的

```c
# define atomic_full_barrier() __asm ("" ::: "memory")
```

此外，默认情况下read_barrier和write_barrier也是用的上述写法

##### atomic_forced_read

```c
# define atomic_forced_read(x) \
  ({ __typeof (x) __x; __asm ("" : "=r" (__x) : "0" (x)); __x; })
```

用于强制将__x写入内存并读取。这里的内联汇编中

* OutputOprand为 `"=r" (__x)` 表示输出到存放`__x`的寄存器
* InputOprand为 `"0" (x)` 表示将x放入和OutputOprand一样的寄存器

因此最终的效果是x被load进`__x`，且`__x`此时在一个寄存器上。

最终返回这个`__x`

##### atomic load

* atomic_load_relaxed
  
  ```c
  #  define atomic_load_relaxed(mem) \
     ({ __typeof ((__typeof (*(mem))) *(mem)) __atg100_val;              \
     __asm ("" : "=r" (__atg100_val) : "0" (*(mem)));                  \
     __atg100_val; })
  ```
  
  使用内联汇编的方式实现，但实际只指定了Input Operand和Ouput Operand，内联汇编定义中没有具体的其他指令

* atomic_load_acquire
  
  ```c
  #  define atomic_load_acquire(mem) \
     ({ __typeof (*(mem)) __atg101_val = atomic_load_relaxed (mem);          \
     atomic_thread_fence_acquire ();                          \
     __atg101_val; })
  ```
  
  load acquire操作在atomic_load之后添加了一个fence，这里可以对比store release操作

##### atomic store

* atomic_store_relaxed
  
  ```c
  #  define atomic_store_relaxed(mem, val) do { *(mem) = (val); } while (0)
  ```

* atomic_store_release
  
  ```c
  #  define atomic_store_release(mem, val) \
     do {                                          \
       atomic_thread_fence_release ();                          \
       atomic_store_relaxed ((mem), (val));                      \
     } while (0)
  ```
  
  store release操作在atomic store前添加了一个fence，可以对照上面的load acquire操作

### 可能存在的bug

##### atomic store

* atomic_store_relaxed
  
  ```c
  /* XXX Use inline asm here?  */
  #  define atomic_store_relaxed(mem, val) do { *(mem) = (val); } while (0)
  ```

* atomic_store_release
  
  ```c
  #  define atomic_store_release(mem, val) \
     do {                                          \
       atomic_thread_fence_release ();                          \
       atomic_store_relaxed ((mem), (val));                      \
     } while (0)
  ```

这里注释也指出了问题，store这个操作应该是原子的，但这里的写法可能生成非原子的代码
