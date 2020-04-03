codeql类似于sql，但用于对代码的查找

在线平台 https://lgtm.com/query

### 数据类型

#### 字符串

```
from string s
where s = "hello"
select s
```

结果

```
hello
```



```
from string s
where s = "he\"llo"
select s
```

结果

```
he"llo
```

##### 内建函数

```
from string s
where s = "hello"
select s.charAt(0)
```

结果

```
h
```

#### 整型与浮点型

```
from float x, int y
where x = 3.6 and y = 3
select x.pow(y)
```

结果为 3.6^3

```
46.656000000000006
```

#### 日期型

```
from date start, date end
where start = "01/10/2019".toDate() and end = "18/12/2019".toDate()
select start.daysTo(end)
```

结果为 2019.10.1到2019.12.18过的天数

```
78
```



```
from date Mydate
where Mydate = "18/12/2019".toDate()
select Mydate.getMonth()
```

结果为该日期字符串的月份，toDate将字符串转换为日期值

```
December
```



#### 布尔型

```
from boolean b
where b = false
select b.booleanNot()
```

结果为

```
true
```



```
from boolean b
where b = false
select b.booleanAnd(true)

from boolean b
where b = false
select b.booleanOr(true)

from boolean b
where b = false
select b.booleanXor(true)
```

结果分别为

```
false
true
true
```

### 数据库

codeql根据代码提交自动生成一个数据库，其中对于不同语言来说会采用不同的提取器，具体来说codeql提取器通过类似编译器词法语法分析的方式构造语义上的数据库。

#### 分析代码

建立数据库后可以使用codeql来分析代码

```
from Function f where count(f.getAnArg()) > 8 select f
```

getAnArg用于获取函数参数，count计算参数个数，因此该语句返回参数大于8的所有函数的查询结果



```
from Parameter p where not exists(p.getAnAccess()) select p
```

getAnAccess返回某个对象是否被访问过，因此该语句返回没有被访问过的函数参数



```
from Comment c where c.getText().regexpMatch("(?si).*\\bTODO\\b.*") select c
```

查找一个注释，符合正则表达式`.*\bTODO\b.*`，(?si)意为匹配单行，大小写不敏感

### 谓词

codeql支持定义谓词，所谓谓词描述的是给定参数与元组集合的关系，见下面。注意谓词一定要以小写字母开头

```
predicate isCountry(string country) {
  country = "Germany"
  or
  country = "Belgium"
  or
  country = "France"
}
 
predicate hasCapital(string country, string capital) {
  country = "Belgium" and capital = "Brussels"
  or
  country = "Germany" and capital = "Berlin"
  or
  country = "France" and capital = "Paris"
}
```

这里定义了两组谓词

* `isCountry = {("Belgium"),("Germany"),("France")}`
* `hasCapital =  {("Belgium","Brussels"),("Germany","Berlin"),("France","Paris")}`

因此调用isCountry("Germany")应返回true（类似py的in操作符）

#### 返回值

无返回值的谓词

```
predicate isSmall(int i) {
  i in [1 .. 9]
}
```

有返回结果的谓词

```
int getSuccessor(int i) {
  result = i + 1 and
  i in [1 .. 9]
}
```

定义一个特殊变量result，感觉这种谓词更像函数

```
string getANeighbor(string country) {
  country = "France" and result = "Belgium"
  or
  country = "France" and result = "Germany"
  or
  country = "Germany" and result = "Austria"
  or
  country = "Germany" and result = "Belgium"
}
```

调用getANeighbor("Germany")将返回("Austria", "Belgium")

#### 反向谓词

若定义了一个谓词 `getAParentOf (Person x)`返回x的父母，那么可以以如下方式通过输入父母的实例反向获取儿女

```
Person getAChildOf(Person p) {
  p = getAParentOf(result)
}
```

#### 递归谓词

##### 理论

为了求解递归型谓词的返回值的集合，QL编译器需要找到递归的最小不动点。具体来说，QL编译器会从一个空集开始求解，通过重复应用谓词以寻找新的、符合要求的值，直到返回值的集合不再变化为止，此时的集合就是递归的最小不动点，也就是递归的结果。类似地，QL查询的结果是查询中引用的谓词的最小不动点。

##### 基础

在上面的例子中，我们发现关系不是对称的，即理论上说若getANeighbor("Germany")返回Austria，那么getANeighbor("Austria")也应包含Germany

```
string getANeighbor(string country) {
  country = "France" and result = "Belgium"
  or
  country = "France" and result = "Germany"
  or
  country = "Germany" and result = "Austria"
  or
  country = "Germany" and result = "Belgium"
  or
  country = getANeighbor(result)
}
```

因此可以使用`country = getANeighbor(result)`反向查找



递归谓词还可以引用自身

```
Person ancestorOf(Person p) {
    result = parentOf(p) or
    result = parentOf(ancestorOf(p))
}
```

这里定义了一个查找祖先的谓词，在谓词中调用parentOf或parentOf(ancestorOf(p))，因此可以调用任意多次parentOf

##### 传递闭包

表示多次应用谓词

