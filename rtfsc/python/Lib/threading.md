## 版本

```
Python 3.7.9 (tags/v3.7.9:13c94747c7, Aug 17 2020, 18:58:18) [MSC v.1900 64 bit (AMD64)] on win32
```

## 锁

首先python使用的底层接口（cpython）提供了操作系统的锁，如在windows中默认创建的锁就是`CreateSemaphore`，具体可见下面链接

https://blog.csdn.net/qq_42174113/article/details/84110067

### RLock

reentrant lock，可重入锁。基本特性是同一个线程可以多次acquire，如果acquire了n次，则需要release n次来释放。**注意，获取和释放锁必须都是同一个线程的**

#### 属性

* `_block`  底层操作系统提供的锁
* `_owner`  进程标识符
* `_count`  重入计数

#### 方法

##### acquire

获取锁

* 若acquire的线程id与`_owner`相同
  * `_count`++
  * 返回
* 否则调用_block的acquire尝试获取锁
  * 若获取到了，`_owner`等于当前线程id，`_count`=1

##### release

释放锁

* 若owner与线程id不同，发生错误，因为当前该RLock没有被该线程获取
* count--
* 若count=0，则调用`_block`的release将RLock释放，此时该RLock可以被其他线程获取

##### _acquire_restore

由Condition使用

* 不判断线程id直接acquire

* 由参数传入owner和count

##### _release_save

由Condition使用

* 若count=0，发生错误，因为当前锁没有被acquire过

* 不判断线程id直接调用`_block`的release，count=0 owner=None

* 返回原来的count和owner

### Condition

条件变量

_lock用于保证同一时间只能有一个线程获取Condition

当wait被调用的时候，一个新的lock（waiter）被添加进`_waiters`列表（注意这里的lock不是RLock，没有对哪个线程acquire和release做出限制）。并且waiter被acquire，从而陷入阻塞，与此同时_lock被释放，使得将要执行notify的唤醒当前阻塞状态的线程能够获取到Condition

当notify被调用时，_waiters列表的n个项的锁被释放，从而实现唤醒

一般在notify被调用后必须调用wait，否则被notify的线程无法获取Condition，从而会被阻塞在`_acquire_restore`这一步

#### 属性

* `_lock`  存储该Condition使用的RLock，若没传入则创建一个
* `_waiters`  存储阻塞线程的deque

#### 方法

##### wait

```python
def wait(self, timeout=None)
```

阻塞一个线程

* 判断RLock所有者是否与线程一致，不一致则发生错误
* 新建一个lock（名字就叫waiter吧），对waiter进行acquire操作，添加进waiters列表
* 保存`_lock`的值并释放`_lock`（使用`_release_save`），此时该条件变量可以被其他线程获取
* 根据timeout值
  * 若timeout为None，acquire一次waiter使线程阻塞
  * 若timeout不为None
    * 若大于0，使用带timeout的参数acquire waiter
    * 若小于等于0，非阻塞acquire waiter
* 先使用 `_acquire_restore` 将 `_lock` 原本的状态恢复
* 根据waiter的返回情况
  * 若没有获取到waiter，说明到了timeout指定的时间，则将waiter从队列中清除
  * 否则不需要清除waiter

##### notify

唤醒n个线程

* 判断RLock所有者是否与线程一致，不一致则报错
* 将_waiters列表的n个锁release，并且将其移出列表

##### wait_for

在timeout内，若`predicate()==True`则返回

```python
def wait_for(self, predicate, timeout=None)
```

#### 示例

见 [producer_consumer](program/producer_consumer.py)

```python
class Producer(threading.Thread):
    def run(self):
        print("Producer tid: %d" %threading.get_ident())
        while True:
            if cond.acquire():
                if num < threshold:
                    self.Produce()
                else:
                    cond.notify()
                cond.release()

    def Produce(self):
        global num
        print("%d: produce 1" %threading.get_ident())
        num += 1
        time.sleep(0.5)

class Consumer(threading.Thread):
    def run(self):
        print("Consumer tid: %d" %threading.get_ident())
        while True:
            if cond.acquire():
                if num >= threshold:
                    self.Consume()
                else:
                    cond.wait()
                cond.release()

    def Consume(self):
        global num
        print("%d: consume 1" %threading.get_ident())
        num -= 1
        time.sleep(0.5)
```

