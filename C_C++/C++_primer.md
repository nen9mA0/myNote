# 第一部分 基本语言

## 第二章 变量和基本类型

#### const限定符

##### 默认为文件的局部常量

全局const变量默认作用域为文件内，若要在其他文件内引用需要extern

## 第三章 标准库类型

### string类型

#### string对象的读写

##### cin

```C++
while(cin >> buffer)
{}
```

##### getline

```c++
getline(istream,string);
```

#### string对象的操作

* empty()  是否为空
* size()
* c_str()
* s[n]
* s1+s2
* s1=s2
* s1==s2
* != >= <= > <

**注意：size()返回类型为string::size_type**

#### string对象中字符的处理

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

### vector类型

#### 基本操作

* empty()
* size()
* push_back()
* v[n]
* v1=v2
* v1==v2
* != <= >= < >

#### 迭代器

##### 声明

```c++
v ector<type>::iterator iter;
```

##### 操作

* begin()
* end()
* 解引用：返回对象
* 自增：指向下一个

#### const_iterator

##### 声明

```c++
vector<type>::const__iterator iter;
```

const_iterator类型的变量只能遍历并读取对象，而不能修改

注意区分：

* vector\<type\>::const_iterator  迭代器指向的对象不可改变

* const vector\<type\>::iterator  迭代器不可改变

### bitset类型

#### 声明

```c++
bitset<n> b;
bitset<n> b(number);
bitset<n> b(string);
bitset<n> b(s,pos,n);  //b = s[pos:pos+n]
```

#### 方法

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

## 第四章 数组和指针

### 引用与指针的差别

引用在初始化时与变量绑定，因此对引用的修改等于对绑定对象的修改

而指针若不解引用，对指针的修改不会影响指向对象的值

### 指针与const限定符

**指向const对象的指针：若对象为const，指针必须声明其指向const对象**

```c++
const int a;
const int* b = &a;
```

**指向const对象的指针：可以用const对象指针指向非const对象**

此时对非const对象的修改同样是非法的

### const指针

```c++
int* const a = &b;
```

#### 指向const对象的const指针

```c++
const int* const a = &b;
```

#### 使用typedef定义复杂的指针

```c++
typedef string* pstring;
const pstring p1;  //三种都是const指针
pstring const p2;
string* const p3;
```

### 创建动态数组

```c++
int *p = new int[10];
myclass *p = new myclass[10]();  //创建长度为10的自定义类数组并初始化
```

#### 零长数组

允许创建零长数组

```c++
char *p = new char[0];
```

此时指针非空，不能解引用，但可用于比较

## 第五章 表达式

### 强制类型转换

* static_cast  隐式类型转换使用的类型
* dynamic_cast
* const_cast  转换为const
* reinterpret_cast  按位直接转换，与(type) xxx转换类型相同

```c++
double a = 1.0;
void *p = &a;
double *b = static_cast<double*>(p)
```

### 运算符

#### 优先级与结合性

##### 右结合

即从右到左，如

```cpp
a = b = c;
```

##### 左结合

即从左到右，如

```cpp
a = b * c / d;
```

#### 优先级及结合性表

| 优先级 | 运算符       | 功能              | 结合性   |
| ------------ | ----------------- | -------- | ------ |
| I | ::           | 作用域            | 从左到右 |
| II | .            | 成员选择          | 从左到右 |
| II | ->           | 成员选择          | 从左到右 |
| II | []           | 下标              | 从左到右 |
| II | ()           | 函数调用 类型构造 | 从左到右 |
| III | ++ --        | 后自增 自减       | 从右到左 |
| III | typeid       | 类型id            | 从右到左 |
| III | castname<>() | 强制类型转换      | 从右到左 |
| IV | sizeof       | 大小              | 从右到左 |
| IV | ++ --        | 前自增 自减       | 从右到左 |
| IV | ~            | 位取反            | 从右到左 |
| IV | !            | 逻辑非            | 从右到左 |
| IV | + -          | 正负号            | 从右到左 |
| IV | * &          | 取值 取址         | 从右到左 |
| IV | (type)       | 强制类型转换      | 从右到左 |
| IV | new delete   | 创建 释放         | 从右到左 |
| V | ->*  ( ptr->*p ) | 指向成员操作的指针 | 从左到右 |
| V | .*    ( obj.*p ) | 指向成员操作的指针 | 从左到右 |
| VI | * / | 乘 除 | 从左到右 |
| VI | % | 模 | 从左到右 |
| VII | + - | 加 减 | 从左到右 |
| VIII | << >> | 左移右移 | 从左到右 |
| IX | < <= > >= | 比较 | 从左到右 |
| X | == != | 相等 不等 | 从左到右 |
| XI | & | 位与 | 从左到右 |
| XII | ^ | 位异或 | 从左到右 |
| XIII | \| | 位或 | 从左到右 |
| XIV | && | 逻辑与 | 从左到右 |
| XV | \|\| | 逻辑或 | 从左到右 |
| XVI | ?: | 三元运算符 | 从右到左 |
| XVII | = *= /= %= += -= | 赋值和复合赋值 | 从右到左 |
| XVII | <<= >>= &= ^= \|= | 赋值和复合赋值 | 从右到左 |
| XVIII | throw | 抛出异常 | 从右到左 |
| XIX | , | 逗号 | 从左到右 |

## 第六章 语句

### goto语句

**注意：goto跳转的标签只能在同一个函数内，注意goto运行时的变量作用域问题**

```c++
begin:
    int a = 1;
    if(a)
        goto begin;
```

这里goto执行后a的定义被取消，执行到int a=1时被重新创建

### 异常

#### 标准异常类

```c++
#include <stdexcept>
```

#### 预处理器变量

```c++
__FILE__  文件名
__LINE__  行号
__TIME__  文件被编译时间
__DATE__  文件被编译日期
```

#### assert

若未定义**NDEBUG**，assert中的表达式为0时执行异常处理代码

## 第七章 函数

### 形参与实参

#### 形参

函数**定义时**指定的函数，因为没有实际值称为形式参数

#### 实参

函数**调用时**的参数

### const形参

当函数使用const形参时，可传入const或非const参数，因为形参采用的是参数副本

特殊的，当形参定义为const，且类型存在于C语言中时

```c++
int fcn(const int a);
```

为兼容C语言，编译器会自动转换为非const的定义

### 传参与引用

若定义如下函数：

```c++
bool isshorter(string s1, string s2)
{
    return s1.size() < s2.size();
}
```

则在调用函数时会对传入的string对象进行一次复制

可以使用传指针的方式避免复制：

```c++
bool isshorter(string *s1, string *s2)
{
    return (*s1).size() < (*s2).size();
}
```

或是传引用

```c++
bool isshort(string &s1, string &s2)
{
	return s1.size() < s2.size();
}
```

#### const 引用

const引用可以避免函数对引用对象的修改

```c++
bool isshort(const string &s1, const string &s2)
{
    return s1.size() < s2.size();
}
```

且对于**非const引用**，有一些**限制**

* 不能传递右值（如常数和字符串字面值），因为右值没有对应的存储空间
* 不能传递需要转换类型的对象，如short型不能传递给int&，因为若允许，函数内部在对引用赋值时将以给int类型赋值的形式赋给原本的short类型变量
* 不能把const引用赋值给非const引用，但可以把非const引用赋值给const引用

**因此若不对引用对象进行修改，都应定义为const引用**

### 数组的传递

#### 非引用

数组的非引用传递不像其他类型的形参进行复制，而是将参数自动类型转换为**数组第一个元素的指针**

此时若定义如下函数

```c++
int print_arr(int arr[10])
{
    for(int i=0; i<10; i++)
        cout << arr[i] << endl;
}
```

不会对数组长度进行检查，因此若传入元素数量不为10的数组则编译时可以通过而运行时可能导致错误

#### 引用

引用传递则会在编译时对传入元素进行长度检查

```c++
int print_arr(int (&arr)[10])
{
	for(int i=0; i<10; i++)
		cout << arr[i] << endl;
}
```

### 返回值

#### 非引用

非引用的返回值一般也是运算结果或对象的副本

#### 引用

* 返回的引用可以作为左值，即，**可以对返回引用的函数赋值**
* **绝对不能返回任何局部变量的引用或指针**

### 默认实参

可以在函数声明中提供默认的参数值，默认参数可以是任何表达式

```c++
void fn(int a=0, int b=getvalue());
```

### 内联函数

* 内联函数必须在头文件中定义
* inline关键字对于编译器是个建议，编译器可以选择忽略
* 编译器隐式地将定义在类声明中的成员函数当做inline函数

### 类的成员函数

#### 定义

##### 定义在类声明中

可以定义在类中，**默认为内联**

```c++
class test{
    public:
    	void set_num(double n)
        	{ num = n; }
    	double get_num() const;
    		{ return num; }
    private:
    	double num;
}
```

也可以定义在类外

```c++
bool test::compare(const test &a) const
{
    return a.num == num;
}
```

#### this指针

上述例子中，num没有在函数内声明但直接被使用，原因是成员函数拥有**默认形参this**

this指向**调用成员函数的类的实例**，而在成员函数声明括号外可以指定this的类型（是否为const指针）

因此上述函数在编译时等价于（注意实际上不能这样写）

