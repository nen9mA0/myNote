答案 https://sicp.readthedocs.io/en/latest/

## 1

### 1.1

```
10
12
8
3
6
a=3
b=4
19
false
4
16
6
16
```

### 1.2

```
( /
    ( + 5 4 (- 2 (- 3 (+ 6 (/ 4 5)))) )
	( * 3 (- 6 2) (- 2 7) )
)
```

### 1.3

```
(define (max_sum x y z)
  (cond
    ( (and (< x y) (< x z)) y+z )
    ( (and (< y x) (< y z)) x+z )
    ( (and (< z x) (< z y)) x+y )
  )
)
```

### 1.4

```
(define (a-plus-abs-b a b)
  ((if (> b 0) + -) a b)
)
```

若b>0，则表达式变为 `+ a b` ，否则变为 `- a b`

### 1.5

参考 https://sicp.readthedocs.io/en/latest/chp1/5.html

正则序求值过程

```
(test 0 (p)) => ( if (= 0 0) 0 (p) ) => 0
```

应用序求值过程

```
(test 0 (p)) => (test 0 (p)) => (test 0 (p)) ...
```

这是因为在正则序中，(p)只有在使用到时才会被求值，而由于if的判断，因此(p)不会被求值。而在应用序中，(p)在传入时就被求值，因此会进入无限循环，解释器卡死

### 1.6

参考 https://sicp.readthedocs.io/en/latest/chp1/6.html

由于new-if是被作为一个普通函数调用的，而if是作为一个特殊语句调用的。因此若执行的是if的语句，解释器在判断 (good-enough guess x) 后，根据结果只会选择其中一条路径执行。

```lisp
(define (sqrt-iter guess x)
    (if (good-enough guess x)
        guess
        (sqrt-iter (improve guess x) x)
     )
)
```

而对于使用new-if的代码来说

```lisp
(define (sqrt-iter guess x)
    (new-if (good-enough guess x)
        guess
        (sqrt-iter (improve guess x) x)
     )
)
```

由于new-if是作为一个普通的函数调用的，因此在传递参数前，由于解释器在解释语句时采用的是应用序，解释器会先试图解析各个传入的参数，而在解析到sqrt-iter时会发生递归，因此解释器会遇到“到达最大递归深度”的报错

### 1.7

根据描述，可以设计一个新的迭代终止条件（见sicp_program/newton_sqrt.scm）

```lisp
(define (good-enough guess x)
    (< (/ (abs(- guess (improve guess x))) guess) 0.001)
)
```

### 1.8

根据描述，可以设计一个新的迭代算法（见sicp_program/newton_cuberoot.scm）

```lisp
(define (improve guess x)
    (/ (+ (/ x (* guess guess)) (* 2 guess)) 3)
)
```

### 1.9

第一种方法如下

```
(+ 4 5)
=> (inc (+ (dec 4) 5)) => (inc (+ 3 5))
=> (inc (inc (+ (dec 3) 5))) => (inc (inc (+ 2 5)))
=> (inc (inc (inc (+ 1 5))))
=> (inc (inc (inc (inc (+ 0 5)))))
=> (inc (inc (inc (inc 5))))
=> (inc (inc (inc 6)))
=> (inc (inc 7))
=> (inc 8)
=> 9
```

因此是递归过程



第二种方法如下

```
(+ 4 5)
=> (+ (dec 4) (inc 5)) => (+ 3 6)
=> (+ (dec 3) (inc 6)) => (+ 2 7)
=> (+ (dec 2) (inc 7)) => (+ 1 8)
=> (+ (dec 1) (inc 8)) => (+ 0 9)
=> 9
```

因此是迭代过程

### 1.10

