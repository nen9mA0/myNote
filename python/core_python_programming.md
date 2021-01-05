## Chapter 4  python对象

### 内部类型

##### 代码对象

​	代码对象：可以被exec()或eval()内建函数执行

##### 帧对象

​	表示python的执行栈帧

​    **属性**：

 * 正在执行的代码对象
 * 本地及全局命名空间
 * 当前指令

##### 跟踪记录对象

​	用于异常处理

##### 切片对象

​	使用切片语法时（如用a[1:3:1]获取a中1~3的内容）

​	可以由内建函数slice()生成

##### 省略对象

​	Ellipsis用于切片语法中，表示省略号

### 标准类型操作符

#####  对象身份比较

```python
a is b
id(a) == id(b)
```

两者等价

##### 对象类型判断

```python
a = 1.0
print isinstance(a,float)		#判断a是不是float类型
```

**关于对象缓存**

当执行

```python
a = 1.0
b = 1.0
```

a is b == False

当执行

```python
a = 1
b = 1
```

a is b == True

但当执行

```python
a = 1000
b = 1000
```

a is b == False

因为小整型在程序中经常使用因此python会缓存

### 标准类型内建函数

**cmp(obj1,obj2)**	比较两个obj，如果是用户自定义对象会调用用户对象的\_\_cmp\_\_()

**repr(obj) / \`obj\`** 返回一个obj的字符串表示，**一般**可以用 a = eval(repr(obj)) 重新获得对象

**str(obj)**			返回一个obj可读性好的字符串表示 

**type(obj)**		返回对象类型

### 标准类型的分类

#### 更新模型

按照对象创建后能否进行更新分类

* 可变：	列表，字典
* 不可变： 数字，字符串，元组

不可变类型在重新赋值时会重新创建对象，即

```python
a = 1
print id(a)
a += 1
print id(b)
```

返回值不相同

#### 访问模型

* 直接访问		数字
* 顺序访问		字符串 列表 元组
* 映射访问		字典

## Chapter 5  数字

### 操作符

##### 自动类型转换

* 若一个操作数是复数则都转为复数
  * 若一个操作数是浮点则都转成浮点
    * 若一个操作数是长整型则全部转为长整型
* 否则退出

#### 内建函数与工厂函数

工厂函数 -> 内建函数都是类对象，调用时创建类实例

##### 数值工厂函数

* bool(obj)
* int(obj,base=x)
* long(obj,base=x)
* float(obj)
* complex(str)/complex(real,inag=x)

##### 功能函数

* abs()

* coerce()	自定义数值类型转换

  ```python
  coerce(1.3,134L)
  >>> (1.3,134.0)
  coerce(1j,134L)
  >>> (1j,134+0j)	
  ```

* divmod(a,b)	返回值 (a/b,a%b)

* round(obj,n)	返回值 n位的obj四舍五入值

##### 类型转换函数

* unichr(num)		将num转换为unicode
* bool(obj)		没有__nonzero__()方法的对象默认值为True

##### 十进制浮点型

不存在精度不够的问题

```python
from decimal import Deciaml
```

#### 数值计算常用库

* decimal
* array		高效数值数组
* math/cmath	C库运算函数
* operator	数字操作符的函数实现
* random
* numpy（第三方）
* scipy（第三方）

## Chapter 6  序列：字符串、列表和元组

### 字符串

#### 字符串模板

```python
from string import Template
s = Template('There are ${howmany} ${lang} Quotation Symbols')
#方法1
s.substitute(lang='Python',howmany=3)
#方法2
s.safe_substitute(lang='Python',howmany=3)
#区别：
#方法1当调用如下时报错
s.substitute(lang='Python')
#方法2则简单地将lang置换为None
```

#### 原始字符串

字符串前加r表示原始字符串，即不转义的字符串，如

```\
print '\n'
#输出一个换行
print r'\n'
#输出 \n
```

#### unicode字符串

字符串前加u

**不能通过str()，chr()等string类的函数对unicode进行处理，而需要用unicode(),unichar()等**

**unicode字串的格式化打印**：使用unicode类型的格式化字符串会使参数先转化为unicode

#### 内建函数

* cmp

* len

* max和min  返回字符串中ascii最大的字符和最小的字符

* enumrate

  ```python
  s = 'foobar'
  for i in enumerate(s):
      print i
  
  >>> (0,'f')
  	(1,'o')
      (2,'o')
      (3,'b')
      (4,'a')
      (5,'r')
  ```

* zip

  ```python
  s,t = 'foa','obr'
  zip(s,t)
  
  >>> [('f','o'),('o','b'),('a','r')]
  ```

#### 字符串类型函数

* str()
* unicode()
* chr()
* unichr()
* ord()   可以与chr和unichr配对

#### 常用字符串内建函数

* .count(str,beg=,end=)			返回str在字符串beg~end间出现的次数
* .decode(encoding=,errors=)
* .encode(encoding=,errors=)
* .endswith(obj,beg=,end=)		检查字符串是否以obj结尾
* .startswith(obj,beg=,end=)
* .expandtabs(tabsize=)			把字符串中的tab转成空格
* .find(str,beg=,end=)
* .rfind(str,beg=,end=)
* .index(str,beg=,end=)			与find相同但若没找到会报异常
* .rindex(str,beg=,end=)
* .join(seq)					以string为分隔符连接所有seq的元素
* .ljust(width)					左对齐并用空格填充至width
* .lower()
* .upper()
* .lstrip()						去除左边空格
* .rstrip()
* .strip()
* .partition(str)				从str出现的第一个位置起，把string分为(str_pre,str,str_post)
* .rpatition(str)
* .replace(str1,str2,num=)
* .split(sep="",maxsplit=)

#### 三引号

```python
hi = '''hi
there'''
print hi

>>> 'hi\nthere'
```

### 列表

#### 列表删除

* del( mylist[n] )	删除下标为n的元素
* remove( obj )		删除内容为obj的元素
* pop( n )			删除并返回下标为n的元素

#### 列表关系操作
* in | not in
* +/+=/.extend(list2)	+会导致新列表的创建，+=和.extend()直接在原列表上修改

#### 序列类型函数
* sorted()
* reversed()
* enumerate()
* zip()			行为都与字符串中类似
* list()
* tuple()		两个函数一般用于列表和元组间的转换

#### 列表内建函数
* .append(obj)
* .count(obj)			返回obj在list中出现次数
* .extend(seq)			连接
* .index(obj,i=,j=)		返回obj在list中的下标
* .insert(index,obj)	在index的位置插入对象obj
* .pop(index=)
* .remove()
* .reverse()
* .sort(func=,key=,reverse=)	如果func指定以func方式比较元素，reverse=True反序排列
* .extend()

### 元组
##### 元组是不可变类型

## Chapter 7 映像和集合类型

### 字典

#### 创建

```python
dict1 = { 'name1':'1','name2':'2' }	#直接声明
dict2 = {}.fromkeys(('x','y'),1)	#这种方法只接受一种键值，因此所有键对应的值一样