```c++
bool test::compare(const test *const this, const test &a)
{
    return a.num == this->num;
}
```

但在成员函数中可以显式地使用this，如下是可以通过编译的

```c++
bool test::compare(const test &a) const
{
    return a.num == this->num;
}
```

### 类的构造函数

类的构造函数在类声明之初被执行，**函数名与类名相同，类型为public**

```c++
class test{
    public:
    	test(): num(0.0) {  }
    private:
    	double num;
}
```

其中:和{}中间的代码为构造函数的**初始化列表**，用于初始化类的成员，其余部分都与普通函数一致

**一个类可以有多个构造函数，即重载的构造函数**

### 函数重载

#### 重载作用域

所有的重载函数都应**在一个作用域被声明**，否则函数声明将互相覆盖而不是重载

#### 函数匹配与实参转换

##### 重载确定的步骤

* **选择候选函数**  与被调用函数同名的函数

* **选择可行函数**  满足以下条件的函数
  * 参数个数匹配
    * 参数类型匹配
      * 准确匹配，为最优
      * 能够类型转换为目标参数的类型
        * 类型提升后匹配
        * 标准转换后匹配
        * 类类型转换后匹配

* 寻找最佳匹配，以实参与形参类型越接近越佳

##### 含多个形参的重载确定

确定多个形参的重载函数的原则为

* 每个实参的匹配都不劣于其他可行函数需要的匹配
* 至少有一个实参匹配优于其他可行函数提供的匹配

如对于函数调用 fn(1,1.0)，有可行函数 fn(int,int)和 fn(double,double)，则不符合上述条件，含有二义性

**注意：当函数拥有默认参数时，对于重载函数导致二义性的出现，如**

```c++
int fn(int a, int b, int c=0);
int fn(int a, int b);
```

此时若调用fn(1,2)不能通过编译，因为对于参数c有默认参数，导致二义性

#### const形参

对于**非引用const形参**，是否有const**不能**实现函数重载

但对于引用const形参，编译器**可以根据引用是否为const重载函数**

此外，编译器**可以根据指针指向的是否为const对象重载函数**，但不能根据指针是否为const类型重载

### 函数指针

#### 使用typedef定义函数指针

```c++
typedef bool (*cmpfcn)(const string &s1, const string &s2);
cmpfcn pf1 = 0;		//声明一个函数指针
```

#### 初始化和赋值

只能用0或函数或同类型函数指针初始化

不同类型函数指针不存在转换

```c++
pf1 = compare_str;
pf1 = &compare_str;
```

函数名前无论是否取地址结果完全相同

#### 调用

无论是否解引用都可以

```c++
pf1("string1","string2");		//隐式解引用
(*pf1)("string1","string2");	//显式解引用
```

#### 以函数指针作为参数或返回值

##### 形参

```c++
void fn( int (*)(const string &s1) );
void fn( int (const string &s1) );
```

##### 返回值

```c++
void (*fn(int)) (int,int);
```

这里，返回值为void (*)(int,int)类型

##### 使用typedef

```c++
typedef void (*pf)(int,int);
pf fn(int);
```

**注意：函数类型和函数指针类型的区别，形参可以是函数类型，但返回值必须为函数指针**

因此若采用如下定义

```c++
typedef void pf(int,int);		//一个函数类型的类型pf
int f1(pf);					//正确，形参可以是函数类型
pf f2(int);					//错误，返回值不能是函数而必须是函数指针
pf *f3(int);				//正确
```

#### 指向重载函数的函数指针

赋值时必须跟重载函数的各个参数类型**精确匹配**

## 第八章 标准IO库

### 标准IO库

* iostream
* fstream
* sstream

#### 继承关系

```F#
        istream               ostream
     /     |     \          /    |    \
ifstream   |       iostream      |    ofstream
     istringstream     |    ostringstream
                      / \
              fstream     stringstream
```

#### 宽字符

类名为ascii字符的类前+w

wstream wfstream wiostream

#### IO对象不可复制或赋值

即如下操作不合法

```c++
ostream out1,out2;
out1 = out2;
```

且意味着如下的限制：

* 由于只有支持复制的类型可以存储在容器中，因此io对象不能存储在容器
* 需要传递或返回IO对象只能使用引用，且对于需要操作IO的函数，引用必须为**非const**的，因为IO操作会改变对象

### 条件状态

用于标记IO对象是否处于可用状态或是否发生错误

#### iostate

##### 类型

**strm::iostate**

##### 常量

- strm::badbit  指出被破坏的流
- strm::failbit  指出失败的IO操作
- strm::eofbit  指出流已到达文件尾

##### 方法

* s.eof()  eofbit是否置位
* s.fail()  failbit是否置位
* s.bad()  badbit是否置位
* s.good()  流是否处于有效状态
* s.clear()  将流设置成有效状态
* s.clear(flag)  复位流状态某个bit
* s.setstate(flag)  置位流状态某个bit
* s.rdstate()   返回当前状态值

##### 状态值

* badbit   strm::badbit置位，系统级故障
* failbit   strm::failbit置位，可恢复错误，如cin对象为int时输入了字符
* eof    strm::eofbit置位，文件结尾
* good

除非good为true，否则检查流本身会返回false状态

##### 访问

```cpp
istream::iostate old = cin.rdstate();
cin.clear();
...;	// process input
cin.clear(old);			//reset state to old
//cin.setstate(old);	//same as clear
```

### 输出缓冲区的管理

#### 刷新缓冲区的条件

##### 程序正常结束

正常结束时最后会刷新一次缓冲区

##### 缓冲区满

缓冲区满后会刷新以清理

##### 显式刷新

* endl  刷新缓冲区且添加一个换行符
* ends  刷新缓冲区并添加一个NULL
* flush  刷新缓冲区且不添加字符

##### unitbuf

这个操纵符可以指定当前流每次执行写操作后进行刷新

下面两段程序等价

```cpp
cout << unitbuf << "test" << "1" << nounitbuf;
cout << "test" << flush << "1" << flush;
```

##### 输入与输出流绑定

使用tie方法绑定流，一个ostream每次只能绑定一个istream，且流绑定之后任何IO操作都会刷新缓冲

```cpp
ostream *old_tie = cin.tie();	//获取当前绑定
cin.tie(&cout);	//标准输入绑定标准输出
cin.tie(0);		//清空已有绑定
cin.tie(&cerr);	//标准输入绑定到标准错误
```

### 文件流

#### 类

##### ifstream

继承istream

##### ofstream

继承ostream

##### fstream

继承iostream

#### 基本操作

##### 打开文件

```cpp
ifstream infile(infilename.c_str());
ofstream outfile(outfilename.c_str());
//使用构造函数初始化，假设infilename和outfilename分别是输入和输出文件的文件名的string

infile.open(infilename.c_str());
outfile.open(outfilename.c_str());
//使用open方法打开
```

模式：

* ifstream
  * in
* ofstream
  * out
  * trunc  打开文件作为输出并清空原有内容
  * app  追加写入
* fstream  fstream可以同时指定一个文件为输入和输出，此时除非指定trunc否则不清空文件
* 通用选项
  * ate  第一次读或写时文件指针置于末尾
  * binary  以二进制形式打开

##### 检查文件是否打开成功

直接以ifstream或ofstream对象作为判断条件

```cpp
if(!infile)
    cerr << "open file error" << endl;
```

##### 关闭文件

```cpp
infile.close();
```

##### 清除文件流状态

当文件流对象需要被复用时，应关闭前一个文件并清除文件流状态

```cpp
infile.clear();
```

### 字符串流

#### 类

##### istringstream

继承istream

##### ostringstream

继承ostream

##### stringstream

继承iostream

#### 基本用法

##### 构造

```cpp
stringstream strm(s);	//以string类型元素s初始化stringstream
strm.str();				//返回s
strm.str(s1);			//更新strm存储的s
```

该类存在的原因很简单，它可以让程序使用`<< >>`读取和格式化字符串

```cpp
stringstream ss;
string str;

ss << "hello";
ss >> str;
```

## 第九章 顺序容器

顺序容器按照位置来存储和访问元素，排列次序与元素值无关

STL提供三种顺序容器，还有三种**顺序容器适配器**

* 顺序容器
  * vector
  * list
  * deque  双端队列
* 适配器
  * stack  LIFO
  * queue  FIFO
  * priority_queue  带优先级FIFO

### 基本操作

#### 构造

```cpp
C<T> c;		//空容器
C c(c2);	//c2容器的副本
C c(b, e);	//迭代器b和e之间的元素的副本
C c(n, t);	//n个值为t的元素的容器
C c(n);		//n个值初始化的容器
```

#### 容器存放的类型限制

标准库的容器中的元素需要满足的基本条件是

* 可以赋值
* 可以复制

此外，一些容器对于类型有额外要求，比如map容器中的key等

容器类型可以作为容器的元素，即可以创建容器的容器

```cpp
vector< vector<int> > array;
```

#### 迭代器

##### 运算

所有标准库容器的迭代器都支持以下运算（操作类似C语言的指针，但其实概念不同）

```
*iter		解引用
iter->mem	引用mem成员
++iter
iter++
--iter
iter--
iter1 == iter2
iter1 != iter2
```

vector和deque还支持以下运算

