[The Sail instruction-set semantics specification language (alasdair.github.io)](https://alasdair.github.io/manual.html#_a_tutorial_risc_v_style_example) 

### 介绍

Sail的主要设计思想

* 可以精确地定义现代ISA的语义

* 使用类似ARM或IBM Power的较为友好的伪代码语法描述

* 支持描述顺序ISA与relaxed内存并发模型的语法

* 提供可以描述bitvector与索引运算的语法

* 支持由sail定义进行架构的模拟

* 支持自动生成定理证明器定义，以便对ISA规范进行推理

* 尽可能最小化语言，以简化推理与定理证明器定义自动生成

Sail定义可以被转换为AST，并且可以通过模拟器模拟运行时的行为

通过Sail语言描述ISA，可以生成下述内容

* 含有类型标注的内部定义，并且可以被翻译器翻译为Lem OCaml和其他多种定理证明器使用的语言；同时翻译器也可以被用于分析指令的定义来验证其寄存器和内存操作

* 浅层嵌入定义（原文为shallow embedding of the definition），这类语言可以被更直接地翻译为定理证明器语言，目前支持生成的目标是Isabelle/HOL和HOL4

* 可以直接翻译到OCaml

* 可以直接翻译为C语言

目前主要支持几种架构

* Arm-v8/v9

* MIPS  手写规则

* CHERI-MIPS  手写规则

* RISCV  手写规则

### Sail语法

#### 函数

##### 声明

```ocaml
val my_replicate_bits : forall 'n 'm, 'm >= 1 & 'n >= 1.
    (int('n), bits('m)) -> bits('n * 'm)
```

使用`'` 标识的为类型变量，int和bits用于标识n和m的类型

函数声明中对n和m添加了约束，并指定了函数输出为`bits('n * 'm)`

##### 定义

```ocaml
function my_replicate_bits(n, xs) = {
    var ys = zeros(n * length(xs));
    foreach (i from 1 to n) {
        ys = ys << length(xs)
        ys = ys | zero_extend(xs, length(ys))
    };
    ys
}
```

##### 隐含参数

SAIL支持定义隐含参数，其会根据调用的上下文确定输入或输出的类型

```ocaml
val extz : forall 'n 'm, 'm >= 'n. (implicit('m), bits('n)) -> bits('m)
function extz(m, xs) = zero_extend(xs, m)
```

这里的m就是一个隐含参数，根据输出的长度来确定

```ocaml
let xs: bits(32) = 0x0000_0000;
let ys: bits(64) = extz(xs)
```

#### 块

用括号定义块，并且与rust和OCaml类似，SAIL中每个语句/语句块都是一个表达式，因此应有返回值

```ocaml
{
    let x : int = 3;
    var y : int = 2;
    y = y + 1;
    x + y
}
```

#### unit

与rust中的思想类似，SAIL采用`()`这种类似空元组的形式表示空
