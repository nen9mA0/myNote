#### 原语

所有的atomic都是基于原语实现的，原语主要就是CAS操作

即对一个内存值mem进行比较，分别比较newval和oldval，并做如下操作

```c
if(*mem == oldval)
{
	mem = newval;
	return true;
}
else
	return false;
```



##### catomic_compare_and_exchange_val_acq

```c
#ifndef catomic_compare_and_exchange_val_acq
# ifdef __arch_c_compare_and_exchange_val_32_acq
#  define catomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  __atomic_val_bysize (__arch_c_compare_and_exchange_val,acq,		      \
		       mem, newval, oldval)
# else
#  define catomic_compare_and_exchange_val_acq(mem, newval, oldval) \
  atomic_compare_and_exchange_val_acq (mem, newval, oldval)
# endif
#endif
```

若定义了`__arch_c_compare_and_exchange_val_32_acq`，则使用`__atomic_val_bysize`宏来定义，这个宏实际上是根据mem（mem是一个指针）的长度判断当前计算机的指针长度，根据指针长度的返回值如下

* 8  `__arch_c_compare_and_exchange_val_8_acq`
* 16  `__arch_c_compare_and_exchange_val_16_acq`
* 32  `__arch_c_compare_and_exchange_val_32_acq`
* 64  `__arch_c_compare_and_exchange_val_64_acq`

否则直接使用`atomic_compare_and_exchange_val_acq`函数作为CAS操作原语

##### atomic_compare_and_exchange_val_rel

与atomic_compare_and_exchange_val_acq相同

##### atomic_compare_and_exchange_bool_acq

与[catomic_compare_and_exchange_val_acq](#catomic_compare_and_exchange_val_acq)类似，若定义了`__arch_c_compare_and_exchange_bool_32_acq`，则直接调用`__arch_c_compare_and_exchange_bool_n_acq`，其中n可能是8 16 32 64

否则只使用`atomic_compare_and_exchange_val_acq`函数，实现如下

```c
#  define catomic_compare_and_exchange_bool_acq(mem, newval, oldval) \
  ({ /* Cannot use __oldval here, because macros later in this file might     \
	call this macro with __oldval argument.	 */			      \
     __typeof (oldval) __atg4_old = (oldval);				      \
     catomic_compare_and_exchange_val_acq (mem, newval, __atg4_old)	      \
       != __atg4_old;							      \
  })
```

很简单，因为oldval和newval都是bool值

#### 派生原子操作

