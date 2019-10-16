## 基本使用

```bash
awk [options] 'program' file1,file2...
```

基本处理方式：读入文件的一行，运行一次program

### 内置变量

```bash
awk '{print $1,$2}' test.txt
```

以**空格/tab**为间隔输出第一行和第二行内容

#### $0

整行文本

#### $NF

最后一个字段

#### NF

文本被分成了几段

### print使用

```bash
awk '{print "col1:"$1,"col2:"$2}' test.txt
```

会在第一列加入col1：，第二列加入col2：

### BEGIN和END

BEGIN指定开始执行前的动作

END指定最后一个执行的动作

```bash
awk 'BEGIN{print "begin"} {print "col1:"$1} END{print "end"}' test.txt
```

### 指定分隔符

* 输入分隔符  field separator  FS
* 输出分隔符  output field separator  OFS

#### 输入分隔符

指定方式：-F或-v FS=''

```bash
awk -F# '{print $1}' test.txt
```

或

```bash
awk -v FS='#' '{print $1}' test.txt
```

#### 输出分隔符

指定方式：-v OFS=''

```bash
awk -v OFS='---' '{print $1,$2}' test.txt
```

## 变量

### 内置变量

* FS  输入分隔符
* OFS  输出分隔符
* RS  输入换行符
* ORS  输出换行符
* NF  字段数量
* NR  行号
* FNR  各文件分别计数的行号
* FILENAME  当前文件名
* ARGC  命令行参数个数
* ARGV  命令行参数

FNR与NR区别：若有多个文件NR不会按照不同的文件进行计数，即

NR

```bash
awk '{print NR,$0}' test1.txt test2.txt
```

```
1 NR FNR test1
2 NR FNR test2
```

FNR

```bash
awk '{print FNR,$0}' test1.txt test2.txt
```

```
1 NR FNR test1
1 NR FNR test2
```

### 自定义变量

-v var_name=""

```bash
awk -v myvar="myvar" 'BEGIN{print "test"; print myvar}' test.txt
```

#### 引用bash变量

```bash
bash_var="myvar"
awk -v myvar=$bash_var 'BEGIN{print "test"; print myvar}' test.txt
```

## 格式化

一般与printf配合使用

```bash
awk 'BEGIN{printf "col1\tcol2\n"} {printf "%s\t%s\n",$1,$2}' test.txt
```

## 模式

可以用于对每行指定不同条件

上面已介绍过的模式

* BEGIN和END模式
* 空模式

### 关系运算模式

#### 关系运算符

* <
* <=
* \>
* \>=
* ==
* !=
* ~  正则匹配为真
* !~  正则匹配不为真

#### 示例

```bash
awk 'BEGIN{printf "col1\tcol2\n"} NF==2{printf "%s\t%s\n",$1,$2}' test.txt
```

```bash
awk 'BEGIN{printf "col1\tcol2\n"} NF>1 && NF<3{printf "%s\t%s\n",$1,$2}' test.txt
```

```bash
awk '$2~/by/{print $0}' test.txt
```

### 正则模式

```bash
awk '/regex/{program}' filename
```

#### 示例

```bash
awk '/separa.*/{print $0}' test.txt
```

### 行范围模式

```bash
awk '/regex1/,/regex2/{program}' filename
```

返回regex1到regex2间的结果，即匹配regex1的行到匹配regex2的行之间的行都会执行program

#### 示例

```bash
awk '/space/,/tab/{print $0}' test.txt
```

## 动作

动作被{}所包围，包含了各种常用循环、分支语句

### if

```bash
awk '{ if(exp1){} else if(exp2){} else{} }' filename
```

#### 示例

```
awk '{ if(NR==1) print $0 }' test.txt
```

### for和while

```bash
awk ' for(exp1;exp2;exp3){program} ' filename
awk ' for(var in var){} ' filename		# for 变量 in 数组

awk ' while(exp){program} ' filename
awk ' do{program}while(exp) ' filename
```

#### 示例

```bash
awk 'BEGIN{for(i=0;i<2;i++){print i}}'
```

### continue与break

与C中一样

### next与exit

* next  直接对下行进行处理
* exit  跳到END块执行，执行完END后结束程序

### 三元运算符

与C中一样，var=exp1?exp2:exp3;

## 常用内置函数

### 算数函数

* srand()
* rand()
* int()  转换为整数

### 字符串函数

* gsub

  gsub(exp1,char,exp2)  exp1为要替换的字符或字符的正则，char为替换成的字符，exp2为范围，默认为$0

  ```bash
  gsub('a-c','d')  #把本行的a-c间的字母替换为d
  ```

* sub(exp1,char,exp2)  与gsub区别为sub只会替换第一个匹配

* length  字符串长度

* index(str1,str2)  获取str2位于字符串str1的位置，若无为0

* **spilt**

  split(var1,var2,str) 根据str对var1对应的内容进行分割，结果作为**数组**放在var2中，且**第一个元素下标为1**

  ```bash
  awk '{if(NR>7 && NR<10){ split_len=split($0,a,"==="); for(i=1;i<=split_len;i++){ print a[i] } }}' test.txt
  ```

  输出：

  ```
  
  test split
  
  a
  b
  c
  
  ```

* asort  根据元素值进行排序，输出结果为第一个元素下标为1的以元素值为内容的数组
* asorti  根据元素的下标进行排序，输出结果为第一个元素下标为1以元素下标为内容的数组

## 数组

### 定义

数组类型为关联数组，即下标可以为任意字符串

```bash
awk 'BEGIN{a[0]="1";a[2]="3"} END{for(i=0;i<4;i++){print a[i]}}' test.txt  #实际上没有对输入进行处理
```

输出：

```
1

3

```

**注意**：这里元素若未被初始化则默认赋值为""

## 实例1：词频统计

这里用到一个特性：**awk中对字符串进行加法时其值被当做0**

```
awk 'BEGIN{a="test"; print a+1}'
```

输出1

而已知对未定义过的数组元素的引用返回值为""

因此统计词频的awk语句如下

```bash
awk '{ for(i=1;i<=NF;i++){count[$i]++} } END{for(i in count){printf "%s\t%d\n",i,count[i]}}' test.txt
```

## 实例2：奇偶行输出

```
awk 'i=!i' test.txt  #奇行
awk '!(i=!i)' test.txt  #偶行
```

原理：i默认值为0，动作为空时默认为print $0