dict3 = dict( (['name1',1],['y',2]) ) #工厂方法

dict4 = dict( zip( ('x','y'),(1,2) ) )
dict5 = dict( ['xy'[i-1],i] for i in range(1,3) )
dict6 = dict(x=1,y=2)
 >>> {'x':1,'y':2}						#以上构造相同的字典
```

#### 读取与测试

```python
for key in dict1:	#遍历键
    ...
    
if 'name1' in dict1:	#测试是否有该键
    ...
```

#### 添加/更新与删除

```python
dict1['name2'] = '1'		#更新/添加

dict1.pop('name2')
del dict1['name2']			#删除条目

del dict1
dict1.clear()				#删除全部
```

#### 函数

* dict()
* len()		返回键值对数目
* hash(obj)	返回obj的哈希值，可用于判断该对象可否作为键

#### 内建方法

* .clear()
* .copy(dict)
* .fromkeys(seq,val=None)
* .get(key)		若存在该键返回value，否则None
* .has_key(key)	已使用in代替
* .items()		返回键值对列表
* .keys()		返回键列表
* .iteritems()
* .iterkeys()
* .itervalues()	返回迭代子
* .pop(key)		删除并返回dict[key]
* .setdefault(key,default=None)	若不存在key则用default给key赋值
* .update(dict)	将dict的键值对添加到当前字典
* .values()		返回包含字典中所有值的列表

#### 键必须可哈希

* 不可变类型，且值相等的数字表示相同键，如1和1.0相同
* 实现了\_\_hash()\_\_方法的类

#### 打印字典元素的一个技巧

```python
dict = {'name':'Me','port':80}
print "hostname: %(name)s  port: %(port)d" %dict
```

### 集合

#### 创建

```python
s = set('cheeseshop')			#创建可变集合
>>> set(['c', 'e', 'h', 'o', 'p', 's'])
t = frozenset('bookshop')		#创建不可变集合
>>> frozenset(['b', 'h', 'k', 'o', 'p', 's'])
```

#### 更新

```python
s.add('z')				#加入元素'z'
s.update('pypi')		#将集合set('p','y','p','i')加入s

s.remove('z')			#移除元素'z'
s -= set('pypi')
```

#### 集合运算

* in/not in		属于/不属于
* ==/!=			等价/不等价
* <,<=/>,>=		真子集/子集
* |/.union()			并
* &/.intersection()		交
* -/.difference()		差
* ^/.symmetric_difference()	对称差分，即仅包含只属于A或只属于B的成员，即A|B-A&B

可用运算符 |= &= ^= -=

#### 集合类型

* 可变  set()
* 不可变 frozenset()

当两集合运算时，若左操作数与右操作数类型不一致，生成的集合为**左操作数的类型**

#### 方法

##### 可变不可变均适用

* .copy(t)		浅复制，速度比set等快
* .issubset(t)
* .issuperset(t)
* .union(t)
* .intersection(t)
* .difference(t)
* .symmetric_difference(t)

##### 仅可变有

* .update(t)
* .intersection_update(t)
* .difference_update(t)
* .symmetric_difference_update(t)
* .add(obj)
* .remove(obj)	若无obj则引发KeyError
* .discard(obj)	若obj是s中的元素则删除
* .pop()
* .clear()

所有对象参数应是可哈希的

## Chapter 8 条件和循环

#### for的三种常见迭代方法

```python
for iter_var in iterable:
    ...							#优点：快  缺点：无法获得下标

for i in xrange(len(iterable)):
    ...							#优缺点反过来
    
for i,iter_var in enumerate(iterable):
    ...							#要获取两者的最优方案
```

#### 迭代器

##### 基本原理

每个迭代器都内置一个**next()方法**用于迭代，当迭代完成时会产生一个**StopIteration**异常

```python
mytuple = ('1','2','3')
i = iter(mytuple)
i.next()	#'1'
i.next()	#'2'
i.next()	#'3'
i.next()	#StopIteration
```

字典迭代时相当于迭代.key()序列（纯迭代器有.iterkeys(),.itervalues()等）

文件迭代时每次调用readline()

##### 实现

* iter(obj)    若obj是序列则直接根据索引迭代，若不是则检查**\_\_iter()\_\_**和**next()**方法
* iter(func,sentinel)  重复调用func直到下个值是sentinel

##### 常用迭代器

* enumerate()
* reversed()

##### any()和all()

* any(iter)		当迭代器中全部为False返回False，仅列表为空、0、None返回False
* all(iter)		当迭代器中全部为True返回True

#### 列表解析

```python
#===求0~5的平方===
#列表解析
[x**2 for x in range(6)]
#lambda
map(lambda x:x**2,range(6))

#===筛选奇数===
#列表解析
[x for x in seq if x%2]
#lambda
filter(lambda x:x%2,seq)

