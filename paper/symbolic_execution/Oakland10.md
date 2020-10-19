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

执行的上下文可以通过下列几个符号描述

* Σ   表示程序语句的集合
* µ   表示当前内存状态
* ∆   表示当前变量状态
* pc  程序计数器
* $ι$     当前指令

#### 一些语法表示

* ∆[x]  在context ∆中变量x的值
* ∆[x  ← 10]  给context ∆中的变量x赋值10
* $µ, ∆ \vdash e ⇓ v$  在context µ, ∆中，将表达式e的值赋给v

### 使用SimpIL表示操作

下面使用SimpIL定义一些常用语句

格式为
$$
\frac{ 实际操作 }{ 影响的context \vdash 操作名 ⇓ 影响的变量 }
$$

$$
\frac{实际操作}{<当前context>, stmt \ \leadsto \  <执行后的context>,stmt'}
$$



#### INPUT

get_input
$$
\frac{v \ is \ input \ from \ src}{ µ, ∆ \vdash get\_input(src) ⇓ v }
$$

#### LOAD

load，从内存v1处加载一个值
$$
\frac{µ, ∆ \vdash e ⇓ v_1  \ \   v = \mu [v_1]}{ µ, ∆ \vdash load \ e ⇓ v }
$$

#### VAR

var，表示变量声明
$$
\frac{  }{ µ, ∆ \vdash var ⇓ \Delta[var] }
$$

#### UNOP

表示一元运算
$$
\frac{µ, ∆ \vdash e ⇓ v \ \ v' = ◇_ue}{µ, ∆ \vdash ◇_ue ⇓ v'}
$$

#### BINOP

表示二元运算
$$
\frac{µ, ∆ \vdash e_1 ⇓ v_1 \ \ µ, ∆ \vdash e_2 ⇓ v_2 \ \  v' = v_1 ◇_b v_2}{µ, ∆ \vdash e_1 ◇_b e_2 ⇓ v'}
$$

#### CONST

表示定义常量
$$
\frac{}{µ, ∆ \vdash v ⇓ v}
$$

#### ASSIGN

表示赋值
$$
\frac{µ, ∆ \vdash e ⇓ v \ \ \Delta^{'} = \Delta[ var \leftarrow v ] \ \ \iota = \Sigma[pc+1] }{\Sigma,\mu,\Delta,pc,var := e \leadsto \Sigma,\mu,\Delta^{'},pc+1,\iota}
$$

#### GOTO

$$
\frac{\mu,\Delta \vdash e \Downarrow v_1 \ \ \iota = \Sigma[v_1]}{\Sigma,\mu,\Delta,pc,goto \ e \leadsto \Sigma,\mu,\Delta,v_1,\iota}
$$

#### TCOND

$$
\frac{µ, ∆ \vdash e ⇓ 1 \ \ \ ∆ \vdash e_1 ⇓ v_1 \ \ \iota = \Sigma[v_1]}{\Sigma,\mu,\Delta,pc, if \ e \ then \ goto \ e_1 \ else \ goto \ e_2 \leadsto \Sigma,\mu,\Delta,v_1,\iota}
$$

#### FCOND

$$
\frac{µ, ∆ \vdash e ⇓ 0 \ \ \ ∆ \vdash e_1 ⇓ v_2 \ \ \iota = \Sigma[v_2]}{\Sigma,\mu,\Delta,pc, if \ e \ then \ goto \ e_1 \ else \ goto \ e_2 \leadsto \Sigma,\mu,\Delta,v_2,\iota}
$$

#### STORE

$$
\frac{µ, ∆ \vdash e_1 ⇓ v_1 \ \ \ µ, ∆ \vdash e_2 ⇓ v_2 \ \ \iota = \Sigma[pc+1] \ \ \ \mu^{'} = \mu[v_1 \leftarrow v_2]}{\Sigma,\mu,\Delta,pc, store(e_1, e_2) \leadsto \Sigma,\mu',\Delta,pc+1,\iota}
$$

#### ASSERT

$$
\frac{µ, ∆ \vdash e ⇓ 1 \ \ \ \iota = \Sigma[pc+1]}{\Sigma,\mu,\Delta,pc, assert(e) \leadsto \Sigma,\mu',\Delta,pc+1,\iota}
$$

### 示例

#### 简单例子

```
x := 2 * get_input(.)
```

下面展示当get_input的输入为20时的SimpIL
$$
\frac{\frac{\frac{}{\mu,\Delta \vdash 2 \Downarrow 2} CONST \ \ \frac{20 \ is \ input}{\mu, \Delta \vdash get\_input() \Downarrow 20 } \ \  INPUT \ \ v' = 2*20}{\mu, \Delta \vdash 2*get\_input() \Downarrow 40} BINOP \ \  \Delta' = \Delta[x \leftarrow 40] \ \ \iota = \Sigma[pc+1]}{\Sigma, \mu, \Delta, pc, x := 2 * get\_input() \leadsto \Sigma, \mu, \Delta', pc+1, \iota}
$$

#### 将SimpIL用于复杂的调用

这里通过手动压栈和加载栈上的地址实现跳转

```
esp := esp + 4
store(esp, 6)
goto 9

6: halt
9: ...    ; func
goto load(esp)
```

为了分析这种情况，需要加入新的符号

* λ    栈的状态
* ζ    函数局部变量
* φ   函数地址

$$
\frac{\mu, \Delta \vdash e_1 \Downarrow v_1 ... \mu, \Delta \vdash e_i \Downarrow v_i \ \ \Delta' = \Delta[x_1 \leftarrow v_1, ... , x_i \leftarrow v_i] \ \ pc'=\phi[f] \ \ \iota = \Sigma[pc']}{\lambda,\Sigma,\phi,\mu,\Delta,\zeta,pc,\ \ call \ f(e_1,...e_i) \leadsto (pc+1)::\lambda,\Sigma,\phi,\mu,\Delta',\Delta::\zeta,pc',\iota} \ CALL

\\
~
\\
\frac{\iota = \Sigma[pc']}{pc'::\lambda',\Sigma,\phi,\mu,\Delta,\Delta'::\zeta,pc,\ \ return \leadsto \lambda',\Sigma,\phi,\mu,\Delta',\zeta',pc',\iota} \ RET
\\
~
\\
\frac{\mu, \Delta \vdash e \Downarrow v \ \ v \notin dom(\Sigma) \ \ s = disassemble(\mu[v]) \ \ \Sigma' = \Sigma[v \leftarrow s] \ \ \iota = \Sigma'[v]}{\Sigma,\mu,\Delta,pc,\ \ jmp \ e \leadsto \Sigma',\mu,\Delta,v,\iota} \ GENCODE
$$



