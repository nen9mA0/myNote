# 概要

## 设计思想

mbuild设计思想有点类似于makefile，因为构建工程时生成的各个文件的依赖结构基本都是一个树形结构，因此可以根据树形依赖来决定编译的先后顺序，包括在重编译时也可以根据依赖关系实现部分编译

一个不同点就是makefile和其他类似的构建系统依赖关系主要还是靠用户指定（如若不指定`%.c:%.h`，改变.h文件不会导致.c文件需要重编译。当然有些时候隐含了这一规则）。mbuild主要针对的还是C和nasm工程，因此实现了一套自动化机制来解决头文件依赖，即可以自动解析包含的头文件，区分是系统头文件还是用户头文件（mbuild的规定是用户头文件全部使用`#include "xxx.h"`而不是尖括号），并自动生成依赖关系

mbuild实现了一些makefile所不具有的特性，这可能也是为什么intel不用其他构建方案而要自己重新实现一套的理由。因为在xed工程中有大量头文件是在构建时生成的，而这些头文件根据配置部分生成，C程序也是根据配置进行条件编译的。dag.py文件里的注释说明了一些设计思想

# util

## 关键函数

### run_command_unbufferred

使用subprocess.Popen执行一个指令，返回输出

### run_command_output_file

使用subprocess.Popen执行一个指令，返回到文件

# dfs

深度优先搜索模块，该模块主要功能是查找一个图是否存在回环

## 类

### node_t

代表图中的一个节点

* afters  后继节点列表
* befores  前驱节点列表
* color  节点颜色
  * 0  尚未访问过
  * 1  以及被访问过，但还没有遍历完子节点的节点
  * 2  遍历已经结束的节点
* predecessor  保存遍历到当前节点的前驱节点

## 函数

### dfs

传入一个node_t列表，以每个node_t为根，使用深度优先搜索寻找其对应的依赖树中是否存在环（如果有环，说明会造成循环依赖）。

方法很简单，每次遍历一个颜色为0的新节点时，先将其染为1，再递归遍历所有子节点。若该节点的所有子节点都被遍历完，则染为2。此时，若在递归遍历所有节点时发现下一个节点颜色为1，说明当前图存在一个环，因为这说明该图有一条回边指向自己的父节点

# plan

## 类

### plan_t

为dag_t创建一个记录型的类

```python
def __init__(self, command, args=None, env=None, input=None, output=None, name=None)
```

command可以是一个字符串或一个python函数，或一个可以包含字符串或python函数的列表

args env为运行环境变量和参数。input output为输入输出对应的文件名。name是为该plan起的一个名字（字符串）

#### 重要属性

##### command

指令，可以是字串、python函数或list

##### args

参数列表

##### env

环境变量

##### input

命令的输入文件

##### output

命令的输出文件



# dag

## 类

### _mbuild_dep_record_t

用于记录一个文件，表示其是一个依赖文件。并保存了文件的哈希值

#### 重要属性

##### creator

一个command_t，指向创建当前文件的指令

##### files_that_depend_on_this

以当前文件为依赖的文件列表

##### files_that_are_inputs

创建当前文件所使用的command_t的input文件列表。即当前文件依赖的文件列表

### dag_t

#### 重要属性

##### recs

记录与当前dag_t有关的所有_mbuild_dep_record_t结构

##### require_set

保存dag_t的所有输出文件

#### 重要内部函数

##### _make_command_object

传入plan_t创建对应command_t

##### _add_one_input

参数： input文件名  该input文件对应的command_t

_add_dagfood被调用，为dag_t加入一个input文件，并为该文件创建一个文件依赖

* 若当前文件名没有记录在recs列表中，则创建对应的_mbuild_dep_record_t结构，并加入dag_t的recs列表
* 为该input文件的`_mbuild_dep_record_t`结构添加一个依赖项（即files_that_depend_on_this属性），指向command_t的targets。即表示targets文件依赖于当前input文件

##### _add_one_output

参数： output文件名  该output文件对应的command_t

类似`_add_one_input`，但因为是输出文件所以有一些不同的处理

* 当前文件名没有记录在recs列表中，则创建对应的_mbuild_dep_record_t结构，并加入dag_t的recs列表
* 当前dag_t的require_set加入该项
* 修改该输出文件的`_mbuild_dep_record_t`的creator字段为传入的command_t，表示该文件是由该命令创建的
* 将传入的command_t的所有input文件加入`files_that_are_inputs`列表，表示该output文件依赖于这些文件

##### _scan_headers

扫描一个input文件的依赖

* 将input添加到扫描列表
* 调用scanner.mbuild_scan，返回所有input包含的头文件的mbuild_header_record_t类
* 

##### _add_dagfood

将一个plan_t加入当前dag_t

* 先转换为command_t（使用_convert_to_dagfood）
* 循环调用`_add_one_input`将所有plan_t的input加入dag_t
* 循环调用`_add_one_output`将所有plan_t的output加入dag_t
* 对所有的input调用_scan_headers，扫描其依赖的C或nasm头文件



#### 重要接口函数

##### add

将传入的任务合并到当前的dag中，传入的可以是dict或plan_t，若为dict则会经过一次转换为plan_t

加入的指令将根据plan_t提供的指令创建command_t（`_make_command_object`）。此后调用`_add_one_input`和`_add_one_output`将输入输出文件添加到dag中，并根据输入输出关系创建依赖



