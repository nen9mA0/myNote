因为东西太多放一个文档太卡把它拆成两个

# 基础语法

## 变量/常量声明

### 变量可变性

使用`let`关键字声明变量

在rust中，变量默认是不可变的（immutable），即首次赋值后无法重新进行赋值

若要声明可变，需要加上`mut`关键字

```
let x = 10;            // immutable
let mut y = 10;        // mutable
```

### 常量

使用`const`关键字声明常量。常量无法用mut修饰，且作用域为全局，其他与不可变的变量基本相同。

```
const THREE_HOURS_IN_SECONDS: u32 = 60 * 60 * 3;
```

注意命名规则：大写与下划线结合

### 变量覆盖

在不同作用域上可以声明同名的变量，该声明会覆盖之前的声明

## 数据类型

### 基础类型（Scalar）

#### Integer

##### 整数类型

| 变量类型（有符号/无符号） | 长度   |
| ------------- | ---- |
| i8/u8         | 8b   |
| i16/u16       | 16b  |
| i32/u32       | 32b  |
| i64/u64       | 64b  |
| i128/u128     | 128b |
| isize/usize   | 架构相关 |

其中isize/usize有点类似于C中的int/uint，变量长度与处理器有关

##### 数字的表示

| Number literals | Example     |
| --------------- | ----------- |
| Decimal         | 98_222      |
| Hex             | 0xff        |
| Octal           | 0o77        |
| Binary          | 0b1111_0000 |
| Byte (u8 only)  | b'A'        |

##### 整数溢出

使用debug编译的rust程序包含溢出检测，但使用release的不包含。

若需要处理可能出现的溢出，文档建议使用标准库提供的一系列方法来实现数值操作

- `wrapping_*`系列方法，适用于所有的运算，如`wrapping_add`
- `checked_*`系列方法，当发生溢出时返回None
- `overflowing_*`系列方法，返回一个布尔值标识是否发生溢出
- `saturating_*`系列方法，会将返回值限制在一个最大和最小值间

#### 浮点类型

有两种浮点：f32和f64。默认使用f64。此外未定义的浮点数由`NaN`表示

```
let x = 1.0;        // f64
let y: f32 = 1.0;    // f32
```

#### 布尔类型

```
let t = true;
let f: bool = false;
```

#### 字符类型

```
let c = 'z';
```

**注意**：为了适配unicode，rust中一个char类型有4字节

### 复合类型（compound）

#### 元组

元组中的每个元素可以是不同类型

```
let tup: (i32, f64, u8) = (500, 6.4, 1);    // create 创建
let (x, y, z) = tup;                        // destructuring 解包
let a = x.0;                                // indexing 索引
```

一个空元组表示为`()`

#### 数组

数组中的每个元素必须是同一类型

```
let a = [1, 2, 3, 4, 5];            // 5个isize组成的数组
let b = [u32; 5] = [1, 2, 3, 4, 5];    // 5个u32组成的数组
let c = [3; 5];                        // 5个isize组成的数组，且初始为3
```

##### 越界访问

rust会检查每个数组索引，当索引超出数组范围会触发异常

## 函数

### 表达式与声明

在介绍函数前需要厘清两个概念：表达式（expression）和声明（statement）

最主要的区别在于，表达式有返回值，声明没有

一个函数由表达式和声明组成

注意下例，展示了两者的区别

```
let x = (let y = 1);        // 报错，因为let y = 1为声明，没有返回值
let z = {                    // 正确，末尾为一个表达式。注意末尾的表达式没有分号
    let a = 3;
    a + 1
};
```

### 函数定义

以`fn`为关键字进行声明，参数必须显式指定类型，使用`->`声明返回值类型

```
fn add_one(x: i32) -> i32 {
    let y = 1;
    x + y
}
```

默认最后一个表达式为返回值，也可以使用`return`关键字显式返回

可以通过指定返回值为`()`定义一个无返回值的函数；若函数无显式return，末尾也不是一个表达式，则会隐式地返回`()`

```
fn clear(text: &mut String) -> () {
  *text = String::from("");
}
```

最后，可以通过指定函数返回值为`!`创建一个永不返回的函数

```
fn forever() -> ! {
    loop {
        ...;
    }
}
```

## 分支语句

### if

#### 作为块语句

if...else if...else块的使用。其中需要注意，与C不同，if语句严格要求表达式为bool类型，不会进行隐式类型转换，因此下例中形如`if x`的语句会报错

```
let x = 1;
if x == 0 {
    ...;
} else if x == 1 {
    ...;
} else {
    ...;
}
```

#### 作为右值

```
let cond = true;
let number = if cond { 5 } else { 6 };
```

## 循环语句

### 循环控制语句

break与continue，用法与其他语言类似

### 循环语句

#### loop

死循环语句，一般配合循环控制语句使用。此外，loop可以作为右值，如下例

```
let mut counter = 0;
let result = loop {
    counter += 1;
    if counter == 10 {
        break counter * 2;        // 注意这里使用break返回了counter*2的值
    }
};
```

loop还可以有一种配合标签的用法，如下例：

```
let mut count = 0;
'counting_up: loop {
    println!("count = {count}");
    let mut remaining = 10;

    loop {
        println!("remaining = {remaining}");
        if remaining == 9 {
            break;
        }
        if count == 2 {
            break 'counting_up;        // 这里若count为2，则表示跳出couting_up标签处的loop，即直接跳出两层循环外，程序结束
        }
        remaining -= 1;
    }

    count += 1;
    }
}
```

#### while

与其他语言的while类似

```
let mut number = 3;

while number != 0 {
    println("{number}");
    number -= 1;
}
```

#### for

一般是用作迭代器的循环

```
let a = [1, 2, 3, 4, 5];
for elem in a {
    println!("{elem}");
}
```

也可以作为确定次数的循环，这里跟python类似，使用标准库中的`Range`或下例

```
for elem in (1..5) {
    println!("{elem}");
}
```

# Ownership

ownship是rust独有的机制，是rust为了安全管理内存提出的新方法，不同于其他语言的手工管理或使用GC管理。

## Ownership Rules

ownership的基本规则如下：

- rust中的每个值都对应一个owner
- 每个值在同一时刻只能有一个owner
- 当owner离开作用域（scope），该值将被释放

## Rust内存管理机制概述

### Variable Scope

Rust的思想与C++类似，采用RAII（Resource Acquisition Is Initialization，资源获取即初始化）的思想。因此一个变量在其作用域结束前将会被`drop`方法释放。

### Memory and Allocation

rust对不同类型的变量采取不同的管理方式（原文似乎按照变量在堆上还是栈上区分，但我目前无法确定这套机制是否就是依据变量所在的内存区域区分变量的。但核心思想应该是按照变量是否含有指向其他区域的资源来进行区分）

#### Variable and Data Interacting with Move

一般若变量类型可能含有指向其他区域的资源，默认都按照该方式操作。该方式有点类似于其他语言中的浅拷贝，但存在一个很大的区别：由于ownership规则规定一个变量在同一时刻只能有一个owner，因此move操作会将owner传递给左值，也就是说：

```
let s1 = String::from("hello");
let s2 = s1;                    // 这里将owner由s1传递给s2

println!("{}, world!", s1);        // 因此这里以s1作为参数将报错，因为s1此时已经不在作用域内
```

#### Variable and Data Interacting with Clone

实际上就是上述情况中的深拷贝方法，使用clone后不会改变owner，因为此时内存中有变量的两份副本

```
let s1 = String::from("hello");
let s2 = s1.clone();

println!("s1 = {}, s2 = {}", s1, s2);    // 这里不会报错
```

#### Stack-Only Data: Copy

这种类型的数据默认操作都是拷贝。rust中可以为数据类型定义`Copy`方法，若该类型实现了`Copy`，则默认会采取拷贝操作。但若该类型已经实现过`Drop`方法，则不可以定义`Copy`方法（原因是定义了`Drop`方法的一般都含有堆上的资源）

默认有如下的类型实现了`Copy`：

- 所有的整数类型（`u32`等）
- 布尔类型（`bool`）
- 所有的浮点类型（`f64`等）
- 字符类型（`char`）
- 元组类型，但要求元组中的每一个元素类型都实现了`Copy`

### Ownership and Functions

由于前面定义的数据和变量传递方式，若在调用函数时采用传值的方式，且变量是采取move交互的，则可能出现如下的问题：

```
fn main() {
    let s = String::from("Hello");    // String类型默认采用move传递
    takes_ownership(s);                // 这里ownership被传递给函数
    println!("{}", s);                // 因此这里报错
}

fn takes_ownership(some_string: String) {
    println!("{}", some_string);    // 传入的字符串的owner现在是some_string，因此在函数退出时将被释放
}
```

在函数返回时也存在类似问题

```
fn main() {
    let s1 = gives_ownership();
}

fn gives_ownership() {
    let some_string = String::from("Hello");
    some_string            // 这里some_string被返回，字符串的owner被传递给外层的s1，因此这里不会被销毁
}
```

可以看到这里与C/C++存在比较大的不同，后者不允许将内部作用域的变量传递到外部

因此在按值传递参数时，可以用下列方式重新将owner返回给参数

```
fn main() {
    let s1 = String::from("Hello");
    let (s1, len) = calculate_length(s1);    // 这里返回值的第一个参数将字符串的owner返回
}

fn calculate_length(s: String) -> (String, usize) {
    let length = s.len();
    (s, length)            // 通过元组的方式构造返回值
}
```

## 引用

### 引用的使用

使用引用时不会传递owner，相对地，rust称其为borrow，即借用

```
fn main() {
    let s1 = String::from("Hello");
    let len = calculate_length(&s1);
            // 这里s1和len都还在作用域内
}

fn calculate_length(s: &String) -> usize {
    s.len()
}
```

若需要修改引用内容，则可以传递mutable引用，如

```
fn main() {
    let mut s = String::from("hello");
    change(&mut s);
    println!("{}", s);
}

fn change(some_string: &mut String) {
    some_string.push_str(" world");
}
```

### 引用的限制

#### 竞争

竞争现象一般发生在如下情况

- 两个或更多的指针同时访问同一个资源
- 至少一个指针对资源进行了写入
- 没有同步的机制来控制对资源的访问

为了防止竞争现象的出现，rust对于引用采取了如下的限制：

- 在作用域内若出现了对资源的mutable引用，则不能存在其他引用（不管是mutable还是immutable）
- 若只有immutable引用，则可以出现多个

#### 悬垂引用

rust禁止悬垂引用的出现，因此对于下列程序无法通过编译

```
fn main() {
    let reference_to_nothing = dangle();
}

fn dangle() -> &String {
    let s = String::from("hello");
    &s            // 这里返回了悬垂引用
}
```

正确的返回方式是返回值

```
fn no_dangle() -> String {
    let s = String::from("hello");
    s
}
```

## 切片

切片是一种特殊的引用，定义方式如下：

```
let s = String::from("Hello");

let slice1 = &s[0..3];    // 0 1 2三个元素
let slice2 = &s[1..];    // 1到5四个元素
let slice3 = &s[..];    // 全部元素
```

## 一些问题

### 潜在data race？

发现下列代码可以通过编译，且按预期运行

```
fn main() {
    let mut s = String::from("Hello");
    let r1 = &mut s;
    r1.push_str(" World");
    println!("{}", r1);
    println!("{}", s);
}
```

