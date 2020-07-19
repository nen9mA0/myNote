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

可以使用键作为下标访问，若使用`=`对某个键的元素进行赋值时但没有对应的键时，会自动插入一个新的键值对

```cpp
map<string, int> m;
m["1"] = 1;
int value = m["1"];
```

##### 插入

除了使用上述方法插入元素，还可以使用insert

```cpp
map<string, int> m;
pair<string, int> e;
pair<string, int>::iterator beg, end, iter;

m.insert(e);		//插入一个pair
m.insert(beg, end);	//插入迭代器之间的元素(若没有对应键)
m.insert(iter, e);	//若e.first不在map内则创建，并以iter为起点搜索元素存储的位置
```

