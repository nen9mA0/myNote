# 关于EOF

原文链接：[https://latedev.wordpress.com/2012/12/04/all-about-eof/](https://latedev.wordpress.com/2012/12/04/all-about-eof/)

## 介绍

对于C/C++初学者来说，在Reddit和StackOverflow上提出的问题很多都是关于在与用户交互或读取文件时如何处理EOF。我估计有95%以上的问题都是因为误解了EOF的含义。这篇文章试图解释所有关于EOF的问题，特别是对于使用Windows和类UNIX操作系统的C/C++程序员。

## EOF符号的历史

许多程序员第一次遇到关于EOF的问题是当他们处理EOF符号的时候——其实并不存在所谓EOF符号但人们总觉得它存在。不论是Windows还是Linux都没有任何特殊的标记字符用于指出一个文件的末尾。如果你在Windows上用记事本打开一个文本文件，或在Linux上用Vim打开，抑或是任何你能想到的编辑器/操作系统，文件中任何地方都不会包含有一个特殊字符用于标记文件末尾。因为Windows和Linux都有文件系统来标识文件的大小，没有必要通过一个特殊字符指出文件末尾。



既然Windows和Linux都没有EOF符号，那这个符号是谁定义的？很久以前有一个运行在8位处理器上的叫CP/M的操作系统。CP/M的文件系统没有记录文件的确切大小——它只能记录文件占用了几个扇区。这意味着如果你只想创建一个只含有字符串"hello world"的文件，CP/M不知道这个文件只有11字节——它只知道它占用了一个扇区，即至少128字节的空间。（译者吐槽：这什么憨憨系统）如果人们想知道他们文件具体有多大，就需要文件末尾标识符。CP/M重用了Ctrl-Z字符（ascii码0x1A，原用于表示替换）（译者注：原文貌似有误）：当CP/M系统的应用程序读到一个Ctrl-Z字符时它可以认为自己已经读到文件末尾。操作系统没有强制程序这样解释0x1A这个值；那些处理二进制数据的程序需要使用其他方法来知道它们是否已经到达文件末尾。操作系统本身没有对这个值有任何特殊处理。



当MS-DOS出现时，兼容CP/M很重要，因为很多MS-DOS的程序都是直接通过机器翻译将程序的8080/Z80处理器机器码转换成8086机器码。为了不重写程序，MS-DOS依然将Ctrl-Z视为文件末尾标志。事实上，如果一个文件以文本模式打开，这种对于Ctrl-Z的操作内置于微软的C运行库中。这里必须重申Windows操作系统本身并不处理Ctrl-Z字符，这些操作是微软的运行库实现的。同样必须说明的是这些问题只出现在Windows上，Linux和其他类UNIX系统从没把Ctrl-Z当做文件末尾标志。



## 一些demo

可以通过这些代码来演示微软运行库的这一特性。首先，写一个程序，并且在文件里加入一个Ctrl-Z字符

```C
#include <iostream>
#include <fstream>
using namespace std;

int main() {
    ofstream ofs( "myfile.txt" );
    ofs << "line 1\n";
    ofs << char(26);
    ofs << "line 2\n";
}
```

如果你在Windows或Linux上编译或运行这段代码，它将生成一个包含Ctrl-Z的文件。对于两种系统来说Ctrl-Z都不能被打印。现在你可以尝试使用命令行来读取文件，在Windows中：

```
c:\users\neilb\home\temp>type myfile.txt
line 1
```

注意文件只有第一行被打印出来

在Linux中：

```
[neilb@ophelia temp]$ cat myfile.txt
line 1
?line 2
```

两行都被显示了，注意两行之间还有一个字符?，因为cat命令像读取其他字符一样读取Ctrl-Z并试图打印它——打印出的结果取决于你用的哪种终端。



这似乎表明Windows操作系统确实会对Ctrl-Z字符做出处理，但这是错误的——只有应用程序代码处理它。如果你用Notepad打开该文件，你会看到下图：

![](I:\Git\Note\translate\program\all about EOF\1.jpg)

每一行文本都被显示了，包括中间的Ctrl-Z——这表明notepad不知道Ctrl-Z是文件末尾标识符。

## 文本模式和二进制模式

所以type指令和notepad到底差别在哪？这很难说明。可能type指令有一些特殊代码用于检查输入中是否有Ctrl-Z。不过Windows使用的C++ iostream库和C stream库有标识打开的文件是二进制模式还是文本模式的选项，这会对文件读取造成影响



这是一个典型的使用C++读取文本文件的例子：

```C++
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    ifstream ifs( "myfile.txt" );
    string line;
    while( getline( ifs, line ) ) {
        cout << line << '\n';
    }
}
```

如果在Windows上编译并运行这个程序，你会发现Ctrl-Z被视为文件末尾标志。输出为：

```
line 1
```

然而，如果你使用二进制模式打开文件，如下：

```C++
ifstream ifs( "myfile.txt", ios::binary );
```

输出为

```
line 1
?line 2
```

Ctrl-Z字符只在文本模式中被视为特殊字符，在二进制模式中，它只被当做普通字符。

**注意：**这只适用于Windows程序，在Linux程序上两段代码的行为完全相同。



所以这里有两点需要记住：

* 如果你希望文件以文本形式打开，并且保证它的可移植性，不要在文件中加入Ctrl-Z字符
* 如果你必须加入Ctrl-Z字符，且你希望保持文件的可移植性，请以二进制模式打开该文件。

这里可移植性指使用任意程序，即使你只在Windows上编程。

## 那Ctrl-D呢

一些Linux用户可能在想，“所以当我想结束一个shell输入时输入的Ctrl-D呢？这也不是一个文件末尾标志么？” 当然，它不是。如果你在一个文本文件中插入Ctrl-D字符，Linux并不关心它。实际上，当你希望结束shell输入的时候输入的Ctrl-D只是一个用于通知shell关闭输入流的信号（译者注：Linux中的signal），没有任何字符将被插入输入流中。使用stty命令你可以将终止输入的Ctrl-D改为任何其他的字符，但不论如何不会有任何字符插入到输入流中，就算有，Linux也不会把它当做文件末尾标志。



## C/C++中EOF的值

还有一个很令人疑惑的事，C/C++都定义了一个名为EOF的值，在C中，被定义在<stdio.h>里：

```C
#define	EOF	(-1)
```

C++的\<cstdio\>有相同的定义。

注意这里的EOF和Ctrl-Z没任何关系。它的值不是26，它甚至不是个字符而是个整数。它一般被用作函数的返回值：

```C
int getchar(void);
```

getchar()函数用于从标准输入读入字符，当读到文件末尾时返回值为EOF。所谓文件末尾可能是Ctrl-Z所标记的也可能不是（如上面对于文件末尾标志的讨论）。但不管怎么样EOF的值都不会等于26.**事实上getchar()返回值是int而不是char**，所以将返回值放到一个int变量中很重要，因为一个char类型和有符号整型的比较不能保证正确性。典型的使用getchar()读入标准输入的代码如下：

```C
#include <stdio.h>

int main() {
    int c;
    while( (c = getchar()) != EOF ) {
        putchar( c );
    }
}
```

## eof()和feof()函数

还有一个关于EOF的误区是C/C++都提供了函数来检查输入流的状态。几乎所有初学者都对这几个函数感到疑惑，因此预先说明它们用来做什么，不能怎么用：

**eof()和feof()都用于检查输入流是否满足已经读到文件末尾的条件。当且仅当程序尝试读入时才可能触发到达文件末尾的条件。如果你没有先进行读操作就调用这两个函数，你的程序就可能出错。永远不要循环调用eof函数**（译者注：此处译者理解为永远不要在没有进行读操作时循环调用eof函数）

为了解释，我们写了一个读取文件并打印行号和文件内容的程序。为了简化程序我们使用静态文件路径名并且忽略错误处理。大多数初学者会写出这样的代码：

```C++
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    ifstream ifs( "afile.txt" );
    int n = 0;
    while( ! ifs.eof() ) {
        string line;
        getline( ifs, line );
        cout << ++n << " " << line << '\n';
    }
}
```

这看起来没什么问题，但请记住前面的建议——如果你没有先进行读操作就调用这两个函数，你的程序就可能出错。这里在读操作前确实调用了一次eof()。为了说明为什么这样写是错的，考虑文件**afile.txt**是个空文件。第一次进入循环时eof()将返回True，因为之前没有任何读入操作。接下来我们读入了一行，eof条件将被触发，但为时已晚，我们接下来会打印行号为1的空行，即使文件中并没有任何内容。同样的，对于其他文件该程序将总是多打印一行。



为了改正程序，我们应该在读操作后调用eof()，或者根本不调用这个函数。比如如果你不想处理除了读取到达文件末尾外的任何其他问题，你可以这样写：

```C++
int main() {
    ifstream ifs( "afile.txt" );
    int n = 0;
    string line;
    while( getline( ifs, line ) ) {
        cout << ++n << " " << line << '\n';
    }
}
```

这里直接使用getline()函数的返回值作为循环判断条件，此处使用了隐式类型转换，将getline()的返回值转换为while循环条件表达式可接受的值——只要输入流没有到达文件末尾或产生其他错误将一直读取。



C中可能出现类似的错误程序：

```C
#include <stdio.h>

int main() {
    FILE * f = fopen( "afile.txt", "r" );
    char line[100];
    int n = 0;
    while( ! feof( f ) ) {
        fgets( line, 100, f );
        printf( "%d %s", ++n, line );
    }
    fclose( f );
}
```

这个程序也会多打一行

所以应该这么写：

```C
#include <stdio.h>

int main() {
    FILE * f = fopen( "afile.txt", "r" );
    char line[100];
    int n = 0;
    while( fgets( line, 100, f ) != NULL ) {
        printf( "%d %s", ++n, line );
    }
    fclose( f );
}
```

那么eof()和feof()看起来没什么用为什么C/C++标准库支持它们呢？其实它们在处理不是由于读到文件末尾而产生的错误很有用。这种情况下你需要区分是否是因为读到文件末尾才产生了错误：

```C++
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    ifstream ifs( "afile.txt" );
    int n = 0;
    string line;
    while( getline( ifs, line ) ) {
        cout << ++n << " " << line << '\n';
    }
    if ( ifs.eof() ) {
        // OK - EOF is an expected condition
    }
    else {
        // ERROR - we hit something other than EOF
    }
}
```

（译者注：显然在try catch里也很有用）



## 总结

如上所述关于EOF的问题看起来很复杂，但其实无外乎三个基本的规则：

* 没有所谓EOF字符，除非你在Windows下以文本模式打开文件或者你自己实现一个
* C/C++的EOF符号不是文件末尾标志，而只是对于一些库函数的特殊返回值
* 别循环调用eof()和feof()

如果你将这些规则牢记于心，你将避免几乎所有因为误解C/C++中文件结束条件而引发的bug