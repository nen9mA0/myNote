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

#### 指向const对象的指针：若对象为const，指针必须声明其指向const对象

```c++
const int a;
const int* b = &a;
```

#### 指向const对象的指针：可以用const对象指针指向非const对象

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

### 面向对象的标准库

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