```
iter + n
iter - n
iter1 += iter2
iter1 -= iter2
iter1 - iter2

> >= <= <  四种运算都支持
```

##### 范围

迭代器的 first和last 或 beg和end

first指向第一个元素，而last指向**最后一个元素的下一个元素**

##### 迭代器失效

当容器执行了删除操作时，可能有迭代器失效的问题

#### 与容器有关的类型

```
size_type				表示容器的大小
iterator				迭代器
const_iterator			只读迭代器
reverse_iterator		逆序迭代器
const_reverse_iterator	 逆序只读迭代器
difference_type			两个迭代器差值
value_type				元素类型
reference				元素的左值类型，即 value_type&
const_reference			常量左值，即 const value_type&
```

定义方法如 `list<int>::iterator iter`

#### begin与end方法

```
begin()
end()
rbegin()  逆序迭代器
rend()
```

#### 添加元素

```cpp
c.push_back(t);		//尾部加入t
c.push_front(t);	//头部加入t

c.insert(p, t);		//迭代器p前加入t
c.insert(p, n, t);	//迭代器p前加入n个t
c.insert(p, b, e);	//迭代器p前加入迭代器b到e间的元素
```

**push_front不能用于vector**

此外vector使用insert效率较低

添加元素可能导致迭代器失效，可能得根据容器类型来调整迭代器，如下面代码在每一个vector元素后添加一个元素

```cpp
vector<int>::iterator first = v.begin();
while(first != v.end())
{
    first = v.insert(first, 42);	//这里insert返回新插入元素的迭代器，在原来的first前。但注意原来的first与插入后的位置不一定相同
    ++first;				//指向原来的first的位置
    ++first;				//指向下一个原有元素
}
```

#### 关系运算

容器类型和元素类型相同的才能比较

有点类似strcmp的形式，长度内容完全相同的才相等，不相同的根据每个元素的大小关系来确定大于或小于

#### 容器大小

```cpp
c.size();
c.max_size();
c.empty();
c.resize(n);		//调整容器大小为n
c.resize(n, t);		//调整容器大小为n并把多的元素全部初始化为t
```

#### 访问元素

```
c.back()		最后一个元素的引用
c.front()		第一个元素的引用

=====仅适用于vector和deque=====
c[n]
c.at(n)			两者用法相同
```

注意bac和front返回的类型是`container<type>::reference`，所以 `c.back() == *c.begin()`

#### 删除元素

```
c.erase(p)		删除迭代器p指向的元素
c.erase(b, e)	删除迭代器b和迭代器e之间的元素
c.clear()		删除所有
c.pop_back()	删除最后一个元素，返回void
c.pop_front()	删除第一个元素，返回void（vector不支持该操作）
```

#### 赋值和交换

```cpp
c1 = c2;			//使用c2构造一个c1，相当于全部值复制过去
c1.swap(c2);		//交换，一般不执行赋值，因此速度较快
c1.assign(b, e);	//重新设置c1为迭代器b和e之间的元素
c1.assign(n, t);	//重新设置c1为n个t
```

assign和=的区别：**使用=时容器和元素必须同类型**，使用assign时则可以不同类型（只要有相应的类型转换函数），如可以将char*元素的vector赋给string元素的list容器

### 容器的选用

#### 容器异同的分析

* vector  对应数据结构的线性表结构，内存布局即类似C的数组，但是变长的
  * 在尾部加入和删除数据是O(1)
  * 随机访问速度是O(1)，直接用偏移量寻址
  * 在vector中插入或删除元素是O(n)，实际开销较大，因为要将插入点后面的元素整体后移
* list  对应数据结构是链表
  * 在任何地方加入和删除数据是O(n)，需要查找该元素并插入链表，但实际开销小于vector，因为不需要整体移动内存
  * 随机访问速度是O(n)
* deque  对应数据结构是双端链表，实际实现可能更类似一个环形缓冲区，因为其支持随机访问和在头部尾部插入删除元素
  * 在头部和尾部插入和删除数据是O(1)
  * 随机访问是O(1)
  * 随机插入是O(n)，实际开销大于vector

#### vector的预留空间

vector在实际使用的时候不会只分配程序给定的空间，而是会预分配部分内存，以减少重新分配空间造成的开销

**size()**方法可以用于获取当前vector已被使用的空间，**capacity()**可以获取vector实际占用的空间。此外可以用**reserve()**指定每次重新分配的空间大小，如

```cpp
v.reserve(50);
```

则在vector元素少于50个时，实际分配50个元素的空间。而在赋值第51个时，vector会重新分配100个元素的空间，并整体迁移到新地址

### string

#### 与顺序容器相同的操作

string虽然不是顺序容器，但支持很多顺序容器的操作

* *与容器有关的类型* 小节中列出的
* 除了只有长度一个参数的构造函数外的所有容器构造函数
* vector容器支持的添加元素的操作，删除操作不支持pop_back
* *容器大小* 列出的
* `[]`和at
* begin end
* erase clear
* =和assign
* capacity和reserve

#### 特有的操作

其中**arg1**可以是下面几种

```
s2				string类型
s2, pos2, len2	 s2中下标pos2开始的len2个字符
cp				char*
cp, len2		cp指向的char[]的len2个元素
n, c			n个char
b2, e2			迭代器b2到e2之间的元素
```

**arg2**是以下几种

```
c, pos			从下标pos开始查找字符c
s2, pos			从下标pos开始查找string s2
cp, pos			从下标pos开始查找char[] cp
cp, pos, n		从下标pos开始查找char[] cp的前n个字符
```

* insert assign erase函数除了可以用迭代器作为参数，还有使用下标作为参数的版本
* substr
  * substr(pos, n)
  * substr(pos)
  * substr()  副本
* append
  * append(**arg1**)
* replace
  * replace(pos, len, **arg1**)
  * replace(b, e, **arg1**)
* find
  * find(**arg2**)
  * rfind(**arg2**)
  * find_first_of(**arg2**)  查找arg2中的任意一次字符第一次出现的位置
  * find_last_of(**arg2**)
  * find_first_not_of(**arg2**)
  * find_last_not_of(**arg2**)
* compare
  * compare(s2)
  * compare(pos1, n1, s2)
  * compare(pos1, n1, s2, pos2, n2)
  * compare(cp)
  * compare(pos1, n1, cp)
  * compare(pos1, n1, cp, n2)

### 容器适配器

适配器类似于提供一套操作接口，其后端是容器，负责实际的存储工作，而前端是适配器，用于按照适配器提供的操作方式来操作存储的元素。但适配器在使用容器初始化时会被拷贝

#### 与适配器有关的类型

```
size_type
value_type
container_type
```

适配器支持所有关系操作符 `== != < <= > >=`

#### 构造

默认情况下，几种适配器对应的容器是固定的

* stack  deque
* queue  deque
* priority_queue  vector

一般构造的时候可以接受一个容器来初始化适配器的容器内容

```cpp
deque<int> deq;
stack<int> stk(deq);
```

但可以改变默认适配器

```cpp
vector<int> vec;
stack< int, vector<int> > stk(vec);
```

#### 操作

##### stack

```
empty()
size()
pop()		出栈，且不返回值
top()		返回栈顶元素的值
push(item)
```

##### queue priority_queue

优先级队列与队列区别就在于优先级队列把新元素放在比它优先级低的元素前面而非队尾。

比较优先级使用的是 `> <`

```
empty()
size()
pop()

push(item)  对于队列，在对尾压入一个元素
			对于优先级队列，基于优先级压入一个元素

=====仅适用于队列=====
front()		队首元素
back()		队尾元素

=====仅适用于优先级队列=====
top()		返回具有最高优先级的元素
```

## 第十章 关联容器

* map  通过键存储和读取
* set  大小可变的集合，用键来快速读取
*  multimap  支持同一个键多次出现的map
* multiset  支持同一个键多次出现的set

### pair

用于表示一个成对的元素

#### 创建

```cpp
pair<T1, T2> p1;
pair<T1, T2> p1(v1, v2);
make_pair(v1, v2);
```

#### 比较

```
p1 < p2		相当于 p1.first < p2.first && p1.second < p2.second
p1 == p2	相当于 p1.first == p2.first && p1.second == p2.second
```

#### 访问

```
p.first
p.second
```

### 关联容器

很多操作与顺序容器通用，主要有

* 构造函数
  * `C<T> c`
  * `C<T> c1(c2)`
  * `C<T> c(b, e)`
* 关系运算
* begin rbegin end rend
* 类型，但其中map的value_type并非元素的类型，而是描述pair的类型
* swap和=
* clear erase
* 除resize外的关于容器大小的操作

#### map

##### 构造

k为key的类型，v为value的类型

``` cpp
map<k, v> m;
map<k, v> m(m2);
map<k, v> m(b, e);
```

##### 键的类型

键的类型必须支持排序，具体说来支持严格弱排序，支持“小于”运算

##### 与map有关的类型

```
map<K, V>::key_type			键的类型
map<K, V>::mapped_type		值的类型
map<K, V>::value_type		一个pair类型，first为key_type，second为mapped_type
```

##### 迭代器

迭代器获得的引用类型是`map<K, V>::value_type`

##### 访问

###### 下标