```
(A 1 10)
=> (A 0 (A 1 9))
=> (A 0 (A 0 (A 1 8)))
=> (A 0 (A 0 (A 0 (A 1 7))))
=> (A 0 (A 0 (A 0 (A 0 (A 1 6))))
=> ...
=> (A 0 ... *9 ... (A 1 1))
=> (A 0 ... *8 ... (A 0 2))
=> (A 0 ... *7 ... (A 0 4))
=> ...
=> 1024
求2的10次方

(A 2 4)
=> (A 1 (A 2 3))
=> (A 1 (A 1 (A 2 2)))
=> (A 1 (A 1 (A 1 (A 2 1))))
=> (A 1 (A 1 (A 1 2)))
=> (A 1 (A 1 (A 0 (A 1 1))))
=> (A 1 (A 1 (A 0 2)))
=> (A 1 (A 1 4))
=> (A 1 (A 0 (A 1 3)))
=> (A 1 (A 0 8))
=> (A 1 16)
=> 65536

(A 3 3)
=> (A 2 (A 3 2))
=> (A 2 (A 2 (A 3 1)))
=> (A 2 (A 2 2))
=> (A 2 4)
=> 65536
```



```
(A 2 4) = (A 1 (A 2 3))
(A 2 3) = (A 1 (A 2 2))
(A 2 2) = (A 1 (A 2 1)) = (A 1 2)

(A 2 1) = 2
(A 2 2) = 4
(A 2 3) = 16
(A 2 4) = 65536
```

所以可以归纳如下

```
(A 0 n) = 2*n
(A 1 n) = 2^n
(A 2 n) = 2^(A 2 n-1) = 2^( 2^(A 2 n-2) ) = ...
```

### 1.11

递归

```lisp
(define (f n)
    (cond ( (< n 3) n)
          ( else (+ (f (- n 1)) (* 2 (f (- n 2))) (* 3 (f (- n 3))) ) )
    )
)
```

可以发现该程序实现的功能类似斐波那契数列，但包含了系数

```lisp
(define (f-iter a b c count)
    (if (< count 3)
        a
        (f-iter (+ a (* 2 b) (* 3 c)) a b (- count 1))
    )
)

(define (f n)
    (if (< n 3)
        n
        (f-iter 2 1 0 n)
    )
)
```

### 1.13

$$
\begin{aligned}
& 假设 \ Fib(n) = \frac{\phi^n - \gamma^n}{\sqrt{5}}
\\
& 则 \ Fib(n+1) = \frac{\phi^{n+1} - \gamma^{n+1}}{\sqrt{5}} = \frac{\phi^n * \phi - \gamma^n * \gamma}{\sqrt{5}} = \frac{\phi^n * \frac{1+\sqrt{5}}{2} - \gamma^n * \frac{1-\sqrt{5}}{2}}{\sqrt{5}} = \frac{1}{2} \frac{\phi^n - \gamma^n}{\sqrt{5}} + \frac{\phi^n+\gamma^n}{2} = \frac{1}{2} Fib(n) + \frac{\phi^n+\gamma^n}{2}
\\
& Fib(n+2) = \frac{\phi^{n+2} - \gamma^{n+2}}{\sqrt{5}} = \frac{\phi^n * \phi^2 - \gamma^n * \gamma^2}{\sqrt{5}} = \frac{\phi^n * \frac{3+\sqrt{5}}{2} - \gamma^n * \frac{3-\sqrt{5}}{2}}{\sqrt{5}} = \frac{3}{2} \frac{\phi^n - \gamma^n}{\sqrt{5}} + \frac{\phi^n+\gamma^n}{2} = \frac{3}{2} Fib(n) + \frac{\phi^n+\gamma^n}{2}
\\
& 因此由上式可得Fib(n+2) = Fib(n+1) + Fib(n)，又\frac{\phi^0 - \gamma^0}{\sqrt{5}} = 0，\frac{\phi^1 - \gamma^1}{\sqrt{5}} = 1，所以得证
\\
& 则 Fib(n) = \frac{\phi^n - \gamma^n}{\sqrt{5}} = \frac{\phi^n}{\sqrt{5}} - \frac{\gamma^n}{\sqrt{5}} \ \ ，因为\gamma = \frac{1-\sqrt{5}}{2} < -1，所以 |\gamma| > |\gamma^n|，而0 >\frac{1-\sqrt{5}}{2 \sqrt{5}} >
- \frac{1}{2}
\\
& 所以Fib(n)的第二项的绝对值|\frac{\gamma^n}{\sqrt{5}}|总是小于\frac{1}{2}，因此命题得证
\end{aligned}
$$

