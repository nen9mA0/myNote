## util

#### 关键函数

##### run_command_unbufferred

使用subprocess.Popen执行一个指令，返回输出

##### run_command_output_file

使用subprocess.Popen执行一个指令，返回到文件

## dfs

深度优先搜索模块，该模块主要功能是查找一个图是否存在回环



## plan

### 类

#### plan_t

为dag_t创建一个记录型的类

```python
def __init__(self, command, args=None, env=None, input=None, output=None, name=None)
```

command可以是一个字符串或一个python函数，或一个可以包含字符串或python函数的列表

args env为运行环境变量和参数。input output为输入输出对应的描述符。name是为该plan起的一个名字（字符串）

## dag

### 类



## work_queue

### 类

#### dir_cmd_t

用于在一个目录下运行一条命令

#### command_t

执行多条命令，可以是shell指令或python函数

可以添加before_me和after_me，对象同样是command_t，来构造一个command链。但command_t的execute方法只执行当前command_t命令列表的指令

#### work_queue_t

##### 工作队列

一共有三个队列，分别是

* new_queue  新任务队列
* out_queue  执行队列
* back_queue  执行后返回的队列

##### 线程池

有一个线程池`threads`

###### 线程池的初始化

在`__init__`中启动这个线程池，一共有max_parallelism个监控线程，调用`_start_daemons`，作用是创建多个以`_worker`为运行函数的线程

* `_worker`函数循环执行`_worker_one_task`直到其返回false
* `_worker_one_task`接收两个queue，作为输入和输出，对应为线程池的out_queue和back_queue。queue内都为command_t。`_worker_one_task`一次从取一个command_t执行，**若其terminator属性被设为true则返回**，否则直接执行该command_t，并将结果放入outgoing

###### 线程池的添加

* `_add_one`
  * 判断传入的command_t是否设置了completed（表示已经执行过）
    * 若设置了则递归调用add，判断command_t的后一条指令
  * 判断是否设置了submitted（表示已经提交进队列过）
    * 若设置了则直接返回
  * 设置submitted为True
  * 加入new_queue队列，job_num加1，pending加1

###### 线程池任务的分配

* `_start_a_job`  将接收的command_t结构放入out_queue，且将其加入running_commands。若该command_t没有设置terminator（即其不是用于暂停运行的），则加入all_commands。`sent+=1  running+=1`
* `_start_more_jobs`  将任务从new_queue复制到pending_commands，查看这些command的`_ready`，把准备好的压入ready列表。若线程池的running字段（指示正在运行的程序）小于max_parallelism，则从ready列表取一个command，调用`start_a_job`运行，并将其从pending_commands移除，pending减1

###### 线程池任务的暂停

* `_stop_daemons`  创建一个空command_t，**将其terminator设为true**，调用`start_a_job`。并循环max_parallelism次

###### 线程池任务的返回

* `_wait_for_jobs`  使用阻塞模式从back_queue取出一个command_t（阻塞超时为join_timeout）。running减1，finished加1，从running_command删除当前command_t，最后调用一次back_queue的task_done

###### 线程池的结束

* `_join_threads`  直接调用所有的守护线程的join方法，因此在join前必须调用`_stop_daemons`
* `_terminate`  调用`_stop_daemons`再调用`_join_threads`

##### 接口

###### add

添加一个列表的command_t或一个command_t，调用`_add_one`

###### add_sequential

将一个list的指令实例化为command_t，并调用add加入new_queue。此外将其串成链表

##### 任务创建

使用传入的dag图构建任务并放入线程池执行

## scanner

主要用于扫描目标文件及目录中的依赖关系。只根据`#include`和`%include`来扫描

#### 接口

##### mbuild_scan

扫描某文件的依赖头文件，指定一个列表的作为搜索路径。返回依赖关系的列表，列表元素类型为`mbuild_header_record_t`

## header_tag

若选项为-s，则将源文件视作C程序，调用`apply_header_to_source_file`将legal-header文件内容用`/*BEGIN_LEGAL`和`END_LEGAL */`注释后添加到source-file的文件开头（如果之前已经有这个开头则先移除再写入）

若选项为-t，则将源文件视作shell程序，调用`apply_header_to_data_file`，若能在source-file开头找到`!#`或者在第一第二行找到`-\*-`，则视为shell脚本，首先将有`!#`或者`-\*-`所在行写入，再将legal-header文件内容注释后（#）写入，最后将原文件内容写入。否则直接写入注释后的legal-header和原文件内容