#===生成3*5矩阵===
[(i,j) for i in range(3) for j in range(5)]
```

## Chapter 9  文件输入和输出

#### 打开

```python
f = open(file_name,access_mode=,buffering=)
f = file(file_name,access_mode=,buffering=)
```

描述符

* r/w/a  只读/只写（清空原文件）/追加（不清空）
* \+     读写模式
* b     二进制模式，注意二进制模式和非二进制在unix下一样

上面三种选项可以结合

一些需要注意的选项

* rU/U   读方式打开，且提供通用换行符支持
* r+    读写方式，文件指针在头部，不清空
* w+   读写方式，清空原文件
* a+    读写方式，文件指针在尾部，不清空

#### 内建方法

##### 输入

注意：输入函数不会删除换行符

返回列表

* .read()
* .readline()
* .readlines()

返回迭代器

* .xreadlines()    不常用
* iter(file)
* for line in f:    文件对象本身可作为迭代器

##### 输出

注意：输出函数不会添加换行符

* .write()
* .writelines()

##### 关闭

```python
f.close()
```

##### 其他方法

* .fileno()  返回文件描述符
* .flush()    刷新文件缓冲区（即将缓冲区的内容马上写入）
* .isatty()    文件是否是类tty设备
* .next()    文件的下一行
* .seek(offset,whence=)    移动文件指针
* .tell()    返回文件指针位置
* .truncate(size)    截取文件头到size字节写回文件

#### 内建属性

* .closed   文件已被关闭->True
* .encoding
* .mode
* .name
* .newlines  返回文件中出现的所有行结束符列表
* .softspace  0表示输出一个数据后加上1个空格符

#### 标准输入输出

* sys.stdin
* sys.stdout
* sys.stderr

#### 命令行参数

* sys.argv  列表

使用**getopt**或**optparse**获取命令行选项

#### 文件系统

os模块

##### 文件处理

* .mkfifo()/mknod()    创建命名管道/文件系统节点
* .remove()/unlink()    删除文件
* .rename()/renames()    renames可以重命名目录，具体行为先创建新目录，将旧目录文件移动，如果旧目录为空则删除
* .stat()   返回文件信息
* .symlink()  创建符号链接
* .utime()  更新时间戳
* .tmpfile()  创建并打开一个新的临时文件
* .walk()  生成目录树下所有文件名

##### 目录/文件夹

* chdir()/fchdir()  改变工作目录
* chroot()  改变根目录
* listdir()
* getcwd()/getcwdu()  返回当前工作目录（unicode）
* mkdir()/makedirs()   创建目录/多层目录
* rmdir()/removedirs()

##### 访问/权限

* .access()  检验权限
* .chmod()
* .chown()/.lchown()
* .umask()  设置默认权限

##### 文件描述符操作

* .dup()/dup2()  复制文件描述符
* .open()
* .read()/write()

##### 设备号

* .makedev()
* .major()/minor()

##### 路径名访问

os.path

分隔

* .basename()  仅返回文件名
* .dirname()
* .expanduser()  提供对~目录的支持
* .join()  将各部分组合成文件名
* .split()  ->(dirname() , basename())
* .splitdrive() ->( drivename,pathname )
* .splitext()  -> (filename,extension)

信息

* .getattime()  最近访问时间
* .getctime()  创建时间
* .getmtime()  修改时间
* .getsize()

查询

* .exist()
* .isabs()  是否为绝对路径
* .isdir()
* .isfile()
* .islinke()
* .ismount()  是否是挂载点
* .samefile()  是否指向同一文件

#### 其他模块

* pickle/marshal  用于转储和还原python对象，但marshal只能处理简单的python对象
* dbhash/bsddb/dbm/gdb,/dumbdbm  数据库模块
* shelve  将dbm和序列化模块结合

## Chapter 10  错误和异常

#### 异常类

* NameError
* ZeroDivisionError
* SyntaxError
* IndexError
* KeyError  不存在的字典关键字
* IOError
* AttributeError  不存在属性
* ValueError
* TypeError

#### try-except

捕获多个异常

```python
try:
    try_suite
except Exception1[, reason1]:
    except_suite_1
except Exception2[, reason2]:
    except_suite_2
except (Exception1,Exception2)[, reason3]:
    except_suite_3
```

捕获除**KeyBoardInterrupt和SystemExit**异常

```python
try:
    try_suite
except Exception,e:
    except_suite
```

捕获所有异常

```python
try:
    try_suite
except BaseException,e:
    except_suite
```

因为 **KeyBoardInterrupt SystemExit Exception**都是**BaseException**的派生类

e是错误的一个实例

#### else子句

```python
try:
	try_suite
except Exception,e:
	except_suite
else:
	else_suite
```

try块若完全执行完则跳到else

#### finally子句

```python
try:
    try_suite
except Exception,e:
    except_suite
else:
    else_suite
finally:
    finally_suite
```

不管是否有异常，都会到finally执行，即程序执行流

* try_suite -> else_suite -> finally_suite
* try_suite -> except_suite -> finally_suite

实例：

```python
try:
    try:
    	cfile = open(filename,"w")
    	lines = cfile.readlines()
	except IOError:
        except_suite
finally:
    cfile.close()
    
```

##### 注意：如果finally中的代码引发另一个异常或终止，原来的异常将丢失

#### with

##### 用法

```python
with open(filename,'r') as f:	#在退出with时会自动执行关闭等操作
    for line in f:
        ...
```

##### 上下文管理

使用with语句，需要类对with做适配

* with和as间的内容用于获取一个上下文对象，即调用**\_\_context\_\_()**方法

* 获取上下文后调用**\_\_enter()\_\_**，若**as**后有变量则将**\_\_enter()\_\_**返回值赋给变量，否则丢弃
* 退出with或发生异常时调用**\_\_exit()\_\_**
* 常用库：contextlib

#### 触发异常

##### raise

```python
raise [SomeException [, args [, traceback]]]

raise  						#触发最近一个异常，若无触发TypeError
raise exclass  				#触发一个异常，从exclass生成一个实例
raise exclass, args			#加了参数
raise exclass, args, tb		#提供一个traceback对象

raise exclass()
raise exclass(args)			#通过函数调用符作用于类名生成exclass实例

raise exclass, instance		#通过实例触发异常，若实例instance是exclass子类，异常类型是子类类型
							#否则使用instance作为参数生成exclass实例
raise instance				#同 raise instance.__class__, instance
```

#### 断言

```python
try:
	assert expression[, args]
except AssertionError, args:
    ...
```

#### 自定义异常

可以以一个异常类为派生类，通过**raise**语句触发

## Chapter 11 函数和函数式编程

#### def语句

```python
def function_name(arguments):
    "document"				#文档字段
    function_body
	return value
```

##### 内部函数

```python
def foo():
    def bar():
        print "bar"
    bar()
```

#### 装饰器

基本语法

```python
def funA(func):
    ...
    
@funA
def funB():
    ...
```

@funA的作用相当于 **funB = funA(funB)**，因此funA必须有一个参数func用于传入funB，**且装饰器的返回值将作为funB()调用的函数**

##### 最简单形式

用于包装函数，在函数前后插入一段通用代码

```python
def funA(func):			#定义了一个装饰器
    print "111"
    def wrap():			#装饰器的包装函数
        print "funA"	#显示funA
        return func()	#显示完返回原来要调用的函数
    return wrap			#进入funA后返回了wrap指针，因此程序调用foo()实际上调用了wrap()
						
    
@funA					#相当于funA(foo),这里funA在执行完不会释放,因为原来的foo指向了
def foo():				#wrap函数
    print "foo"
    
foo()					#调用foo函数，实际调用了wrap()

>>>111
funA
foo
```

经过tsfunc包装后的foo每次执行前都会打印一个时间戳


##### 嵌套装饰

```python
@deco2
@deco1
def foo()：
	...
