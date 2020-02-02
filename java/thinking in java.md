## 第一章 对象导论

### 编码风格

大括号不换行

命名时，函数以多个单词命名则**每个单词首字母大写，不用下划线**

对于变量则首字母不需要大写

## 第二章 基本数据类型

### 基本类型

| 类型    | 位数       | 包装器    |
| ------- | ---------- | --------- |
| boolean | true/false | Boolean   |
| char    | 16b        | Character |
| byte    | 8b         | Byte      |
| short   | 16b        | Short     |
| int     | 32b        | Integer   |
| long    | 32b        | Long      |
| float   | 32b        | Float     |
| double  | 64b        | Double    |
| void    |            | Void      |

### 高精度

* BigInteger
* BigDecimal

### 作用域

大括号中为作用域

不允许类似C++不同作用域可以隐藏变量的行为，即不允许以下操作

```java
{
	int x = 12;
	{
		int x = 96;
	}
}
```

作用域外new定义的对象仍然占据空间

```java
{
    String s = new String("string");
}
```

括号外空间不会被马上释放，但会因gc而被释放

### 类

方法、属性的定义类似C++

### 枚举

#### 声明

```java
public enum Test {
    test1, test2
}
```

#### 调用

```java
Test.test1
```

#### 方法

* ordinal()  获取enum常量声明顺序
* toString()  获取enum选项名字
* values()  获取enum值

### 名字空间

java通过类似域名的`.`来指定名字空间及目录结构

通过**import**导入，可以使用通配符`*`

```
import java.util.*
```


## 第三章 操作符

### 赋值

对于普通类型赋值为复制，对于类赋值为复制引用

### 算术操作符

#### 二元

* `+ - * / %`
* 支持 `+= -=`等操作

#### 一元

* `-`
* `+`   用于把类型较小的操作数提升为int

#### 递增递减

`++ --`

### 关系操作符

#### 基本类型

`== != > < >= <=`

#### 对象

对于对象的比较`!= ==`默认行为是**比较引用**

对于Integer等数值类可以使用`.equals()`

```
Integer n1 = new Integer(47);
Integer n2 = new Integer(47);
n1.equals(n2)
```

但对于**自定义类默认依然比较引用**，除非覆写方法

### 逻辑操作符

`&& || !`

### 直接常量

`L F D`

### 指数记数

以10为底

float f = 1e-43f;

### 按位操作符

`| & ~ ^`

### 移位操作符

* `<<` 补0
* `>>` 符号扩展
* `>>>` 补0

### 三元运算符

`?:`

### 字符串运算符

`+ +=`

### 类型转换

和C一样

## 第四章 控制语句

**所有条件语句的表达式结果一定要是boolean**

### if-else

### while

### do-while

### for

### foreach

相当于C++迭代器

```java
Random rand = new Random(47);
float f[] = new float[10];
for(int i=0; i<10; i++)
    f[i] = rand.nextFloat();
for(float x : f)
    System.out.println(x);
```

### return

### break

### continue

### 标签

类似goto，但是**标签必须在循环前**，标签用于标识循环，如

```java
outer:
while(True) {
	System.out.println("outer loop");
    inner:
    while(True) {
        System.out.println("inner loop");
        break outer;
    }
}
```

这里代码只执行了一次就可以直接跳出外层循环，同样continue+标签名可以继续指定的循环

### switch

## 第五章 初始化与清理

### 构造器

与类名相同，区分大小写

定义构造器时不需要定义返回值（返回值为对象引用）

### 方法重载

同C++，区分方法由参数个数 参数类型确定

对于基本类型的参数，确定重载函数时会先比较是否有同类型参数的函数，否则向上类型提升一级再比较

**不能通过返回值确定重载**

### this

#### 使用this在构造器中调用构造器

* 只能调用一个构造器
* 调用必须在函数起始处

```java
class Test {
    int a;
    int b;
    Test(int num) {
        a = num;
    }
    Test(int numa, int numb) {
        this(numa);
        b = numb;
    }
}
```

#### static

如果属性或方法前加了static关键字，则类将创建一个单独的内存空间，不同实例对于该属性的修改都只会修改该位置的值，且可以直接通过类名引用

由于上面的属性，因此static方法中不存在this，即static方法不同于传统类的方法是通过传递this指针识别操作的对象并执行操作的

```java
class test {
    static int num = 0;
    static void inc() { num++; }
}

test t1;
test t2;
t1.num ++;	// num = 1
t2.num ++;	// num = 2
test.num ++;	// num = 3
test.inc();		// num = 4
```

