# JAVASCRIPT权威指南

## 第2章 词法结构

### 字符集

#### 区分大小写

HTML不区分大小写但js区分，因此对于js和HTML同名的属性和对象在js中必须小写

#### 空格、换行和格式控制符

js忽略程序中标识间的空格和换行符

#### Unicode转义

对于无法输入或显示unicode的设备，js定义了转义序列，如\\u00e9

在进行转义序列与对应unicode比较时是完全一样的，如

```javascript
console.log("caf\u00e9" === "café")
VM190:1 true
```

#### 注释

* //
* /*  */

#### 可选的分号

```javascript
a=3;b=4;
//与
a=3
b=4
//效果相同
```

但对于没有分号的情况，可能会导致一些代码错误。如

```javascript
x
++
y
```

会被解析为 x;++y;

## 第3章 类型、值和变量

### 数字

#### 整型

表示范围：-2^53 ~ 2^53

**整型**为**54**位整数，**下标**为**32**位整数

十六进制：0x或0X开头

八进制：0开头

#### 浮点数

64位

#### 算术运算

* \+ \- \* \\ %
* Math.pow()
* Math.round()
* Math.ceil()
* Math.floor()
* Math.abs()
* Math.max()
* Math.min()
* Math.random()
* Math.Pi
* Math.E
* Math.sqrt()
* Math.pow()
* Math.sin()
* Math.log(x)   x以e为底的自然对数
* Math.log(x)/Math.LN10  x以10为底的对数

#### 