* parentOf+(p)  表示对变量p应用一次或多次parentOf
* parentOf*(p)  表示对变量p应用零次或多次parentOf

```
Person relativeOf(Person p) {
    parentOf*(result) = parentOf*(p)
}
```

可以找出p所有的亲属，即返回所有与p祖先相同的查询结果

##### 相互递归

即A调用B，B也调用A

##### 空递归

```
Person getAnAncestor() {
  result = this.getAParent().getAnAncestor()
}
```

将报错，因为初始时，getAnAncestor被定义为一个空集，因此无法添加新值。所以应给个递归起点

```
Person getAnAncestor() {
  result = this.getAParent()
  or
  result = this.getAParent().getAnAncestor()
}
```

##### 非单调递归

```
predicate isParadox() {
  not isParadox()
}
```

上述递归是不成立的，因为isParadox成立的条件是isParadox不成立，这导致递归过程不存在不动点

但若只会出现偶数次否定，则可以成立

```
predicate isExtinct() {		/* 灭绝 */
  this.isDead() and			/* this死了 */
  not exists(Person descendant | descendant.getAParent+() = this |
    not descendant.isExtinct()
  )							/* 获取一个后代且对它调用isExtinct */
}
```

##### 递归示例

###### 0到100之间的整数

```
int getANumber() {
  result = 0
  or
  result <= 100 and result = getANumber() + 1
}

select getANumber()
```

###### 0到100所有偶数

```
int getAnEven() {
  result = 0
  or
  result <= 100 and result = getAnOdd() + 1
}
 
int getAnOdd() {
  result = getAnEven() + 1
}
 
select getAnEven()
```



#### 示例

假设person类有一个属性表示自己居住的方位，则可以定义谓词

```
predicate southern(Person p) {
    p.getLocation() = "south"
}
```

则可以这样使用查询语句

```
from Person p
where southern(p)
select p
```

### 类

```
class Southerner extends Person {
    Southerner() { southern(this) }
}
```

这里Southerner类似于OOP中的构造函数，但它其实是用于表示传入的this是否属于该对象

这样可以把谓词中示例的查询代码改为

```
from Southerner s
select s
```

#### 谓词重载

```
imort tutorial

predicate southern(Person p)		/* 定义southern谓词，返回住在南边的人 */
{
	p.getLocation() = "south"
}

class Southerner extends Person		/* 定义Southerner类，表示住在南边的人 */
{
	Southerner() { southern(this) }
}

class Child extends Person
{
	Child() { this.getAge() < 10 }	/* 定义Child类，表示年龄小于10的人 */
	
	override predicate isAllowedIn(string region)	/* 重载isAllowedIn，改写为只返回region与getLocation相同的集合 */
	{
		region = this.getLocation;
	}
}


from Southerner s
where s.isAllowedIn("north")
select s, s.getAge()
```





### 逻辑连接词

and not or，注意括号

```
where t.getAge() > 30
  and (t.getHairColor() = "brown" or t.getHairColor() = "black")
  and not t.getLocation() = "north"
```

年龄大于30且发色是棕或黑且方位不在北边

### 聚合操作

常见函数有max min avg sum count，语法为

```
max(定义对象 | 逻辑字句 | 参与max运算的值)
```



```
from Person t
where t.getAge() = max(int i | exists(Person p | p.getAge() = i) | i)
select t
```

找出getAge不为空的所有person p，返回最大的，即找到年龄最大的

也可以用order by查询实现，这里不需要提供逻辑子句

```
select max(Person p | | p order by p.getAge())
```

### 示例

#### 过河问题

