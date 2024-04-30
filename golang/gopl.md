[Go语言圣经 - Go语言圣经 (golang-china.github.io)](https://golang-china.github.io/gopl-zh/index.html) 

# Go语法

## 程序结构

### 命名

* 作用域
  
  * 函数内部定义  作用域为函数内
  
  * 函数外部定义  作用域为包内，**若名字开头字母为大写，则可以在包外访问**

### 声明

#### 变量

##### 基本声明

```go
var var_name var_type = expression
```

其中类型和表达式可以省略其中的一个（因为只要有一个就可以推导变量类型）

示例：

```go
var i, j, k int                    // 定义3个int，默认初始化为0
var b, f, s = true, 2.3, "four"    // 分别定义bool float string类型
```

还可以通过调用函数的返回值来初始化

```go
var f, err = os.Open(name)
```

##### 简短变量声明

```go
var_name := expression
```

注意，若var_name先前已经定义过，则`:=` 与赋值相同，且左值中必须包含一个新的变量。因此下例会报错

```go
f, err := os.Open(infile)
f, err := os.Open(outfile)    // 报错
```

此外，需要注意使用`:=`的左值会创建新的变量，因此如下例

```go
var cwd string

func init() {
    cwd, err := os.Getwd()    // 不会更新全局的cwd变量
}
```

##### 指针

语法与C的指针类似

```go
x := 1
p = &x
*p = 2
```

##### new函数

用法与C++类似

```go
p := new(int)
```

go是带GC的语言，因此new实际上与变量声明的语义是一致的，即下面两个函数一致

```go
func newInt() *int {        // 使用new声明新的int
    return new(int)
}

func newInt() *int {        // 使用变量声明新的int
    var num int
    return &num
}
```

##### 变量的生命周期

由于go有GC，因此局部变量的生命周期取决于程序对该内存的引用。全局变量则与整个程序的运行周期一致

注意，存在GC不代表go不存在栈变量。实际上go会根据函数中变量的返回情况决定内存的分配位置（与是否使用了new关键字无关），如下例

```go
var global *int

func f() {
    var x int = 1
    global = &x        // 这里外部引用了函数内变量，称为逃逸。变量在堆上
}

func g() {
    y := new(int)
    *y = 2            // 这里虽然使用了new，但不存在变量逃逸，因此变量在栈上
}
```

#### 常量

##### 基础声明

```go
const pi = 3.14159
const (
    e = 2.718
    e_2 = e*2
)
```

若const块中忽略初始化表达式，则值沿用块中的上一个值

```go
const (
    a = 1
    b
    c = 2
    d
)        // a=1 b=1 c=2 d=2
```

##### 常量生成器

可以使用iota初始化顺序常量

```go
const (
    sunday int = iota    // 0
    monday               // 1
    tuesday              // 2
)
```

还可以使用iota初始化位域常量

```go
const (
    ZF uint = 1 << iota    // 1<<0
    CF                     // 1<<1
    OF                     // 1<<2
    PF                     // 1<<3
)
```

##### 无类型常量

为了表示更高的精度，go可以定义无类型常量。如：

```go
var x float32 = math.Pi
var y float64 = math.Pi
var z complex128 = math.Pi
```

这里math.Pi是个无类型常量，可以赋值给支持对应转换的变量

```go
const (
    deadbeef = 0xdeadbeef    // 定义无类型常量
    a = uint32(deadbeef)
    b = float32(deadbeef)
)
```

#### 类型

类似C的typedef，可以为现有类型定义别名

```go
type alias_name type_name
```

例如

```go
type barray []byte
```

若两种类型拥有相同的底层类型，则默认可以进行显式强制类型转换

```go
type f1 float64
type f2 float64

var var_f1 f1
var var_f2 f2 = f2(var_f1)
```

#### 函数

```go
func func_name(params) results {
    ...
}
```

其中，若多个参数类型一致，则可以省略部分类型声明

```go
func f(i, j, k int, s, t string)
```

此外可以对返回值进行命名

```go
func sub(x, y int) (z int) {z = x-y; return}
```

可以定义多返回值

```go
func Size(rect image.Rectangle) (width, height int)
```

### 赋值

go支持元组赋值，即

```go
x, y = y, x
```

可以使用`_`忽略元组赋值中不需要的变量

```go
_, y = y, x
```

### 包和文件

#### 声明与创建

```go
package pkg_name
```

使用该语句声明包，存在同样声明的被视为同一个包，并且可以在包内引用相关的变量和函数

#### 导入

```go
import pkg_name
```

#### 初始化

默认情况下，go编译器会将所有package相关的文件排序，并顺序执行初始化代码。也可以在包中定义一个init函数来定义初始化过程

```go
func init() {
    ...
}
```

### 作用域

作用域是compile-time概念，生命周期是runtime概念

go的作用域大多数与其他语言类似

* 内部块的声明可以屏蔽外部块的声明

## 基础数据类型

### 整型

#### 基础类型

| 无符号   | 有符号    |
| ----- | ------ |
| int8  | uint8  |
| int16 | uint16 |
| int32 | uint32 |
| int64 | uint64 |

此外，还有几种底层类型也是整型的基础类型

* rune  等价于int32，用于表示unicode字符

* int uint  机器相关，可能是32或64位

* uintptr  用于存放指针

#### 运算

支持的运算符及其优先级如下：

```
*      /      %      <<       >>     &       &^
+      -      |      ^
==     !=     <      <=       >      >=
&&
||
```

其中&^为位清空的意思

### 浮点数

有float32和float64两种类型。此外math包提供了一系列特殊的浮点数

```
math.MaxFloat32  math.MaxFloat64  math.NaN()  math.Inf(1)  math.Inf(-1)
```

### 复数

有complex64和complex128两种类型（对应两种float）。使用`real()  imag()`访问实部和虚部

### 布尔

bool，取值为true和false

### 字符串

#### 定义

```go
s := "hello world"
var s string = "hello world"
```

默认都是UTF8编码的，rune类型的**只读**数组

#### 基本用法

字符串本质是数组，因此可以使用**切片**

由于字符串是只读的，字符串操作可能涉及很多内存的重新分配，因此可以使用bytes.Buffer直接对string内存进行操作

#### 与数字转换

```go
// int -> string
x := fmt.Sprintf("%d", 123)
y := strconv.Itoa(123)

// string -> int
a, err := strconv.Atoi("123")
b, err := strconv.ParseInt("123", 10, 64)    // 十进制，int64
```

## 复合数据类型

### 数组

#### 声明

下面是数组的几种声明方式

```go
var a [3]int
var b [3]int = [3]int{1,2,3}
c := [...]int{1,2,3}              // 长度为...时，根据初始化值确定数组大小
d := [...]string{0:"H", 1:"E", 2:"L"}    // 使用下标初始化
```

#### 运算

##### 比较运算

在比较运算中，两个数组必须长度一致且内容一致才会被认为相同

### slice

#### 基本语法

切片是对数组子序列的引用，语法与python的切片相同

```go
var q1 []string
months := [...]string{1:"Jan", 2:"Feb", 3:"Mar", 4:"Apr"}
q1 = months[:4]
```

#### 基本操作

切片允许对引用的数组内容进行修改

切片间不支持直接的比较操作，只能使用`bytes.Equal`函数进行比较。原因见[Slice - Go语言圣经 (golang-china.github.io)](https://golang-china.github.io/gopl-zh/ch4/ch4-02.html#42-slice)

唯一合法的比较是与`nil`比较，但判断slice是否为空应该使用`len(s)==0`而非`s==nil`

可以使用make创建slice。注意这里创建的实际上也是一个数组，包含缓冲区

```go
make([]T, len, cap)    // T为对应类型，cap参数可省略
```

可以使用append函数将内容添加到数组末尾

```go
var x []int
x = append(x, 1)
x = append(x, 1, 2)    // 添加多个元素
x = append(x, x...)    // 添加切片
```

#### slice内存技巧

由于slice实际上是对数组的引用，因此可以通过一些技巧来操作slice对应的内存。如下例的模拟栈

```go
stack = append(stack, v)      // push
top := stack[len(stack)-1]    // 栈顶元素
stack = stack[:len(stack)-1]  // pop，实际上只是改了slice大小

func remove(slice []int, i int) []int {
    copy(slice[i:], slice[i+1:])    // 将i+1后的元素覆盖到i处
    return slice[:len(slice)-1]
}
```

### Map

#### 创建

```go
ages := make(map[string]int)
ages := map[string]int {
    "alice": 10,
    "bob": 11,
}
```

#### 增删改查

```go
ages["jack"] = 12
delete(ages, "jack")
ages["bob"] += 1
bob_age, ok = ages["bob"]
```

#### 比较

map同样无法比较，但可以通过nil判空

```go
var ages map[string]int
fmt.Println(ages==nil)    // true
fmt.Println(len(ages)==0) // true
```

当map为nil时不能向其添加元素，而需要使用make创建

### 结构体

#### 声明与访问

若成员名开头为大写，则说明是一个导出的成员，否则为私有

```go
type Employee struct {
    ID    int
    Name  string
}
```

结构体可以使用字面值进行初始化

```go
bob := Employee{100, "bob"}
bob := Employee{ID:100, Name:"bob"}
```

可以直接通过`.`访问，也可以取址后访问

```go
bob.ID = 100        // 直接访问

ref_id = &bob.ID    // 通过地址访问
*ref_id = 200

ref_bob = &bob      // 若直接引用结构体，则在使用.访问成员时会自动解引用
id := ref_bob.ID
```

#### 比较

若结构体的所有成员都是可以比较的，则结构体也可以比较，同样使用`==` `!=`运算符

#### 结构体嵌入与匿名成员

实际使用中经常会遇到需要嵌套结构体的情况

```go
type Point struct {
    X, Y int
}

type Circle struct {
    Center Point
    Radius int
}

type Wheel struct {
    Circle Circle
    Spokes int
}
```

此时要访问Wheel结构的X属性，只能使用`Wheel.Circle.Center.X`。Go提供了一种语法糖，可以让此类结构嵌入到大的结构体中

```go
type Circle struct {
    Point
    Radius int
}

type Wheel struct {
    Circle
    Spokes int
}
```

其中Circle和Point称为匿名成员。此时可以直接通过`Wheel.X`访问成员。但不能直接使用`Wheel{X:10}`初始化X属性，而是必须遵循结构体的嵌套结构

```go
w := Wheel{ Circle{ Point{8, 8}, 5}, 20 }
w := Wheel{
    Circle: Circle{
        Point: Point{X:8, Y:8},
        Radius: 5,
    },
    Spokes: 20,
}
```

使用匿名成员时需要注意如下问题：

* 不能包含两个类型相同的匿名成员，因为会导致名字冲突

* 匿名成员对于包外部的可见性取决于结构体是否导出以及成员本身是否导出

### JSON

主要使用的方法

```
json.Marshal        // 序列化
json.MarshalIndent  // 序列化为包含缩进的json
json.Unmarshal      // 反序列化

json.Decoder        // 流式解析json
json.Encoder
```

## 函数

### 错误

go在错误处理的典型编码风格为：先检查是否有错误，若有则直接处理，无错误的逻辑不写在else，而是直接写在函数体中

#### 错误处理策略

错误处理的五种常用策略如下：

* 传递错误，可以直接传递也可以格式化后传递

```go
resp, err := http.Get(url)
if err != nil {
    return nil, err
}
```

* 重试

```go
func WaitForServer(url string) error {
    const timeout = 1 * time.Minute
    deadline := time.Now().Add(timeout)
    for tries := 0; time.Now().Before(deadline); tries++ {
        _, err := http.Head(url)
        if err == nil {
            return nil
        }
        log.Printf("server not responding (%s);retrying…", err)
        time.Sleep(time.Second << uint(tries)
    }
    return fmt.Errorf("server %s failed to respond after %s", url, timeout)
}
```

* 输出错误信息并结束程序。Fatalf会在打印错误信息后退出程序

```go
if err := WaitForServer(url); err != nil {
    log.Fatalf("Site is down: %v\n", err)
}
```

* 只输出错误信息

```go
if err := Ping(); err != nil {
    fmt.Fprintf(os.Stderr, "ping failed: %v; networking disabled\n", err)
}
```

* 直接忽略错误

```go
os.RemoveAll(dir)    // 该函数会返回执行状态，而这边选择直接忽略
```

#### 文件结尾错误EOF

```go
in := bufio.NewReader(os.Stdin)
for {
    r, _, err := in.ReadRune()
    if err == io.EOF {
        break
    }
    if err != nil {
        return fmt.Errorf("read failed:%v", err)
    }
}
```

### 函数值

可以声明类似函数指针的变量并赋值，这类变量在go中称为函数值

```go
func square(n int) int { return n*n }

var f func(int) int
f = square
if f != nil {
    f(3)
}
```

函数值可以跟nil比较，但不能跟其他函数值比较，也不能作为map的key

### 匿名函数

下面的例子每次调用都会返回下一个数的平方，这也是go中闭包的使用方法

```go
func squares() func() int {
    var x int
    return func() int {
        x++
        return x*x
    }
}
```

# Go的一些陷阱

## 捕获迭代变量