是否存在竞争的可能？还有一个核心问题在于：mutable reference可以修改变量内容，但变量的owner并不是该引用，这是否打破了安全模型

### 结构体、数组等如何实现引用的互斥

前面的规则规定一个变量若存在mutable引用，则不能存在其他引用。那么这里是以什么为单元维护该规则的？比如对于一个数组，存在一个对其第一个元素的mutable引用，那么可不可以存在对其第二个元素的mutable引用

```
fn main() {
    let mut a = [1, 2, 3, 4, 5];
    let r0 = &mut a[0];
    let r1 = &a[1];
    println!("{} {}", r0, r1);
}
```

会报错，因此数组应该是按整体来维护的

# 复合类型

## 结构体

### 声明

```
struct User {
    active: bool,
    username: String,
    email: String,
    sign_in_count: u64,
}
```

### 创建实例与修改成员

#### 基本方法

注意，若定义结构体为mutable的，整个结构体都将是mutable的，不能单独设置成员的读写属性

```
fn main() {
    let mut user1 = User {
        active: true,
        username: String::from("someusername123"),
        email: String::from("someone@example.com"),
        sign_in_count: 1,
    };
    user1.email = String::from("anotheremail@example.com");
}
```

#### 使用函数初始化

```
fn build_user(email: String, username: String) -> User {
    User {
        active: true,
        username: username,
        email: email,
        sign_in_count: 1,
    }
}
```

这边有一个语法糖：若参数名与成员名一致，可以省略成员名，即

```
fn build_user(email: String, username: String) -> User {
    User {
        active: true,
        username,
        email,
        sign_in_count: 1,
    }
}
```

#### 基于其他实例初始化

```
fn main() {
    // --snip--

    let user2 = User {
        active: user1.active,
        username: user1.username,
        email: String::from("another@example.com"),
        sign_in_count: user1.sign_in_count,
    };
}
```

或可以使用下述语法糖简化成员的初始化

```
fn main() {
    // --snip--

    let user2 = User {
        email: String::from("another@example.com"),
        ..user1
    };
}
```

**注意**：这种方式中，变量的赋值同样符合之前提及的owner机制，因此如上例在赋值后，由于username成员类型为String，默认采用move方法，user1的该成员将不可用。其他两个成员默认为copy，因此依然可用

### 元组结构体

元组结构体更像一个元组，没有成员名。可以视为给特定类型的元组创建一个类型（或别名）

```
struct Color(i32, i32, i32);
struct Point(i32, i32, i32);

fn main() {
    let black = Color(0, 0, 0);
    let origin = Point(0, 0, 0);
}
```

### Unit-Like Struct

没有成员的结构体，一般用在trait的实现中

### 方法

#### 定义与调用

使用`impl`块定义方法，一般使用`.`调用方法

```
struct Rectangle {
    width: u32,
    height: u32,
}

impl Rectangle {
    fn area(&self) -> u32 {
        self.width * self.height
    }
}

fn main() {
    let rect1 = Rectangle {
        width: 30,
        height: 50,
    };

    println!(
        "The area of the rectangle is {} square pixels.",
        rect1.area()
    );
}
```

#### 参数

##### self参数

一般第一个参数是self，一般传引用`&self`，若要在方法中对实例进行修改，则传入`&mut self`，在极少的情况下也可以传入`self`，但一般是利用copy语义，使得用户只能操作实例的副本

对于第一个参数为self的方法，一般采用`.`运算符进行调用

##### 无self参数

一般这类方法用于创建新的类实例，如`String::from`就是一个典型的无self参数方法。一般使用`::`进行调用，本质上是引用对应类型的命名空间。

## 枚举与模式匹配

### 枚举

#### 枚举值

与其他语言类似，枚举类型可以是值

```
enum IpAddrKind {
    V4,
    V6,
}
```

使用类似命名空间的方式来使用

```
let four = IpAddrKind::V4;
```

#### 枚举中的变量

可以将枚举值定义为变量，自定义类型同样可以作为一个enum的变量，如

```
enum IpAddr {
    V4(String),
    V6(String),
}

enum IpAddr {
    V4(u8, u8, u8, u8),    // tuple类型
    V6(String),
}

enum Message {
    Quit,                        // unit tuple
    Move { x: i32, y: i32 },    // struct
    Write(String),                // string
    ChangeColor(i32, i32, i32),    // tuple
}
```

#### 枚举中的方法

与struct类似，也可以在枚举中定义一个方法

```
impl Message {
    fn call(&self) {
        // method body would be defined here
    }
}
```

#### Option枚举

是标准库中自带的枚举，且被默认引入（引用时不需要指定命名空间）。该枚举主要的作用是为rust引入类似空值的特性。rust在设计时没有引入Null空值的概念，因为在很多语言中，使用空值对非空的变量进行赋值常会导致问题，且这种影响会传播；但引入空值也有一个重要的作用：标记一个变量当前是不可用的。

Option枚举的定义如下

```
enum Option<T> {
    None,
    Some(T),
}
```

T类似C++中模板的概念，可以表示任何类型

```
let some_number = Some(5);
let some_char = Some('e');

let absent_number: Option<i32> = None;
```

使用Option的原因之一如下：

```
let x: i8 = 5;
let y: Option<i8> = Some(5);

let sum = x + y;
```

这段代码在编译时，会因为`i8`和`Option<i8>`类型不一致而无法通过编译，因此正确的处理方法是使用一段代码显式地判断是否为None后，再将Option包裹的值取出使用，这样避免了其他语言中忘记处理空值导致的问题，提高了安全性

### match语句

类似C中的switch case

#### 匹配值与变量

```
enum Coin {
    Penny,
    Nickel,
    Dime,
    Quarter(String),
    Michael,
}

fn value_in_cents(coin: Coin) -> u8 {
    match coin {
        Coin::Penny => {
            println!("Lucky penny!");
            1
        }
        Coin::Nickel => 5,
        Coin::Dime => 10,
        Coin::Quarter(mystr) => {
            println!("Input is {}", mystr);
            25
        }
        other => {
            println!("Other is {}", other);
            20
        }
    }
}
```

语法使用`=>`将语句分为两部分，前半部分为match的值，后半部分为要执行的语句，可以是语句块也可以是简单的值。若enum值为变量，则可以用上述第四条进行匹配。other可以匹配所有额外的情况，且可以将other作为变量使用；此外，若不想作为变量使用，也可以采用`_`代替other。注意，other应该放在最后，因为match执行时匹配是按顺序进行的。

**注意**：rust中的match语句要求对**所有可能出现的值**都进行匹配。

#### 匹配Option

下面展示的用法也是Option类的标准用法：使用match匹配来处理可能存在空值的情况

```
fn plus_one(x: Option<i32>) -> Option<i32> {
    match x {
        None => None,
        Some(i) => Some(i + 1),
    }
}

let five = Some(5);
let six = plus_one(five);
let none = plus_one(None);
```

### if let语句

match的语法糖，当enum中只有少数的情况需要处理时使用，如下述match语句

```
let config_max = Some(3u8);
match config_max {
    Some(max) => println!("Max value is {}", max),
    _ => (),
}
```

只有一种情况需要被枚举，因此可以改写为

```
let config_max = Some(3u8);
if let Some(max) = config_max {
    println!("Max value is {}", max);
}
```

可以加上else，这里就相当于`_`或`other`

**注意**：若使用了`if let`语法糖，则rust不会像`match`一样检查是否枚举了所有的情况

# 代码的组织

## Package与Crates

crates是rust的编译单元，分为binary crates和library crates。前者一般是一个二进制程序，拥有main函数；后者一般对应其他语言中库的概念，没有main函数。

crate root是一个源文件，编译时rust编译器将以该文件为起点开始编译。

Package是组织在一起的一个或多个crates，用于实现某一类功能。每个package中都含有一个`Cargo.toml`用于表示如何构建该包。

## 模块的组织

### 创建一个package

```
cargo new [name]
```

将创建一个目录如下的工程

```
[name]
    |__ Cargo.toml
    |__ src
        |__ main.rs
```

### 模块的引入与创建

使用mod关键字引入模块

```
mod module_1;
mod module_2 { ... };
```

在main.rs中引入了一个名为garden的模块。当编译器到达此处时，默认按照下列顺序查找模块：

- 如module_2，在大括号中包含模块路径
- 如module_1没有包含路径，则默认查找 `src/module_1.rs`和`src/module_1/mod.rs`

若存在二级子模块，查找路径类似。

## 模块的路径

有绝对路径和相对路径两种，使用`::`表示对命名空间的引用

如有下列目录结构

```
[name]
|__ Cargo.toml
|__ src
        |__ main.rs
        |__ module_1
                |__ mod.rs
```

假设想在main中调用module_1中的test函数

- 绝对路径 `crate::module_1::test()`
- 相对路径 `module_1::test()`

此外，模块在创建时默认是private的，对于需要对外开放的接口，应在模块和函数声明时使用`pub`关键字声明

模块的声明和实现可以分开，因此一般来说库的主文件用于声明接口，其他文件用于实现具体的函数功能

## 使用use关键字引入命名空间

### 基本使用

如上例的目录结构，可以改写为

```
use crate::module_1;

fn main() {
    module_1::test();
}
```

一般不会选择直接引入函数（`use crate::module_1::test`），因为这样可能造成歧义，让人误以为test函数是内部函数

### 使用as创建别名

若出现两个模块引入的函数名相同，可以用as关键字创建别名，如

```
use std::fmt::Result;
use std::io::Result as IoResult;
```

### 使用pub导出命名空间

假设在模块内使用use引入命名空间，当外部函数调用模块函数时，引入的命名空间对其不可见，但可以使用`pub use`导出命名空间，使得外部函数可以直接引用这些被导入的函数

### 多模块导入

可以使用下列方法同时导入多模块

```
use std::{cmp::Ordering, io};    // 导入了std::cmp::Ordering和std::io
use std::io::{self, Write};        // 导入了std::io和std::io::Write
use std::collections::*;        // 导入了std::collections下的所有子模块
```

# 常用容器

## Vector

### 创建

```
let v: Vec<i32> = Vec::new();
let v = vec![1, 2, 3];
```

第一种方法不包含初始化元素，因此需要指定类型；第二种是使用rust预定义的宏进行创建与初始化

### 添加元素

```
let mut v = Vec::new();
v.push(4);
```

需要注意的是容器同样需要加入`mut`关键字才能修改

### 读取元素

有两种方法

```
let third: &i32 = &v[2];
let third: Option<&i32> = v.get(2);
```

注意，这两种方法的主要区别在于，当下标大于数组长度时，前者触发panic，后者则只会返回`Option::None`（因此get方法返回的都是Option类）

此外，由于rust对于引用的限制，下例：

```
let first = &v[0];
v.push(6);
```

无法通过编译，因为在含有只读引用的情况下对元素本身进行更新。虽然更新的不是同一个元素，但由于vector在添加元素时可能存在内存重分配的现象，所以不允许此类操作

### 迭代

```
let mut v = vec![100, 32, 57];
for i in &mut v {
    *i += 50;
}
```

### 使用枚举使得vector可以保存不同的值

vector中只能保存同一类型的值，但可以通过定义枚举类型来使其包含不同类型，在定义时则使用该枚举作为类型

## String

### 创建

```
let mut s = String::new();

let s = String::from("hello");
let s = "hello".to_string();
```

### 修改

#### append

```
s.push(' ');        // 加入一个字符
s.push_str("World");// 加入一个字符串
```