### 1.15

#### 1

trace输出结果中，p被调用了5次

#### 2

因为每次迭代a都减小为原来的1/3，而迭代在x<0.1时停止，因此迭代次数应为 $log_3 \frac{a}{0.1}$ ，因此增长的阶为 $\Theta (log \ a)$

### 1.16

思路：如2^7可以分解为 2^(1+2+4)，2^9可以分解成2^(1+8)

这里状态转换的核心思想是定义一个状态变量a，定义状态变换使得每次状态转移时 $a * b^n$ 不变，则

* n为偶数  $b' = b^2 \ \ n' = \frac{n}{2} \ \ a' = a$ ，则 $a' * b'^{n'} = a * (b^2)^{\frac{n}{2}} = a*b^n$
* n为奇数  $b' = b \ \ n' = n-1 \ \ a' = a*b$ ，则 $a' * b'^{n'} = a*b*b^{n-1} = a*b^n$

```lisp
(define (fast-expt-iter b n product)
    (cond
        ( (= n 0) product )
        ( (even n) (fast-expt-iter (* b b) (/ n 2) product) )
        ( else (fast-expt-iter b (- n 1) (* b product)) )
    )
)

(define (fast-expt b n)
    (fast-expt-iter b n 1)
)
```

### 1.17

按照fast-expt的思路

* a为偶数 $mul(a,b) = 2 * mul(a/2, b)$ 
* a为奇数 $mul(a,b) = b + mul(a-1, b)$
* a=0  返回0

```lisp
(define (fast-mul a b)
    (cond
        ( (= a 0) 0 )
        ( (even a) (double (fast-mul (halve a) b)) )
        ( else ( + b (fast-mul (- a 1) b) ) )
    )
)
```

### 1.18

按照1.16的思路，同样设置状态量n，并且保证状态转移时 $a*b+n$ 不变，这是因为迭代过程可以视为不断减少a*b，增加n的过程。

* b为偶数  $b' = b / 2 \ \ a' = a * 2 \ \ n' = n$ 
* b为奇数  $b' = b-1 \ \ a' = a \ \ n' = n + a$ 

迭代结束条件是b=0

```lisp
(define (fast-mul-iter a b n)
    (cond
        ( (= b 0) n )
        ( (even b) (fast-mul-iter (double a) (halve b) n) )
        ( else (fast-mul-iter a (- b 1) (+ n a)) )
    )
)

(define (fast-mul a b)
    (fast-mul-iter a b 0)
)
```

### 1.19

