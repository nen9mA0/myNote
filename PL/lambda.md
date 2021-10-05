为这一系列blog的笔记

https://blog.csdn.net/g9yuayon/article/details/748684

https://cgnail.github.io/academic/lambda-1/

### Lambda Calculus 句法

* 函数定义：  `lambda x . <函数体>` 注意这里称这个lambda表达式**绑定**了参数x
* 标识符引用： lambda函数体中引用的标识符名即参数中的名字
* 函数应用： 把要传入的参数直接放到函数定义后，如 `(lambda x . plus x x) y` 

### 柯里化 Currying

这里的currying翻译为局部套用函数。因为在lambda的定义里，只能存在一个参数，这种写法通过写一个接受第一个参数的函数，返回一个接受第二个参数的函数来解决该问题，例如计算x+y

```
lambda x . (lambda y . plus x y)
```

这样，比如我们调用

```
( lambda x . (lambda y . plus x y) ) 1 2
=>
(lambda y . plus 1 y) 2
=>
(lambda y . plus 1 2)
```

通过这种方法，lambda可以保持只使用一个参数的简洁性

也因为有柯里化方法，可以简单地将多个参数的函数转换为单个参数的lambda，即

```
   lambda x y z . plus x y z
=> (lambda x . (lambda y . plus x (lambda z . plus y z) ))
```

因此多个参数的lambda也可以作为一种语法糖来表示多参数的函数

### 自由变量与约束变量

当一个变量出现在lambda表达式的参数里，并被lambda表达式引用，则称之为约束变量，否则称为自由变量。一般使用free(exp)来表示表达式exp中自由变量的集合

几个例子如下

```
lambda x . plus x y   plus和y都是自由变量，x是约束变量
lambda x y . y x      x y都是约束变量
lambda y . (lambda x . plus x y)  plus是自由变量，而在内嵌表达式里，y是自由变量，x是约束变量；但在整个表达式中x y都是约束变量
```

### lambda算子计算规则

#### Alpha转换

alpha转换即将lambda中某个约束变量的名字整体替换为另一个名字，这个转换不改变lambda语义。如

```
lambda x . if (= x 0) then 1 else x^2
=>
lambda y . if (= y 0) then 1 else y^2
```

#### Beta归约

应用一个lambda表达式等价于通过把表达式中约束变量替换为调用时对应参数的实际值，其严格定义如下
$$
(lambda \ x \ . \ B) \ e \ \ = \ B[x:=e]
\\
if \ free(e) \subset \ free(B[x:=e])
$$
后面这个条件蕴含的意义在于，仅当beta化简不会引起约束变量和自由变量的冲突时，才可以使用beta归约。如下例

```
(lambda z . (lambda x . x+z)) (x+2)

若直接应用Beta化简，则有
   (lambda z . (lambda x . x+z)) (x+2)
=> lambda x . x+x+2
这里就使得原本在最外层是自由变量的x变为了约束变量，也就是所谓
free(e) 不是 free(B[x:=e]) 的子集，因为x在free(e)中，但不在free(B[x:=e])中

因此应该先使用Alpha转换，再使用Beta归约，如
   (lambda z . (lambda x . x+z)) (x+2)
=> (lambda z . (lambda y . y+z)) (x+2)
=> lambda y . y+x+2
```

### 丘奇数

lambda演算中不存在变量的概念，只有函数，因此需要用函数来表示数，这就叫做丘奇数

丘奇数中，所有的数字都是两个参数的函数

```
0: lambda s z . z
1: lambda s z . s z
2: lambda s z . s (s z)
```