#### concat

rust的String提供`+`运算

```
let s1 = String::from("Hello, ");
let s2 = String::from("world!");
let s3 = s1 + &s2;
```

注意，这里第一个元素传入的是值，后面的操作数传入的才是引用，因此在这条执行后，s3将获取owner，s1将失效。这是由于String类的运算定义如下：

```
fn add(self, s: &str) -> String {
}
```

self对应的元素不是引用

#### 格式化字符串

```
let s1 = String::from("tic");
let s2 = String::from("tac");
let s3 = String::from("toe");

let s = format!("{s1}-{s2}-{s3}");
```

使用`format!`宏进行格式化

#### index

rust不支持index操作，因为字符串都是由utf8编码的，使用index操作不但可能无法得到预期的对应字符，反而可能导致bug

#### slicing

比较神奇的是不支持index但支持slicing，但不建议这样做。同样使用类似`s[0..4]`的语法获取切片

#### 迭代

```
for c in "Зд".chars() {
    println!("{c}");        // 打印的是utf8解码后的字符
}

for b in "Зд".bytes() {
    println!("{b}");        // 按字节打印
}
```

## Hash Map

### 创建

```
use std::collections::HashMap;

let mut map = HashMap::new();
```

### 插入

```
map.insert(String::from("Blue"), 10);
```

### 读取

```
let key = String::from("Blue");
let item_ref = map.get(&key);            // 方法1
let item_cpy = map.get(&key).copied();    // 方法2
```

get返回的是引用，对于此例来说是`Option<&i32>`；调用copied后类型为`Option<i32>`

### 迭代

```
for (key, value) in &map {
    ...
}
```

### 改动

若对同一个键有改动，则存在下列情况

#### 覆写

若要采用覆写的方法，直接调用insert

```
map.insert(String::from("Blue"), 10);
map.insert(String::from("Blue"), 20);
```

#### 当不存在key时才增加

```
map.insert(String::from("Blue"), 10);

map.enrty(String::from("Yellow")).or_insert(50);
map.entry(String::from("Blue")).or_insert(50);
```

`entry`方法返回一个枚举类型`Entry`，用于表示该键是否存在。`or_insert`方法则是当`Entry`类非空时返回对应值的**mutable引用**，否则插入该键值

#### 改变存在的值

上面说了`or_insert`返回的是引用，因此可以直接修改

```
map.insert(String::from("Blue"), 10);

let value_ref = map.enrty(String::from("Blue")).or_insert(0);
*value_ref += 1;
```

### hashmap的ownership

```
use std::collections::HashMap;

let field_name = String::from("Favorite color");
let field_value = String::from("Blue");

let mut map = HashMap::new();
map.insert(field_name, field_value);
```

如上例，在insert后，因为是传值，field_name和field_value两个值的owner都会被hashmap拿走。而若改为传入引用，则需要保证在hashmap可用期间引用的值的可用性（由lifetime保证）

# 错误处理

## 不可恢复错误

使用`panic!`宏，关于一些调试技巧见https://doc.rust-lang.org/book/ch09-01-unrecoverable-errors-with-panic.html

## 可恢复错误

### 基本用法

对于如文件打开等可以恢复的错误，rust返回`Result`类型的变量，定义如下

```
enum Result<T, E> {
    Ok(T),
    Err(E),
}
```

一种处理文件打开的基本用法如下

```
use std::fs::File;
use std::io::ErrorKind;

fn main() {
    let greeting_file_result = File::open("hello.txt");

    let greeting_file = match greeting_file_result {
        Ok(file) => file,
        Err(error) => match error.kind() {
            ErrorKind::NotFound => match File::create("hello.txt") {
                Ok(fc) => fc,
                Err(e) => panic!("Problem creating the file: {:?}", e),
            },
            other_error => {
                panic!("Problem opening the file: {:?}", other_error);
            }
        },
    };
}
```

### 一些方便的方法

`unwrap()`方法和`expect()`方法一般包装了常用类的错误处理，因此可以直接调用。两者的区别主要是expect可以自己额外指定报错内容（同时unwrap的报错也会出现）

```
use std::fs::File;
use std::io::ErrorKind;

fn main() {
    let file_result_1 = File::open("hello.txt").unwrap();
    let file_result_2 = File::open("hello.txt").expect("Error");
}
```

### 错误传播

因为rust没有try catch，要将错误向上层函数传递就需要函数返回`Result`类，如下例

```
use std::fs::File;
use std::io::{self, Read};

fn read_username_from_file() -> Result<String, io::Error> {
    let username_file_result = File::open("hello.txt");

    let mut username_file = match username_file_result {    // 打开文件，若失败则返回Err
        Ok(file) => file,
        Err(e) => return Err(e),
    };

    let mut username = String::new();

    match username_file.read_to_string(&mut username) {        // 读取文件内容，若失败则返回Err
        Ok(_) => Ok(username),
        Err(e) => Err(e),
    }
}
```

但上述写法太繁琐，rust给了`?`语法糖

```
use std::fs::File;
use std::io::{self, Read};

fn read_username_from_file() -> Result<String, io::Error> {
    let mut username_file = File::open("hello.txt")?;
    let mut username = String::new();
    username_file.read_to_string(&mut username)?;
    Ok(username)
}
```

或者更简洁的，直接合并两个函数调用

```
File::open("hello.txt")?.read_to_string(&mut username)?;
```

`?`语法糖做的事情与前面match写法的差不多，但若当前函数自定义了错误类型，`?`会默认调用`From`trait将其他类型的错误类型转换为当前函数的错误类型

`?`语法糖适用的范围：

- 函数返回值为`Result`、`Option`等类（实现了`FromResidual`方法的类）
- 函数中遇到错误时会直接返回（early return）

## 一些错误处理的通用方法

### 何时使用panic

- 在例子、原型或测试中，因为这些情况下往往程序还没有完整的错误处理，开发者需要快速定位到错误的点
- 在一些不可能发生错误，但之后可能需要进一步错误处理的地方，使用`unwrap`或`expect`占位。如下例

```
use std::net::IpAddr;

let home: IpAddr = "127.0.0.1"
    .parse()
    .expect("Hardcoded IP address should be valid");
```

这里目前IP地址是硬编码的，所以不可能发生错误，但之后可能需要修改为变量传入或用户传入，因此在这里使用`expect`占位

- 在写库函数的时候，调用者可能以错误格式调用你的函数，而这个错误可能是不可修复的，这时为了防止调用者错误使用库，应该调用panic
- 类似函数传入参数的类型错误，因为这种一般都是由于调用者的程序逻辑有误，应及时反映

# 泛型/Trait/Lifetime

## 泛型

### 泛型结构体

```
struct Point<T> {
    x: T,
    y: T,
}
```

若需要两种类型，则

```
struct Point<T, U> {
    x: T,
    y: U,
}
```

### 泛型enum

如Result和Option的定义

```
enum Option<T> {
    Some(T),
    None,
}

enum Result<T, E> {
    Ok(T),
    Err(E),
}
```

### 泛型函数

```
fn larest<T>(list: &[T]) -> &T {
    let mut largest = &list[0];
    for num in list {
        if num > largest {
            largest = num;
        }
    }
    largest
}
```

### 泛型方法

```
struct Point<T> {
    x: T,
    y: T,
}

impl<T> Point<T> {
    fn x(&self) -> &T {
        &self.x
    }
}
```

`impl`关键字后的`<T>`用来指出当前是泛型方法

可以为特定的泛型类型指定专属的方法，如

```
impl Point<f32> {
    fn distance(&self) -> f32 {
        (self.x.powi(2) + self.y.powi(2)).sqrt()
    }
}
```

## Trait

特征，在C++也有出现，也类似java中的接口。为同一类的数据类型定义统一的一套操作接口

### 定义

如下例，定义了一个名为Summary的特征，其中含有summarize和get_status两种方法，后者定义了默认方法，因此在为类型实现特征时可以忽略该方法的实现。

```
pub trait Summary {
    fn summarize(&self) -> String;
    fn get_status(&self) -> String {
        format!("status: {}", self.status)
    }
}
```

**孤儿规则**：若想为类型A实现特征T，那么A或T至少有一个必须是在当前作用域中定义的。这个规则可以保证开发者不会莫名其妙地破坏其他库的代码。

### 为类型实现特征

这里为Test类实现了Summary特征，其中get_status存在默认方法，因此不一定需要实现

```
pub struct Test {
    pub name: String,
    pub status: String,
}

impl Summary for Test {
    fn summarize(&self) -> String {
        format!("name:{} status:{}", self.name, self.status)
    }
}
```

为符合条件的类型实现特征，如下例为所有实现了Display的类型实现ToString方法

```
impl<T: Display> ToString for T {

}
```

### 使用特征作为函数参数

可以为函数参数指定实现的特征，以此来限定传入的参数类型

```
pub fn notify(item: &impl Summary) {
    println!("Summary: {}", item.summarize());
}
```

存在一个语法糖，一般用来应对含有多个需要约束特征类型的参数

```
pub fn notify<T: Summary>(item1: &T, item2: &T) {
    println!("Summary1: {}", item1.summarize());
    println!("Summary2: {}", item2.summarize());
}
```

也可以约束多个类型，这种写法也可以使用语法糖的形式表示

```
pub fn notify(item: &(impl Summary + Display)) {
    println!("Summary: {}", item.summarize());
}
```

在遇到更复杂的约束时，可以使用`where`语法糖

```
fn func<T, U>(t: &T, u: &U) -> i32
    where T: Display + Clone,
          U: Clone + Debug
{

}
```

### 使用特征作为返回类型

通过如下方式表示函数返回的是一个实现了Summary特征的类型，但使用这种方法时必须保证一个函数内只能返回同一种类型。即，若存在if和else，且两者返回的类型不同，则无法通过编译

```
fn return_summarizable() -> impl Summary {

}
```

### 特征对象

是rust中实现多态的方法。由于rust不存在继承，试想这样一个问题：若使用rust实现窗口渲染程序，此时要存储不同的类型，并调用这些类型的draw方法，但rust的vector只能存储固定类型。一个实现思路是采用enum，但这意味着每添加一种类型都需要在enum中补充一个声明。

#### 使用方式

特征对象作为类型可以描述一系列实现了对应特征的类型

```
trait Draw {
    fn draw(&self) -> String;
}

impl Draw for u8 {
    fn draw(&self) -> String {
        format!("u8: {}", *self)
    }
}

impl Draw for f64 {
    fn draw(&self) -> String {
        format!("f64: {}", *self);
    }
}

fn draw1(x: Box<dyn Draw>) {
    x.draw();
}

fn draw2(x: &dyn Draw) {
    x.draw();
}

fn main() {
    let x = 8u8;

    draw1(Box::new(x));
    draw2(&x);
}
```

这里通过两种方式实现特征对象，其中draw1的参数是`Box<dyn Draw>`形式的特征对象，通过`Box::new(x)`创建；draw2的参数是`&dyn Draw`形式的特征对象，直接通过引用`&x`创建

#### 特征对象的动态分发

泛型在编译器就完成了类型的绑定：编译器会为每个泛型参数进行推到并绑定到具体类型，称为**静态分发**

特征对象则是**动态分发**，因为编译器在编译器无法获知参数的类型