# work_queue

## 类

### dir_cmd_t

用于在一个目录下运行一条命令

#### 重要属性

##### dir

要运行的目录

##### command

要运行的命令

##### output_file

命令的输出文件

### command_t

执行多条命令，可以是shell指令或python函数

可以添加before_me和after_me，对象同样是command_t，来构造一个command链。但command_t的execute方法只执行当前command_t命令列表的指令

#### 重要属性

##### command

一个指令字串，一个python函数或一个dir_cmd_t

##### before_me

表示应该在当前命令执行前执行的命令列表

##### after_me

表示应该在当前命令执行后执行的命令列表

##### inputs

该command的输入文件

##### targets

该command的输出文件

#### 重要函数接口

##### execute

根据command的类型（字串、python函数、dir_cmd_t）以及其他配置（有无输入输出的文件等）运行指令

### work_queue_t

#### 工作队列

一共有三个队列，分别是

* new_queue  新任务队列
* out_queue  执行队列
* back_queue  执行后返回的队列

#### 线程池

有一个线程池`threads`

#### 线程池的初始化

在`__init__`中启动这个线程池，一共有max_parallelism个监控线程，调用`_start_daemons`，作用是创建多个以`_worker`为运行函数的线程

* `_worker`函数循环执行`_worker_one_task`直到其返回false
* `_worker_one_task`接收两个queue，作为输入和输出，对应为线程池的out_queue和back_queue。queue内都为command_t。`_worker_one_task`一次从取一个command_t执行，**若其terminator属性被设为true则返回**，否则直接执行该command_t，并将结果放入outgoing

#### 线程池的添加

* `_add_one`
  * 判断传入的command_t是否设置了completed（表示已经执行过）
    * 若设置了则递归调用add，判断command_t的后一条指令
  * 判断是否设置了submitted（表示已经提交进队列过）
    * 若设置了则直接返回
  * 设置submitted为True
  * 加入new_queue队列，job_num加1，pending加1

#### 线程池任务的分配

* `_start_a_job`  将接收的command_t结构放入out_queue，且将其加入running_commands。若该command_t没有设置terminator（即其不是用于暂停运行的），则加入all_commands。`sent+=1  running+=1`
* `_start_more_jobs`  将任务从new_queue复制到pending_commands，查看这些command的`_ready`，把准备好的压入ready列表。若线程池的running字段（指示正在运行的程序）小于max_parallelism，则从ready列表取一个command，调用`start_a_job`运行，并将其从pending_commands移除，pending减1

#### 线程池任务的暂停

* `_stop_daemons`  创建一个空command_t，**将其terminator设为true**，调用`start_a_job`。并循环max_parallelism次

#### 线程池任务的返回

* `_wait_for_jobs`  使用阻塞模式从back_queue取出一个command_t（阻塞超时为join_timeout）。running减1，finished加1，从running_command删除当前command_t，最后调用一次back_queue的task_done

#### 线程池的结束

* `_join_threads`  直接调用所有的守护线程的join方法，因此在join前必须调用`_stop_daemons`
* `_terminate`  调用`_stop_daemons`再调用`_join_threads`

## 接口

### add

添加一个列表的command_t或一个command_t，调用`_add_one`

### add_sequential

将一个list的指令实例化为command_t，并调用add加入new_queue。此外将其串成链表

### 任务创建

使用传入的dag图构建任务并放入线程池执行

# scanner

主要用于扫描目标文件及目录中的依赖关系。只根据`#include`和`%include`来扫描

## 类

### mbuild_header_record_t

记录头文件属性的结构

#### 重要属性

##### filename

记录文件名

##### system

是否为系统头文件

##### found

是否被依赖查找程序遍历过（在dag.py的`_scan_headers`被用到）

## 接口

### mbuild_scan

扫描某文件依赖的C或nasm头文件，指定一个列表的作为搜索路径。返回依赖关系的列表，列表元素类型为`mbuild_header_record_t`

# header_tag

若选项为-s，则将源文件视作C程序，调用`apply_header_to_source_file`将legal-header文件内容用`/*BEGIN_LEGAL`和`END_LEGAL */`注释后添加到source-file的文件开头（如果之前已经有这个开头则先移除再写入）

若选项为-t，则将源文件视作shell程序，调用`apply_header_to_data_file`，若能在source-file开头找到`!#`或者在第一第二行找到`-\*-`，则视为shell脚本，首先将有`!#`或者`-\*-`所在行写入，再将legal-header文件内容注释后（#）写入，最后将原文件内容写入。否则直接写入注释后的legal-header和原文件内容

# env

环境变量，各种配置，如编译器类型、平台类型、cpu位数等相关

## 类

### env_t

#### 重要内部函数

##### _iterative_substitute

接收一个字符串和一个字典dct1

将输入字符串中的所有形如`%(name)s`的内容替换为对应的字典中的内容。有点类似脚本语言中的变量替换

这里会进行递归查找，分为下列几种情况

* dct1[name]为字符串，直接替换
* dct1[name]为tuple，内容为(subkey, dct)，查找dct[subkey]内容并返回判断是否为字符串的地方
* dct1[name]为函数，则执行该函数，返回判断是否为字符串的地方（判断返回值是否为字符串）

#### 重要接口函数

##### expand_string

`_iterative_substitute`的包装函数，差别就是若传入list则循环执行`_iterative_substitute`