可以使用键作为下标访问，若使用`=`对某个键的元素进行赋值时但没有对应的键时，会自动插入一个新的键值对

```cpp
map<string, int> m;
m["1"] = 1;
int value = m["1"];
```

**注意，若使用下标访问一个不存在的键，该键会被创建并初始化**

###### 访问的方法

使用下列方法访问不会创建一个不存在的键

```cpp
m.count(k);		//返回k出现的次数，对于map只有0或1
m.find(k);		//返回k索引的元素迭代器，若无则返回end迭代器
```

##### 插入

除了使用上述方法插入元素，还可以使用insert

```cpp
map<string, int> m;
pair<string, int> e;
container< pair<string, int> >::iterator beg, end, iter;

m.insert(e);		//插入一个pair
m.insert(beg, end);	//插入迭代器之间的元素(若没有对应键)
m.insert(iter, e);	//若e.first不在map内则创建，并以iter为起点搜索元素存储的位置
```

若map中有对应键则不会做任何操作

对于参数为pair的insert方法，会返回一个`pair< map<T1, T2>::iterator, bool >`，前者给出插入的键的迭代器，后者表示是否插入了新的元素

##### 删除

```cpp
m.erase(k);			//删除键为k的元素，返回size_type类型
m.erase(p);			//删除迭代器p所指的元素
m.erase(b, e);		//删除迭代器b~e间的元素，其中遍历的顺序是按照键值排序的大小从小到大
```

#### set

支持关联容器的通用容器操作外，还支持大部分map操作，包括

* map的构造函数
* insert
* count find
* erase

set与map的区别在于，set是集合类型，类似数学中的集合，存储的只有元素，且无法在set内找到两个相同的元素，因此也没有`mapped_type`类型

set不支持下标查找

##### 构造

与map提供的三个方法相同

##### 添加

```cpp
s.insert(e);
s.insert(b, e);
```

##### 获取

```cpp
s.find(e);
s.count(e);
```

#### multimap multiset

multimap的一个键可以对应多个值，因此它不支持下标访问

multiset相当于是一个排序容器，它保证容器内所有元素是有序的，且可以重复出现

##### 添加

multimap一样可以采用insert，每次insert一个键值对，若已经存在对应键则会将该值添加到该键上

multiset则可以直接insert元素

##### 删除

multimap使用`erase(k)`会删除该键对应的所有元素，使用`erase(p)`则只会删除迭代器对应元素

##### 查找

对于这两种类型，同一个键对应的元素是相邻存放的（至少对于迭代器来说是这样）

###### find count

使用count判断键对应的元素个数，find返回一个迭代器，指向第一个正在查找的键的实例

示例代码见`C++_primer_code\chap10`

###### bound

```cpp
m.lower_bound(k);		//一个键对应元素的begin迭代器
m.upper_bound(k);		//一个键对应元素的end迭代器
m.equal_range(k);		//上述两者的pair
```

## 第十一章 泛型算法

泛型算法工作于容器上，并且独立于容器，且其不依赖于容器存储的类型

算法对容器的要求如下

* 可以遍历容器 -> 迭代器
* 能够知道是否到达容器的末尾 -> end()
* 能够对容器中的每个元素与被查找元素进行比较
* 需要可以指出容器的位置，或表示找不到该元素

算法对于容器的操作都是通过迭代器实现的，添加或删除元素时也是通过一种叫做插入器的迭代器实现的。

### 算法种类

#### 只读算法

这种算法只会读取容器内的值，不会修改

**注意**，通常泛型算法都是在两个可以标记容器访问范围的迭代器上操作的（如begin和end间），这两个迭代器类型必须相同。但对于如find_first_of这种带有两对指示范围的迭代器的算法，两对迭代器不需要指向相同元素，只要两者可以比较就行（有对应的重载运算符）

#### 写容器元素的算法

##### 写入序列的元素

会写入与指定输入范围数量相同的元素，比如`fill(b, e, elm)`传入的是两个迭代器

##### 不检查写入操作的算法

比如`fill_n(b, n, elm)`传入的是起始迭代器和个数，这里的个数没有做检查，可能导致出错

##### back_inserter

back_inserter（插入迭代器）是一种迭代器适配器，类似容器适配器，使用这个迭代器赋值时，赋值运算将使用push_back在容器中先添加一个值

fill_n使用下列写法不会出错，等价于调用十次push_back(0)

```cpp
fill_n(back_inserter(vec), 10, 0);
```

##### 写入目标迭代器的算法

这段代码等价于`vector<T> vec(lst.begin(), lst.end())`

```cpp
copy(lst.begin(), lst.end(), back_inserter(vec));
```

##### 算法的_copy版本

很多算法会提供_copy版本，不修改原来容器的内容，算法结果通过一个新的对象返回

如replace和replace_copy

#### 对容器元素重新排序的算法

如`unique(vec.begin(), vec.end())`，返回一个迭代器，其会将所有未重复的元素放到迭代器之前，保证迭代器前的元素的唯一性。注意，该函数只能应用于排序好的容器，其判断是否重复时只与前一个元素比较

### 迭代器的类型

#### 插入迭代器

如前面所述，插入迭代器在赋值的时候是通过插入该元素赋值的，而不是修改现有元素

##### front_inserter

使用push_front插入，因此vector没有front_inserter

##### back_inserter

使用push_back插入

##### inserter

使用insert插入，因此其还需要一个迭代器指示插入的位置，新元素将会在其前面插入

注意，下面两段代码运行结果不一样

```cpp
vector<int> lst2;
copy(lst1.begin(), lst1.end(), front_inserter(lst2));

vector<int> vec;
copy(lst1.begin(), lst1.end(), inserter(lst2, vec.begin()));
```

前者每次插入的元素都保证是最前面的，后者因为每次插入元素后原来的begin指针指向的元素都会后移一位，因此两者的结果顺序是相反的

#### iostream迭代器

iostream不是容器，但也定义了迭代器，称为流迭代器

```cpp
istream_iterator<T> in(strm);	//从输入流读取T对象的迭代器
istream_iterator<T> in;			//istream_iterator的超出末端迭代器（指向end的迭代器）
ostream_iterator<T> out(strm);
ostream_iterator<T> out(strm, delim);	//写入输出流的过程中以delim作为分隔符
```

##### 操作

```cpp
it1 == it2;
it1 != it2;			//使用同一个流构造的it相等，都指向end的it相等
```

```
*it
it->mem				//返回从流中读取对象的mem成员
++it
it++
```

##### 使用实例

```cpp
vector<int> vec;
istream_iterator<int> input(cin);
istream_iterator<int> end;

while(input != end)
{
    vec.push_back(*input++);
}
```

直到遇到eof或者读到不为int的内容才会跳出循环

下面程序是等价的

```cpp
istream_iterator<int> input(cin);
istream_iterator<int> end;

vector<int> vec(input, end);
```

另外，可以使用流迭代器作为算法的输入，使算法直接输出结果而不经过中间变量（将ostream_iterator传入算法）

#### 反向迭代器

反向迭代器遍历时一般使用自减运算

对于正向的迭代器，假设容器size为n，begin和end的范围为`[0, n)`，相当于end指向最后一个元素的下一个元素

因此反向迭代器原理类似，rend和rbegin的范围为`(-1, n-1]`

**反向迭代器可以使用base()指向下一个元素，这是为了适配正向迭代器的范围**

#### const迭代器

若不希望迭代器修改容器内容可以定义为const。但在算法中定义范围时，如前文所述成对的迭代器类型应该相同，因此如果对一个元素非const的容器来说，begin和end返回的都是非const迭代器，因此在这种情况下，若在描述范围的成对迭代器中有一个使用了begin或end的话另一个必须是非const的

### 五种迭代器

不同的算法对于迭代器操作和适配性有不同的要求

* 输入迭代器  可用于读取容器的元素，且若不支持自减运算则无法检查之前的元素，如istream_iterator
  * == !=
  * ++
  * *
  * ->
* 输出迭代器  与输入迭代器性质类似，用途互补，如ostream_iterator
  * ++
  * *
* 前向迭代器  支持输入与输出迭代器的所有操作，只会以一个方向遍历迭代器，但支持对同一元素的多次读写
* 双向迭代器  可以从两个方向读取，在支持前述迭代器基础上支持自减，如map set list
  * --
* 随机访问迭代器  除支持双向迭代器所有操作外，还支持下列运算，如string vector deque
  * < <= >= >
  * `+ += - -=`
  * `-`  获取迭代器距离
  * `[]`

### 泛型算法的结构

#### 形参常用格式

```cpp
alg(beg, end, other);			//对beg到end间的元素进行操作
alg(beg, end, dest, other);		//将从beg到end的操作输出到dest开始的地方，这里没有限定输出范围，因此需确保被调用的容器有足够的空间，或使用inserter迭代器
alg(beg, end, beg2, other);		//这里假定beg2开始的序列与beg到end的序列一样大
alg(beg, end, beg2, end2, other);	//将beg到end的操作输出到beg2到end2
```

#### 命名常用格式

带有_copy的返回的元素是一个新的对象，而非在传入对象上进行操作

### 容器特有算法

list容器是使用双向迭代器，因此不支持需要随机访问迭代器的算法

对于list容器若使用泛型算法提供的merge remove等操作是很慢的，因此一般使用list类定义的方法

