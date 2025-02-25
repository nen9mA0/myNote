## ref

[Dart 基础 | Dart](https://dart.cn/language/)

## Dart开发语言

### 基础表达式

#### 变量

##### 定义

```dart
var name = 'Bob';        // 自动推断
Object name = 'Bob';     // 若一个对象不受限于单一类型可以指定为Object或dynamic
String name = 'Bob';     // 显式指定为字符串
```

##### 空安全

空安全能够防止意外访问null变量导致错误

空安全引入三个更改

* 为变量、参数或其他组件指定类型时，可以控制该类型是否允许为空
  
  ```dart
  String? name;       // 允许使用null
  String name;        // 不允许使用null
  ```

* 可空的变量默认初始化为null，非可空变量强制要求设置初值

* 不可以在可空类型上访问属性或调用方法，除非是null本身支持的属性或方法，如hashCode或toString

编译器可以通过上述机制来发现一些潜在的运行时错误，如

* 未使用非空值进行初始化

* 赋值为null

##### 默认值

可空类型默认值为null

非可空类型必须指定初值

##### 延迟初始化变量

有两个用法

* 声明一个非空变量，不在声明时初始化

* 延迟初始化一个变量，只有在使用的时候才会调用初始化函数
  
  ```dart
  // 这里只有在第一次使用temperature才会调用readThermometer
  late String temperature = readThermometer();
  ```

##### 终值和常量

const为编译时的常量，final则为只能设置一次的变量

```dart
final name = 'Bob';
const bar = 100;
const map = {if (i is int) i: 'int'};
```

#### 操作符

[Operators | Dart](https://dart.cn/language/operators/)

大多数运算符和表达式与其他语言差不多

##### 条件表达式

支持两种条件表达式，一种是三目运算符

```dart
var visibility = isPublic ? 'public' : 'private';
```

另一种是`expr1 ?? expr2`，当expr1非空时返回expr1，否则返回expr2

```dart
String playerName(String? name) => name ?? 'Guest';
```

##### 级联符号

允许对同一个对象进行多次操作，如下例都是操作paint对象

```dart
var paint = Paint()
  ..color = Colors.black
  ..strokeCap = StrokeCap.round
  ..strokeWidth = 5.0;
```

若对象是可空对象，则需要在第一次访问时使用`?..`

```dart
querySelector('#confirm') // Get an object.
  ?..text = 'Confirm' // Use its members.
  ..classes.add('important')
  ..onClick.listen((e) => window.alert('Confirmed!'))
  ..scrollIntoView();
```

该运算符可以嵌套

```dart
final addressBook = (AddressBookBuilder()
      ..name = 'jenny'
      ..email = 'jenny@example.com'
      ..phone = (PhoneNumberBuilder()
            ..number = '415-555-0100'
            ..label = 'home')
          .build())
    .build();
```

注意级联符号只能用在返回了对象的函数上，若函数返回void则不能使用级联符号

##### 传播运算

用于进行集合操作，具体见集合部分

```dart
[...a + b]
```

##### 其他运算符

| 运算符 | 意义                    |
| --- | --------------------- |
| ()  | 函数调用                  |
| []  | 索引                    |
| ?[] | 索引，但允许索引对象为空          |
| .   | 成员索引                  |
| ?.  | 成员索引，但允许索引对象为空        |
| !   | foo!.bar若foo为空会抛出一个异常 |

#### 注释

三种模式

* `//`

* `/*  */`

* `///`

#### 注解

如`@Deprecated @deprecated @override @pragma @Todo`

#### 导入

##### 一般导入

```dart
import 'dart:html';                // 导入官方库
import 'package:test/test.dart'    // 导入本地的包
import 'package:lib1/lib1.dart' as lib1    // 指定函数库前缀
// 部分导入
import 'package:lib1/lib1.dart' show foo;  // 只导入foo函数
import 'package:lib1/lib1.dart' hide foo;  // 导入foo函数
```

##### 懒加载

```dart
// 首先使用deferred as关键字导入
import 'package:greetings/hello.dart' deferred as hello;

// 实际加载时使用loadLibrary()加载
await hello.loadLibrary();
```

### 类型

#### 基本类型

##### 数字

int（小于等于64位，web端就是js的数字类型，从-2^53到2^53-1）和double（64位）

两者都是num的子类型

```dart
// 类型转换
// String -> int
var one = int.parse('1');

// String -> double
var onePointOne = double.parse('1.1');

// int -> String
String oneAsString = 1.toString();

// double -> String
String piAsString = 3.14159.toStringAsFixed(2);
```

##### 字符串

和python差不多，可以用+拼接，可以用`'''`创建多行文本，用r创建raw string

##### 布尔变量

true和false

##### Unicode字符

`\uXXXX`或`\u{XXXX}`

##### 其他类型

* Object  除了null外其他所有类的父类

* Enum  所有枚举类型的父类

* Future Stream  用于异步

* Iterable  用于迭代器

* Never  表明一个表达式永远不会正常执行完毕，一般用于一个总会抛出异常的函数

* dynamic  表示希望取消静态检查机制

* void  一般用于返回值，表示一个永远不会使用的值

#### 记录

##### 语法

记录可以包含键，也可以不包含键

```dart
var record = ('first', a: 2, b: true, 'last');
```

可以用类型标识来声明记录

```dart
// 不包含键
(String, int) record;
record = ('A string', 123);

// 使用{}来声明有键的元素
({int a, bool b}) record;
record = (a: 123, b: true);
```

注意，当记录含有键时，键名不同会导致记录的类型不同

(int a, int b)和(int x, int y)是同一类型，但{int a, int b}和{int x, int y}是不同类型

##### 域

```dart
var record = ('first', a: 2, b: true, 'last');

print(record.$1); // Prints 'first'
print(record.a); // Prints 2
print(record.b); // Prints true
print(record.$2); // Prints 'last'
```

##### 相等性

若两个记录有相同的shape（即每个元素的类型相同，对于含键的元素，其键名必须相同），且每个元素的值相同，则两个记录相等

##### 多返回值

```dart
(String name, int age) userInfo();
var (name, age) = userInfo();

({String name, int age}) userInfo();
var (:name, :age) = userInfo();
```

#### 集合

##### 列表

与python列表基本一致

##### 集合

集合元素类型必须相同，使用`{}`表示

```dart
var halogens = {'fluorine', 'chlorine', 'bromine', 'iodine', 'astatine'};
var names = <String>{};
```

##### 映射

与python基本类似，差别在于有类型

```dart
var gifts = Map<String, String>();
```

##### 运算符

###### 传播运算符

`...`或`...?`

```dart
var list = [1, 2, 3];
var list2 = [0, ...list];
```

###### 控制流运算符

```dart
var nav = ['Home', 'Plants', if (promoActive) 'Outlet'];
```

#### 泛型

```dart
abstract class Cache<T> {
    T getByKey(String key);
    void setByKey(String key, T value);
}
```

实际上集合的声明中就使用了很多泛型

```dart
var views = Map<int, View>();
```

可以使用extends关键字限定泛型的类型，使得类型只能是指定类型的子类型

```dart
class Foo<T extends Object> { }
```

限定为Object可以防止使用null作为泛型参数

#### 别名

```dart
typedef IntList = List<int>;
```

#### 类型系统

Dart是类型安全语言，使用静态类型检查和运行时检查来保证类型安全

##### 静态检查

其中下面几条静态检测规则需要注意：

* 重写方法时，使用类型安全返回值
  
  具体实践中，即子类方法中返回值类型应与父类返回值类型相同，或是其子类型

* 重写方法时，使用类型安全的参数
  
  具体实践中，即子类方法中参数类型应与父类参数类型相同，或是其父类型

* 不要将动态类型的List看做有类型的List
  
  动态列表可以在一个List中存放不同类型对象，但是其不应被分配给固定类型的列表，如
  
  ```dart
  List<Cat> foo = <dynamic>[Dog()];    // Error
  ```

##### 类型检查

分析器会试图通过各种信息推断类型，当分析器没有足够信息时会使用dynamic作为类型

```dart
// 显式将参数作为dynamic
Map<String, dynamic> arguments = {'argA': 'hello', 'argB': 42};

// 分析器自动推断为Map<String, Object>
var arguments = {'argA': 'hello', 'argB': 42};
```

##### 替换类型

在重写方法时，可以用一个新类型替换旧类型，此时可能用到一个类型的子类型或父类型，那么什么情况下替换是安全的

可以使用生产者和消费者模型来考虑：**可以使用父类型替换消费者类型，使用子类型替换生产者类型**

###### 普通类型赋值

如

```dart
Cat c = Cat();    // Cat c为消费者，Cat()为生产者
```

在消费者的位置，用父类替换特定类型是安全的，即可以

```dart
Animal c = Cat();
```

即消费者可以扩大范围，生产者则应该缩小范围，否则可能出现生产者提供了比消费者范围更大的对象

###### 泛型赋值

上述规则同样适用于泛型类型，比如可以将子类型的列表赋值给一个父类型列表

若想进行相反的操作，需要进行显式类型转换，将父类型转为子类型

###### 方法重写

也是一样原理，在上面静态检查部分也描述了该规则

### 模式匹配

#### 概览

##### 匹配

###### switch

switch可以使用下列匹配模式

```dart
const a = 'a';
const b = 'b';

switch (object) {
  // Constant pattern matches if 1 == number.
  case 1:                     // 匹配数字1
    print('one');
  case [a, b]:                // 匹配['a', 'b']
    print('$a, $b');
  case [a || b, var c]:       // 匹配['a', Object]或['b', Object]
    print(c);
  case >= first && <= last:
    print("In Range");
}
```

还可以在switch中加入判断指令，注意这边若满足某条件并进入语句执行，执行完毕会退出switch

```dart
switch (pair) {
  case (int a, int b):                          // 匹配两个整数
    if (a > b) print('First element greater');  // 若为真，打印，并退出switch
  // If false, prints nothing and exits the switch.
  case (int a, int b) when a > b:               // 与上一个case作用相同
    // If false, prints nothing but proceeds to next case.
    print('First element greater');
  case (int a, int b):
    print('First element not greater');
}
```

还有一种switch类似rust的写法

```dart
var isPrimary = switch (color) {
  Color.red || Color.yellow || Color.blue => true,
  _ => false
};
```

可以加入更复杂的判断

```dart
switch (shape) {
  case Square(size: var s) || Circle(size: var s) when s > 0:
    print('Non-empty symmetric shape');
}
```

###### for

for...in语句中也存在类似匹配

```dart
for (var MapEntry(key: key, value: count) in hist.entries) { }
```

#### 模式类型

* 或 ||

* 与 &&

* 关系型 != == <= >= < >

* 类型转换
  
  ```dart
  (num, Object) record = (1, 's');
  var (i as int, s as String) = record;
  ```

* null检查 ?  在取属性或方法前会检查对象是否为null，若为null则不匹配

* null断言 !  在取属性或方法前会检查对象是否为null，若为null则抛出异常

* 常数  即最常见的case 1

* 变量  `case (var a, var b)`

* 标识符  上面所有定义了标识符的情况都可以视作标识符模式，一种特殊类型是`_`，可以匹配任何类型的值（但使用该标识符表示不使用其匹配的值）

* 带括号的  表示匹配模式中包含括号表达式的情况

* 列表  如`case [a, b]` 的情况

* rest element
  
  ```dart
  var [a, ..., b] = [1, 2, 3, 4]; // a = 1, b = 4
  ```

* 映射  如`(:name, :age) = userInfo();`

* 记录  如`var (myString: foo, myNumber: bar) = (myString: 'string', myNumber: 1);`

* 对象  如`var Point(:x, :y) = Point(1, 2);`

* Wildcard  即`_`

### 函数

所有的函数都是对象，继承自Function类

#### 定义

```dart
// 传统形式的定义
bool isNoble(int atomicNumber) {
  return _nobleGases[atomicNumber] != null;
}

// 若只包含一条语句，可以采用这种定义
bool isNoble(int atomicNumber) => _nobleGases[atomicNumber] != null;
```

#### 参数

参数分为位置参数和命名参数两种。

* 位置参数直接根据参数顺序传入

* 命名参数通过`name: value`的形式传入

* 命名参数除非标注了required，否则为可选

* 可以为参数指定默认值

* 位置参数默认都是必须的，但可以通过`[]`包裹表示可选

```dart
// 使用命名参数声明
void enableFlags({bool bold=false, bool hidden=false});
// 使用命名参数调用
enableFlags(bold: true);

// 可选的位置参数
void enableFlags(bool bold, [bool hidden=false]);
```

#### main函数

```dart
void main(List<String> arguments) { }
```

#### 函数作为参数或变量

```dart
void PrintElement() { }

// 作为参数
var list = [1,2,3]
list.forEach(printElement);

// 作为变量
var func = PrintElement;
list.forEach(func);
```

#### 函数类型

定义一个函数类型的方法很简单，将一个函数定义中函数名替换成Function就是该函数属于的函数类型

```dart
viud Function() func = PrintElement;
```

#### 匿名函数

语法如下

```dart
([ [Type] param1[, ...] ]) {
    code;
}
```

如

```dart
var func = (String item) {
    return item.toUpperCase();
}
```

若只有一行，可以写为

```dart
var func = (String item) => item.toUpperCase();
```

#### 作用域

与其他语言基本一致，主要就是嵌套作用域和闭包作用域

#### Tear-offs

算是一个语法糖，若传入函数名，但不加上括号来调用函数，默认会传入一个函数闭包，这种实现方式要优于lambda，见下例

```dart
var charCodes = [1, 2, 3];

// 闭包实现
charCodes.forEach((code) {
    print(code);
});

// tear-off实现
charCodes.forEach(print);
```

#### 返回值

每个函数都会有返回值，没显式指定的返回值默认为null

#### 生成器

有异步的和同步的，异步的生成器返回Stream对象，同步的生成器则返回Iterable对象

```dart
// 同步
Iterable<int> naturalsTo(int n) sync* {
    int k = 0;
    while (k<n) yield k++;
}

// 异步
Stream<int> asynchronousNaturalsTo(int n) async* {
    int k = 0;
    while (k<n) yield k++;
}
```

若生成器为递归生成，可以使用yield*来提升性能

```dart
Iterable<int> naturalsDownFrom(int n) sync* {
  if (n > 0) {
    yield n;
    yield* naturalsDownFrom(n - 1);
  }
}
```

#### 外部函数

用于声明一个外部的库函数

```dart
external void func(int i);
```

### 控制流

#### 循环

```dart
// for
for (var i=0; i<2; i++) { }
for (final c in callbacks) { }

// while
while(flag) { }

do {
} while(flag);

// break continue与其他语言一样
```

#### 分支

##### 基本语句

```dart
// if
if () {

} else if () {

} else {

}

// if-case
if (pair case [int x, int y]) { }

// switch
switch (cmd) {
    case 'a':
        func_a();
    case 'b':
        func_b();
    default:
        func_default();
}
```

若switch中某个case没有具体内容，则会默认运行下一个case的内容，除非使用break。此外可以使用`continue <label>`改变switch运行顺序

```dart
switch (cmd) {
    case 'a':
    case 'b':
        func_ab();
    case 'c':
        continue newCase;

    newCase:
    case 'x':
        func_x();
}
```

##### switch表达式

可以将一个用于赋值的switch块写为switch表达式，如

```dart
switch(code) {
    case slash || star:
        token = operator(code);
    case >= digit0 && <= digit9:
        token = number(code);
    default:
        throw FormatException();
}
```

改写为

```dart
token = switch(code) {
    slash || star => operator(code),
    >= digit0 && <= digit9 => number(code),
    _ => throw FormatException()
};
```

switch也可以匹配类，与rust类似

```dart
double Area(Shape shape) => switch (shape) {
    Square(length: var l) => l * l,
    Circle(radius: var r) => math.pi * r * r
};
```

##### Guard clause

使用when来设置Guard clause，该关键字可以在匹配后为分支添加更多条件，且若条件执行结果为false，不会退出switch，而是转到下一条case执行

```dart
// switch语句中使用when
switch (something) {
    somePattern when some || boolean || expression:
        body;
}

// switch表达式中使用when
var value = switch (something) {
    somePattern when some || boolean || expression => body,
}

// if-case语句中使用when
if (something case somePattern when some || boolean || expression) {
    body;
}
```

#### 错误处理

##### Throw

可以抛出一个Exception类，也可以抛出任意对象

```dart
throw FormatException("error");
throw "error";
```

##### Catch

可以捕获多种类型的异常

```dart
try {
    ...
} on FormatException {            // 捕获FormatException 
    ...
} on Exception catch (e) {        // 捕获任意其他Exception
    ...
} catch (e, s) {                     // 捕获任意其他非Exception对象
    rethrow;
}
```

注意，这边catch可以设置两个变量，第一个会捕获异常对象，第二个则会捕获调用栈

rethrow语句可以将异常抛到上层，让异常继续传播

##### Finally

与其他语言的一致，不管发生异常与否都执行finally中的语句

##### Assert

与其他语言一致，当表达式为帧时正常执行，否则抛出异常。第二个参数可以指定assert异常的消息

```dart
assert(text != null, "text is null");
```

### 类&对象

#### 类

dart的继承模式基于mixin，除Null外所有的类都继承自Object

##### 使用类成员

```dart
var a = p.y;
var b = p?.y;
```

##### 使用构造器

```dart
var p1 = new Point(2, 2);                        // 构造函数
var p2 = new Point.fromJson({'x': 1, 'y': 2});   // 使用其他方法构造
var p = const ImmutablePoint(1, 1);              // 常量对象，可以在编译时构造
```

##### 获取对象类型

```dart
a.runtimeType;
```

##### 实例变量

类会隐式为所有的变量生成getter函数，并且为非final变量和未初始化的late final变量生成setter函数

若在class内直接初始化一个非late变量，其执行顺序在构造器和初始化列表运行前，因此无法访问诸如this成员

```dart
double initialX = 1.5;
class Point {
    double? x = initialX;    // 可以访问initialX变量
    double? y = this.x;      // 报错，无法访问，因为this还未初始化
    late double? z = this.x; // 加上late代表使用懒加载机制，之后才会初始化
    final double? xx = initialX;    // 可以使用final初始化成员变量

    Point(this.x, this.y);   // 这里是函数声明，因此没问题
}
```

若希望在实例初始化后再对final变量进行初始化，可以使用late final或工厂构造函数

##### 隐含接口

每个类都会隐式创建对应其实现函数和实例成员的接口，若类A希望支持类B的操作且不继承B，则需要实现B的接口。一个类可以实现多个接口

```dart
class Person {
    final String _name;
    Person(this._name);
    String greet(String who) => 'Hello, $who. I am $_name.';
}

class Impostor implements Person {
  String get _name => '';
  String greet(String who) => 'Hi $who. Do you know who I am?';}
```

##### 类变量和方法

###### 静态变量

全部类共享一个静态变量

```dart
class Queue {
    static const initialCapacity = 16;
}
```

###### 静态方法

静态方法不会传入this指针，但可以访问静态变量

```dart
class Point {
    static double distanceBetween(Point a, Point b) { }
}
```

#### 构造方法

Dart默认实现了一系列构造方法，主要有下列几类

##### 构造方法类型

###### 生成式构造方法

```dart
class Point {
    double x;
    double y;

    Point(this.x, this.y);
}
```

###### 默认构造方法

若未声明构造方法则会使用默认构造方法，该方法不接受参数

###### 命名构造方法

```dart
const double xOrigin = 0;
const double yOrigin = 0;

class Point {
    double x;
    double y;

    Point(this.x, this.y);

    Point.origin()
        : x = xOrigin,
          y = yOrigin;
}
```

子类不会继承父类的命名构造器

###### 常量构造方法

若一个对象是不可变的，可以通过常量构造方法创建编译时的常量。这里要将所有的实例变量设置为final变量

```dart
class ImmutablePoint {
    static const ImmutablePoint origin = ImmutablePoint(0, 0);
    final double x, y;
    const ImmutablePoint(this.x, this.y);
}
```

###### 重定向构造方法

```dart
class Point {
    double x, y;
    Point(this.x, this.y);
    Point.alongXAxis(double x) : this(x, 0);    // 该方法覆盖了原本的空构造函数
}
```

###### 工厂构造方法

一个工厂构造方法不总是构造一个新的对象，一般可能还会返回

* 一个已存在的实例

* 一个子类的新实例

```dart
class Logger {
    final String name;
    static final Map<String, Logger> _cache = <String, Logger>{};

    factory Logger(String name) {
        return _cache.putIfAbsent(name, () => Logger._internal(name));
    }

    factory Logger.fromJson(Map<String, Object> json) {
        return Logger(json['name'].toString());
    }

    Logger._internal(this.name);
}
```

注意，这里的_cache变量有下划线，代表这是类的private成员

###### 重定向工厂构造方法

将对当前类的工厂构造方法调用重定向到另一个类的工厂构造方法

```dart
factory Listenable.merge(List<Listenable> listenables) = _MergingListenable
```

一般有如下使用场景

* 一个抽象类可能需要提供一个来自其他类的常量构造方法

* 重定向工厂构造方法可避免转发器重复使用形式参数及其默认值

###### 构造方法tear-offs

Dart允许将构造方法作为参数提供，称为tear-off，其可以作为一个闭包

```dart
var strings = charCodes.map(String.fromCharCode);
var buffers = charCodes.map(StringBuffer.new);
```

其含义与下列lambda一致，因此不要使用下列lambda

```dart
var strings = charCodes.map((code) => String.fromCharCode(code));
var buffers = charCodes.map((code) => StringBuffer(code));
```

##### 实例变量初始化

有三种方式可以初始化实例中的成员变量

* 在声明中初始化  不赘述

* 使用初始化正式参数  即在构造函数声明`this.xxx`的情况
  
  注意有三种情况无法获取this
  
  * 工厂构造方法
  
  * 初始化列表的右边（不太明白什么意思）
  
  * 父类构造方法的参数
  
  ```dart
  class PointB {
      double x;
      double y;
  
      PointB(this.x, this.y);
      // 初始化正式参数
      PointB.optional([this.x=0.0, this.y=0.0]);
  }
  ```
  
  对于命名变量一样可以使用该方法
  
  ```dart
  class PointC {
      double x;
      double y;
  
      PointC.named({this.x=1.0, this.y=1.0});
  }
  ```

* 使用初始化列表
  
  ```dart
  Point.fromJson(Map<String, double> json)
          : x = json['x']!,
            y = json['y']! {
      print("xxx");
  }
  ```
  
  此外还可以在初始化列表处加入assert或其他逻辑，一个常见用法是用于初始化final变量
  
  ```dart
  class Point {
      final double x;
      final double y;
      final double distance;
  
      Point(double x, double y):
              : assert(x >= 0),
                x = x,
                y = y,
                distance = sqrt(x*x + y*y);
  }
  ```

##### 构造方法的继承

类不会继承其父类的构造方法，若没有定义构造方法则会用默认的构造方法

类可以继承父类的参数

子类可以调用父类的构造方法

###### 非默认父类构造方法

构造方法执行顺序如下

* 初始化列表

* 父类非命名、无参数的构造方法

* 主类无参数构造方法

可以使用`super.xxx()`调用父类构造函数，可以在初始化列表中调用

```dart
class Employee extends Person {
    Employee() : super.fromJson(fetchDefaultData());
}
```

###### 父类参数

[Constructors | Dart](https://dart.cn/language/constructors/#super-parameters)

#### 成员方法

可以重载大部分运算符，例子见[Constructors | Dart](https://dart.cn/language/constructors/#super-parameters)

类默认为每个成员变量实现Getter和Setter，也可以自己使用set get关键字实现

```dart
double get right => left + width;
set right(double value) => left = value - width;
```

可以定义抽象方法和抽象getter setter，抽象方法只能出现在抽象类或mixins中

```dart
abstract class Doer {
    void doSomething();
}
```

#### 继承

使用extends关键字继承

使用`@override`覆盖父类的方法，覆盖的方法需要符合下列条件

* 返回值类型必须相同或为原返回值的子类

* 参数类型必须相同或为原参数的父类

* 若原方法接收n个位置参数，新方法也需要接收n个位置参数

* 泛型方法无法覆盖非泛型方法，反之一样

可以覆盖noSuchMethod方法，该方法是当实例调用不存在方法时的回调

```dart
class A {
    @override
    void noSuchMethod(Invocation invocation) {
    }
}
```

#### Mixins

Mixins用于定义一些能在多个类层次中可以复用的代码

```dart
mixin Mixin_1 {
    ...
}

class MyClass with Mixin_1 {
    ...
}
```

##### 可以调用自身的mixin成员

[Mixins | Dart](https://dart.cn/language/mixins/#specify-members-a-mixin-can-call-on-itself)