![](https://intranetproxy.alipay.com/skylark/lark/0/2023/png/65256907/1689759977329-cbff64b5-94a6-452a-be11-030f454b8f0f.png)

#### 特征对象的对象安全

##### self与Self

rust中这两者不一样，`self`指代当前的实例对象，`Self`指代特征类型或方法类型的别名

```
trait Draw {
    fn draw(&self) -> Self;
}
```

self表示传入的参数为实例的引用，Self则表示返回值类型与传入的实例类型一致

一个典型例子就是clone特征

```
pub trait Clone {
    fn clone(&self) -> Self;
}
```

其返回的是传入的类型对应的类实例

##### 特征对象的限制

**只有对象安全的特征才能定义特征对象**，若特征的所有方法拥有以下属性，则是对象安全的

- 方法返回的类型不是`Self`
- 方法没有任何泛型参数

个人的理解是：rust希望将动态分发的过程限制在特征对象内，而上述两种情况中，都可能使得函数的返回值类型不确定，导致在特征对象外仍存在类型不确定的情况。

## Lifetime

### 生命周期的引入

lifetime机制主要是用来解决UAF的问题。在rust中，每个变量会有自己的生命周期，若存在变量间的引用，编译器将检查被引变量与引用变量间的生命周期关系。若引用的生命周期大于被引量，则有潜在的UAF风险，编译器将报错

一个例子如下：

```
fn longest(x: &str, y: &str) -> &str {
    if x.len() > y.len() {
        x
    } else {
        y
    }
}
```

该函数将出现编译错误，因为返回值为x或y，编译器无法确定两者的声明周期是否有差别

### 生命周期标记语法

用于告诉编译器某个变量的存活时间，**注意**，生命周期标记只是给编译器的提示，实际的变量生命周期并不会因为标注而改变

使用`'`来标注生命周期，后面通常跟随一个字母，表示一个生命周期的域

```
fn longest<'a>(x: &'a str, y: &'a str) -> &'a str {
    if x.len() > y.len() {
        x
    } else {
        y
    }
}
```

这里将参数和返回值全部标注为`'a`，表示参数和返回值的生命周期是一致的（生命周期至少与`'a`一样久）

### 生命周期消除

编译器会提供一套默认规则为函数标注生命周期，只有默认规则无法处理的函数必须显式地标注生命周期，当然对于默认规则可以处理的函数也可以显式标注生命周期

- 规则一：每个引用参数都会获得独自的生命周期
- 规则二：若只有一个输入生命周期（即函数中只有一个引用类型），那么该生命周期会被赋给所有的输出生命周期
- 规则三：若存在多个输入生命周期，且其中一个是`&self`（`&mut self`），则`&self`的生命周期被赋给所有的输出生命周期

下面通过几个例子说明规则

```
// 例1
    // 初始定义
    fn first_word(s: &str) -> &str { }
    // 先使用规则一对参数进行初始标注
    fn first_word<'a>(s: &'a str) -> &str { }
    // 再使用规则二对输出标注生命周期
    fn first_word<'a>(s: &'a str) -> &'a str { }
// 例2
    // 初始定义
    fn longest(x: &str, y: &str) -> &str { }
    // 规则一
    fn longest(x: &'a str, y: &'b str) -> &str { }
    // 注意，此时为x和y分别单独标注了生命周期
    // 而因为此时输入中不存在self，因此无法使用规则二和三，报错
```

此外还有一些常见的消除规则

- impl块消除

```
impl<'a> Reader for BufReader<'a> {
    ...
}
```

若块内部并没有用到`'a`，可以重新写作

```
impl Reader for BufReader<'_> {
    ...
}
```

表示有一个不使用的生命周期。但注意，这里不能忽略生命周期标注`'_`，因为这也是类型的一部分

- 生命周期约束消除

```
// 旧版本定义
struct Ref<'a, T: 'a> {
    field: &'a T
}

// 新版本定义，可以消除T: 'a
struct Ref<'a, T> {
    field: &'a T
}
```

### 函数中的生命周期

若函数的返回值存在引用类型，则它的生命周期只可能来源于：

- 函数参数的生命周期
- 函数体中某个新建引用的生命周期

道理很简单，因为引用的来源只可能是别的地方传递来的，或者是在作用域中新建的

通过生命周期的标注，编译器使用该信息进一步推导

```
fn main() {
    let string1 = String::from("long string is long");

    {
        let string2 = String::from("xyz");
        let result = longest(string1.as_str(), string2.as_str());
            // 在longest标注中，result/string1/string2的生命周期都是'a
        println!("The longest string is {}", result);
            // result对string1或string2的引用并没有超出两者的生命周期
    }
}
```

上述程序可以顺利编译，而若改为下述形式，则会报错

```
fn main() {
    let string1 = String::from("long string is long");

    {
        let string2 = String::from("xyz");
        let result = longest(string1.as_str(), string2.as_str());
            // 在longest标注中，result/string1/string2的生命周期都是'a
    }
    println!("The longest string is {}", result);
            // result对string1或string2的引用超出了string2的生命周期，报错
}
```

### 结构体中的生命周期

当结构体中出现引用时，也应为其标注生命周期

```
struct ImportantExcerpt<'a> {
    part: &'a str,
}
```

该标注说明part引用的字符串生命周期必须长于结构体

### 方法中的生命周期

```
struct ImportantExcerpt<'a> {
    part: &'a str,
}

impl<'a> ImportantExcerpt<'a> {
    fn announce_and_return_part(&self, announcement: &str) -> &str {
        println!("Attention please: {}", announcement);
        self.part
    }
}
```

**注意：**在声明时需要带上结构体的生命周期标注`ImportantExcerpt<'a>`，因为这也是结构体类型的一部分

在默认情况下，会进行如下的推导

```
impl<'a> ImportantExcerpt<'a> {
    // 规则一 =>
    fn announce_and_return_part<'b>(&'a self, announcement: &'b str) -> &str {
        println!("Attention please: {}", announcement);
        self.part
    }
    // 规则二，存在&self =>
    fn announce_and_return_part<'b>(&'a self, announcement: &'b str) -> &'a str {
        println!("Attention please: {}", announcement);
        self.part
    }
}
```

若存在两个生命周期，则很多时候需要确定生命周期间的关系。如手动将上例中的返回值生命周期改为'b，则由于返回值的引用生命周期为'b，被引用值self生命周期为'a，则'a必须大于'b，以保证不会出现悬垂引用，因此需要添加约束

```
impl<'a:'b, 'b> ImportantExcerpt<'a> {
    fn announce_and_return_part<'b>(&'a self, announcement: &'b str) -> &'b str {
        println!("Attention please: {}", announcement);
        self.part
    }
}
```

也可以使用where关键字表示约束

```
impl<'a> ImportantExcerpt<'a> {
    fn announce_and_return_part<'b>(&'a self, announcement: &'b str) -> &'b str
        where
            'a:'b,
        {
        println!("Attention please: {}", announcement);
        self.part
    }
}
```

### 静态生命周期

`'static`拥有该生命周期的引用表示生命周期与整个程序一样久，但同样的，这里只是一个标注，若实际生命周期没有那么久则可能引入bug

### 无界生命周期

这种情况一般出现在unsafe中，如

```
fn f<'a, T>(x: *const T) -> &'a T {
    unsafe {
        &*x
    }
}
```

这里直接解引用了一个裸指针x，其会被默认赋予`'unbounded`生命周期，这种生命周期可以适配任何其他的生命周期，因此总是可以通过检查

### 闭包函数的生命周期消除规则

```
fn fn_elision(x: &i32) -> &i32 { x }
let closure_elision = |x: &i32| -> &i32 { x };
```

如上例，作为函数的fn_elision编译成功，但作为闭包的closure_elision编译失败

由于函数的消除规则`如果函数参数中只有一个引用类型，那该引用的生命周期会被自动分配给所有的返回引用`，因此编译成功。而对于闭包则无法编译成功，因为闭包的消除规则与函数不一样，这是因为闭包可能与当前的上下文有关，而函数是上下文无关的，后者可以简单地通过确定的输入参数和输出参数进行优化，而闭包则无法进行这样的操作

上述闭包可以通过下列泛型和Fn特征包裹通过编译

```
fn fun<T, F: Fn(&T) -> &T>(f: F) -> F {
    f
}

fn main() {
    let closure_elision = fun( |x: &i32| -> &i32 { x } );
}
```

### NLL(Non-Lexical Lifetime)

旧版本的rust生命周期依据变量的作用域来计算，但引入NLL特性后，变成了：**引用的声明周期从借用处开始，一直持续到最后一次使用的地方**

```
let mut u = 0i32;
let mut v = 1i32;
let mut w = 2i32;

// lifetime of `a` = α ∪ β ∪ γ
let mut a = &mut u;     // --+ α. lifetime of `&mut u`  --+ lexical "lifetime" of `&mut u`,`&mut u`, `&mut w` and `a`
use(a);                 //   |                            |
*a = 3; // <-----------------+                            |
...                     //                                |
a = &mut v;             // --+ β. lifetime of `&mut v`    |
use(a);                 //   |                            |
*a = 4; // <-----------------+                            |
...                     //                                |
a = &mut w;             // --+ γ. lifetime of `&mut w`    |
use(a);                 //   |                            |
*a = 5; // <-----------------+ <--------------------------+
```

### Reborrow 再借用

```
fn main() {
    let mut p = Point { x: 0, y: 0 };
    let r = mut& p;                // 借用p
    let rr: &Point = &*r;        // 对p的再借用
    println!("{:?}", rr);
    println!("{:?}", r);
}
```

如上例，rr是对借用r的再借用，此时允许在包含可变借用时同时拥有不可变借用（不使用再借用时会破坏借用规则）。但需要遵循的规则是**在再借用的生命周期内不能使用原来的借用**，如上例中rr的生命周期在r使用前就结束了，因此没有报错

### 生命周期约束

#### 'a:'b

表示`'a>='b`

#### T:'a

表示类型T生命周期必须比'a久

```
struct Ref<'a, T: 'a> {
    r: &'a T
}
```

在旧版本rust才需要指定

# 闭包/迭代器

## 闭包

### 基本语法

```
|param1, param2, ...| {
    ...
}
```

如

```
let sum = |x,y| x+y
```

默认情况下，闭包可以进行隐式类型推导，但也可以显式标注

```
let sum = |x: i32, y: i32| -> i32 {
    x + y
}
```

需要注意的是，闭包不是泛型，因此当编译器隐式推导出一个闭包的类型，接下来若传入其他类型的参数则会报错

### 结构体中的闭包

```
struct Cacher<T>
where
    T: Fn(u32) -> u32,
{
    query: T,
    value: Option<u32>,
}
```

这里的T存在一个特征约束`Fn(u32)->u32`，该约束表示T是一个闭包类型

### 闭包与作用域

闭包与函数还有一个很大的不同：闭包可以捕获当前作用域的变量，而函数不可以

```
fn main() {
    let x = 4;
    let equal_to_x = |y| y==x;        // x在作用域内，因此可以直接使用
}
```

### 三种Fn特征

函数参数有三种传入方式：转移所有权、可变借用、不可变借用，因此对应三种Fn。Fn定义了**闭包如何使用被捕获的变量**

#### FnOnce

该类闭包会拿走被捕获变量的所有权，因此该类闭包只能被调用一次。但若闭包实现了Copy特征，则可以被调用多次

```
fn fn_once<F>(func: F)
where
    F: FnOnce(usize) -> bool + Copy,// 改动在这里
{
    println!("{}", func(3));
    println!("{}", func(4));
}

fn main() {
    let x = vec![1, 2, 3];
    fn_once(|z|{z == x.len()})
}
```

可以通过`move`关键字（详见下节）强制闭包获取变量所有权，通常用于闭包的生命周期大于捕获变量的生命周期，如将闭包返回或移入其他线程

```
use std::thread;
let v = vec![1, 2, 3];
let handle = thread::spawn(move || {
    println!("Here's a vector: {:?}", v);
});
handle.join().unwrap();
```

#### FnMut

表示包含可变借用的闭包

注意这里在定义update_string时使用了mut关键字修饰

```
fn main() {
    let mut s = String::new();
    let mut update_string = |str| s.push_string(str);
}
```

#### Fn

表示包含不可变借用的闭包

```
fn main() {
    let s = "hello, ".to_string();

    let update_string =  |str| println!("{},{}",s,str);

    exec(update_string);

    println!("{:?}",s);
}

fn exec<'a, F: Fn(String) -> ()>(f: F)  {
    f("world".to_string())
}
```

### move关键字与Fn

move关键字定义了一个**闭包使用move的方式捕获变量**

```
fn main() {
    let s = String::new();

    let update_string =  move || println!("{}",s);
    exec(update_string);
    println!("{}",s);        // 这里将报错，因为s的所有权已经被拿走
                            // 若去掉move关键字则成功运行
}

fn exec<F: FnOnce()>(f: F)  {
    f()                        // 这里传入的闭包update_string只借用了s的引用
                            // 因此使用FnOnce没有问题，此外，它也符合Fn特征
}
```

move与Fn规定了一个闭包函数两个方面的特性：move规定了闭包如何捕获变量，Fn规定了变量如何在闭包中被使用

此外，三种Fn特征并不冲突，一个闭包可能同时实现多种特征：

- 所有闭包都自动实现了FnOnce特征，因此每个闭包都至少可以被调用一次
- 没有移出所捕获变量的所有权的闭包自动实现了FnMut特征（即闭包中没有获取变量的所有权）
- 不需要对捕获变量进行改变的闭包自动实现了Fn特征

如下例，闭包update_string同时满足三种特征

```
fn main() {
    let s = String::new();

    let update_string =  || println!("{}",s);

    exec(update_string);
    exec1(update_string);
    exec2(update_string);
}

fn exec<F: FnOnce()>(f: F)  {
    f()
}

fn exec1<F: FnMut()>(mut f: F)  {
    f()
}

fn exec2<F: Fn()>(f: F)  {
    f()
}
```

更进一步地，Fn特征的前提是同时满足FnMut和FnOnce，FnMut的前提是满足FnOnce

### 闭包作为函数返回值

首先，由于闭包无法指定特定的大小，因此需要通过返回特征的方式返回闭包

```
fn factory(x:i32) -> impl Fn(i32) -> i32 {
    |x| x+1
}
```

但这种方式的局限性在于：若可能返回一个以上的闭包，如存在一个if else分支，编译器会报错。因为返回特征要求特征的类型一致

因此最佳的方式是返回特征对象

```
fn factory(x:i32,  opt:bool) -> Box<dyn Fn(i32) -> i32> {
    if opt {
        Box::new(move |x| x+1)
    } else {
        Box::new(move |x| x-1)
    }
}
```

## 迭代器

### for循环与迭代器

```
let arr = [1, 2, 3];
for v in arr {                // 数组迭代
    println!("{}", v);
}

for v in arr.into_iter() {    // 数组显式转换为迭代器
    println!("{}", v);
}

for i in 1..10 {            // 数值序列迭代
    println!("{}", i);
}
```

上述的数组和数值序列都实现了`IntoIterator`特征，因此可以转换为迭代器

### 惰性初始化

迭代器在被使用前不会发生任何迭代行为，因此创建迭代器不会有性能损耗

### 迭代器相关的特征

#### Iterator特征

##### next方法

next方法被定义在Iterator特征中，用于取迭代器的下一个元素

```
pub trait Iterator {
    type Item;

    fn next(&mut self) -> Option<Self::Item>;
}
```

#### IntoIterator特征

该特征主要定义了`into_iter` `iter` `iter_mut`三种显式获得迭代器的方法，区别如下

- into_iter 会夺走所有权
- iter 借用
- iter_mut 可变借用

```
impl<I: Iterator> IntoInterator for I {
    type Item = I::Item;
    type IntoIter = I;

    fn into_iter(self) -> I {
        self
    }
}
```

#### 使用Iterator和next迭代

用这种方法显式迭代返回的是`Option`类型，其中不同迭代器返回的有所差别

- iter `Option<&T>`
- iter_mut `Option<&mut T>`
- into_iter `Option<T>`

可以通过Iterator和next实现for循环

```
let values = vec![1, 2, 3];
{
    let result = match IntoIterator::into_iter(values) {
        mut iter => loop {
            match iter.next() {
                Some(x) => { println!("{}", x); },
                None => break,
            }
        }
    }
}
```

### 消费者与适配器

#### 消费者适配器

只要迭代器上的某个方法内部调用了next方法，就称其为消费者适配器。因为next方法会消费迭代器的元素。如`sum`方法，会拿走迭代器所有权，并调用`next`方法对元素进行求和

#### 迭代器适配器

迭代器适配器用于返回新的迭代器，是迭代器链式方法调用的关键，如`v.iter().map().filter()`

迭代器适配器是惰性的，因此需要使用消费者适配器作为链式调用的收尾，如

```
v.iter().map(|x| x+1);
```

会报错，因为map产生的迭代器没有被消费，因此应该使用

```
let v2: Vec<_> = v.iter().map(|x| x+1).collect();
```

其中编译器会自动推导`_`代表的类型。而由于collect方法在消费时可以生成多种不同的集合类型，所以应对v2进行类型标注显式地告诉编译器要生成的集合类型。下例是使用collect生成HashMap的例子

```
use std::collections::HashMap;
fn main() {
    let names = ["sunface", "sunfei"];
    let ages = [18, 18];
    let folks: HashMap<_, _> = names.into_iter().zip(ages.into_iter()).collect();
    println!("{:?}", folks);
}
```

#### 常用适配器

- 消费者适配器

- collect 上节有描述

- sum 对迭代器中所有元素进行求和

- 迭代器适配器

- map 会对迭代器中每个值进行一个操作，一般搭配函数或闭包使用，对每个值运行一个函数，并将其结果作为迭代器

- zip 将两个迭代器的内容压缩到一起，形成类似`Iterator<Item=(ValueFromIterA, ValueFromIterB)>`的迭代器

- filter 对迭代器中的元素进行过滤，一般搭配函数或闭包使用，保留返回值为true的元素并将其作为新的迭代器

- enumrate 生成新的迭代器，由原迭代器元素和其索引组成

# 智能指针

智能指针大多数时候用于管理堆上的数据。在Rust中，智能指针需要实现两个特性

- Deref 用于定义解引用运算的具体行为
- Drop 用于指定智能指针超出作用域后的行为（析构函数）

## Box堆对象分配

### Rust中的堆栈

Rust中默认main函数的栈大小限制为8MB，普通线程为2MB

Rust中的所有权传递实际是智能指针的传递，如下例

```
fn main() {
    let = foo("world");
    println!("{}", b);
}

fn foo(x: &str) -> String {
    let a = "Hello ".to_string() + x;
    a
}
```

foo中的a是String类型，实际上是一个智能指针，存储在栈上并指向堆上的字符串。当函数返回时，a对应的智能指针被赋给main中的b，实现了所有权的转移

### Box的使用场景

#### 将数据存储在堆上

若需要将数据定义在堆上，可以使用`Box<T>`

```
fn main() {
    let a = Box::new(3);    // 声明了一个堆上的int
    println!("a = {}", a);    // 这里a是一个指针，但println会自动进行Deref操作
    let b = a + 1;            // 这句则会报错，因为1是integer，a是Box<integer>
}
```

#### 避免栈上数据的拷贝

当栈上数据转移所有权时需要拷贝，但堆上只需要复制指针

```
fn main() {
    let arr1 = [0;1000];
    let arr2 = arr;        // arr在栈上，所以这里直接发生了一次深拷贝

    let arr3 = Box::new([0:1000]);
    let arr4 = arr3;    // arr3指向的数据在堆上，arr3实际上只是个指针
                        // 因此只会拷贝指针
}
```

#### 将动态大小类型转换为固定大小

Rust在编译时需要计算每个类型的大小，若一种类型在编译时无法知道大小，被成为动态大小类型DST，将报错。例如递归类型

```
enum List {
    Cons(i32, List),    // 在类型定义中引用自身造成DST
    Nil,
}
```

此时将List转为一个Box类型，将实际动态分配的大小转到堆上，则可以解决该问题

```
enum List {
    Cons(i32, Box<List>),
    Nil,
}
```

#### 特征对象

在Rust中想实现不同类型组成的数组只有enum和特征对象两种办法，后者一般通过Box类型存储

```
trait Draw {
    fn draw(&self);
}

struct Button {
    id: u32,
}

impl Draw for Button {
    fn draw(&self) {
        println!("{}", self.id);
    }
}

struct Select {
    id: u32,
}

impl Draw for Select {
    fn draw(&self) {
        println!("{}", self.id);
    }
}

fn main() {
    let elems: Vec<Box<dyn Draw>> = Vec![Box::new(Button {id:1}), Box::new(Select{id:2})];
    for e in elems {
        e.draw()
    }
}
```

### Box内存布局

```
let vec1 = vec![1, 2, 3, 4];
```

```
(stack)    (heap)
┌──────┐   ┌───┐
│ vec1 │──→│ 1 │
└──────┘   ├───┤
           │ 2 │
           ├───┤
           │ 3 │
           ├───┤
           │ 4 │
           └───┘
```

```
let arr = vec![Box::new(1), Box::new(2), Box::new(3), Box::new(4)];
```

```
                    (heap)
(stack)    (heap)   ┌───┐
┌──────┐   ┌───┐ ┌─→│ 1 │
│ vec2 │──→│B1 │─┘  └───┘
└──────┘   ├───┤    ┌───┐
           │B2 │───→│ 2 │
           ├───┤    └───┘
           │B3 │─┐  ┌───┐
           ├───┤ └─→│ 3 │
           │B4 │─┐  └───┘
           └───┘ │  ┌───┐
                 └─→│ 4 │
                    └───┘
```

```
fn main() {
    let arr = vec![Box::new(1), Box::new(2)];
    let (first, second) = (&arr[0], &arr[1]);
    let sum = **first + **second;
}
```

在这里，因为表达式无法解引用，所以需要显式地做两次解引用。first为`&Box::<i32>`类型，`*first`为`Box::<i32>`类型，`**first`为`i32`类型

### Box::leak

该函数可以消费掉Box，并强制目标值从内存中泄露。它的主要作用是改变堆中变量的生命周期，如：

```
fn main() {
    let s = gen_static_str();
    println!("{}", s);
}

fn gen_static_str() -> &'static str{
    let mut s = String::new();
    s.push_str("hello world");

    Box::leak(s.into_boxed_str())
}
```

这样可以将函数中创建的String转换为`&str`切片，且将其生命周期转为`'static`

## Deref解引用

### *运算

对一个智能指针对象y使用`*y`运算，实际上是进行了调用`*(y.deref())`。也因此`y.deref()`返回的应是一个普通的值引用，再被`*`解引用。采用这样的运算方法也可以防止在解引用时，左值直接获取对应值的所有权

### 函数和方法中的隐式Deref转换

若一个类型实现了Deref特征，那它的引用在传给函数或方法时会根据参数签名来决定是否进行隐式的Deref转换

```
fn main() {
    let s = String::from("hello world");
    display(&s);
}

fn display(s: &str) {
    println!("{}", s);
}
```

`&s`类型为`&String`，在传给display时自动解引用为`&str`。注意，只有引用类型的实参才会触发自动解引用，因此这里传入的参数必须是一个引用

#### 连续隐式Deref

```
fn main() {
    let s = Box::new(String::from("hello world"));
    display(&s);
}

fn display(s: &str) {
    println!("{}", s);
}
```

s本身是`Box<String>`类型，首先被deref为String，再被deref为`&str`

从这个例子也可以看出隐式deref的作用：若没有这个机制，display调用的方式应为

```
display(&(*s)[..]);    // *s为String，而&String为&str，[..]表示不定长
```

下面的例子体现了赋值中自动deref

```
fn main() {
    let s = Box::new(String::from("hello world"));
    let s1: &str = &s;
    let s2: String = s.to_string();
}
```

s经过两次隐式deref转为`&str`赋值给s1；经过两次deref转为`&str`，此后调用to_string转为String

#### Deref规则总结

- 一个类型为T的对象foo，若有`T: Deref<Target=U>`，那么`&foo`实际会转换为`&U`

- 引用归一化：Rust编译器会将所有对智能指针或`&&&v`（多重引用）类型的指针归一化为`&v`，再进行处理

- 把智能指针（Box Rc Arc Cow等）转换为内部的引用类型，即对于`Box<String>`转为`&String`

- 把多重引用`&&&v`转为`&v`

#### 三种Deref转换

可变引用和不可变引用间也可以互相转换，有下列形式

- 当`T: Deref<Target=U>`，可以将`&T`转换为`&U`，也可以将`&mut T`转换为`&U`
- 当`T: DerefMut<Target=U>`，可以将`&mut T`转换为`&mut U`

注意要实现`DerefMut`特征前必须实现`Deref`特征，因为DerefMut的定义为

`pub trait DerefMut: Deref`

## Drop释放资源

### 自动回收

不管是结构体内的成员还是结构体本身，Rust会在生命周期的末尾为每个成员插入Drop。当多个变量拥有同样的作用域时，drop顺序为：对于变量，先进后出；对于结构体内成员，按顺序方式，且会先调用结构体本身的Drop才调用结构体成员的Drop

### 手动回收

在一些情况下我们希望显式地调用Drop，如资源在使用时需要锁，使用完毕后想立即释放锁

```
struct Foo;

impl Drop for Foo {
    fn drop(&mut self) {
        println!("Dropping Foo");
    }
}

fn main() {
    let foo = Foo;
    drop(foo);
}
```

这里调用了drop函数显式析构，实际上是将foo的所有权拿走，使得编译器自动析构该函数

### Copy与Drop

一个类型**不可以同时实现Copy和Drop特征**，因为实现了Copy特征的变量会被编译器隐式复制，因此很难预测析构函数执行的时间

## Rc与Arc

### Rc<T>

Reference Count 引用计数。当希望在堆上分配一个对象供程序的多个部分使用，且无法确定哪个部分最后一个结束时，就可以使用Rc作为数据的所有者

#### 使用

```
use std::rc::Rc;

fn main() {
    let a = Rc::new(String::from("Hello World"));
    let b = Rc::clone(&a);

    println!("Ref count: {}", Rc::strong_count(&a));
}
```

可以使用strong_count获取引用计数的值

#### Rc::clone

注意，`Rc::clone`不是用于深拷贝，而是复制一份智能指针，并为引用计数加一。也可以用`a.clone`，但可读性较差，不建议使用

Rc实现了Drop特征，因此在离开变量作用域时会自动减少引用计数；当计数为0时将释放资源

#### 不可变引用

注意，`Rc<T>`是一个不可变引用，因此无法使用它来修改数据。这也符合Rust的借用规则：要么存在多个不可变借用，要么只能存在一个可变借用

### Arc<T>

Atomic Reference Count，基本特性和使用方式都与Rc相同，差别只是Arc具有原子性，因此支持多线程

Arc包的引用：

```
use std::sync::Arc;
```

### 总结

- Rc/Arc是不可变引用
- 当最后一个拥有者消失（引用计数清零），资源会被自动回收
- Rc只能用于同一线程内部，需要跨线程则需要Arc
- Rc/Arc是一个智能指针，实现了Deref特征，因此无需先解开Rc指针再使用里面的T，可以使用自动解引用的特性

## Cell与RefCell

为了给程序提供同时拥有可变引用和不可变引用的灵活性，Rust使用了Cell和RefCell。在底层这两者是通过unsafe实现的

### Cell

`Cell<T>`一般适用于T实现了Copy特征的情况

```
use std::cell::Cell;

fn main() {
    let c = Cell::new("hello");
    let one = c.get();
    c.set("world");
    let two = c.get();
    println!("{} {}", one, two);
}
```

get用于取值，set用于赋值

该程序结果为

```
hello world
```

可以看到，one和two的结果不同，因此这里one取得的是"hello"的拷贝，而two取得的是"world"的拷贝，这也是为什么Cell要求类型实现Copy特征

### RefCell

#### 基本用法

`RefCell<T>`较常用，因为其针对的是可变/不可变引用共存的问题

智能指针的出现对于Rust的变量所有权规则造成了一些改变，如下：

| Rust规则               | 智能指针带来的额外规则                |
| -------------------- | -------------------------- |
| 一个数据只有一个所有者          | Rc/Arc让一个数据可以拥有多个所有者       |
| 要么有多个不可变借用，要么有一个可变借用 | RefCell实现**编译期**可变、不可变引用共存 |
| 违背规则会导致编译错误          | 违背规则会导致运行时panic            |

但实际上，RefCell并没有打破Rust对于借用规则的限制，只不过把判断问题的时间滞后到了运行时

```
use std::cell::RefCell;

fn main() {
    let s = RefCell::new(String::from("hello world"));
    let s1 = s.borrow();
    let s2 = s.borrow_mut();
}
```

这段代码可以通过编译，但在运行时将panic，因为这里违反了不能同时拥有可变和不可变借用的规则

#### 内部可变性

使用RefCell包裹后，可以将一个不可变引用转换为一个可变引用，如

```
pub trait Messenger {
    fn send(&self, msg: String);
}

struct MsgQueue {
    msg_cache: RefCell< Vec<String> >,
}

impl Messenger for MegQueue {
    fn send(&self, msg: String) {
        self.msg_cache.borrow_mut().push(msg);
    }
}
```

这里self是一个非mut引用，但还是可以通过RefCell对内容进行修改

### Rc与RefCell的组合使用

通过组合Rc和RefCell，可以使得数据同时拥有多个所有者，且每个所有者都能对数据进行改变

```
use std::cell::RefCell;
use std::rc::Rc;

fn main() {
    let s = Rc::new( RefCell::new(String::from("hello ")) );
    let s1 = s.clone();
    let s2 = s.clone();
    s2.borrow_mut().push_str("world");
}
```

### 通过Cell:from_mut解决借用冲突

主要是新版rust引入的实用方法

- `Cell::from_mut` 可以将`&mut T`转为`&Cell<T>`
- `Cell::as_slice_of_cells` 可以将`&Cell<[T]>`转为`&[Cell<T>]`

下面通过一个例子介绍如何使用这两者解决借用冲突问题

```
fn is_even(i: i32) -> bool {
    i % 2 == 0
}

fn retain_even(nums: &mut Vec<i32>) {
    let mut i = 0;
    for num in nums.iter().filter(|&num| is_even(*num)) {
        nums[i] = *num;
        i += 1;
    }
    nums.truncate(i);
}
```

上述代码无法通过编译，因为在filter存在一个不可变引用，而在nums存在一个可变引用，可以修改如下

```
use std::cell::Cell;

fn retain_even(nums: &mut Vec<i32>) {
    let slice: &[Cell<i32>] = Cell::from_mut(&mut nums[..])
                                .as_slice_of_cells();
    let mut i = 0;
    for num in slice.iter().fliter( |num| is_even(num.get()) ) {
        slice[i].set(num.get());
        i += 1;
    }
    nums.truncate(i);
}
```

这里先将`mut& [i32]`类型转换为`&Cell<[i32]>`，再转换为`&[Cell<i32>]`，此后就可以将每个作为`Cell<T>`使用了

### 总结

- Cell用于实现了Copy特征的值，RefCell用于引用
- RefCell只是将借用规则的判断时机从编译器推迟到运行期
- RefCell适用于编译期误报或者一个引用被在多处代码使用、修改以至于难于管理借用关系时
- 使用RefCell时，违背借用规则会导致运行期的panic

# 深入类型

## 类型转换

### as转换

数值类型，在转换时应注意值的表示范围，否则会发生溢出

```
let a = 3.1 as u8;
let b = 100_i8 as i32;
let c = 'a' as u8;
```

内存地址转为指针

```
let mut values: [i32; 2] = [1, 2];
let p1: *mut i32 = values.as_mut_ptr();        // 转为指针
let first_address = p1 as usize;            // 指针地址转为整数
let second_address = first_address + 4;        // 第二个元素的内存地址
let p2 = second_address as *mut i32;        // 转回指针
unsafe {
    *p2 += 1;            // 使用unsafe特性，通过指针对第二个元素进行修改
}
```

**注意**：as转换没有传递性，即若a可以转为t1，t1可以转为t2，也不代表a可以直接转换为t2

### TryInto转换

TryInto会捕获类型转换时的溢出等错误

```
fn main() {
    let b: i16 = 1500;
    let b_: u8 = match b.try_into() {
        Ok(b1) => b1,
        Err(e) => {
            println!("{:?}", e.to_string());
            0
        }
    }
}
```

### 通用类型转换

#### 强制类型转换

某些情况下rust可以进行隐式强制转换，但下列情况不会进行隐式强制转换

- 匹配特征时不会做强制转换。即类型T可以强制转换为U，不代表`impl T`可以强制转换为`impl U`

#### 点操作符

##### 类型推导规则

点操作符在调用时会进行隐式类型转换，以求找到合适的方法。

假设value为T类型，则在调用`value.foo()`时会依次做下列处理

- 值方法调用 检查是否可以直接调用`T::foo(value)`
- 引用方法调用 检查是否可以调用`<&T>::foo(value)`或`<&mut T>::foo(value)`
- 解引用方法调用 尝试对T使用`Deref`解引用，若可以被解引用为U，则编译器使用U类型重复进行上述尝试
- 若T不能被解引用，且T是定长类型，则会尝试将T转换为不定长类型，如`[i32; 2]`转换为`[i32]`
- 若都没有找到匹配，则报错

##### 几个例子

###### 1

```
let array: RC<Box<[T; 3]>> = ...;
let first_entry = array[0];
```

- array[0]作为语法糖，被转换为`array.index(0)`

- 值方法调用 检查`RC<Box<[T; 3]>>`是否实现了Index特征，结果为否

- 引用方法调用 检查`&RC<Box<[T; 3]>>`和`&mut RC<Box<[T; 3]>>`是否实现了Index特征，也为否

- 解引用方法调用 检查`Box<[T; 3]>`是否实现了Index特征，并递归地以上面方式检查该类型

- 值方法，检查`Box<[T; 3]>`

- 引用方法，检查`&Box<[T; 3]>` `&mut Box<[T; 3]>`

- 解引用方法，检查`[T; 3]`

- 值方法，检查`[T; 3]`

- 引用方法，检查`&[T; 3]` `&mut [T; 3]`

- 解引用方法——无法解引用

- 尝试转换为不定长类型`[T]`，它实现了Index特征，找到对应特性，返回

```
fn do_stuff<T: Clone>(value: &T) {
    let cloned = value.clone();
}
```

这里存在特征约束，因此T本身肯定可以通过值方法调用clone，但这里若将特征约束去除，且T没有实现Clone特征，则编译器会先尝试引用方法调用：将T变为&T，此时clone方法变为`fn clone(&&T) -> &T`，此时&T实现了Clone特征（因为所有引用类型都可以被复制）

###### 2

```
![derive(Clone)]
struct Container<T>(Arc<T>);

fn clone_containers<T>(foo: &Container<i32>, bar: &Container<T>) {
    let foo_cloned = foo.clone();
    let bar_cloned = bar.clone();
}
```

复杂类型派生Clone的规则：一个复杂类型能否派生Clone，需要它内部的所有子类型都能进行Clone。对于上例，foo的类型可以确定，且i32可以被clone，因此编译器会为其生成下列clone函数

```
impl<T> Clone for Container<T> where T: Clone {
    fn clone(&self) -> Self {
        Self(Arc::clone(&self.0))
    }
}
```

可以看到，clone函数的实现最终依赖于泛型本身是否可以被clone（`Arc::clone(&self.0)`），因此编译器没有为`Container<T>`实现Clone特征，因此bar在调用clone时按照引用方法，所以bar_cloned的类型是`&Container<T>`

### unsafe的类型转换

这是Rust的非安全行为（unsafe）

#### transmutes

使用`mem::transmutes<T, U>`将T直接转换为U，唯一的限制是两种类型的大小相同

- 将&转换为&mut是UB（undefined behavior）
- 变形为一个未指定生命周期的引用会导致无界生命周期
- 复合类型间的转换需要保证内存排布，对于rust来说`repr(C)`和`repr(transparent)`的排布有精确定义，但`repr(Rust)`没有直接的限制

#### transmute_copy

使用`mem::transmute_copy<T, U>`从T类型中拷贝出U类型所需的字节数，而后转换为U。若U比T大，则UB

#### 应用

将裸指针变为函数指针

```
fn foo() -> i32 {
    0
}

let pointer = foo as *const ();
let function = unsafe {
    std::mem::transmute::<*const (), fn() -> i32>(pointer)
};
```

延长生命周期或缩短一个静态生命周期变量的寿命

```
struct R<'a>(&'a i32);

// 将生命周期从b延长到static
unsafe fn extend_lifetime<'b>(r: R<'b>) -> R<'static> {
    std::mem::transmute::<R<'b>, R<'static>>(r)
}

// 将生命周期从static缩短到c
unsafe fn shorten_invariant_lifetime<'b, 'c>(r: &'b mut R<'static>) -> &'b mut R<'c> {
    std::mem::transmute::<&'b mut R<'static>, &'b mut R<'c>>(r)
}
```

## newtype和类型别名

### newtype

newtype就是用元组结构体的方式将已有的类型包裹起来

```
struct Meters(u32);
```

主要有如下几个作用

#### 为外部类型实现外部特征

由于[孤儿规则](https://yuque.antfin-inc.com/hxy69c/ozpx3f/oykugc6qe9ht8xrh#TAv2p)，要为类型A实现特征T，则A和T必须至少有一个在当前的作用范围内

如要格式化输出Vec，需要为其实现Display特征，可以使用newtype进行包裹

```
use std::fmt;

struct Wrapper(Vec<String>);

impl fmt::Display for Wrapper {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "[{}]", self.0.join(", "));
    }
}
```

这里通过创建名为Wrapper的newtype实现对Vec的格式化输出

#### 更好的可读性及类型异化

通过newtype可以为同一基础类型的数据创建不同的自定义类型，使得可读性更好，也可以通过类型系统对传入的参数类型进行约束

#### 隐藏内部类型的细节

可以将一些内部方法包裹在newtype中，使得调用者无法直接调用

### 类型别名

类似C/C++的typedef

```
type Meters = u32;
```

类型别名也可以指定特征、生命周期等（这些都包含在Rust的类型签名中）。下例为一个生命周期为static的Send特征对象创建了别名

```
type Thunk = Box<dyn Fn() + Send + 'static>;
```

标准库中的一个常见应用是简化`Result<T, E>`枚举。如在`std::io`库中需要定义自己的错误类型`std::io::Error`，那么Result的类型模板为`std::result::Result<T, std::io::Error>`。为了简化可以直接定义

```
type Result<T> = std::result::Result<T, std::io::Error>;
```

### 永不返回类型

`!`用于表示一个函数永不返回任何值

## Sized和不定长DST

### 动态大小类型DST

Dynamically sized types，这种类型无法在编译器得知类型值的大小，只有在程序运行时才能动态获知。一般动态大小类型底层数据保存在堆上，栈上只存有一个引用类型

常见的DST有：

- 切片 所有的切片类型都是DST
- str 字符串类型，一般在定义一个字符串时使用`let s: &str = "aaa";`而不是`let s: str = "aaa";`，就是因为str是动态类型，无法直接定义
- 特征对象 使用`Box<dyn T>`声明

上述DST都无法直接单独被使用，而是需要通过引用或Box间接使用

### Sized特征

Sized特征用于指出函数参数类型是定长的。对于大多数函数来说都会默认地添加该约束，但也可以显式添加

```
fn generic<T: Sized>(t: T) {

}
```

若参数可能不是固定大小，则可以添加约束。注意，此时传入的必须是引用类型

```
fn generic<T: ?Sized>(t: &T) {

}
```

### Box<str>

如特征对象的实现方式，Box可以将DST包裹为一个Sized对象，同样对于动态对象str也提供了使用Box包裹的方法

```
let s: Box<str> = "Hello".into();
```

## 枚举与整数

有时候需要通过枚举与整数的对比来决定控制流，在C/C++中实现很简单，但由于Rust的安全限制，如下代码：

```
enum MyEnum {
    A = 1,
    B,
    C,
}

fn main() {
    // 将枚举转换成整数，顺利通过
    let x = MyEnum::C as i32;

    // 将整数转换为枚举，失败
    match x {
        MyEnum::A => {}
        MyEnum::B => {}
        MyEnum::C => {}
        _ => {}
    }
}
```

在match x处将i32和MyEnum进行了比较，报错。因此需要通过一些方法才可以实现

### 第三方库

num-traits、num-derive或num_enums

### TryFrom+宏

首先为enum类型实现TryFrom特征，定义i32到MyEnum的转换

```
use std::convert::TryFrom;

impl TryFrom<i32> for MyEnum {
    type Error = ();

    fn try_from(v: i32) -> Result<Self, Self::Error> {
        match v {
            x if x == MyEnum::A as i32 => Ok(MyEnum::A),
            x if x == MyEnum::B as i32 => Ok(MyEnum::B),
            x if x == MyEnum::C as i32 => Ok(MyEnum::C),
            _ => Err(()),
        }
    }
}
```

则可以使用TryInto特征来实现转换，避免本节开头例子的报错

```
use std::convert::TryInto;

fn main() {
    let x = MyEnum::C as i32;

    match x.try_into() {
        Ok(MyEnum::A) => println!("a"),
        Ok(MyEnum::B) => println!("b"),
        Ok(MyEnum::C) => println!("c"),
        Err(_) => eprintln!("unknown number"),
    }
}
```

但上述方法在定义TryFrom时需要遍历每个枚举值，因此可以使用宏来简化（宏编程的内容看后面章节）

```
#[macro_export]
macro_rules! back_to_enum {
    ($(#[$meta:meta])* $vis:vis enum $name:ident {
        $($(#[$vmeta:meta])* $vname:ident $(= $val:expr)?,)*
    }) => {
        $(#[$meta])*
        $vis enum $name {
            $($(#[$vmeta])* $vname $(= $val)?,)*
        }

        impl std::convert::TryFrom<i32> for $name {
            type Error = ();

            fn try_from(v: i32) -> Result<Self, Self::Error> {
                match v {
                    $(x if x == $name::$vname as i32 => Ok($name::$vname),)*
                    _ => Err(()),
                }
            }
        }
    }
}

back_to_enum! {
    enum MyEnum {
        A = 1,
        B,
        C,
    }
}
```

### std::mem::transmute

可以使用transmute直接进行unsafe转换

```
#[repr(i32)]        // 用于控制底层类型的大小
enum MyEnum {
    A=1,
    B,
    C,
}

fn main() {
    let x = MyEnum::C;
    let y = x as i32;
    let z: MyEnum = unsafe { std::mem::transmute(y) };

    match z {
        MyEnum::A => { println!("Found A"); }
        MyEnum::B => { println!("Found B"); }
        MyEnum::C => { println!("Found C"); }
    }
}
```

# Unsafe Rust

unsafe并不意味着完全的不执行编译器检查。unsafe主要提供五种场景下的功能

## 解引用裸指针

有两种裸指针

- 可变 `*mut T`
- 不可变 `*const T`

裸指针有如下的特性

- 可以绕过借用规则，对于数据拥有多少可变/不可变指针没有限制
- 不保证指向合法内存
- 可以是null
- 没有实现任何自动回收（drop）

### 裸指针的创建

创建裸指针不需要指定unsafe，因为只有解引用裸指针的操作是unsafe的

#### 基于引用创建裸指针

```
let mut num = 5;
let r1 = &num as *const i32;    // 不可变裸指针
let r2 = &num as *mut i32;        // 可变裸指针

let r3: *const i32 = &num;        // 也可以使用隐式转换的方式声明
```

#### 基于内存地址创建裸指针

```
let address = 0x012345usize;
let r = address as *const i32;
```

#### 基于智能指针创建裸指针

```
let a: Box<i32> = Box::new(10);
let b: *const i32 = &*a;                // 直接创建
let c: *const i32 = Box::into_raw(a);    // 使用into_raw创建
```

### 裸指针的解引用

与C/C++类似，直接使用`*`运算进行解引用

```
let a = 1;
let b: *const i32 = &a;
unsafe {
    println!("{}", *b);
}
```

## 调用unsafe函数或方法

可以在fn前加入unsafe关键字定义一个unsafe函数，在调用时需要包裹在unsafe块中

```
unsafe fn dangerous() {}
fn main() {
    unsafe {
        dangerous();
    }
}
```

### 用安全抽象包裹unsafe代码

本节举了一个例子，从代码逻辑上保证了安全性，虽然使用了unsafe特性，但不必将函数本身声明为unsafe

考虑要将一个数组分为两个切片，且每个切片可变的情况。这种情况使用正常特性肯定报错，因为对同一个数组含有两个可变借用。但实际上两者访问的内存区域并不相同，因此从逻辑上并无安全问题

```
use std::slice;

fn split_at_mut(slice: &mut [i32], mid: usize) -> (&mut [i32], &mut [i32]) {
    let len = slice.len();
    let ptr = slice.as_mut_ptr();

    assert!(mid <= len);        // 边界检查，保证了访问不越界

    unsafe {
        (
            slice::from_raw_parts_mut(ptr, mid),
            slice::from_raw_parts_mut(ptr.add(mid), len-mid);
        )
    }
}
```

## FFI

Foreign Function Interface，用于与其他语言进行交互

### 调用

常见的就是调用C语言的函数

```
extern "C" {
    fn abs(input: i32) -> i32;
}

fn main() {
    unsafe {
        println!("{}", abs(-3));
    }
}
```

### 被调用

可以采用该方式声明一个可以被其他语言调用的接口

```
#[no_mangle]            // 该宏保证了函数不会被编译器重命名或修饰
pub extern "C" fn call_from_c() {
    println!("hello");
}
```

## 访问或修改一个可变的静态变量

因为全局静态变量在多线程可能有同步问题，因此认为是unsafe的

```
static mut REQUEST_RECV: usize = 0;
fn main() {
    unsafe {
        REQUEST_RECV += 1;
    }
}
```

## 实现unsafe特征

若特征包含有编译器无法验证的内容，则需要用到unsafe特征，如有时编译器无法验证类型是否在线程间安全传递

```
unsafe trait Foo {

}

unsafe impl Foo for i32 {

}

fn main() {

}
```

## 访问union字段

主要用于与C代码进行交互，因此Rust的union有与C的union类似的结构

Rust无法保证当前存储在union中的数据类型，因此认为是不安全的

```
#[repr(C)]
union MyUnion {
    f1: u32,
    f2: f32,
}

fn main() {
    let u = MyUnion{ f1: 1 };
    let f = unsafe {u.f1};
}
```

# 宏

https://veykril.github.io/tlborm/introduction.html 这本书对宏进行了系统的讲解

Rust中的宏分为3种：

- 声明式宏 `macro_rules!`

- 过程宏

- 派生宏 `#[derive]`

- 类属性宏

- 类函数宏

## 声明式宏

声明式宏可以写出一种类似`match`的代码，将表达式与多个模式匹配，一旦匹配了某模式，则与特定的代码相关联

### 简化版的vec!

```
#[macro_export]
macro_rules! vec {
    ( $( $x:expr ),* ) => {
        {
            let mut temp_vec = Vec::new();
            $(
                temp_vec.push($x);
            )*
            temp_vec
        }
    };
}
```

#### 模式

```
( $( $x:expr ),* )
```

- 外层括号用于包裹宏模式
- `$()` 用于捕获，与其中表达式相匹配的值会被捕获
- `$x:expr` 可以捕获任何rust表达式，并给予该模式一个名称`$x`
- `,` 说明表达式后存在一个可选的逗号分隔符
- `*` 表示之前的模式会被匹配任意次

若调用`vec![1, 2, 3]`，则`$x`会被匹配3次，分别为`1 2 3`

#### 关联代码

```
{
    let mut temp_vec = Vec::new();
    $(
        temp_vec.push($x);
    )*
    temp_vec
}
```

`$()`中的代码会根据模式匹配的次数生成，因此对于`vec![1, 2, 3]`生成的代码为

```
{
    let mut temp_vec = Vec::new();
    temp_vec.push(1);
    temp_vec.push(2);
    temp_vec.push(3);
    temp_vec
}
```

## 过程宏

过程宏以源码作为输入参数，基于代码进行一系列操作后，再输出一段全新的代码

### 创建

创建过程宏时，其定义必须放入一个独立的包中

```
use proc_macro;

#[proc_macro_derive(HelloMacro)]
pub fn some_name(input: TokenStream) -> TokenStream {
}
```

### 自定义derive过程宏

derive宏只能用于结构体和枚举

该例子稍微有点复杂，因为过程宏需要在工程下新建一个包，见https://course.rs/advance/macro.html#%E8%87%AA%E5%AE%9A%E4%B9%89-derive-%E8%BF%87%E7%A8%8B%E5%AE%8F

derive过程宏的输入是rust字符串，可以使用syn包先将其解析为AST。该例子实现的操作是为结构体添加接口HelloMacro的实现，因此一般的调用如下

```
#[derive(HelloMacro)]
struct Sunface;
```

过程宏定义如下：

```
#[pub_macro_derive(HelloMacro)]
pub fn hello_macro_derive(input: TokenStream) -> TokenStream {
    let ast:DeriveInput = syn::parse(input).unwrap();

    impl_hello_macro(&ast)
}
```

这里先对传入的rust代码进行解析，再将AST传入impl_hello_macro中进行实际的处理

```
fn impl_hello_macro(ast: &syn::DeriveInput) -> TokenStream {
    let name = &ast.ident;
    let gen = quote! {
        impl HelloMacro for #name {
            fn hello_macro() {
                println!("Hello, Macro! My name is {}!", stringify!(#name));
            }
        }
    };
    gen.into()
}
```

`quote!`宏可以返回对应的rust代码，但最后需要`into`方法将代码转为`TokenStream`。`#name`可以引用宏外定义的变量，可以见https://docs.rs/quote

上述宏展开后如下，注意这里有一段println!展开后的结果

```
struct Sunface;
impl HelloMacro for Sunface {
    fn hello_macro() {
        {
            ::std::io::_print(
                ::core::fmt::Arguments::new_v1(
                    &["Hello, Macro! My name is ", "!\n"],
                    &[::core::fmt::ArgumentV1::new_display(&"Sunface")],
                ),
            );
        };
    }
}
```

### 类属性宏

与derive宏类似，但可以用于函数等

```
#[route(GET, "/")]
fn index() {
}
```

其中route属性就可以看作一个过程宏

```
#[proc_macro_attribute]
pub fn route(attr: TokenStream, item: TokenStream) -> TokenStream {
}
```

类属性宏有两个参数

- attr用于说明属性包含的内容（Get "/"）
- item是属性所标注的类型项，即`fn index() {}`

### 类函数宏

类函数宏则与macro_rules类似，用于定义像函数那样调用的宏。但与后者采用的类似match的语法不同，类函数宏的定义形式类似derive

```
#[proc_macro]
pub fn sql(input: TokenStream) -> TokenStream {
}
```

### 一个例子

因为整个流程稍微比较麻烦，且macro目前需要单独创建一个包（据说以后会改），这里用一个自己测试过的例子来演示（Rust语言圣经中的一个例子，但原示例代码有些错误）

该例为一个derive宏，用于给结构体赋初值

#### 项目目录

首先目录如下：

```
├── Cargo.lock
├── Cargo.toml
├── macro_mydefault
│   ├── Cargo.toml
│   └── src
│       └── lib.rs
├── src
│   ├── lib.rs
│   └── main.rs
```

#### 主函数

主函数如下

```
use macro_mydefault::MyDefault;

#[derive(MyDefault)]
struct SomeData(u32, String);

#[derive(MyDefault)]
struct User {
    name: String,
    data: SomeData
}

fn main() {
}
```

这里需要为项目添加macro_mydefault依赖

```
[package]
name = "macro_test"
version = "0.1.0"
edition = "2021"

# See more keys and their definitions at https://doc.rust-lang.org/cargo/reference/manifest.html

[dependencies]
macro_mydefault = { path = "./macro_mydefault" }
```

#### 宏包

对于macro_mydefault包，其依赖于syn和quote包，且需要为其打开proc-macro开关

```
[package]
name = "macro_mydefault"
version = "0.1.0"
edition = "2021"

# See more keys and their definitions at https://doc.rust-lang.org/cargo/reference/manifest.html
[lib]
proc-macro = true

[dependencies]
syn = "1.0"
quote = "1.0"
```

最后需要实现MyDefault宏

```
extern crate proc_macro;
use proc_macro::TokenStream;
use quote::quote;
use syn::{self, Data};
use syn::DeriveInput;

#[proc_macro_derive(MyDefault)]
pub fn my_default(input: TokenStream) -> TokenStream {
    let ast: DeriveInput = syn::parse(input).unwrap();
    let id = ast.ident;

    let Data::Struct(s) = ast.data else {
        panic!("MyDefault derive macro must use in struct");
    };

    let mut field_ast = quote!();

    for (idx, f) in s.fields.iter().enumerate() {
        let (field_id, field_type) = (&f.ident, &f.ty);

        if field_id.is_none() {
            let field_idx = syn::Index::from(idx);
            field_ast.extend(quote! {
                #field_idx: #field_type::default(),
            });
        } else {
            field_ast.extend(quote! {
                #field_id: #field_type::default(),
            });
        }
    }

    quote! {
        impl Default for #id {
            fn default() -> Self {
                Self {
                    #field_ast
                }
            }
        }
    }.into()
}
```

#### 测试

可以使用`cargo expand --bin macro_test`对宏展开情况进行测试，bin参数用于指定需要测试的包

```
#![feature(prelude_import)]
#[prelude_import]
use std::prelude::rust_2021::*;
#[macro_use]
extern crate std;
use macro_mydefault::MyDefault;
struct SomeData(u32, String);
impl Default for SomeData {
    fn default() -> Self {
        Self {
            0: u32::default(),
            1: String::default(),
        }
    }
}
struct User {
    name: String,
    data: SomeData,
}
impl Default for User {
    fn default() -> Self {
        Self {
            name: String::default(),
            data: SomeData::default(),
        }
    }
}
fn main() {
    {
        ::std::io::_print(format_args!("Hello, world!\n"));
    };
}
```

可以看到两个struct都初始化为对应的default值

# 参考资料

https://doc.rust-lang.org/book 官方的教程（本笔记的主要参考）

https://course.rs/about-book.html 一份不错的教程（本笔记后半部分的主要参考，基本涵盖了上一个链接的内容）

https://zh.practice.rs/basic-types/numbers.html 一个不错的练习平台

https://study.antgroup-inc.cn/learn/course/2038 内网一个很不错的rust视频，是occlum团队分享的，解释了rust的设计思想

[https://zhuanlan.zhihu.com/p/36590567](https://zhuanlan.zhihu.com/p/365905673) 一篇rust设计思想的总结文章https://blog.frognew.com/archives.html

https://www.oschina.net/translate/exploring-dynamic-dispatch-in-rust?print 深入理解 Rust 的动态分派模型

系列文章 Rust 探索系统本原 https://mp.weixin.qq.com/mp/appmsgalbum?__biz=MzA3NDM0ODQwMw==&action=getalbum&album_id=1701598373151047686&scene=173&from_msgid=2649828922&from_itemidx=1&count=3&nolastread=1#wechat_redirect 该系列文章前四篇为一些rust软件工程的设计与调优方面的文章，后四篇比较深入地讲了rust，因此以下只记录了后四篇

https://mp.weixin.qq.com/s/ZA-_BARVAWe0Q4eM0lYgwg Rust 探索系统本原 | 编程语言

https://mp.weixin.qq.com/s/jaKjzc_1rkDe67rfpnFTgg Rust 探索系统本原 | RAII

https://mp.weixin.qq.com/s/1juaadR3AqHa8H19sHWHmQ Rust 探索系统本原 | 内存管理

https://mp.weixin.qq.com/s/SJsEurfZr4TG-I3rncid5A Rust 探索系统本原 | 泛型

https://rustmagazine.github.io/rust_magazine_2021/chapter_4/ant_trait.html 蚂蚁集团 Trait使用及实现分析

https://rustmagazine.github.io/rust_magazine_2021/chapter_6/ant-rust-data-layout.html 蚂蚁集团 rust数据内存布局

https://iandouglasscott.com/2018/05/28/exploring-rust-fat-pointers/ 探寻rust胖指针