### 清理：终结处理和垃圾回收

#### finalize()

* finalize 不是析构
* 对象可能在程序生命期中都不会被回收（若内存一直够用
* 若gc要回收某对象，会先调用finalize，因此finalize处理的事物应当与内存回收有关
* 一般来说，finalize不是必须的，除非有调用**本地方法**，本地方法用于调用非java代码，因此若使用了本地方法并且调用了C的malloc等函数就需要finalize来执行清理

finalize还有一个重要用法：

```java
class Test {
    int num = 0;
    Test(int n) {
        num = n;
    }
    protected void finalize() {
        if( num != 0 )
            System.out.println("check");
        super.finalize();			// 调用基类的finalize
    }
}
```

若Test中每个num在清零前都不该被释放，而此时有一处未引用，则可以通过调用

```java
System.gc()
```

强制垃圾回收，这里的未引用就可以被找出

#### 垃圾回收

##### 内存分配

不同于C/C++的malloc，java分配内存类似事先建立一个大数组，尔后一个个元素顺序分配

##### 引用计数

开销大（因为要遍历记录对象引用的表），且无法解决循环引用问题

##### 自适应的垃圾回收技术

###### 原理

任何活动对象，堆栈和静态存储区中必有对应的引用。因此从堆栈和静态存储区开始遍历所有引用必能找到所有活动对象。对于活动对象中包含的引用要继续进行遍历。

###### 实现一：停止复制

适用于经常产生垃圾的情况

JVM维护两个堆，需要gc的时候就**首先停止程序**，从堆栈和静态存储区出发，遍历所有引用及引用的引用，将所有的东西顺序复制到另一个堆（这也解决了碎片的问题）。注意这里所有的引用都需要被修正为新的地址。

###### 实现二：标记清扫

适用于没有新垃圾的情况

**首先停止程序**，gc遍历所有引用，找出活动对象。并给每个存活对象设标记，全部标记完毕后释放未标记对象（会产生碎片）

###### 优化：分代

将内存分为几个块，每个块用**代数(generation count)**标记，若块被引用代数增加，因此对于小对象代数大，对于大对象反之。gc时先停止复制代数大的对象，对于没有被gc的块增加代数。

### 成员初始化

java可以显式地指定类成员的初始化值（C++只能在构造器里赋值）

类成员按定义顺序初始化，但总能在构造器前全部完成初始化（即使定义在构造器之后）

**由此可见即使在构造器中对某个成员初始化了，java依旧会先调用该成员的默认构造器，尔后调用自定义构造器中的内容**

### static成员初始化

static成员初始化顺序在非static之前，且只在需要调用的时候调用（第一次使用static对象时

#### static块

```java
class Test {
	static Cup cup1;
     static Cup cup2;
    static {
        cup1 = new Cup(1);
        cup2 = new Cup(2);
    }
}
```

### 数组初始化

类似C

```java
int a[] = {1,2,3,4,5};
```

动态分配

```java
int a[] = new int[5];
```

数组中的元素都是对对象的**引用**

### 可变参数

```java
void f(Test... args)
```

## 第六章 访问权限控制

### 包

#### java源文件

一个java源代码文件称为**编译单元**，每个编译单元后缀为.java，**只有一个与文件同名的public类**，若有其他类则不能是public（允许没有public类）

#### java输出文件

每个源文件对应一个输出文件，后缀名为**.class**

#### java文档文件

不同于编译型语言进行链接，java打包生成**.jar**文件

#### package

多个.class文件生成的一个库文件

若希望某个.java从属于一个类库，则**在除注释的第一句代码写**

```java
package name
```

声明该文件的public类在name类库下，其他程序使用import name来调用

**每个类库文件有单独的名字空间**

#### 导入类库

使用import，import在**CLASSPATH**环境变量下查找文件

##### 使用import提供调试功能

如定义一个叫debug的包和一个叫release的包，类名和方法完全相同但debug中有调试打印的代码，则可以通过import不同包实现切换（java没有条件编译）

### 访问权限修饰词

#### 包访问权限

默认访问权限，同一个包内可以互相访问

当没有指定package的时候默认同目录下的文件都有包访问权限

#### 接口访问权限 public

所有外部类都可以访问

#### 无法访问 private

所有外部类都不可访问

#### 继承访问权限 protected

子类可以访问基类的protected成员

#### 一些限制

* 类本身不能是private或protected的（除内部类），只能是包访问或public
* 若不想让任何其他类访问该类，可以将构造器定义为private，此时只能通过内部的static函数或变量向外部提供接口间接创建类的实例

