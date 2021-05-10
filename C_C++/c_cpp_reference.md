本来想搞成全文档翻译，不过一看有 https://zh.cppreference.com，还是当笔记好了

# C

https://en.cppreference.com/w/c/language

## Basic Concepts



### Objects and alignment

#### object representation



### C memory model

https://en.cppreference.com/w/c/language/memory_model

定义了使用C的抽象模型的机器中计算机内存的语义

C程序可用的数据存储对象（内存）是一个或多个字节组成的序列。每个字节都有一个独立的内存地址。

#### Byte

Byte（字节）是内存中可寻址的最小单位。它被定义为多个连续二进制位的序列，并且足以容纳任意一个*基本字符*（[这96个字符](https://en.cppreference.com/w/c/language/translation_phases) 应能被一字节容纳）。C支持8位及以上的二进制位作为一个字节。

类型 **char / unsigned char / signed char** 使用一个字节用于存储或用于value representation（值的表示）。可以使用 **CHAR_BIT** 来获取一个字节的二进制位数。

关于使用字节来表示其他基本类型（包括内存大小端序），见 [object representation](#object representation)

#### Memory Location

Memory location是

* 一个scalar type（标量类型：算术类型、指针类型、枚举类型）对象
* 最大连续非零bit field（位域）序列

```c
struct S {
    char a;     // memory location #1
    int b : 5;  // memory location #2
    int c : 11, // memory location #2 (continued)
          : 0,
        d : 8;  // memory location #3
    struct {
        int ee : 8; // memory location #4
    } e;
} obj; // The object 'obj' consists of 4 separate memory locations
```

#### Threads and data races

线程是执行程序中的一个控制流，该控制流是由 thrd_create 或其他顶层函数所创建的

线程理论上可以访问程序中的任何对象（即使是auto对象或 thread-local storage duration TLS对象也可以通过指针被其他线程访问）

正在执行的不同线程总能同时访问（读取或修改）不同的 memory location，这一过程互不干扰，因此不需要同步（注意，如果对于同一个结构体中的两个非原子bit field，两者之间的元素都是采用非零bit field方式声明的，那么不管两者相距多少位，同时修改这两个bit field是不安全的）。

若一个表达式的evaluation（求值）对一个 memory location 进行写入，与此同时另一个求值读取或修改同一个 memory location ，那么称该表达式 conflict （冲突）。当出现下列情况时程序中两个conflict evaluation（冲突的表达式）存在 data race （数据竞争）

* 两个evaluation在同一个thread中运行，或在同一个signal handler （信号处理函数）
* 一个conflicting evaluation和另一个的关系是 happen-before（早于） （见[memory order](#memory order)）

当data race出现的时候，程序的行为 undefined （未定义）

（特别的，mtx_unlock与另一线程的mtx_lock同步，从而保证mtx_unlock happen-before mtx_lock，因此可以用互斥锁来消除data race）

### C memory order

https://en.cppreference.com/w/c/atomic/memory_order

```c
enum memory_order {
    memory_order_relaxed,
    memory_order_consume,
    memory_order_acquire,
    memory_order_release,
    memory_order_acq_rel,
    memory_order_seq_cst
};
```

memory order 指定了在一个原子操作的周围，其他内存访问的组织方式。

#### Relaxed ordering

使用`memory_order_relaxed`标记的原子操作是非同步的，它不会对多个内存访问强加一个顺序，而只保证操作的原子性和写操作的顺序一致性

```c
// Thread 1:
r1 = atomic_load_explicit(y, memory_order_relaxed); // A
atomic_store_explicit(x, r1, memory_order_relaxed); // B
// Thread 2:
r2 = atomic_load_explicit(x, memory_order_relaxed); // C
atomic_store_explicit(y, 42, memory_order_relaxed); // D
```

若使用relaxed ordering，则完全可能出现`r1==r2==42`的情况，因为这种组织方式不保证对不同内存的访问顺序，因此即使程序里语句的出现顺序是A在B前，C在D前，编译器编译时完全可能出现thread2的CD顺序调换，使得D首先执行，其后执行AB，最后执行C，就会出现都为42的情况

一个典型应用就是一个自增的计数器，因为这种计数器只需要保证原子性。但需要注意shared_ptr中的计数器减操作需要锁

#### Release-Consume ordering

正确的使用方式是

```c
// Thread 1:
atomic_store_explicit(x, memory_order_release);		// store时使用release

// Thread 2:
r1 = atomic_load_explicit(x, memory_order_consume);	// load时使用consume
```



# Cpp

# Cpp

## Basic concepts

### C++ memory model

https://en.cppreference.com/w/cpp/language/memory_model

#### Byte

与C的一样

#### Memory location

与C的一样

#### Thread and data races

存在data race的情况有下列三种

* 两个evaluation在同一个thread中运行，或在同一个signal handler （信号处理函数）
* 两个 conflicting evaluation 都是原子操作（见std::atomic）
* 一个conflicting evaluation和另一个的关系是 happen-before（早于） （见[memory order](#memory order)）

（特别的，一个线程的std::mutex与另一线程的同步，从而保证mutex的释放 happen-before mutex的获取，因此可以用std::mutex来消除data race）

无锁

```cpp
int cnt = 0;
auto f = [&]{cnt++;};
std::thread t1{f}, t2{f}, t3{f}; // undefined behavior
```

有锁

```cpp
std::atomic<int> cnt{0};
auto f = [&]{cnt++;};
std::thread t1{f}, t2{f}, t3{f}; // OK
```

#### Forward progress

##### Obstruction freedom

当只有一个没有阻塞在标准库函数中的线程执行一个lock-free的原子函数时，则保证该执行可以完成（所有标准库中的lock-free操作都是obstruction-free的）

（详细定义见背景知识中的Non-blocking Algorithm）

##### lock freedom

当一个或多个lock-free的函数并行执行时，可以保证其中至少一个执行可以完成（所有在标准函数中的lock-free操作都是lock-free的——这里的实现应确保这些操作不会被其他线程造成无限的live-lock，如以连续窃取cache line的方式）

##### Progress guarantee

在一个可用的C++程序中，每个线程的操作最终肯定会执行下列操作之一

* 终止
* 调用IO函数
* 通过 volatile 泛左值进行访问
* 进行原子操作或同步操作

任何线程都不可能无限地执行并且不进行上面的几个操作之一。

所以如果一个程序含有无限循环或无限递归，则它的行为是undefined。所以允许编译器在不需要证明程序是否最后会以某个条件终止的情况下，移除没有操作（no observable behavior）的循环。

一个线程如果具有上述四种操作，或阻塞在标准库中，或调用了一个由于某个未阻塞的并发线程（non-blocked concurrent thread）而未完成的原子 lock-free 函数，则称其 make progress

###### Concurrent forward progress

保证程序只要未终止，就可以在有限时间内make progress，不论其他的线程是否make progress

标准鼓励但不要求主线程和其他使用std::thread启用的线程提供 Concurrent forward progress 保证

###### Parallel forward progress

不需要保证线程最终执行上述的四种操作之一，但一旦执行了四种操作之一，则保证之后的程序是 Concurrent forward progress 的（这条规则的最典型的场景就是线程池的情况，当线程池没有任务时所有线程都在等待，但一旦有任务，最后线程都需要make progress）

###### Weakly parallel forward progress

不管其他线程有没有make progress，都不需要保证当前线程make progress。但在下述情况下保证make progress：

若一个线程P以此种方式阻塞了线程集合S，则S中至少一个线程应提供与P相同或更强的 forword progress guarantee。当这个线程完成后，S中剩余的其他线程也会依次以同样方式进展，直到S为空解锁





# 一些背景知识

## Non-blocking Algorithm

https://en.wikipedia.org/wiki/Non-blocking_algorithm

非阻塞算法：当一个算法的一个线程失败或挂起时，不会影响其他线程

### 动机

非阻塞算法的目的就是消除程序运行中的锁，即用一些设计使得程序可以不依赖锁的同步来运行。好处显而易见

### 实现

一般的实现依赖于硬件提供的原子操作，一般Non-blocking级别分为下面几种

* Wait-freedom

  最高等级的Non-blocking。首先它保证所有线程都不会被饿死，其次保证所有的线程都能在有限步内取得进展（make progress），即没有线程会被永久阻塞

  含义就是没有两个线程间是互锁的，因为如果有互锁的线程，那么若一个线程崩溃或挂起，另一个线程可能进入死锁，如当崩溃的线程拥有一个锁的时候

* Lock-freedom

  次一等级的Non-blocking。它保证所有的线程中至少有一个线程能在有限步内取得进展，即不会发生所有线程都被阻塞的情况

  含义就是当一个线程崩溃或挂起，其他线程可以继续取得进展。举例来说，如果一共有两个线程，这两个线程需要争夺一个mutex或spinlock，那么这是非lock-free的，因为如果拿到锁的线程被挂起，那么另一个线程将无限等待

  一个Lock-free算法可以分为四步

  * 完成自己的操作
  * 协助进行阻塞操作
  * 中止阻塞操作
  * 等待

  其中 *完成自己的操作* 这一步骤可能被其他线程的 *协助阻塞* 或 *中止阻塞*  的过程打断

* Obstruction-freedom

  最低等级的Non-blocking。它保证在任何时候，当一个线程运行在一个隔离的环境下时（如所有可能发生竞争的线程都被挂起），它可以在有限步内取得进展

  Obstruction-freedom的要求只是线程中的任何部分完成的操作（partially completed operation）都可以被暂停，且更改可以被回滚。

  不使用lock-freedom中的同步协助（concurrent assistance）可以使算法更加简单。此外，contention manager可以防止系统进入“活锁”（live-locking）

  一些Obstruction-freedom算法在数据结构中使用一对一致性标记“consistency markers”来同步。进程在读取数据时先读取一个标记，此后加载对应的数据并处理，然后再读取另一个标记。若两个标记不是相同的，则说明可能先前在处理的过程中数据被其他线程修改过，这时就丢弃结果重新读取并处理。

  **上面例子的解释**：我觉得这里的意思应该是让这两个标记作为“开始操作”和“结束操作”的标志，其中对于标记的操作应该是原子的

  * 正常操作流程： 在开始操作的时候读入旧标记，并放上一个新标记，结束操作的时候比较旧的开始标记和结束标记，此后再将结束标记更新为与新的开始标记一样的值。

  * 被抢占时的流程： 如果A读完，处理一半被B抢占，首先B读取的开始标记和当前的结束标记已经不同（因为A已经更新了），那么B处理完在比较标记时肯定失败，那么修改就会被丢弃（包括将B更新的开始标记重新还原为A的）。此时A继续执行，则可以正常结束）
  * 轮流抢占时的流程：如果上面例子中，B也没有处理完就被A抢占，那么A执行完正常流程后比较标记为正确（因为A本身保存了旧的标记，即使当前开始标记已经被B的新值覆盖）。B执行完后比较标记错误，那么又会将开始标记还原为当时A更新的值