```cpp
lst.merge(lst2);
lst.remove(val);		//删除所有等于val的元素
lst.reverse();
lst.sort();
lst.splice(iter, lst2);	//移动元素，有多个重载版本
lst.unique()		   //删除相同的值，运算后list每个值只出现一次
```

## 第十二章 类

### 定义和声明

要点如下

#### 成员

可以是数据，函数或别名

属性有如下三种

* private
* public
* protect

#### 构造函数

与类同名，一般应使用一个构造函数初始化列表初始化对象的数据

#### 成员函数

成员函数必须在类定义中被声明（定义在类中的函数默认为inline的），函数体定义可以在类声明外

可以将const关键字加在形参表之后，表示声明该成员函数为const成员函数，其**不能改变操作的对象的数据成员**

```cpp
class test
{
    public:
    	test(): num(0.0), member(0) {  }
    	int read() const;
    private:
    	int member;
    	double num;
}

int test::read() const
{
    xxx;
    return xxx;
}
```

#### 别名

可以在类定义中定义别名，以使用户更方便地使用数据类型

#### 重载

成员函数都可以被重载，方法和规则与定义普通函数的重载类似

#### inline

在类定义里定义的成员函数默认是inline的，定义在类定义外的成员函数可以显式inline

#### 类声明

可以声明一个类而不定义这个类，但类的定义必须在其他地方出现

这种方式可以用于在类中定义一些与类本身有关的成员，比如

```cpp
class linklist;
class linklist
{
    int num;
    linklist* next;
};
```

### this指针

#### 指针类型

* 非const成员函数   const指针，可以改变对象的属性
* const成员函数       指向const对象的const指针

#### 概述

this指针是隐式定义的，它指向当前调用成员函数或访问成员的实例对象。在成员函数被调用时其作为一个隐式参数被传入

一般来说在程序里不需要使用this，因为对当前实例的所有成员的访问都不需要显式调用this（即对于属性x，可以直接写x=1而不必写this->x=1）

#### this指针的使用

使用this的时机是我们需要使用整个实例对象作为参数时，或者将整个实例对象返回时。这里有个比较常见的情况，如对于类Test，有set和move两个成员函数，一般操作如下

```cpp
test1.set(1);
test1.move(dst);
```

但可能我们希望可以这样写

```cpp
test1.set(1).move(dst);
```

在这种情况下，我们需要使test1.set(1)的返回值是当前的对象，因此可以这样写

```cpp
Test& set(int num)
{
    xxx;
    return *this;
}
```

#### 可变数据成员 mutable

可以将成员声明为mutable，这样的成员可以使用const成员函数修改

mutable永远没有const属性，就算对象为const

### 类作用域

#### 作用域限定名

* 函数体和形参表在类作用域内

  即若**形参表**中需要使用类中声明的类型可以不加作用域限定符，函数体中也不需要

* 在类定义外定义的成员函数返回类型不在类作用域中

#### 类作用域中的名字查找

* 一般变量和函数
  * 在使用该名字的块中查找
  * 在包围的作用域中查找
* 类成员声明（如类成员声明中使用了自定义类型）
  * 检查在该类型使用前的类成员
  * 检查包含类定义的作用域的声明以及出现在**类定义以及类的成员函数定义之前**的声明（即若一个函数在类定义前没声明，但在某个成员函数定义前被声明，则也可以识别到）
* 类成员定义
  * 检查成员函数作用域中的声明
  * 检查所有类成员的声明
  * 检查此成员函数定义之前的作用域中出现的声明

#### 重名问题

* 一般变量和函数

  * 与C语言相同，如果不同作用域出现同名，上一个作用域的定义将被屏蔽

* 类

  * 若成员函数出现与类成员同名的情况，可以使用作用域限定符指定作用域

    如类Test有成员data，成员函数setdata参数data，还有全局变量data

    ```cpp
    class Test
    {
        public:
        	void setdata(int data);
        private:
        	int data;
    };
    
    void Test::setdata(int data)
    {
        this->data = data;		//下面两种方法都指定名为data的类成员	
        // Test::data = data;
        ::data = data;			//这里指定的是全局作用域的变量data
    }
    ```

### 构造函数

* 构造函数可以被重载
* 构造函数不能被定义为const函数

#### 初始化列表

##### 格式

初始化列表例子如下

```cpp
Test::Test(): data(0)
{
    xxx;
}
```

data(0)位于`:`和`()`之间，为初始化列表

##### 必须使用的情况

如果没有显式在初始化列表对成员进行初始化，编译器会在调用构造函数的函数体前使用默认的构造函数

因此，**对于const或引用类型的数据成员必须调用初始化列表进行初始化**

因为前者在默认初始化函数调用后无法在构造函数体内赋其他的值，后者必须通过初始化为引用类型绑定一个变量

##### 初始化顺序

顺序不是按照初始化列表的顺序，而是按照**成员在类中声明的顺序**，因此若有成员的初始化依赖其他成员，则需要注意这点（但一般不要这么写）

#### 构造函数的默认实参

```cpp
void Test::test(int data=0)
{
    xxx;
}
```

这里提供了一个默认实参，这样其实就可以合并`void Test::test()`和`void Test::test(int data)`这两个函数的定义

#### 默认构造函数

默认构造函数即没有参数的构造函数

当一个类没有定义构造函数时，编译器才会生成默认的构造函数

##### 编译器生成的默认构造函数

默认构造函数一般采用下列生成方式

* 对于通用类型的成员，与通用类型的变量默认初始化相同
* 对于类类型的成员，调用该类的默认构造函数
* 对于内置和复合类型（如数组等）
  * 全局作用域的对象会被初始化
  * 局部作用域的对象不会被初始化

##### 没有默认构造函数的后果

若类有带参数的构造函数，则不会生成默认构造函数。此时若没有显式定义默认构造函数则会带来限制

* 每个该类成员的初始化必须传递参数用于调用非默认构造函数
* 该类**不能用作动态分配数组的元素类型**
* 该类的容器初始化时必须提供元素初始化的参数

##### 隐式类类型转换

对于只含一个参数的构造函数，相当于构造了一个隐式类型转换，比如对于上面的Test类，将一个int值赋给Test类相当于调用`Test::Test(int data)`，并将构造后的对象赋值给Test类

但这种行为可能导致一些错误，因此可以将构造函数声明为**explicit**来防止这种隐式类型转换的发生。该关键字只能用于类内部的构造函数声明上，即不可以出现`explicit Test::Test()`，该关键字只能用在class声明内

一般来说要避免定义隐式类型转换

### 友元

友元是为了让其他类访问本类的私有成员而设置的。友元的对象可以是一个类，也可以是一个函数（这个函数也可以是成员函数，即只允许该类的该成员函数对本类私有成员进行访问）

#### 作用域问题

友元声明与友元定义互相依赖

```cpp
class Test1
{
    public:
    	Test1();
    	int read_Test2(Test2& test);
};

class Test2
{
    public:
    	Test2();
    	friend int Test1::read_Test2(Test2& test);
   	private:
    	int private_data;
};

int Test1::read_Test2(Test2& test)
{
    ...;
}
```

注意，这里Test2的声明中使用了Test1进行定义，因此Test1的声明应在Test2前。此外Test2声明friend的时候也使用了Test1的read_Test2函数，因此Test2应在Test1的成员函数定义前（见类作用域-类作用域中的名字查找）

### static类成员

#### static数据成员

static成员类似全局变量，但其作用域在类中，且static成员可以是private的来限制访问

##### 定义

static数据成员不能在构造函数中初始化，因为这样每次调用构造函数都将改变其值

static数据成员应在类外被定义

```cpp
int Test::num = 0;
```

因此static数据成员最好是与类外的成员函数的定义放在一起

此外，const static也是类似的定义方式，除非const static的内容是一个常量表达式

##### 访问

static成员的访问可以有以下几种（假设Test类有静态成员num）

```cpp
Test tst1;
Test* tst2;

tst1.num		//下面为三种访问方法
tst2->num
Test::num
```

##### 其他性质

* static成员可以作为成员函数的默认实参，但普通成员不行

* 可以在类中声明一个该类的static成员，即下面声明合法

  ```cpp
  class Test
  {
      static Test tst;
  }
  ```

#### static成员函数

* static成员函数可以在类中声明成员函数时通过static关键字声明
* static成员函数没有this指针，因此不能访问实例对象的非static成员
* static不能被声明为const函数或虚函数

## 第十三章 复制控制

### 复制（拷贝）构造函数

只有单个形参，且该形参是对本类对象的引用（通常是const引用）的构造函数叫拷贝构造函数，由编译器隐式调用

#### 调用时机

* 根据另一个同类型对象显式或隐式初始化一个对象
* 复制一个对象，作为实参传递
* 函数返回时复制一个对象
* 初始化顺序容器中的元素
* 初始化数组元素

##### 显式或隐式初始化

复制初始化与直接初始化

```cpp
string tst1 = "1234";		//创建一个临时对象string("1234")，再用拷贝构造函数复制给tst1
string tst2("1234");		//直接调用初始化函数
string empty1 = string();	//创建一个临时对象为空string，再用拷贝构造函数赋值给empty1
string empty2;				//直接调用string的默认构造函数
```

