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
* 否则调用_block的acquire尝试获取锁
  * 若获取到了`_owner`等于当前线程id，`_count`=1

##### release

释放锁

* 若owner与线程id不同，发生错误，因为一个线程不能在没有acquire的情况下release
* count--
* 若count=0，则release掉锁

##### `_acquire_restore`

由Condition使用，不判断线程id直接acquire，并且由参数传入owner和count

##### `_release_save`

由Condition使用，不判断线程id直接release，count=0 owner=None

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

阻塞一个线程

* 判断RLock所有者是否与线程一致，不一致则raise
* 新建一个lock（名字就叫waiter吧），添加进waiters列表
* 对这个lock进行acquire操作
* 保存`_lock`的值并释放`_lock`（使用`_acquire_restore`）
* 再次acquire waiter，使线程阻塞（因为这个锁被不是可重入锁并且被acquire过一次）
* 这里是acquire waiter返回后的处理，有两种情况，一是timeout返回，二是获取到了锁。无论哪种情况首先获取`_lock`，并且把先前保存的状态恢复（`_acquire_restore`）
* 如果没有获取到waiter则把waiter从列表中清除

##### notify

唤醒n个线程

* 判断RLock所有者是否与线程一致，不一致则raise
* 将_waiters列表的n个锁release，并且将其移出列表

##### wait_for

在timeout内，若`predicate()==True`则返回

```python
def wait_for(self, predicate, timeout=None)
```

### Semaphore

实现了信号量

#### 属性

* `_cond`  用于执行唤醒和等待操作的条件变量
* `_value`  信号量值

#### 方法

##### acquire

P操作，表示申请资源

* 若`_value==0`，则调用`_cond.wait`阻塞
* 否则`_value-=1`

##### release

V操作，表示释放资源

* `_cond.notify`
* `_value+=1`

这里有个有趣的写法。本来V操作应该判断value是否为0的，因为如果为0且有多个进程阻塞，直接+=1会导致资源数量与实际不符（唤醒进程后应该-=1）。而这里因为acquire使用了一个while else结构避免了这个问题

### BoundedSemaphore

加了个不允许当前value大于初始的value的特性

### Event

实现了事件的逻辑。当事件没有发生时，线程等待。当发生时唤醒这些线程

#### 属性

* `_cond`  用于执行唤醒和等待操作的条件变量
* `_flag`  表示当前事件是否发生

#### 方法

##### set

事件发生，唤醒所有等待的线程

* 获取条件变量
* `_flag = True`
* notify_all

##### wait

加入事件等待队列

* 获取条件变量
* 如果flag不为True，则`_cond.wait`

### Barrier

作用是同步n个线程，当第n个线程到达时同时唤醒这些线程

#### 属性

* `_cond`  条件变量
* `_state`  记录当前的状态，有如下四种
  * 1  draining  线程已唤醒状态
  * 0  filling  正在填充等待线程
  * -1  resetting  重置状态，需要等待还在线程等待池中的所有线程运行后才会置为filling
  * -2  broken  exception状态
* `_count`  当前等待的线程数
* `_partis`  记录要同步的线程数
* `_action`  同步线程唤醒前要执行的函数

#### 方法

##### _enter

wait调用的内部方法，用于在进入wait前等待其他状态运行完成

* 如果在reset或drain状态，`_cond.wait`等待执行完
* 执行完理应state==0，否则raise

##### _wait

wait调用的内部方法，等待state由0变为1

* `_cond.wait_for`  等待`_state`直到其由0（filling）变为1（draining）

##### _release

wait调用的内部方法，唤醒所有线程

* 执行`self._action()`
* `_state`  置一
* `_cond.notify_all`

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

### Thread

thread部分等看cpython的时候吧

#### 属性

* _target  线程要执行的函数
* _started  一个event class
* _args  _kwargs  参数
* 

#### 方法



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