$$
\begin{aligned}
& T_{pq} =
\left \{
\begin{aligned}
& a \leftarrow bq + aq + ap
\\
& b \leftarrow bp + aq
\end{aligned}
\right .
\\

& 应用两次 T_{pq}，则

\\

& \left \{
\begin{aligned}
& a' = bq + aq + ap
\\
& b' = bp + aq
\end{aligned}
\right .

\\
& \left \{
\begin{aligned}
& a'' = b'q + a'q + a'p = (bp+aq) q + (bq + aq + ap)q + (bq + aq + ap)p = 2aq^2 + bq^2 + ap^2 + 2bpq + 2apq = b(q^2+2pq) + a(q^2+2pq) + a(p^2+q^2)
\\
& b'' = b'p + a'q = (bp+aq)p + (bq + aq + ap)q = aq^2 + bp^2 + bq^2 + 2apq = b(p^2+q^2) + a(q^2+2pq)
\end{aligned}
\right .

\\
& 所以根据T_{pq}的变换法则，可以得出p和q的迭代规则
\\
& \left \{
\begin{aligned}
& p' = p^2 + q^2
\\
& q' = q^2 + 2pq
\end{aligned}
\right .
\end{aligned}
$$

下面的程序设计思路就是将p和q作为转换的一组基，若n为奇数，则直接按照Tpq的变换更新a和b；若n为偶数，则直接更新p q这组基（假设新的基为p' q'）。这会使得 $T_{p'q'} = (T_{pq})^2$ 

```lisp
(define (fib n)
    (fib-iter 1 0 0 1 n)
)

(define (fib-iter a b p q count)
    (cond
        ( (= count 0) b )
        ( (even count) (fib-iter a b (+ (* p p) (* q q)) (+ (* q q) (* 2 p q)) (/ count 2) ) )
        (else
            (fib-iter (+ (* b q) (* a q) (* a p)) (+ (* b p) (* a q)) p q (- count 1))
        )
    )
)
```

### 1.20

应用序

```
(gcd 206 40)
=> (gcd 40 (remainder 206 40)) = (gcd 40 6)
=> (gcd 6 (remainder 40 6)) = (gcd 6 4)
=> (gcd 4 (remainder 6 4)) = (gcd 4 2)
=> (gcd 2 (remainder 4 2)) = (gcd 2 0)
```

正则序

```
(gcd 206 40)
=> (gcd 40 (remainder 206 40))
=> ( gcd (remainder 206 40) (remainder 40 (remainder 206 40)) )
=> ( gcd (remainder 40 (remainder 206 40)) (remainder (remainder 206 40) (remainder 40 (remainder 206 40))) )
=> ( gcd (remainder (remainder 206 40) (remainder 40 (remainder 206 40))) (remainder (remainder 40 (remainder 206 40)) (remainder (remainder 206 40) (remainder 40 (remainder 206 40)))) )
```

### 1.22

注意新版本runtime以秒为单位，因此应该用real-time-clock代替（但也只是以毫秒为单位）

```lisp
(define (time-prime-test n)
    (newline)
    (display n)
    (start-prime-test n (real-time-clock))
)

(define (start-prime-test n start-time)
    (if (prime n)
        (report-prime (- (real-time-clock) start-time) n)
        (start-prime-test (next-odd n) start-time)
    )
)

(define (report-prime elapsed-time n)
    (display " *** ")
    (display elapsed-time)
    n
)

; ==== search for primes ====

(define (next-odd n)
    (if (= (remainder n 2) 1)
        (+ n 2)
        (+ n 1)
    )
)

(define (search-for-prime n)
    (time-prime-test n)
)
```

### 1.23

```lisp
(define (smallest-divisor n)
    (find-divisor n 2)   ; 从2开始测试
)

(define (divide a b)
    (= (remainder a b) 0)
)

(define (find-divisor n test-divisor)
    (cond
        ( (> (square test-divisor) n)  n )           ; 若test-divisor^2 > n，说明没有找到因子，这里直接返回n
        ( (divide n test-divisor)  test-divisor )    ; 若能被整除，则找到因子
        ( else (find-divisor n (next-divisor test-divisor)) ) ; 否则测试下一个数是否为因子
    )
)

(define (next-divisor n)
    (cond
        ((= n 2) 3)
        (else (+ n 2))
    )
)

(define (prime n)
    (= n (smallest-divisor n))
)
```

### 1.25

不行因为计算量太大了

### 1.26

因为这种写法将导致解释器计算两次 `(expmod base (/ exp 2) m)` ，最终使得每次迭代时计算量没有减半，因此为O(n)

### 1.27

程序见sicp_program/ex_1_27.scm

### 1.28

核心就是在expmod中加入对非平凡根的判断

```lisp
(define (expmod base exp m)
    (cond
        ( (= exp 0) 1 )
        ( (nontrivial base m) 0 )
        ( (even exp) (remainder (square (expmod base (/ exp 2) m)) m) )
        (else (remainder (* base (expmod base (- exp 1) m)) m))
    )
)

(define (nontrivial a n)
    (cond
        ( (= a 1)  false )
        ( (= a (- n 1))  false )
        ( (= (remainder (square a) n) 1) true )
        ( else false)
    )
)
```

### 1.29

1.29~1.31 程序直接见sicp_program

### 1.32

1.32和1.33对先前的运算定义了一个抽象程度更高的算子，其中1.33用到了一个技巧，即当函数调用点只有一个参数，但需要传入两个参数时，可以采用lambda闭包传入第二个参数，类似柯里化

```lisp
(define (filtered-accumulate combiner null-value filter term a next b)
    (cond 
        ( (> a b) null-value )
        ( (filter a) (combiner (term a) (filtered-accumulate combiner null-value filter term (next a) next b)) )
        ( else (filtered-accumulate combiner null-value filter term (next a) next b) )
    )
)
```

这里的filter参数是一个函数，用来判断当前传入的a是否符合条件

在第一小问中，只需判断a是否为素数，因此传入的参数就是a

在第二小问中，需要判断a和b是否互素，而判断是否互素的函数肯定需要两个参数，即 `(co-prime a b)` ，其中co-prime的第二个参数始终是b，因此可以传入这样一个函数作为filter

```lisp
(lambda (x) co-prime(x, b))
```

因此为了实现第二小问功能，调用的方式为

```lisp
(filtered-accumulate * 1 (lambda (x) (coprime x b)) ret_self a next b)
```

### 1.34

```lisp
(define (f g)
    (g 2)
)
```

则调用(f f)时

```lisp
   (f f)
=> (f 2)
=> (2 2)
```

因为这里相当于试图将2作为函数调用，因此报错

### 1.35

$$
\begin{aligned}
& \phi = \frac{1+\sqrt{5}}{2}
\\
& 对于变换 f(x) = 1 + \frac{1}{x}，不动点即
\\
& x = 1 + \frac{1}{x} \Rightarrow x^2 - x - 1 = 0
\\
& 即 x = \frac{1 \pm \sqrt{5}}{2}，得证
\end{aligned}
$$

### 1.36

平均值版本收敛较快，见ex_1_36

### 1.41

```lisp
   (( (double (double double)) inc) 5)
```

`(double double)` => 4次double，`(double (double double))` => 8次double，因此对inc使用8次double，得到16次inc

### 1.42~44

见程序

### 1.45~46

还没写

### 2.1

写进2_1_1_rat中了

### 2.2

见程序

### 2.3

这里采用了两种定义矩形的方式，一种是指定4个点，一种是指定对角线。这样写是因为只有这样才能定义斜向的矩形。

check-rect通过检测矩形构造的四条边是否垂直来检查矩形的合法性

### 2.4

```lisp
(define (cons x y)
    ( lambda (m) (m x y) )
)

(define (car z)
    ( z (lambda (p q) p) )
)
```

假设有如下调用

```
   (define x (cons 1 2))
=> x = ( lambda (m) (m 1 2) )

   (define y (car x))
=> y = ( x (lambda (p q) p) )
=> y = ( (lambda (m) (m 1 2)) (lambda (p q) p) )
=> y = ( (lambda (p q) p) 1 2 )
=> y = 1
```

所以cdr定义应该为

```lisp
(define (cdr z)
    ( z (lambda (p q) q) )
)
```

### 2.6 丘奇数

```lisp
( define zero (lambda (f) (lambda (x) x)) )

(define (add-1 n)
    ( lambda (f) (lambda (x) (f ((n f) x))) )
)
```

若调用

```
   (add-1 zero)
=> (  ( lambda (f) (lambda (x) (f ((n f) x))) )  (lambda (f) (lambda (x) x)) )
```





### 有趣的写法

[条件表达式返回运算符](#1.4)

