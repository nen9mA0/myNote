## 数学记号

$$
\begin{aligned}
&\#{} \ \ 集合的元素个数
\\
&|m| \ \ m的长度
\\
&\{0,1\}^n\ \ 由0|1组成的长度为n的串
\end{aligned}
$$

## 概率

### 事件

对于集合U的子集A，其中P(U) = 1
$$
P(A) = \sum_{x \in A}P(x) \in [0,1]
$$
则A称为一个事件

### 离散分布

随机变量X = {x1, x2, x3, ... , xn}

符合 P(x1) = p1  P(x2) = p2 ...

称 (X, P)为离散分布

#### 均匀分布

是离散分布的一种，满足对于集合U
$$
U = \{0, 1\}^n
\\
对于 a \in U: P(r=a) = \frac{1}{|U|}
$$

### 独立性

对于事件A和B为独立事件当A和B的概率满足下式
$$
P(A \cap B) = P(A) * P(B)
$$

## 异或

异或的一个重要性质是，对于一个**任意分布的变量A**和一个**均匀分布的变量B**，结果**C是均匀分布的**
$$
\begin{aligned}
& 假设随机变量A为任意分布，A = \{0, 1\}^n。随机变量B为均匀分布，B = \{0,1\}^n
\\
& P(An=1) = p_1
\\
& P(Bn=1) = 0.5
\end{aligned}
$$
那么对于Cn
$$
\begin{aligned}
P(Cn=1) &= P( ((An=1)\cap(Bn=0)) \cup ((An=0) \cap (Bn=1)) )
\\
& = p_1 * 0.5 + (1-p_1) * 0.5
\\
& = 0.5
\end{aligned}
$$
同样
$$
\begin{aligned}
P(Cn=0) &= P( ((An=0)\cap(Bn=0)) \cup ((An=1) \cap (Bn=1)) )
\\
& = (1-p_1) * 0.5 + p_1 * 0.5
\\
& = 0.5
\end{aligned}
$$

## One TIme Pad

### 方法

#### 密钥空间

$$
K = \{0, 1\}^n
$$

#### 明文空间

$$
M = \{0, 1\}^n
$$

#### 密文空间

$$
C = \{0, 1\}^n
$$

#### 加密

$$
C = K \oplus M
$$

#### 解密

$$
M = K \oplus C
$$

#### 正确性

$$
\begin{aligned}
D(K, E(K,M)) &= D(K, K \oplus M)
\\
&= K \oplus K \oplus M
\\
&= M
\end{aligned}
$$

### 安全性

#### 完美安全性

对于(K,M,C)对的加解密算法(E,D)具有完美安全性，当满足下式
$$
\forall m_0, m_1 \in M,\ k \in K,\ c \in C \ \ \ (|m_0| = \ |m1|)
\\
P[ E(k,m_0) = c ] = P[E(k,m_1)=c]
$$
m0与m1通过密钥k加密后得到c的概率是相同的，即若知道密文c，无法从概率上获知其由m0加密而来还是m1，因为概率完全相同

然后看P[E(k,m) = c]的概率
$$
\forall m,c:\ \ P[E(k,m)=c] \ = \ \frac{\#\{k \in K | E(k,m)=c\}}{\#{K}}
$$
即，**能将m映射到c的密钥数除以密钥总数**

所以，根据上式，若具有完美安全性，则对于每个m、c，因为要求P[E(k,m)=c]相等，而因为密钥空间大小一定，因此$\#\{k \in K | E(k,m)=c\} = 常数$

**上述过程也证明了**

#### OTP具有完美安全性的证明

因为对于每一个m，c
$$
c = E(k,m)
$$
则
$$
k = m \oplus c
$$
因此
$$
\#\{k \in K | E(k,m)=c\} = 1
$$

###  局限性

由上述过程可知，要满足完美安全性，有
$$
\#\{k \in K | E(k,m)=c\} = const
$$
这里表示能把m映射为c的密钥总数，因此肯定是自然数。因此要满足完美安全性，有
$$
|K| \geq |M|
$$
即密钥空间大于等于明文空间，因此密钥长度大于明文长度

## 流密码

### 伪随机数生成器（PRG）

#### 定义

以一个随机数种子作为输入，将随机数种子空间的s位种子映射到n位字符串，其中n >> s
$$
G:  \{0, 1\}^s \rightarrow \{0, 1\}^n
$$

#### 性质

##### 不可预测性

###### 定义

一个算法G是可预测的，当满足下面定义
$$
\exist \  1 \leq i \leq n-1, 对于伪随机算法G，有一个算法A
\\
P[ A(G(k)) \big|_{1,...,i} = G(k) \big|_{i+1} ] \geq \frac{1}{2} + \varepsilon
$$
即，使用A可以通过G的前i个输出预测第i+1个输出，并且被预测的可能性大于一个**不可忽略的量$\varepsilon$**

###### 注：不可忽略

实用的定义：

* $\varepsilon \geq \frac{1}{2^{30}}$   不可忽略
* $\epsilon \leq \frac{1}{2^{80}}$  可忽略

形式化定义

$函数 \epsilon: Z^{\geq0} \rightarrow R^{\geq0}$

* 不可忽略 $\exist d: \epsilon(\lambda) \geq \frac{1}{\lambda^d}$
* 可忽略  $\forall d, \lambda \geq \lambda_d: \varepsilon(\lambda) \leq \frac{1}{\lambda^d}$

即若存在$\lambda_d$，使得当

#### 一些可预测的随机算法

##### 线性同余

```
srand(seed):
	r[0] = seed

rand():
	r[i] = (a * r[i-1] + b) mod p
	return r[i++] & 0xfff			//输出r[i]的其中几位
```

glibc random

```
rand():
	r[i] = (r[i-3] + r[i-31]) mod 2^32
	return r[i] >> 1
```

