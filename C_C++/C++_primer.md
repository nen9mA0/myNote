# 第一部分 基本语言

## 2. 变量和基本类型

#### const限定符

##### 默认为文件的局部常量

全局const变量默认作用域为文件内，若要在其他文件内引用需要extern

### 3.标准库类型

#### string类型

##### string对象的读写

###### cin

```C++
while(cin >> buffer)
{}
```

###### getline

```c++
getline(istream,string);
```

##### string对象的操作

* empty()  是否为空
* size()
* c_str()
* s[n]
* s1+s2
* s1=s2
* s1==s2
* != >= <= > <

**注意：size()返回类型为string::size_type**

##### string对象中字符的处理

```c++
#include <cctype>
isalnum();
isalpha();
iscntrl();
isdigit();
isgraph();
islower();
isprint();
ispunct();
isspace();
isupper();
isxdigit();
tolower();
toupper();
```

#### vector类型

##### 基本操作

* empty()
* size()
* push_back()
* v[n]
* v1=v2
* v1==v2
* != <= >= < >

##### 迭代器

###### 声明

```c++
vector<type>::iterator iter;
```

##### 操作

* begin()
* end()
* 解引用：返回对象
* 自增：指向下一个

##### const_iterator

###### 声明

```c++
vector<type>::const__iterator iter;
```

const_iterator类型的变量只能遍历并读取对象，而不能修改

注意区分：

* vector\<type\>::const_iterator  迭代器指向的对象不可改变

* const vector\<type\>::iterator  迭代器不可改变

#### bitset类型

##### 声明

```c++
bitset<n> b;
bitset<n> b(number);
bitset<n> b(string);
bitset<n> b(s,pos,n);  //b = s[pos:pos+n]
```

##### 方法

```c++
any();  //存在1
none(); //不存在1
count();//1的个数
size();
test(pos);
set();
set(pos);
reset();
reset(pos);
flip(); //取反
flip(pos);
to_ulong();
```

### 数组和指针

##### 引用与指针的差别

引用在初始化时与变量绑定，因此对引用的修改等于对绑定对象的修改

而指针若不解引用，对指针的修改不会影响指向对象的值

##### 指针与const限定符

###### 指向const对象的指针：若对象为const，指针必须声明其指向const对象

```c++
const int a;
const int* b = &a;
```

###### 指向const对象的指针：可以用const对象指针指向非const对象

此时对非const对象的修改同样是非法的

###### const指针

```c++
int* const a = &b;
```

###### 指向const对象的const指针

```c++
const int* const a = &b;
```

###### 使用typedef定义复杂的指针

```c++
typedef string* pstring;
const pstring p1;  //三种都是const指针
pstring const p2;
string* const p3;
```

##### 创建动态数组

```c++
int *p = new int[10];
myclass *p = new myclass[10]();  //创建长度为10的自定义类数组并初始化
```

##### 零长数组

允许创建零长数组

```c++
char *p = new char[0];
```

此时指针非空，不能解引用，但可用于比较

### 表达式

##### 强制类型转换

* static_cast  隐式类型转换使用的类型
* dynamic_cast
* const_cast  转换为const
* reinterpret_cast  按位直接转换，与(type) xxx转换类型相同

```c++
double a = 1.0;
void *p = &a;
double *b = static_cast<double*>(p)
```

### 语句

##### goto语句

**注意：goto跳转的标签只能在同一个函数内，注意goto运行时的变量作用域问题**

```c++
begin:
    int a = 1;
    if(a)
        goto begin;
```

这里goto执行后a的定义被取消，执行到int a=1时被重新创建

##### 异常

###### 标准异常类

```c++
#include <stdexcept>
```

###### 预处理器变量

```c++
__FILE__  文件名
__LINE__  行号
__TIME__  文件被编译时间
__DATE__  文件被编译日期
```

###### assert

若未定义**NDEBUG**，assert中的表达式为0时执行异常处理代码

### 函数

##### const形参

当函数使用const形参时，可传入const或非const参数，因为形参采用的是参数副本

特殊的，当形参定义为const，且类型存在于C语言中时

```c++
int fcn(const int a);
```

为兼容C语言，编译器会自动转换为非const的定义