```c++
/**
 * A solution to the river crossing puzzle.
 */
 
/** A possible cargo item. */
class Cargo extends string {		
  Cargo() {
    this = "Nothing" or
    this = "Goat" or
    this = "Cabbage" or
    this = "Wolf"
  }
}
 
/** One of two shores. */
class Shore extends string {
  Shore() {
    this = "Left" or
    this = "Right"
  }
 
  /** Returns the other shore. */
  Shore other() {
    this = "Left" and result = "Right"
    or
    this = "Right" and result = "Left"
  }
}
 
/** Renders the state as a string. */
string renderState(Shore manShore, Shore goatShore, Shore cabbageShore, Shore wolfShore) {
  result = manShore + "," + goatShore + "," + cabbageShore + "," + wolfShore
}
 
/** A record of where everything is. */
class State extends string {
  Shore manShore;
  Shore goatShore;
  Shore cabbageShore;
  Shore wolfShore;
 
  State() { this = renderState(manShore, goatShore, cabbageShore, wolfShore) }
 
  /** Returns the state that is reached after ferrying a particular cargo item. */
  State ferry(Cargo cargo) {
    cargo = "Nothing" and
    result = renderState(manShore.other(), goatShore, cabbageShore, wolfShore)
    or
    cargo = "Goat" and
    result = renderState(manShore.other(), goatShore.other(), cabbageShore, wolfShore)
    or
    cargo = "Cabbage" and
    result = renderState(manShore.other(), goatShore, cabbageShore.other(), wolfShore)
    or
    cargo = "Wolf" and
    result = renderState(manShore.other(), goatShore, cabbageShore, wolfShore.other())
  }
 
  /**
   * Holds if the state is safe. This occurs when neither the goat nor the cabbage
   * can get eaten.
   */
  predicate isSafe() {
    // The goat can't eat the cabbage.
    (goatShore != cabbageShore or goatShore = manShore) and
    // The wolf can't eat the goat.
    (wolfShore != goatShore or wolfShore = manShore)
  }
 
  /** Returns the state that is reached after safely ferrying a cargo item. */
  State safeFerry(Cargo cargo) { result = this.ferry(cargo) and result.isSafe() }
 
  string towards() {
    manShore = "Left" and result = "to the left"
    or
    manShore = "Right" and result = "to the right"
  }
 
  /**
   * Returns all states that are reachable via safe ferrying.
   * `path` keeps track of how it is achieved.
   * `visitedStates` keeps track of previously visited states and is used to avoid loops.
   */
  State reachesVia(string path, string visitedStates) {
    // Trivial case: a state is always reachable from itself.
    this = result and
    visitedStates = this and
    path = ""
    or
    // A state is reachable using pathSoFar and then safely ferrying cargo.
    exists(string pathSoFar, string visitedStatesSoFar, Cargo cargo |
      result = this.reachesVia(pathSoFar, visitedStatesSoFar).safeFerry(cargo) and
      // The resulting state has not yet been visited.  保证查找的路径不重复
      not exists(int i | i = visitedStatesSoFar.indexOf(result)) and
      visitedStates = visitedStatesSoFar + "/" + result and
      path = pathSoFar + "\n Ferry " + cargo
    )
  }
}
 
/** The initial state, where everything is on the left shore. */
class InitialState extends State {
  InitialState() { this = renderState("Left", "Left", "Left", "Left") }
}
 
/** The goal state, where everything is on the right shore. */
class GoalState extends State {
  GoalState() { this = renderState("Right", "Right", "Right", "Right") }
}
 
from string path
where any(InitialState i).reachesVia(path, _) = any(GoalState g)
select path + "."
```



### 使用codeql分析代码

#### 查询特定代码

这段代码可以查询if后跟个pass的语句

```python
import python	# 引入python语法解析的包
 
from If ifstmt, Stmt pass
where pass = ifstmt.getStmt(0)	# 获取if下面的第一个语句
  and pass instanceof Pass		# 属于pass
select ifstmt, "This 'if' statement is redundant."
```

但如果if后跟pass，但存在else分支，则该判断不是无用的，因此可以加上更多约束来过滤出真正无用的if

```python
import python	# 引入python语法解析的包
 
from If ifstmt, Stmt pass
where pass = ifstmt.getStmt(0)	# 获取if下面的第一个语句
  and pass instanceof Pass		# 属于pass
  and not exists(ifstmt.getOrelse())	# 加了一句不包含else
select ifstmt, "This 'if' statement is redundant."
```

#### 用于分析语法的类

##### 作用域

* Scope
  * Module
  * Class
  * Function

下面语句用于查找声明该函数的作用域仍是一个函数的函数（即，在函数中声明函数）

```python
import python
 
from Function f
where f.getScope() instanceof Function
select f
```

##### 语句

Stmt类，常用以下方法访问各个部分，以一个for语句为例

```python
for var in seq:
    pass
else:
    return 0
```

* getTarget()  返回表示var的Expr
* getIter()  返回表示seq的Expr
* getBody()  返回语句列表主体
* getStmt()  返回特定编号的语句，如调用getStmt(0)返回pass
* getOrElse  返回else下的StmtList

##### 表达式

可以用下列语句查询运算符一边是名称另一边是数字的语句（如a+2）

```python
import python
 
from BinaryExpr bin
where bin.getLeft() instanceof Name and bin.getRight() instanceof Num
select bin
```

##### 变量

使用Variable类表示

* Variable
  * GlobalVariable
  * LocalVariable

##### 一些示例

查找所有final语句

```python
import python

from Try t
select t.getFinalbody()
```

查找所有块内只有pass语句的except块

这里注意，`not s instanceof pass`说明查找不属于pass的s，即若块内含有非pass语句则exist返回true，而exist前的not表示选取块内只含有pass语句的

```python
import python

from ExceptStmt ex
where not exist(Stmt s | s = ex.getAStmt() | not s instanceof pass)
select ex
```

可以等价于下面的语句

```python
import python

from ExceptStmt ex
where forall(Stmt s | s = ex.getAStmt() | s instanceof pass)
select ex
```

#### 用于分析控制流的类

* BasicBlock  一组没有分支的控制流节点
* ControlFlowNode  一个控制流节点

查找长度最长的BasicBlock

```python
import python
 
int bb_length(BasicBlock b) {
    result = max(int i | exists(b.getNode(i)) | i) + 1	# 返回块b的最大长度
}
 
from BasicBlock b
where bb_length(b) = max(bb_length(_))		# _表示没有特定输入，即任意值
select b
```



### ref

https://www.4hou.com/posts/gQOZ