### Semaphore

实现了信号量

#### 属性

* `_cond`  用于执行唤醒和等待操作的条件变量
* `_value`  信号量值

#### 方法

##### acquire

P操作，表示申请资源

```python
def acquire(self, blocking=True, timeout=None)
```

* 若`_value==0`
  * 若blocking不为为True，则直接返回
  * 若timeout大于0，则减去花费的时间
  * 调用`_cond.wait`阻塞
* 否则说明 `_value!=0` `_value-=1`

##### release

V操作，表示释放资源

```python
def release(self)
```

* `_value+=1`
* `_cond.notify`

这里有个有趣的写法。本来V操作应该判断value是否为0的，因为如果为0且有多个进程阻塞，直接+=1会导致资源数量与实际不符（唤醒进程后应该-=1）。而这里因为acquire使用了一个while else结构避免了这个问题

### BoundedSemaphore

加了个不允许当前value大于初始的value的特性

## 事件

### Event

实现了事件的逻辑。当事件没有发生时，线程等待。当发生时唤醒这些线程

#### 属性

* `_cond`  用于执行唤醒和等待操作的条件变量
* `_flag`  表示当前事件是否发生

#### 方法

##### set

事件发生，唤醒所有等待的线程

```python
def set(self)
```

* 获取条件变量
* `_flag = True`
* notify_all

##### is_set

```python
def is_set(self)
```

* 返回 `_flag` 参数

##### clear

```python
def clear(self)
```

* 获取条件变量

* `_flag = False`

##### wait

```python
def wait(self, timeout=None)
```

加入事件等待队列

* 获取条件变量
* 如果flag不为True，则`_cond.wait`

##### _reset_internal_locks

用于重置内部锁的状态

* 重新初始化 `_cond`

## 屏障

### Barrier

作用是同步n个线程，当第n个线程到达时同时唤醒这些线程

#### 属性

##### 内部属性

* `_cond`  条件变量
* `_state`  记录当前的状态，有如下四种
  * 1  draining  线程已唤醒状态
  * 0  filling  正在填充等待线程
  * -1  resetting  重置状态，需要等待还在线程等待池中的所有线程运行后才会置为filling
  * -2  broken  exception状态
* `_count`  当前等待的线程数
* `_partis`  记录要同步的线程数
* `_action`  同步线程唤醒前要执行的函数

##### 外部属性

* `parties`  返回 `_partis` 内部属性，记录要同步的线程数

* `n_waitring`  若`_state` 正在等待，返回当前正在等待的线程数`_count`

* `broken`  判断当前`_state`是否为broken状态

#### 方法

##### _enter

wait调用的内部方法，用于在进入wait前等待其他状态运行完成

```python
def _enter(self)
```

* 如果在reset或drain状态，`_cond.wait`等待执行完
* 执行完理应state==0，否则为-2，即broken，抛出错误

##### _wait

wait调用的内部方法，等待state由0变为1

* `_cond.wait_for`  等待`_state`直到其由0（filling）变为1（draining）

##### _release

wait调用的内部方法，唤醒所有线程