```

结果为 **deco2( deco1(foo) )**

##### 正确传递参数

若被装饰函数有参数，则应通过装饰器正确传递这些参数，可以用可变长度参数传参方式实现

```python
def funA(func):
    print "111"
    def wrap(*args,**kargs):		#可变长度参数 关键字和非关键字的都有
        print "funA"
        return func(*args,**kargs)
    return wrap

@funA		
def foo(a,b):
    print "foo"
    print a,b
    
foo("yeah","abc")

>>>111
funA
foo
yeah abc
```

##### 使用()调用装饰器

这里没有直接的用处，主要用于讲解下一个用途

```python
def funA():
    print "111"
    def wrap(func):
        print "funA"
        return func()
    return wrap

@funA()			
def foo():
    print "foo"
    
>>>111
funA
foo
```

两例间明显的区别就是这里没有显式调用foo()，且func参数被写到了wrap()

实际上调用顺序是这样

```python
#funA() ->返回值wrap,因此实际程序为
@wrap
def foo():
	print "foo"
```

##### 使用参数指定装饰函数

可以通过给装饰函数传参实现对装饰器的选择，但要注意因为上例，如果在装饰器上进行形如下面的调用

```python
@funA("wrapper1")
```

实际上是将fun("wrapper1")的返回值作为@操作数

```python
def funA(selection):					#这层用于选择返回的装饰函数
    def wrap1(func):					#这层用于装饰
        def wrap1_func(*args,**kargs):	#这是最后foo()调用的真实函数
            print "This is wrapper 1"
            return func(*args,**kargs)
        return wrap1_func

    def wrap2(func):
        def wrap2_func(*args,**kargs):
            print "This is wrapper 1"
            return func(*args,**kargs)
        return wrap2_func
    
    try:
        return {"select1":wrap1,"select2":wrap2}[selection]
    except:
        print "must be select1 or select2"
        return None
     
       
@funA("select1")			
def foo(a,b):
    print "foo"
    print a,b
    
foo("yeah","abc")

>>>This is wrapper 1
foo
yeah abc
```

#### 函数的引用

```python
def foo():
    pass

bar = foo	#传递foo函数对象的引用给bar
bar()		#这里调用foo函数
```

同样函数可作为参数传递

```python
def convert(input,func):
    a = func(input)
    print a
    print type(a)
    
convert("123",int)
convert("1.23",float)

#输出
>>> 123
<type 'int'>
1.23
<type 'float'>
```

#### 可变长度的参数

##### 非关键字（元组）

```python
def foo(*args):
    print args

foo(1,2,3)
b = (1,2,3)
foo(b,[],3)

>>> (1,2,3)
>>> ((1,2,3),[],3)
```

##### 关键字（字典）

```python
def foo(**args):
    for i in args.key():
        print "%s:%d" %(i,args[i])
foo(a=1,b=2,c=3)

>>> a:1
c:3
b:2
```

**同一个函数中出现非关键字和关键字参数的话需要吧关键字参数声明放在非关键字后**

##### 自行构造传入的可变长参数

```python
def test(arg1,*arg_tuple,**arg_dict):
    print "arg1:%s" %arg1
    for i in arg_tuple:
        print "tuple:%s" %i
    for i in arg_dict.keys():
        print i,arg_dict[i]
        
test(1, *(1,2,3), **{'dict1':1,'dict2':2} ) #自行构造的参数需要加* 或**
tuple1 = (1,2)
dict1 = {'a':1}
print "===================="
test(1,2,*tuple1,**dict1)					#这里的2没有对应参数，但是最终被加入到arg_tuple中
											#因此自行构造的tuple1实际只是arg_tuple的子集
```

#### 匿名函数

##### lamda

```python
#命名函数：
def add(x,y):
    return x+y
#匿名函数
lambda x,y:x+y
#可以加入默认参数
lambda x,y=2:x+y
```

##### 内建函数

* filter(func,seq)   对seq每个元素调用func，返回使func为True的元素列表

  ```python
  #筛选偶数
  def odd(x): return x%2==0
  num = [1,2,3,4,5,6,7]
  print filter(odd,num)
  
  >>> [2, 4, 6]
  
  #使用lambda
  print filter(lambda x:x%2==0, num)
  #使用列表解析
  print [x for x in num if x%2 ==0]
  ```

* map(func,seq)  对seq每个元素调用func

  若seq有多项，则会将seq0[n],seq1[n]...归并为返回值的第n项

  ```python
  map(lambda x:x,[1,2,3],[4,5,6])
  >>> [(1,4),(2,5),(3,6)]
  ```

* reduce(func,seq)  递归调用

  ```python
  # reduce(func,[1,2,3]) = func(func(1,2),3)
  reduce(lambda x,y:x+y,[1,2,3])
  >>> 6
  ```

#### 偏函数应用

用于封装常用调用

```python
#封装二进制字符串转换调用
from functools import partial
baseTwo = partial(int,base=2)
baseTwo("1111")
>>> 15
```

注意：若没指定参数名默认从左边开始传入

#### 变量作用域

局部变量与全局变量概念与其他语言相同

##### global

```python
a = 100
print id(a)

def foo():
    a = 10
    print id(a)
    global a		#这里把局部变量a global了，将覆盖全局变量a
    print a
    
foo()
print a
print id(a)

>>> 52066528		#这是全局变量a的id
52064704			#这是局部变量a的id
10
10
52064704			#global语句后全局变量a指向局部变量a，因此仅执行了浅拷贝
```

#### 闭包

在一个内部函数里，对在外部作用域（不在全局作用域）的变量进行引用，那么内部函数被认为是闭包

概念有点类似偏函数，可以用类代替但较为小巧

```python
def counter(start=0):
    count = [start]			#初始化计数器
    def inc():
        count[0] += 1
        return count[0]
    return inc				#返回inc函数对象

new_count = counter(100)	#创建一个100开始计数的对象，并将对应的inc函数回传
print new_count()
>>> 101
```

#### 生成器

生成器是类似迭代器的东西

简单例子：

```python
def simpleGen():
    yield 1
    yield 2
```

此时调用**.next()**方法获得下一个元素

```python
myG = simpleGen()
myG.next()
>>> 1
myG.next()
>>>2
myG.next()
抛出StopIteration异常
```

类似迭代器，在最后会抛出**StopIteration**异常

##### yield关键字

yield关键字有点类似于一个return，但yield返回一个值后下次进入函数时从yield下一句开始执行

```python
def simgen():
    yield 1
    yield 2
    print "yield 2"
    yield 3

for i in simgen():
    print i