若类不允许复制（如IO类型）或构造函数为explicit，可能无法通过隐式类型转换构造，如

```cpp
Test::Test(string str) { }		//无explicit关键字
//explicit Test::Test(string str) { }

Test a = "1234";	//对于非explicit版本可用，因为相当于先将"1234"隐式类型转换为Test，再拷贝构造
```

##### 形参与返回值

函数的形参及返回值为非引用类型时，会调用复制构造函数用于传递参数到新的对象及返回对象

##### 初始化容器元素

```cpp
vector<string> svec(5);
```

实际上首先创建了一个空string对象`string()`，然后调用五次复制构造函数初始化每个svec元素

##### 构造函数与数组元素

对于一个类类型数组，如果指定了初始值，如

```cpp
Test tst_table[10] = { string("1"), string("2") };
```

对于前两个有初值的类型，会先创建对应的临时对象，再将对象复制给tst_table（这里还包含了隐式类型转换的步骤）

#### 编译器生成的复制构造函数

若没有定义复制构造函数则编译器会生成，规则如下

* 内置类型  直接复制
* 类类型  调用该类的复制构造函数
* 数组  复制数组的每一个元素

#### 自定义复制构造函数

参数一般为本类型的const引用，不能定义为explicit，因为这样编译器无法显式调用

```cpp
Test(const Test& tst)
{
    str = tst.str;
    length = tst.length;
}
```

#### 禁用复制构造函数

若只是对于非友元类和类本身禁用，使用private定义复制构造函数

若想禁止友元类和类成员调用复制构造，可以声明一个private的复制构造函数，但不做定义

### 赋值操作符

#### 定义

其实是重载运算符的一种，重载=操作理论上有两个参数，分别是左值和右值，但对于类的重载=，左值默认绑定在this上

```cpp
Test::operator=(const Test& rhs);
```

#### 编译器生成的赋值操作符

默认的赋值运算中，行为即对当前类的浅拷贝函数

#### 复制构造函数和赋值运算符

一般来说，**如果一个函数需要自定义复制构造函数，必定需要自定义赋值运算符，且需要定义显式析构函数**。因为编译器生成的函数无法处理类中动态申请了资源的情况

### 析构函数

#### 调用时机

* new的对象指针  delete时调用析构
* 局部变量  超出作用域的时候
* new的数组（或容器等）  delete [] 时析构，即在每个类元素上调用析构
* 局部数组（容器）变量  超出作用域的时候，在每个类元素上调用析构

**数组、容器元素总是逆序撤销**

#### 显式析构函数

重复一下所谓的**三法则(rule of three)**

**如果一个函数需要自定义复制构造函数，必定需要自定义赋值运算符，且需要定义显式析构函数**

显式析构函数使用命名为`~+类名`，无参数无返回值，因此不能重载。且对于析构函数，编译器生成的析构函数会在显式析构函数执行后被执行，这与其他几个特殊函数不一样

```cpp
Test::~Test() {  }
```

#### 编译器生成的析构函数

按类声明中次序的逆序为每个非static成员调用析构函数

对于指针对象不会调用delete，因此需要delete必须自定义析构

### 管理指针成员

#### 自定义的智能指针类

为了防止悬垂指针等问题，引入引用计数解决问题

设计思路就是，如果指针增加一次引用，则引用计数+1，析构一次引用计数-1，若引用次数为0则释放

需要定义两个类来实现指针的管理，其中一个类存放指针，另一个类进行引用计数，因为同一个指针只能对应同一个引用计数，即每个地址相同的对象都应该只对应一个引用计数值，若引用计数与指针直接使用一个类来实现，则会出现下面的问题

```c
int *p = new int(0);
Ptr ptr1(p);		//constructor
Ptr ptr2(ptr1);		//copy constructor
Ptr ptr3(ptr1);
```

ptr2会把ptr1的引用计数+1，复制到ptr2，ptr3也会进行相同的操作，则ptr2和ptr3的引用计数都是2，实际是3。

当然换个思路可以先加ptr1，再赋值给ptr2和ptr3，但这会导致ptr2的引用计数是2，ptr3的是3，ptr1的也是3。总之没法同步所有的Ptr对象

具体实现见`C++PRIMER_CODE/chap13/autoptr.cc`

#### 值型类

这种方式就是把指针指向的对象复制一个副本

## 第十四章  重载操作符与替换

### 定义

非成员函数

```cpp
Test& operator+(const Test& op1, const Test& op2);
```

成员函数

```cpp
Test& Test::operator+(const Test& op);
```

重载运算符作为成员函数时，可以少一个参数，对于二元运算符左边的操作数为默认参数，作为this传入。对于一元运算符则无参数

#### 不可重载的运算符

* `::`
* `.*`
* `.`
* `?:`

不能为内置类型重定义操作符，包括内置类型构成的数组等

#### 优先级

运算符优先级和结合性不变

对于类似 & 这种可以有一个操作数或两个操作数的运算符，重载哪个按照操作数个数指定

#### 没有短路求值特性

若重载了`|| && ,`则**不保证求值顺序**，这与内置操作符不同

### 使用

```cpp
Test tst3 = tst1 + tst2;
Test tst3 = tst1.operator+(tst2);
```

### 重载操作符的设计

#### 具有内置含义的操作符

这些操作符一般编译器默认定义了语义

* `=`  编译器默认生成赋值操作符，具体前文讲过，在一些情况下应重载
* `& ,`  前者一样是取址，后者是从左到右顺序执行并返回最右的执行结果，最好不重载
* `&& ||`  默认对返回值进行逻辑判断，若重载会失去短路求值的特性

#### 常用重载操作符的含义

* `==`  判断是否相等
* `>> <<`  输入输出
* `!`  判断是否空

#### 设计原则

* 若重载了`+ -`等运算符，应重载其复合运算版本，如`+= -=`
* 最好定义`> <`等关系运算，且若定义了大小关系，应同时定义`>= <= != ==`
* 定义为成员函数与否有如下原则
  * `= [] () ->` 必须为成员函数，否则报错
  * 复合赋值应定义为成员函数（如+=），但定义为非成员函数也不会报错
  * 改变对象状态或与类联系紧密的操作符应该定义为成员函数，如`++ -- *(解引用)`
  * 算术操作符 相等操作符 关系操作符 位操作符等最好为非成员

### 一些重载操作符的定义方式

#### 输入输出

```cpp
istream& operator>>(istream& in, const Test& tst);
```

istream不能为const，因为输入输出会改变对象。返回值为istream&以便于多个`>>`一起使用

#### 算术运算符

```cpp
Test operator+(const Test& lhs, const Test& rhs)
{
    Test tmp;
    tmp.a = lhs.a + rhs.a;
    tmp.b = lhs.b + lhs.b;
    return tmp;
}
```

使用`+=`实现+可以防止多次复制构造

```cpp
Test& operator+=(const Test& lhs, const Test& rhs)
{
    lhs.a = lhs.a + rhs.a;
    lhs.b = lhs.b + rhs.b;
    return *lhs;
}
```

#### 赋值操作符

这里要说明的就是**赋值操作符可以有多个重载**，根据右值的类型可以调用不同的重载赋值操作符

且赋值返回值是对类的引用，因此应返回`*this`

#### 下标运算符

下标运算符作用的操作数可能作为左值也可能作为右值，因此应该定义两个重载的运算符，用于读写（分别作为左值和右值）

```cpp
const int& Test::operator[](const size_t index) const;
int& Test::operator[](const size_t index);
```

#### 解引用运算符

与下标相同，需要const和非const两个版本作为左值和右值

```cpp
const Test& TestPtr:operator*() const;
Test& TestPtr::operator*();
```

#### 箭头运算符

箭头运算符比较特殊，比如当执行`tst->action()`时，实际相当于`(tst->action)()`，此时编译器执行以下的判断

* 若tst是个指针，且指向的对象有action成员函数，则直接调用该成员函数
* 否则，若tst指向的对象有重载->，则调用tst的`operator->()`，假设返回值为ret。此后执行`(ret->action)()`，即总体执行`(tst->operator->())->action()`，此后会继续递归进行这三步的判断
* 否则，代码出错

因此，**重载->的返回值必定是一个类指针**，这样才可以继续执行上面的操作

#### 自增自减运算符

需要注意的是前缀自增自减与后缀的区别，因为二者形参数目一致，因此后缀会接受一个无用的int形参以作区别

##### 前缀

```cpp
Test& Test::operator++();
```

##### 后缀

```cpp
Test Test::operator++(int);
```

注意，因为后缀运算符返回的是旧值，因此不能直接`return *this`，因为此时this是已经改变过的值

#### 调用操作符与函数对象

```cpp
int Test::operator()();
```

可以将一个非函数成员的行为变得类似函数

##### 定义函数对象

函数对象概念类似于函数指针，但比函数指针灵活

如在某些标准库算法中需要用到比较函数

```cpp
bool GreaterThan6(const string& s)
{
    return s.size() > 6;
}
```

如果要改变比较的值，需要重新定义函数，但函数对象可以方便地解决这个问题

```cpp
class Greaterthan
{
public:
    Greaterthan(size_t val = 0): bound(val) {  }
    bool operator()(const string& s)
    {
        return s.size() > bound;
    }
private:
    size_t val;
};
```