* 执行`self._action()`
* `_state`  置一
* `_cond.notify_all()`
* 错误处理：若上述流程发生错误，抛出异常并调用 [abort](#abort)

##### _break

检测到内部错误

```python
def _break(self)
```

* `_state = -2`

* `_cond.notify_all()`

##### _exit

如果当前线程是最后一个离开barrier的线程，通知所有正在等待的线程执行

* 若count为0
  * 若state为-1（reset）或1（drain）
    * state设为0（filling）
    * notify_all

##### wait

等待partis个线程同步，若已有partis个内存调用了wait，则一起执行

* 调用_enter，等待barrier就绪
* `count+=1`
* 如果`count==partis`，则调用_release
* 否则调用_wait
* `count-=1`，这里每个线程在barrier阻塞解除后就会运行到这，count-1表示当前还未唤醒的线程还有一个
* _exit  当最后一个线程唤醒时执行操作解除barrier的drain状态

##### reset

重置barrier状态。这里当count不为0时，说明等待池里还有线程未唤醒，所以应该置为-1。当所有线程退出等待池后state自动置0

* 如果count不为0
  * 若state为0（filling），置为-1（reset）
  * 若state为-2（break），置为-1
* 否则state置为0
* notify_all

##### abort

用于处理Barrier的错误

```python
def abort(self)
```

* 获取 `_cond` 锁

* 调用 [_break](#_break) 

## 线程

除了对Thread类的实现外，还有一系列函数和变量来保证多线程的运行

### 全局变量

#### 预定义线程

##### main_thread

[_MainThread](#_MainThread) 类，标识主线程

#### 活动线程相关

##### _active

记录活动线程的字典

##### _limbo

 记录中间态线程的字典，所谓中间态即从 [start](#start) 到 [run](#run) 之间的状态

##### _active_limbo_lock

锁，用于同步 `_active` 和 `_limbo`

##### _dangling

WeakSet类型，用于记录所有使用Thread创建的线程，在 [_after_fork](#_after_fork) 中被使用

#### shutdown集合

用于存放所有没有守护线程的线程的`_tstate_lock`。当所有线程的 `_tstate_lock` 被释放时python解释器才会退出

##### _shutdown_locks

集合，用于存放线程锁

##### _shutdown_locks_lock

集合的锁

#### hook

##### _profile_hook

全局的profile_hook函数

##### _trace_hook

全局的trace_hook函数

### 方法

##### setprofile

```python
def setprofile(func)
```

* 设置 [_profile_hook](#_profile_hook) 函数

##### settrace

```python
def settrace(func)
```

* 设置 [_trace_hook](#_trace_hook) 函数

##### current_thread

获取当前线程的引用

* 调用 [get_ident](#get_ident) 从 [_active](#_active) 字典中获取线程

##### active_count

获取活动线程和中间态线程数目

* 获取[_active_limbo_lock](#_active_limbo_lock)锁，返回 [_limbo](#_limbo)和[_active](#_active) 列表的长度的和

##### enumerate

获取当前的活动线程和中间态线程

* 获取[_active_limbo_lock](#_active_limbo_lock)锁，返回[_limbo](#_limbo)和[_active](#_active) 列表的内容

##### _enumerate

enumerate的内部版本，不获取锁

##### _shutdown

用于等待所有非守护线程执行结束

* 若main_thread已经停止，即 `_is_stopped` 为True，直接返回

* 尝试获取main_thread的 `_tstate_lock` ，若可以获取，说明main_thread执行结束，调用 [_stop](#_stop) 结束main_thread线程

* 死循环
  
  * 获取锁，获取 [_shutdown_locks](#_shutdown_locks) 的内容，并清空该集合
  
  * 若内容为空，则break
  
  * 否则试图循环获取每个锁，若获取到说明线程执行完成，则释放锁。直到所有锁被释放则退出

##### _after_fork

该函数在fork后被调用

* 重新分配一个 [_active_limbo_lock](#_active_limbo_lock) 锁

* 试图从 [_active](#_active) 获取当前线程作为main_thread，若不在字典中（fork函数不是通过theading.Thread创建的情况下），则创建一个 [_MainThread](#_MainThread) 类作为主线程

* 重置 [_shutdown_locks](#_shutdown_locks) 集合，重新创建 [_shutdown_locks_lock](#_shutdown_locks_lock) 锁

* 使用 [_enumerate](#_enumerate) 获取所有的线程，此外加上 [_dangling](#_dangling) 的所有线程（即所有注册过的线程）

* 对上述的每一个线程采取如下操作
  
  * 若是当前线程
    
    * 调用 [_reset_internal_locks](#_reset_internal_locks) 重置锁
    
    * 调用 [get_ident](#get_ident) 获取新的标识符
    
    * 记录当前线程到new_active字典
  
  * 若不是当前线程
    
    * 调用 [_reset_internal_locks](#_reset_internal_locks) 重置锁
    
    * 调用 [_stop](#_stop) 函数回收线程

* 清空 [_limbo](#_limbo) 字典

* 清空 [_active](#_active) 字典

* 使用new_active字典的内容更新 `_active` 字典，此时 `_active`字典内容长度必为1

### Timer

经过指定延迟后调用一个函数

#### 属性

* finished  事件，用于标识延时是否结束

#### 方法

##### 初始化

```python
def __init__(self, interval, function, args=None, kwargs=None)
```

* interval  延时

* function  要调用的函数

* args kwargs  要传的参数

* 初始化一个 [Event](#Event) 

##### cancel

取消定时任务

* 直接触发finished事件

##### run

进行定时任务

* 调用 [wait](#wait) 方法在事件 finished 上等待interval秒

* 若 finished 没有被触发
  
  * 执行function

* 触发事件 finished

* 

### Thread

#### 属性

##### 内部属性

* `_target`  线程要执行的函数
* `_name` 线程名，若为None则自动生成名字 "Thread-%d"
* `_args`  `_kwargs`  参数  传给函数执行的参数
* `_daemonic` 标识当前线程是否为线程，若为None则使用 [current_thread](#current_thread) 的daemon
* `_ident` 标识符，初始化为None
* `_tstate_lock` 一个用于标识线程状态的锁
* `_started` 一个event类，用于标识当前线程是否正在活动
* `_is_stopped`  bool，标识当前线程是否停止
* `_initialized` 标识当前线程是否被初始化过

##### 外部属性

* name  线程名

* daemon  标识当前线程是否为守护线程，注意不能在线程start后设置该属性，否则这里会报错

#### 方法

##### 初始化

* 完成对上述属性的初始化

* 将当前初始化的thread类加入到 `_dangling` 集合

* 将 `_initialized` 置为True

##### start

* 若 `_initialized` 为false，说明没有初始化线程，报错

* 若 `_started` 事件的已经发生（[is_set](#is_set)） 说明该线程调用了两次start，报错

* 将线程加入 [_limbo](#_limbo) 字典

* 调用 `_start_new_thread` 建立新线程，该线程执行 [_bootstrap](#_bootstrap) 函数。这里因为线程实现是操作系统相关，所以由底层库thread实现
  
  * 若发生错误，将线程从 [_limbo](#_limbo) 字典移除

* 调用 [wait](#wait) 使主线程在 `_start` 事件上等待

##### run

* 调用 `_target` 函数

##### join

```python
def join(self, timeout=None)
```

* 几个check
  
  * 首先线程类必须init过
  
  * `_started` 事件必须触发过
  
  * 当前线程类不能是 [current_thread](#current_thread)，因为线程不能与自身join

* 根据timeout调用 [_wait_for_tstate_lock](#_wait_for_tstate_lock) 等待线程执行

##### is_alive

线程是否正在活动，活动线程具体指在run函数执行过程中的线程

* 若 `_is_stopped` 为True，或 `_started` 事件没有被触发，则线程不是活动线程

* 非阻塞调用 [_wait_for_tstate_lock](#_wait_for_tstate_lock) （block参数为False）

* 返回 `_is_stopeed` 取反的结果，若为True说明线程结束，返回值为False

##### _set_ident

获取标识符

* 调用thread中的 get_ident 函数

##### _set_tstate_lock

* 调用 `_set_sentinel` 初始化锁（赋值给 `_tstate_lock`）

* 获取锁

* 若当前线程不是守护线程（`daemon`）
  
  * 获取shutdown锁并将 `_tstate_lock` 加入 `_shutdown_locks`

##### _wait_for_tstate_lock

等待 [_tstate_lock](#_tstate_lock) ，该锁在线程结束后被释放

```python
def _wait_for_tstate_lock(self, block=True, timeout=-1)
```

* 若 `_tstate_lock` 为None，说明已经调用过 [_stop](_stop) 释放线程资源

* 否则先试图获取锁（使用block和timeout参数），若获取到说明线程结束，此时释放锁，并调用 [_stop](#_stop) 进行清理

##### _reset_internal_locks

该函数被 [_after_fork](#_after_fork) 调用，用来重置Thread类内用到的锁，is_alive参数用于指明当前的线程是否为活动线程

```python
def _reset_internal_locks(self, is_alive)
```

* 调用 `_started` 事件的 `_reset_internal_locks` 方法（见[Event](#Event)）

* 若 `is_alive` 为真，说明当前线程为活动线程
  
  * 调用 [_set_tstate_lock](#_set_tstate_lock) 初始化锁

* 否则
  
  * 设置 `_is_stopped` 为真
  
  * 将 `_tstate_lock` 设置为None

##### _bootstrap

* 简单的调用 [_bootstrap_inner](#_bootstrap_inner) ，加上错误处理

##### _bootstrap_inner

* try
  
  * 调用 [_set_ident](#_set_ident) 获取标识符
  
  * 调用 [_set_tstate_lock](#_set_tstate_lock) 为线程上锁
  
  * 将 `_started` 事件设置为True，唤醒所有等待函数。注意：此前在 [start](#start) 函数中主线程最后会在该事件上等待，因此此时主线程被释放。
  
  * 将当前线程加入 [_active](#_active) 字典，并从 [_limbo](#_limbo) 字典移除
  
  * 调用 `_sys.settrace` 设置trace函数为 [_trace_hook](#_trace_hook)
  
  * 调用 `_sys.setprofile` 设置profile函数为 [_profile_hook](#_profile_hook)
  
  * try
    
    * 调用 [run](#run) 方法运行target函数
  
  * 错误处理：
    
    * 若 `_sys.stderr` 不为None，则打印stderr的内容
    
    * 若 `self._stderr` 不为None
      
      * 调用 `_sys.exc_info` 获取执行信息，包括调用栈、错误类型和错误值
      
      * 打印调用栈和错误类型、错误值

* finally
  
  * 将线程从 [_active](#_active) 字典中删除

##### _stop

* 先检查 `_tstate_lock` 的状态，若不为None尝试获取锁，若无法获取则报错（因为此时说明锁还未释放）

* 设置 `_is_stopped` 为 True

* 设置 `_tstate_lock` 为 None

* 若线程不是守护线程（`daemon`属性）
  
  * 获取锁，将线程从 [_shutdown_locks](#_shutdown_locks) 集合移除

##### _delete

* 获取锁，将线程从 [_active](#_active) 字典删除

#### 锁操作

##### _tstate_lock

在线程调用的时候被上锁（[_bootstrap_inner](#_bootstrap_inner)），当线程退出时会解锁，这是在C代码中的逻辑

### _MainThread

继承Thread类，用于定义主线程

* 初始化Thread类，名称为"MainThread"，守护线程为False

* 调用 [_set_tstate_lock](#_set_tstate_lock) 初始化并设置主线程的状态锁

* 触发 `_started` 事件

* 调用 `_set_ident` 获取线程标识

* 获取锁，将线程加入 [_active](#_active) 列表

### _Dummy_Thread

继承Thread类，用于定义一个假线程。该函数只被 [current_thread](#current_thread) 调用，本身定义该类就是为了在调用 [current_thread](#current_thread) 时，若 [_active](#_active) 字典中没有对应当前线程的项，则返回该类。因此该类无法被join，且被设置为守护线程

#### 方法

##### 初始化

* 初始化Thread类，名称为 "Dummy-%d"，守护线程为True

* 触发 `_started` 事件

* 调用 `_set_ident` 获取线程标识
- 获取锁，将线程加入 [_active](#_active) 列表

##### _stop

pass

##### is_alive

判断 `_is_stopped` 是否被置位，以及 `_started` 事件是否被触发

##### join

报错，因为规定不能join一个dummy线程

## 一些问题

### Condition

#### `_is_owned`

```python
def _is_owned(self):
    # Return True if lock is owned by current_thread.
    # This method is called only if _lock doesn't have _is_owned().
    if self._lock.acquire(0):
        self._lock.release()
        return False
    else:
        return True
```

* 如果当前lock没有被acquire过，_is_owned应为false，逻辑上确实返回了false
* 如果当前lock被acquire过
  * 如果当前thread是owner，acquire方法返回1
  * 如果当前thread不是owner，acquire方法返回rc，应为None

这里逻辑好像反了

**原因似乎是因为这段代码不是用来实现Condition的`_is_owned`，Condition的`_is_owned`使用的是RLock的**

2022.07.06 update: 确实好像有点问题，这里涉及到的问题其实挺复杂的

[Condition._is_owned is wrong · Issue #64446 · python/cpython · GitHub](https://github.com/python/cpython/issues/64446)

其实之前我的推断是对的，因为官方文档里Condition可以传入一个Lock或一个RLock，对于RLock来说，它必须实现一个 `_is_owned` 函数，而对于普通lock，由于没有 `_is_owned` 函数，调用的才是Condition中的这个 `_is_owned` ，而对于普通的Lock，由于没法重入，所以没法判断当前到底哪个线程拥有这个锁。而在正常使用条件变量时，进入wait和notify前都需要获取锁，因此此时acquire应该是必然失败的，对于普通锁来说，基本证明了锁为当前线程持有，也只能做到这样的判断了。