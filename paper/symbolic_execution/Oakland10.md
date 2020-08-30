**All You Ever Wanted to Know About Dynamic Taint Analysis and Forward Symbolic Execution (but might have been afraid to ask)**

（标题太长了不直接写）

这篇是概述性的论文，首先用形式化的语言描述了符号执行和动态污点分析，然后介绍了几种实现方案

### 形式化描述语言

#### 语句描述

* program
  * stmt*
* stmt s
  * var := exp
  * store(exp, exp)
  * goto exp
  * assert exp
  * if exp then goto exp else goto exp
* exp e
  * load(exp)
  * exp $◇_b$ exp
  * $◇_u$ exp
  * var
  * get_input(src)
  * v
* $◇_b$  表示二元运算
* $◇_u$  表示一元运算
* v   表示一个32位无符号整数

#### 上下文描述

* Σ   表示程序语句的集合
* µ   表示存放当前value的地址
* ∆   表示当前value的名字
* pc  程序计数器
* $ι$     下一条指令

#### 一些语法表示

* ∆[x]  变量x的值
* ∆[x  ← 10]  给变量x赋值10
* $µ, ∆ \vdash e ⇓ v$