这样可以直接将原来需要传入函数指针的地方，如

```cpp
count_if(words.begin(), words.end(), Greaterthan6);
```

替换为

```cpp
count_if(words.begin(), words.end(), Greaterthan(6));
```

因为在count_if算法中也是通过()来调用函数指针的

##### 标准库函数对象

###### 类型

```
算术函数对象
plus<Type>			+
minus<Type>			-
multiplies<Type>	*
divides<Type>		/
modulus<Type>		%
negate<Type>		-（负号）

关系函数对象
equal_to<Type>		==
not_equal_to<Type>	!=
greater<Type>		>
greater_equal<Type>	>=
less<Type>			<
less_equal<Type>	<=

逻辑函数对象
logical_and<Type>	&&
logical_or<Type>	||
logical_not<Type>	!
```

每个类表示定义一种运算函数对象

###### 用法

```cpp
plus<int> intAdd;
int sum = intAdd(10, 20);	//定义一个函数对象intAdd，可以用于将两个操作数相加

sort(vec.begin(), vec.end(), greater<int>);
		//定义一个greater函数对象，传入sort表示降序排序
```

###### 适配器

**绑定器**可以将一个参数绑定到函数对象的参数上，表示始终作为该函数的参数

```cpp
count_if( vec.begin(), vec.end(), bind2nd(greater<int>, 6) );
```

greater函数对象的参数有两个，比较第一个参数与第二个的大小关系，调用bind2nd后相当于将6绑定到第二个参数，始终调用`greater(x, 6)`

**求反器**将函数对象的返回值求反，not1表示一元函数求反器，not2表示二元函数

```cpp
count_if( vec.begin(), vec.end(), not1(bind2nd(greater<int>, 6)) );
```

该函数调用后可以对小于等于6的所有元素进行计数

#### 类型转换

##### 定义

可以在类中定义转换函数用于该类到其他类型的类型转换

此外，从其他类型到本类型的转换可以由构造函数定义

```cpp
Test::operator type() const;
Test::operator int() const;		//定义一个Test到int的转换
```

type表示任意类型，可以是内置的也可以是类。**该函数没有返回类型，形参表必须为空**

##### 调用时机

在所有用到隐式和显式类型转换的地方编译器都可以调用类型转换

###### 类类型转换和标准类型转换

如Test定义了一个到int的类型转换，则

```cpp
Test a(1234);
double b = 1234.5678;
cout << (a>b) << endl;
```

执行a>b时会先将a转换为int，再将int转换为double

###### 多次类类型转换

不可以多次调用类类型转换

若Tmp类定义了一个到Test的类型转换，则

```cpp
Tmp tmp();
Test tst(tmp);		//正确，tmp转换为Test，再赋值给tst
int a(tmp);			//错误，不可以多次调用类类型转换
```

##### 重载二义性

###### 多个到内置类型的转换

假设Test类定义了到int的转换和到double的转换，对于一个参数为long double的函数将有二义性

```cpp
Test::operator int() const;
Test::operator double() const;

void func(long double);
func(Test());		//产生二义性
```

**因此不建议在类中实现多个到内置类型的转换**

###### 多个内置类型到类的转换

假设Test类定义了int到Test和double到Test的转换，对于一个参数为Test的函数，下列代码有二义性

```cpp
Test::Test(int);
Test::Test(double);

void func(Test&);
long a = 0;
func(a);
```

因为不知道应该将long转为double还是int

###### 两个类定义了同一个相互转换

假设Test1类定义了到Test2的转换，且Test2也定义了Test1到Test2的转换，如

```cpp
class Test1
{
public:
	Test1(Test2);
};

class Test2
{
public:
    operator Test1() const;
};

void func(Test1);
Test2 tst2;
func(tst2);		//出现二义性
```

但若这里`Test1(Test2)`变为`Test1(const Test2&)`，则会始终调用Test2定义的转换，因为前者多了一次引用，被认为是较差的匹配

###### 两个类定义了从同一种类型的转换

假设Test1和Test2都定义了从int的转换，则

```cpp
void func(Test1&);
void func(Test2&);

func(10);			//出现二义性，不知道调用哪个重载
func(Test1(10));	//正确
```

###### 操作符重载与类型转换冲突

下列情况可能出现二义性

```cpp
Test::Test(int);
Test::operator+(const Test&, const Test&);

Test tst1;
int num = tst1 + 3;		//出现二义性
```

此时存在两个转换

* tst1 -> int，调用内置的+
* 3 -> Test，调用int到Test的类型转换，再调用重载的+

##### 重载 转换和操作符

对于下列代码

```cpp
Test tst;
int a = tst + 3;
```

有如下几种可能性

* 有个重载的`operator+(const Test&, int)`
* 有重载的+运算符，但需要先把Test转换为int或把int转换为Test
* 既有类型转换又有匹配的重载运算符，有二义性
* 都没有，非法

###### 重载确定

* 选择候选函数
* 选择可行函数
* 选择最佳匹配函数

候选函数： 与被使用函数同名的函数，对于运算符，包括内置版本和非成员重载，如果左操作数是类，则包括该类的成员重载版本

可行函数： 如果通过类成员调用操作符，则排除非成员版本，否则都考虑

###### 二义性消除

* 不要重复定义转换，若Test1定义了从Test2的构造函数，Test2不能定义operator Test1()
* 避免定义到内置类型的转换，若定义了
  * 不要定义到两个内置类型的转换
  * 不要定义接受这种内置类型的重载运算符，如若定义了Test到int的转换则不要定义`operator+(const Test&, const Test&)`，因为默认的+接受int作为输入

## 第十五章 面向对象编程

### 概念

#### 继承与多态

##### 继承

继承是一种在定义一个类时可以直接复用另一个类的接口和成员的方法。被继承的类称为基类或父类，继承的类称为派生类或子类

如，类B继承了类A，而类A定义了test函数，则类B在定义时自动拥有test函数，且可以对test进行重载

##### 多态

多态指派生类和基类可以根据传入实例类型的不同来调用多种不同的类对应的方法。

假设类B重载了test函数，当使用一个类A的引用或指针调用test时，该指针也可以传入其派生类的引用或指针（在这个例子即类B），具体调用的test函数取决于传入的指针是类A还是类B。

这种根据传入类型确定具体调用函数的行为叫**动态绑定**

### 定义基类与派生类

#### 基类

这里使用一个简单例子：

基类

```cpp
class Father
{
public:
    Father(): num(0), ch('0') {  }
    int GetNum() const { return num; }
    virtual char GetChar() const { return ch; }
    virtual ~Father();
    
    std::string name;
private:
    int num;
protected:
    char ch;
}
```

#### 派生类

##### 格式

```cpp
class classname: access-label base_class
{
    
};
```

* access-label有private public protected三种
* 在派生类定义前应该有对base_class的定义
* 派生类本身可以作为其他类的基类

##### 例子

```cpp
class Child: public Father
{
public:
    char GetChar() const { return mych; }	//重载了虚函数GetChar
private:
    char mych;			//额外定义了成员mych
};
```

### 访问控制与继承

派生类对基类的访问权限由基类成员定义时的关键字决定

而派生类继承的成员的访问控制由继承时的access-label决定

#### 基类成员

| 关键字    | 派生类对象访问基类成员 | 外部访问 | 友元类/函数 |
| --------- | ---------------------- | -------- | ----------- |
| public    | 允许                   | 允许     | 允许        |
| private   | 不允许                 | 不允许   | 允许        |
| protected | 允许                   | 不允许   | 允许        |

**注意，对于派生类来说，只能通过派生类对象来访问基类的protected成员**，如下

假设Test继承TestBase，TestBase定义了一个名为num的protected成员

```cpp
int Test::GetNum(Test& tst, TestBase& base)
{
    int ret;
    ret = num;			//合法 this->num
    ret = tst.num;		//合法 在Test类成员函数中通过Test对象访问protected元素
    ret = base.num;		//非法，在Test类成员函数中直接访问基类对象的protected元素
    return ret;
}
```

#### 继承类型

继承类型不同，将改变基类成员在派生类中的访问权限

##### 成员

下表展示了在不同继承类型下，不同类型的基类成员继承到派生类后的类型

| 基类成员类型 \ 继承类型 | private  | public    | protected |
| ----------------------- | -------- | --------- | --------- |
| private                 | 无法继承 | 无法继承  | 无法继承  |
| public                  | private  | public    | protected |
| protected               | private  | protected | protected |

简单说public继承保持原有属性，private继承全部变为private，protected继承全部变成protected

##### 成员函数

下表展示了不同继承类型下，不同类型的基类成员函数继承到派生类后的类型

| 基类成员类型 \ 继承类型 | private  | public   | protected |
| ----------------------- | -------- | -------- | --------- |
| private                 | 无法继承 | 无法继承 | 无法继承  |
| public                  | 实现继承 | 接口继承 | 实现继承  |
| protected               | 实现继承 | 接口继承 | 实现继承  |

##### 默认继承类型

对于class为private，对于struct为public

PS：这是使用class和struct定义类时的唯一区别

#### 去除个别成员

在继承的时候可以指定某些成员为例外，不按照继承类型来继承。但权限不可能高于基类成员类型本身所赋予的权限