```

返回值为 1 2 yield 2 3

##### .next()

与迭代器类似，生成器可以用.next()获得下一个值

##### StopIteration

与迭代器类似，生成器运行到结尾（生成器的函数结尾）抛出StopIteration异常

##### .send()与.close()

send()用于发送一个值给生成器，close()用于退出生成器

**yield返回值**用于接收send()的数

```python
def simgen(start_at = 0):
    i = start_at
    while True:
        num = yield i
        if num:			#如果接收到send数
            i += num
        else:
            i += 1

myG = simgen()
for i in xrange(10):
    print myG.next(),
for i in xrange(5):
    myG.send(i)
    print myG.next(),
```

返回值：0 1 2 3 4 5 6 7 8 9 11 13 16 20 25

## Chapter 12 模块

#### 模块和文件

##### 路径搜索

默认搜索路径

* PYTHONPATH环境变量

可以用sys.path获得，用列表的append()/insert()方法修改，且仅对当前进程有效

#### 导入模块

##### import 与 from import

* import 用于直接导入一个库，当运行时会在库的命名空间中查询对应函数

* from import可以直接指定函数，免除搜索过程

* as可以提供别名

  ```python
  from A import B as name
  import A as name
  ```

##### 导入时将执行模块的顶层代码

##### 导入与命名空间

使用**import**指令导入的模块有独立的名称空间

使用**from import**指令将使被导入的名称空间被拷贝到当前名称空间

注意：下列代码

```python
#文件b.py
foo = 'b'
def bprint():
    print foo,
    