使用using关键字，如

```cpp
class ChildPrivate : private Father
{
public:
    using Father::name;
protected:
    using Father::ch;
};
```

#### 友元

友元关系无法继承，即基类的友元类不是派生类的友元类，且当某个类的友元类是基类，派生类不是这个类的友元类

#### 静态成员

若基类定义了静态成员，则基类与所有派生类中只会出现这一个静态成员，即不管从哪个类访问都会指向同一个变量

### 虚函数

虚函数使用virtual定义，它的作用就是**触发动态绑定机制**，即使用virtual声明的函数是动态绑定的

对于virtual函数，一旦具有了该属性，则其派生类自动拥有virtual属性

纯虚函数见下面

#### 调用

沿用上面的类

```cpp
Father father;
Child child;
Father *p;

p = &father;
p->GetChar();			//调用Father::GetChar，动态绑定

p = &child;
p->GetChar();			//调用Child::GetChar，动态绑定

p->Father::GetChar();	//强制调用Father::GetChar，编译时确定函数地址
```

即，基类对象的指针或引用可以指向基类或派生类，从而调用不同的函数

#### 默认实参

对于虚函数来说，可以声明默认实参，但**不能让基类和派生类的默认实参不同**

因为默认实参是编译时决定的，因此若以基类指针调用虚函数，则默认实参就是基类虚函数定义的。因此当传入派生类指针时，若没指定实参则将以基类的默认实参来调用派生类的虚函数

### 类型转换与继承

#### 指针和引用

因为基类可以看做作为派生类中的一个子类存在，因此对**派生类的引用可以直接转换为对基类的引用**，但反之则不行

#### 对象的类型转换

对象的类型转换不同于引用的类型转换，对于引用来说，只是改变了引用的解释方式（使用不同的引用类型），不改变类对象本身，但对象的类型转换通常包含赋值、复制等操作

##### 派生类到基类对象

* 情况一： 基类定义了派生类为参数的构造函数（不太可能）
* 情况二：基类定义了operator=和复制构造函数，则传入派生类引用时可以直接转换为基类引用，从而直接实现基类对象初始化

##### 派生类到基类转换的可访问性

* public继承  用户代码和后代类都可以使用派生类到基类的转换
* protected继承 用户代码不能使用，后代类可以转换为基类
* private继承  后代类不能转换为基类

这主要是由于private继承中，后代类（第三代）完全无法访问派生类（第二代）成员，也就无法转换为基类（第一代）了

##### 基类到派生类对象

没有从基类到派生类的隐式转换，因此需要这个功能时不仅得定义类型转换函数，还需要static_cast/dynamic_cast

### 构造函数和复制控制

#### 基类构造函数

基本不受影响，但需要注意将只提供给派生类的函数定义为protected

#### 派生类构造函数

##### 合成的默认构造函数

* 调用基类构造函数
* 调用派生类构造函数的其他部分

##### 自定义构造函数

对于自定义构造函数，也会首先隐式调用基类构造函数

##### 向基类构造函数传递实参

在初始化列表中调用基类构造函数，如

```cpp
Child::Child(): Father(1) {  }
```

但只能显式初始化直接基类，即派生类的派生类不能直接初始化基类

#### 复制控制

##### 复制构造

###### 合成的复制构造

* 调用基类复制构造
* 派生类本身成员按照前面所述的合成复制构造的方法生成

###### 自定义复制构造

不会隐式调用基类复制构造，因此一般需要在初始化列表调用

```cpp
Child::Child(const Child& child): Father(child) {  }
```

##### 赋值

与上述类似，自定义时需显式调用

```cpp
Child& Child::operator=(const Child& rhs)
{
    if(this != &rhs)
    {
        Base::operator=(rhs);
        ...;
    }
}
```

##### 析构函数

派生类的析构只需要清理自定义的内容，基类的析构会被隐式调用

#### 虚析构函数

当delete一个Father指针，实际指向Child类，这种情况下需要使用虚析构函数使析构函数被动态绑定

其他地方符合上小节描述的析构函数

**虽然析构函数应为虚函数，但构造函数和operator=不应是虚函数**，前者运行时对象还未完全确定类型，后者因为虚函数定义后所有的参数都应相同，而operator=对于不同类参数应该不同

#### 对象构造过程

对象的构造过程是循序渐进的，先调用基类构造函数，再是派生类，如果要构造的类是后代类则会层层调用。从某种意义上说，当基类构造函数运行完的这一时刻该类的类型应该是基类类型，等到下一级构造函数运行完才变为下一个类型。

因此若某个层次的类的构造函数调用了虚函数，即使当前构造的类不是该类，也会调用该类定义的虚函数

### 类作用域与名字

类在继承时，作用域可以视作是嵌套的，即派生类作用域嵌套在基类中，这样保证了

* 基类无法找到派生类自定义的成员
* 当派生类定义了与基类同名的成员时将屏蔽基类成员，除非指定作用域
* 当派生类定义了与基类同名的成员函数时也会屏蔽，且不是以重载的形式（即只要同名就算参数不同也会覆盖）
  * 由于上述的原因，若派生类要重载基类的某个函数，则应在派生类中重新实现基类定义的所有版本。或是用using将基类的所有同名函数作用域加入派生类的作用域。
  * 同样由于上述原因，虚函数在每个类中的定义必须是相同的，否则会被屏蔽

### 纯虚函数

当一个类只是用来给其他类继承并覆写接口的，则该类不应被实例化，这种类被称为**抽象基类**

抽象基类可以通过将一个或多个函数（可以是继承的）声明为纯虚函数定义，方法如下

```cpp
class Abstract: Father
{
    char GetChar() const = 0;
};
```

**注意这里的GetChar在Father中是virtual声明的函数**

也可以直接在基类中声明一个纯虚函数

```cpp
class Abstract
{
	virtual void display();
};
```

### 容器与继承

将容器直接用于同时保存基类对象或派生类对象可能导致问题

* 若容器保存了基类对象，那么使用派生类对象赋值将会使其强制转换为基类，派生类对象的成员全部被砍掉
* 若容器保存派生类对象，则没有从基类对象到派生类对象的转换

因此考虑保存指针

### 句柄类与继承

在C++中，要实现动态绑定的特性只能通过指针调用对象

```cpp
Father *p = &child;
Father& ref = child;
Father f = child;		//这里使用子类初始化一个父类对象

p->func();				//这里调用子类child对象的func
ref.func();				//同上
f.func();				//这里只能调用父类的func
```

句柄类旨在通过一个包装类来避免指针的使用的同时实现动态绑定

#### 设计要点

* 应该实现类似智能指针的功能
* 句柄类决定了句柄接口屏蔽或是不屏蔽继承层次，若不屏蔽，用户需了解和使用各个层次的对象

#### 指针型句柄

不屏蔽继承层次。

这种句柄只对类做一个比较简单的包裹，使其可以直接通过类对象调用动态绑定的函数，而不需要通过指针

##### 实现

下面是一个简易的实现，完整代码在`C++PRIMER_CODE/chap15/pointer_handler.cc`

```cpp
class Handler
{
public:
    Handler(): use(new int(1)), class_ptr(nullptr) {  }
    Handler(ClassBase& p): use(new int(1)), class_ptr(&p) {  }
    Handler(const Handler& h): use(h.use), class_ptr(h.class_ptr) { *use++; }
    ~Handler() { DecrUse(); }
    Handler& operator=(Handler& rhs) { *(rhs.use)++; DecrUse(); use = rhs.use; class_ptr = rhs.class_ptr; return *this; }
    ClassBase& operator*() const { if(class_ptr) { return *class_ptr; } else { throw std::logic_error("NULL REF"); } }
    ClassBase* operator->() const { if(class_ptr) { return class_ptr; } else { throw std::logic_error("NULL Pointer"); } }
private:
    void DecrUse() { if(--*use == 0) { delete class_ptr; delete use; } }

    int *use;
    ClassBase* class_ptr;
};
```

这里包含了智能指针的功能，主要通过重载*和->来实现句柄功能

##### 使用

此时假设需要调用一个派生类的成员函数，以往需要

```cpp
int CallFunc(ClassBase& a)
{
    return a.func();
}
```

或者

```cpp
int CallFunc(ClassBase* a)
{
    return a->func();
}
```

现在可以支持这样的调用

```cpp
int CallFunc(Handler a)
{
	return a->func();
}
```

因为Handler类重载了->，因此`a->func()`相当于`p = Handler::operator->(a); p->func();`

#### 值型句柄

屏蔽继承层次

值型句柄不限于简单的包裹类，而是在初始设计的时候就将各个类的继承关系考虑好，对外部用户来说，只需要使用一个同样的句柄类来进行所有的操作，但实际上内部通过某些设计达到通过统一的句柄类调用不同类对象的目的

举例说明，现在有一个表示物品的基类Item，其中一个派生类为书本ItemBook，构造时需要传入书名和价格，另一个派生类为衣服ItemClothes，构造时需要传入款式和颜色。我们在构造时无论输入的是书本还是衣服，都使用`Handler("book name", 10)`或`Handler("Clothes", "red")`。至于如何区分两者则封装在Handler的重载构造函数中。

更具体的例子见`C++PRIMER_CODE/chap15/TextQueryPlus.cc`