#文件a.py
foo = 'a'
print foo,
from b import *
print foo,
foo = 'c'
bprint()
```

输出为 a b b

说明语句foo = 'c'只改变了文件a名称空间的foo值

#### 模块内建函数

##### \_\_import\_\_()

import语句实际上调用\_\_import\_\_()

```python
__import__( [module_name[, globals[, locals[, fromlist]]] )
```

* module_name 要导入的模块名
* globals 调用者全局变量
* locals  调用者局部变量
* fromlist  使用from import导入时的模块

##### globals()

返回全局名称字典

##### locals()

返回局部名称字典

##### reload()

重新导入

#### 包

用于导入有多个子文件夹的模块

##### 导入多个子文件夹下的模块

假设文件结构

```
a
|_a-1.py
|_a-2
    |_b.py
```

要调用b的c函数，可以使用

```
import a as tmp		调用tmp.a-2.b.c()
import a.b as tmp	调用tmp.b.c()
from a import a-2	调用b.c()
from a.a-2 import b 调用c()

import a
from a import a-2.b 调用c()		#相对导入
```

##### from x import *

若在\_\_init\_\_.py下有\_\_all\_\_变量，则导入该变量指定的模块

#### 模块的其他特性

##### 读取导入模块

```
sys.modules.keys()
```

##### 源文件编码和解释器

```
#!/usr/bin/env python
# -*- coding: UTF-8 -*-

或
#coding:UTF-8
```

## Chapter 13 面向对象编程

#### 类

##### 创建

```python
class classname(base_class):
    "class documentation string"
    class_suite
```

#### 属性

##### 类的数据属性

类似于C++的静态变量，即只与类有关与实例无关的变量

```python
class C(object):
    foo = 100
    
C.foo = C.foo + 1
#输出
>>> C.foo
101

#创建一个C的实例a
a = C()
#输出
>>> a.foo
101
```

##### 方法

```python
class C(object):
    def foo(self):
        print "foo"
    
a = C()
a.foo()
```

##### 查看类的属性

* dir(ClassName)
* ClassName.\_\_dict\_\_

##### 特殊属性

* .\_\_name\_\_  类的名字
* .\_\_doc\_\_  类的文档字符串
* .\_\_bases\_\_   所有的父类构成的元组
* .\_\_module\_\_   类C定义所在的模块
* .\_\_class\_\_   实例x对应的类

#### 实例

##### 构造器

\_\_init\_\_()和\_\_new\_\_()

```python
class C(object):
    def __init():
        initialize
```

**注意** \_\_init\_\_()应该返回None

##### 解构器

\_\_del\_\_()

只有当引用计数为0时才会调用

#### 实例属性

##### 打印实例属性

```python
c = C()
dir(c)
```

##### 特殊的实例属性

* a.\_\_class\_\_  实例化a的类
* a.\_\_dict\_\_  a的属性

##### 内建类型属性

可以用dir()打印内建类型属性，但不存在.\_\_dict\_\_属性

##### 实例属性与类属性

对于不可变对象

```python
class C(object):
    x = 1.5
    
c = C()
c.x
>>> 1.5		#这时访问x属性返回值还是1.5

c.x = 1.7
c.x
>>> 1.7		#修改后为1.7

del c.x
c.x
>>> 1.5		#删除该属性后又为1.5

del c.x		#此时执行该语句报错
del C.x		#但如果删除类的属性则可以

c.x			#此时再打印x属性将报错
```

其实这里可以通过一个简单例子解释

```python
class C(object):
    x = 1.5
    
c = C()
C.__dict__
>>> {'x': 1.5, '__module__': '__main__', '__doc__': None}	#这是类C的__dict__
c.__dict__
>>> {}			#这是实例c的__dict__

c.x = 1.7		#相当于实例c创建了一个x属性并覆盖类的定义
c.__dict__
>>> {'x': 1.7}
```

对于可变对象则不一样

```python
class C(onject):
    x = [1,1]
    
c = C()
c.x.append(2)

c.x				#实例c的属性
>>> [1, 1, 2]
c.__dict__
>>> {}

C.x				#对实例C的属性改变影响到类
>>> [1, 1, 2]
```

#### 绑定与方法调用

因为类在调用自己方法的时候总是要传入实例，即self，这样类的方法就与实例绑定

##### 调用绑定方法

实例.方法名()

##### 调用非绑定方法

当在派生一个子类，且要覆盖父类的方法，需要调用父类中想要覆盖的方法时

```python
class A(B):						#类A是B的子类
    def __init__(self,arg1,arg2):  #类A的__init__函数，将覆盖类B的__init__
        B.__init__(self,arg1,arg2)	#显式调用非绑定方法
        A.x = n
        A.y = n
```

#### 静态方法与类方法

```python
class C(object):
    @staticmethod       	#声明为静态方法
    def a():				#静态方法不需要传入self
        ...
        
class D(object):
    @classmethod			#声明为类方法
    def b(cls):				#传入的参数为类
        ...
```

与普通方法不同的是，静态方法与类方法既可通过实例调用，也可通过类调用

```python
myclass_1 = C()
myclass_1.a()				#通过实例调用

C.a()						#通过类调用
```

#### 子类和派生

##### 创建

```python
class C(parentclass1,parentclass2...):		#由parentclass1和parentclass2派生的子类
    class_suite
    
class D:
    class_suite								#没有父类的类
```

#### 继承

##### 类属性

* .\_\_bases\_\_  元组：列出**上一级**父类
* .\_\_class\_\_   获取自己所在的类

##### 覆盖方法

```python
class A():
    def foo(self):
        pass
    def test(self):
    	pass
    
class B(A):
    def foo(self):
        pass
    
b = B()
b.foo()				#这里将调用B的foo
b.test()			#将调用A.test()

A.foo(b)			#通过传实例显式调用A.foo()
```

##### 调用基类的\_\_init\_\_()

同样，当一个类定义了自己的\_\_init\_\_()，**基类的\_\_init\_\_()将被覆盖**

这时通常的做法是在子类的\_\_init\_\_()中调用基类的

```python
class A():
    def __init__(self):
        init_A
        
class B(A):
    def __init__(self):
        A.__init__(self)
        init_B
```

这样写有个缺点是如果基类改变，则要重写代码，因此可以用**super()**函数

```python
class B(A):
    def __init__(self):
        super(B,self).__init__()
        init_B
```

super()可以用来获取类的基类对象

##### 多重继承与MRO

Method Resolution Order，即在多重继承中对于同名方法覆盖问题的查找方式，python总是以找到的第一个函数为调用函数

**查找方式**

* **深度优先搜索**  以前的算法，缺点是假设A的基类是A1和A2，A1的基类是B，若搜索先从A1开始，A1搜不到将搜索A1的子类B，这样即使A2关系与A更为亲近也不会被优先搜索
* **广度优先搜索**  现在的算法
* \_\_mro\_\_属性    保存了搜索的路径信息

#### 其他对象内建函数

* issubclass(sub,sup)   判断sub是不是sup的子类
* isinstance(obj1,class)  判断obj1是不是class或其子类的实例
* attr系列  用于操作类的属性
  * hasattr(class,attr)
  * getattr(class,attr)
  * setattr(class,attr,obj)
  * delattr(class,attr)
* vars(obj)  返回obj.\_\_dict\_\_的值

#### \*类的特殊方法

注意：带*的函数可能有多个版本，比如\*add有add（表示self+obj）,radd（obj+self）和iadd（self=self+obj）三个版本

##### 基础函数

* .\_\_init\_\_(self)
* .\_\_new\_\_(self)  通常用在不可变类的子类的初始化
* .\_\_del\_\_(self)
* .\_\_str\_\_(self)    用于str()和print语句打印类
* .\_\_repr\_\_(self)  用于repr()和``的输出
* .\_\_unicode\_\_(self)   用于unicode()
* .\_\_call\_\_(self,args)  表示可调用的实例
* .\_\_nonzero\_\_(self)  为obj定义False的值，用于bool()
* .\_\_len\_\_(self)  用于len()

##### 对象比较

* .\_\_cmp(self,obj)  用于cmp()
* .\_\_lt\_\_(self,obj)  .\_\_le\_\_(self,obj)  重载<和<=
* .\_\_gt\_\_(self,obj) .\_\_ge\_\_(self,obj)  重载>和>=
* .\_\_eq\_\_(self,obj) .\_\_ne\_\_(self,obj)  重载==和!=(<>)

##### 属性

* .\_\_getattr\_\_(self,attr)  用于getattr()
* .\_\_setattr\_\_(self,attr)
* .\_\_delattr\_\_(self,attr)
* .\_\_getattribute\_\_(self,attr)  以下几个函数用于新式类
* .\_\_get\_\_(self,attr)   以下三个用于使用描述符操作时调用
* .\_\_set\_\_(self,attr,val)
* .\_\_delete\_\_(self,attr)

##### 二元操作符

以下用于重载二元运算符

* .\_\_*add\_\_()  
* .\_\_*sub\_\_()
* .\_\_*mul\_\_()
* .\_\_*div\_\_()
* .\_\_*truediv\_\_()
* .\_\_*floordiv\_\_()
* .\_\_*mod\_\_()
* .\_\_*divmod\_\_()
* .\_\_*pow\_\_()

##### 二进制操作符

* .\_\_*lshift\_\_()
* .\_\_*rshift\_\_()
* .\_\_*and\_\_()
* .\_\_*or\_\_()
* .\_\_*xor\_\_()

##### 一元运算符

* .\_\_neg\_\_()
* .\_\_pos\_\_()
* .\_\_abs\_\_()
* .\_\_invert\_\_()

##### 数值转换

* .\_\_complex\_\_()
* .\_\_int\_\_()
* .\_\_long\_\_()
* .\_\_float\_\_()
* .\_\_oct\_\_()
* .\_\_hex\_\_()
* .\_\_coerce\_\_()
* .\_\_index\_\_()

##### 序列类型

* .\_\_len\_\_()
* .\_\_getitem\_\_()
* .\_\_setitem\_\_()
* .\_\_delitem\_\_()
* .\_\_getslice\_\_(self,ind1,ind2)    获得序列片段
* .\_\_setslice\_\_()
* .\_\_delslice\_\_()
* .\_\_contains\_\_()   内建in关键字
* .\_\_*add\_\_()
* .\_\_*mul\_\_()
* .\_\_iter\_\_()

##### 映射类型

* .\_\_len\_\_()
* .\_\_hash\_\_()
* .\_\_getitem\_\_()
* .\_\_setitem\_\_()
* .\_\_delitem\_\_()
* .\_\_missing\_\_()

##### 私有化

* \_\_   双下划线开头的属性在访问时会自动加上类名，所以假设类名为B，访问其的\_\_num属性实际上访问的是\.\_\_B\_\_num
* \_ 单下划线，用于模块级的私有化

#### 授权

##### python搜索名字

优先级

* 类属性
* 数据描述符  在描述符中，定义了\_\_get\_\_()和\_\_set\_\_()为数据描述符，只定义了\_\_get\_\_()的为非数据描述符
* 实例属性
* 非数据描述符  （函数等属于非数据描述符）
* \_\_getattr\_\_()

因此可以通过该特性使用一个包装的类对其他类进行访问

##### 举例

```python
class wrapme(object):
    def __init__(self,obj):
        self.__data = obj
    
    def __repr__(self):
        return `self.__data`

    def __str__(self):
        return str(self.__data)

    def __getattr__(self,attr):
        return getattr(self.__data,attr)

    def get(self):
        return self.__data
```

假设用此类包装一个列表[1,2,3]

```python
a = wrapme([1,2,3])
a.append(4)
print a

>>> [1, 2, 3, 4]

print a[3]

>>> TypeError: 'wrapme' object does not support indexing
```

可以看到虽然wrapme中没有定义列表的类和方法（如列表的.append），但依然可以使用，这是因为python解释器在名字空间找不到成员时会通过\_\_getattr\_\_方法获取（若有定义）

但对于一些类型没法显式访问的内建属性或方法，如列表切片，依然不能使用

#### 新式类特性

##### \_\_slots\_\_

由于属性列表\_\_dict\_\_是字典类型，对内存占用较大，因此可以用\_\_slots\_\_存储属性列表

```python
class class_a(object):
    __slots__ = ('foo','func','test')

    def __init__(self):
        self.test = 1

    def func(self):
        print "slot test"
        
class_a.foo = 2
a = class_a()

print a.foo			#调用slots内有的属性
>>> 2

a.func()			#调用slots内有的方法
>>> slot test

print a.__dict__	#有__slots__的类没有__dict__
>>> 'class_a' object has no attribute '__dict__'
```

##### \_\_getattribute\_\_()

先前的\_\_getattr\_\_()方法用于找不到名字时的访问，而\_\_getattribute\_\_()则直接替代了每一次的属性访问

b.\_\_getattribute\_\_()把b.x变为type(b).\_\_dict\_\_['x'].\_\_get\_\_(b,type(b))

###### 类属性的访问

对于实例x，以下访问值为

```python
x.foo
>>> <bound method X.foo of <__main__.X object at 0x0000000002EEA208>>

type(x).__dict__['foo']
>>> <function foo at 0x0000000002FC0BA8>

type(x).__dict__['foo'].__get__
>>> <method-wrapper '__get__' of function object at 0x0000000002FC0BA8>

type(x).__dict__['foo'].__get__(x,type(x))
>>> <bound method X.foo of <__main__.X object at 0x0000000002EEA208>>  #与x.foo相同
```

对于类X，以下访问值为：

```python
X.foo
>>> <unbound method X.foo>

X.__dict__['foo']
>>> <function foo at 0x0000000002FC0BA8>

X.__dict__['foo'].__get__(None,X)
>>> <unbound method X.foo>
```



##### 描述符

对象属性的代理，可以通过自定义的方式控制所有对对象的访问和赋值

* \_\_get\_\_(self,obj,typ=None)    用于获取属性的值
* \_\_set\_\_(self,obj,val)    用于设置属性的值
* \_\_delete\_\_(self,obj)    用于删除属性的值

实际上类似于通过一个类重载了原来对某类中某属性的访问

下面通过描述符实现对属性的访问控制

```python
class decr(object):
    def __init__(self,arg,data=None):
        self.permission = arg
        self.data = data
    def __get__(self,obj,tpy=None):
        if self.permission & 4 == 0: #if no read permission
            print "Error:permission denied"
        else:
            return self.data
    def __set__(self,obj,val):
        if self.permission & 2 == 0:  #if no write permission
            print "Error:permission denied"
        else:
            self.data = val
    def __delete__(self,obj):
        if self.permission & 1 == 0:  #if no delete permission
            print "Error:permission denied"
        else:
            print "%s deleted" %str(self.data)
            del self.data

class test(object):
    readonly = decr(4,"readonly")
    writeonly = decr(2,"writeonly")
    readwrite = decr(6,"readwrite")
    delete = decr(1,"delete_test")

a = test()
print "===print readonly data==="
print a.readonly
print "===write readonly data==="
a.readonly = 1

print "===print writeonly data==="
print a.writeonly
print "===write writeonly data==="
a.writeonly = 1

print "===write readwrite data==="
a.readwrite = 1
print "===print readwrite data==="
print a.readwrite

print "delete test"
a.delete = 1
print a.delete
del a.delete
```

输出

```python
===print readonly data===
readonly
===write readonly data===
Error:permission denied
===print writeonly data===
Error:permission denied
None
===write writeonly data===
===write readwrite data===
===print readwrite data===
1
delete test
Error:permission denied
Error:permission denied
None
delete_test deleted
```

##### property()函数

###### 原型

```python
property(fget=None,fset=None,fdel=None,doc=None)
```

使用与上面的描述符基本一致，这种方法的便捷性在于不需要多定义一个描述符类

```python
class hide(object):
    def __init__(self,x):
        self.__x = x
    def get_x(self):
        return self.__x+1
    def set_x(self,x):
        self.__x = x+1
    x = property(get_x,set_x)

a = hide(10)	#self.__x = 10
print a.x		#print self.__x+1 = 11
a.x = 20		#set_x self.__x = 20+1
print a.x		#print self.__x+1 = 22
```

输出

```
11
22
```

##### 元类（metaclass）

元类是类的类，即元类的实例是类

当执行类定义的时候，解释器要为类定义本身创建一个类，因此必须知道其正确的元类

解释器确定元类的步骤

* 类定义中是否有\_\_metaclass\_\_，若有作为元类
* 父类是否有\_\_metaclass\_\_
* 是否有\_\_metaclass\_\_全局变量
* 若都没有，则定义了一个传统类，类型为types.ClassType

###### 传入元类的参数

* cls  类似于实例传入self，只不过这里的实例是个类
* name  类名
* bases  从基类继承数据的元组
* attrd  类的属性字典

###### 简单示例

```python
class MetaC(type):
    def __init__(cls,name,bases,attrd):
        super(MetaC,cls).__init__(name,bases,attrd)
        print "MetaC called"

class foo(object):
    __metaclass__ = MetaC
    def __init__(self):
        print "class foo"
    
f = foo()
```

输出

```
MetaC called
class foo
```

可以通过元类对类进行限定，如希望类对\_\_str\_\_()进行重载

```python
class MetaC(type):
    def __init__(cls,name,bases,attrd):
        if "__str__" not in attrd:
            raise TypeError("Class must rewrite __str__()")
        super(MetaC,cls).__init__(name,bases,attrd)
        
class foo(object):
    __metaclass__ = MetaC
    
f = foo()
```

输出

```
TypeError: Class must rewrite __str__()
```

## Chapter14 执行环境

#### 可调用对象

##### 函数

* 内建函数  作为\_\_bulitins\_\_模块导入解释器
  * .\_\_name\_\_  文档的名字
  * .\_\_self\_\_  设置为None，保留给内建方法
  * .\_\_module\_\_  存放该内建函数定义的模块名
* 用户定义函数  
  * .func_code  代码对象的字节码
  * .func_defaults  默认参数元组
  * .func_globals  全局名称字典
  * .func_dict  函数属性的名称空间
  * .func_closure  闭包，包含自由变量的引用的单元的对象元组
* lambda表达式

##### 方法

* 内建方法
  * .\_\_self\_\_  指向对象，注意内建函数指向None
* 用户定义方法
  * im_class  方法相关联的类
  * im_func  方法的函数对象
  * im_self  若绑定则为对应实例，否则为None

##### 类的实例

类的实例含有\_\_call\_\_()方法，默认没有定义，如果被定义则实例变为可调用的对象

```python
class C(object):
    def __init__(self):
        self.a = 1
        self.b = 2
        self.c = 3
        
    def __call__(self,ins):
        if ins in self.__dict__:
            print self.__dict__[ins]

c = C()
c('a')
```

输出

```
1
```

#### 代码对象

##### callable()

用于判断对象是否可调用（ 通过函数操作符() ）

##### compile()

将字符串（python代码）转换为字节码

```python
compile(obj,str,type)
```

* obj为代码的字符串
* str若不为""则代表代码对象的文件名字
* 类型
  * 'eval'  可求值的表达式（与eval()一起使用）
  * 'single'  单一可执行语句（与exec()一起使用）
  * 'exec'  可执行语句组（与exec()一起使用）

###### 

##### eval()

对表达式求值

```python
eval(obj,global_args,local_args)
```

* obj  代码字符串
* global_args  全局空间对象
* local_args  局部空间对象

##### exec

```python
exec obj
```

obj可以为文件等

##### input()

即为

```python
eval(raw_input())
```

因此input()会对输入进行求值，如输入

[123,'a']

raw_input返回是 "[123,'a']"，类型为字符串

input返回类型为列表

#### 执行其他python程序

##### import

导入时默认会执行所有顶层代码，因此一般声明 if \_\_name\_\_ == '\_\_main\_\_':

##### execfile()

类似exec语句的函数版

```python
execfile(filename,globals=globals(),locals=locals())
```

**注意** execfile不保证不会修改局部名称空间

##### python -m

不解释

#### 执行其他非python程序

##### os.system()

参数为命令行，运行时脚本挂起，返回值一般为0或非0

注意，该函数输入输出没有重定向，即还是标准输入输出

##### os.popen()

参数为命令行，返回一个文件对象，输入输出重定向到该对象上

因此可以使用脚本通过文件操作函数进行交互

##### os.fork()

##### os.wait*()

* os.wait()
* os.waitpid()

##### os.exec*()

* os.execl()
* os.execv()
* os.execle()
* os.execve()
* os.execlp()
* os.execvp()
* os.execlpe()
* os.execvpe()

##### os.spawn*()

##### subprocess模块

```python
from subprocess import call
res = call( ('cat', '/etc/motd') )

from subprocess import Popen,PIPE
f = Popen( ('uname','-a'),stdout=PIPE ).stdout
```

##### 结束执行

###### sys.exit()和SystemExit

###### sys.exitfunc()

默认不可用，但可以改写。当调用sys.exit()时会调用该函数

```python
import sys
prev_exit_func = getattr(sys,'exitfunc',None)

def my_exit_func(old_exit = prev_exit_func)
	...			#执行自定义处理
    if old_exit is not None and callable(old_exit):
        old_exit()
    sys.exitfunc = my_exit_func		#重载函数
```

###### os._exit()

不执行清理直接退出，不建议使用

###### os.kill(pid)

## Chapter15 正则表达式

模块：re

#### 主要函数

##### compile(pattern,flags=0)

编译正则表达式

##### group(n),groups()

group返回第n个匹配对象，group() == group(0)返回所有匹配的字符

groups返回所有匹配的对象元组

##### match(pattern,str)

从字符串开头测试匹配

##### search()

从字符串任意位置测试匹配

##### findall()

返回匹配字符串的列表

##### sub()/subn()

sub(要被替换的模式，替换成的模式，字符串)

subn返回一个替换后结果+替换次数的元组

##### split()

利用正则表达式划分字符串

## Chapter16 网络编程

##### 服务端套接字

* bind()
* listen()
* accept()

##### 客户端套接字

* connect()
* connect_ex()  connect()的扩展版本，出错时返回出错码而不是抛出异常

##### 公共用途的套接字函数

* recv()
* send()
* sendall()
* recvfrom()
* sendto()
* getpeername()
* getsockname()
* getsockopt()
* setsockopt()
* close()

##### 面向模块的套接字函数

* setblocking()  设置阻塞与非阻塞模式
* settimeout()   阻塞套接字的超时时间
* gettimeout()

##### 面向文件的套接字函数

* fileno()  套接字的文件描述符
* makefile()  创建与套接字关联的文件对象

#### SocketServer



## 其他

### 返回值
对于可变对象来说，一些操作没有返回值，对于不可变则一定有返回值（因为每次操作都会导致新对象的建立）

### 关于引用计数
python中的对象都保存了一个计数器用于记录各变量对该对象的引用情况，如
```python
a = "abcde"		# 创建string对象“abcde”，计数器=1
b = a			# 计数器+1
a = "bcde"		# 创建string对象“bcde”，计数器=1，对象“abcde”计数器-1
del(b)			# 对象”abcde“计数器=0，释放
```

### 深拷贝与浅拷贝

```python
person = [ 'name',['savings',100.00] ]	#定义一个初始对象h
hubby = person[:]						#切片方式拷贝
wife = list(person)						#工厂函数拷贝

hubby[0] = "joe"
wife[0] = "jane"
hubby[1][1] = 50.00
```

此时wife\[1\]\[1\]的值为50.0

原因是在拷贝person时仅进行了浅拷贝，即只拷贝了元素的引用

在后面的赋值中，由于'name'是不可变对象，因此赋值时被新对象替换，因此hubby[0]和wife[0]不一样

但hubby[1]与wife[1]指向相同列表，且因为列表可变，因此不会在赋值时被重新构造

#### 使用深拷贝

```python
person = [ 'name',['savings',100.00] ]
hubby = person

import copy
wife = copy.deepcopy(person)

hubby[0] = "joe"
wife[0] = "jane"
hubby[1][1] = 50.00
```

此时wife\[1\]\[1\]仍为100.00

#### 只能执行浅拷贝的对象

* 非容器类型
* 只包含原子类型（数字、字符串等）的元组对象

### 行分隔符的适配

* 行分隔  os.linesep
  * posix \n
  * 旧版Mac \r
  * Win \r\n

* 文件路径名分隔   os.sep
  * posix /
  * 旧版Mac :
  * win \

* 环境变量分隔    os.pathsep

* 当前工作目录    os.curdir

* 当前父目录     os.pardir

    
