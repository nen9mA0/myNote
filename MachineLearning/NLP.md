https://blog.csdn.net/han_xiaoyang/article/details/50545650

http://www.itboth.com/d/JjaUzuNbUvae

### 编辑距离

指两个字符串之间，由一个转成另一个所需要的最少编辑操作次数

操作包括

- 将一个字符替换成另一个字符（substitution，s）
- 插入一个字符（insert，i）
- 删除一个字符（delete，d）

### 垃圾邮件处理

#### 朴素贝叶斯方法

最简单的想法就是直接把垃圾邮件的语句带入贝叶斯公式

$$
P(“垃圾邮件” | “xxxxx”) = \frac{P(“xxxxx” | “垃圾邮件”)P(“垃圾邮件”)}{P(“xxxxx”)}
$$

#### 分词

实际上不可能，所以引入分词，并且假设各词条件独立

$$
P(“xxxxx” | “垃圾邮件”) = P(“词语1” | “垃圾邮件”) * P(“词语2” | “垃圾邮件”) * P(“词语3” | “垃圾邮件”) \cdots
$$

这种方法一个显而易见的弊病就是不管语序是什么，只要出现概率相同就会得到相同的结果。

以上方法也被称为词袋法

#### 重复词语处理

##### 多项式模型

把出现多次的词依旧当做独立的词语来计算，则最后每个词语都有一个指数，指数大小为该词出现的次数

##### 伯努利模型

不管重复几次都当一次

##### 混合模型

计算最终的概率$P(“xxxxx”|“垃圾邮件”)$使不计算重复词，但在统计$P(“词语n”|“垃圾邮件”)$时统计

#### 去除停用词

停用词即对语义影响不大的一类词

#### 平滑技术

训练集中可能有没有出现的词语，导致计算概率时，因为其中一个P为0，最后得出的概率都为0

对于伯努利模型，一种平滑算法是

$$
P(“词语n”|“垃圾邮件”) = \frac{出现词语n的垃圾邮件数 + 1}{所有垃圾邮件中词语出现次数（同词只算一次）+2}
$$

其中**所有垃圾邮件中词语出现次数，对于同一个词语只计算一次**

对于多项式模型，一种平滑算法是

$$
P(“词语n”|“垃圾邮件”) = \frac{垃圾邮件中出现词语n的次数总和+1}{所有垃圾邮件中词语出现次数（同词重复算）+被统计词表的词语数}
$$

#### 一些技巧

##### 取对数

因为朴素贝叶斯方法计算概率是相乘运算，可以将所有$P(词语n|“垃圾邮件”)$转化为$log(P(词语n|“垃圾邮件”))$转化为加法运算

在二分类中，可以进一步处理，转化为$log \frac{P(词语n|“垃圾邮件”)}{P(词语n|“普通邮件”)}$，进一步减少运算，并且可以直接通过大于或小于0判断结果

##### 选取top k词语

直接选取文本中权重最高的k个词进行判断，进一步减少运算

##### 分割样本

选取top k过程可能出现这样的一种情况：一个短文本（短于k）即使每个词都是高权重词，最终权重可能还不如一段长文本

很朴素的一个想法是让k的选取和判定阈值与篇幅相关

* 对于长文本，如500词，分割成小片段，如100词，再使用top k处理，如果某片段超过阈值则作为垃圾邮件
* 对于短文本，如50词，根据标准片段长度的比例取top k和阈值，如100词，top k的k减半，阈值相应减半

##### 位置权重

根据文本在一些特殊的位置来给一些特殊权重，如在标题则加高权重

### 朴素贝叶斯的讨论

#### 贝叶斯公式

贝叶斯公式实际上提供了一个逆向解决问题的方法

$$
P(X|Y) = \frac{P(Y|X) P(X)}{P(Y)}
$$

当$P(X|Y)$不好求解时，可以从$P(Y|X)$求解。

如对于垃圾邮件分类问题，很难直接从文本xxxxx得到其为垃圾邮件的概率$P(“垃圾邮件” | “xxxxx”)$，但可以很简单的从概率得到垃圾邮件中文本xxxxx出现的概率$P(“xxxxx” | “垃圾邮件”)$，从而使用贝叶斯公式求得前者概率

#### 最大似然法

最大似然法源于这样的情景，当存在多分类问题时

$$
\begin{aligned}
P(Y_1 | X) &= \frac{P(X|Y_1)P(Y_1)}{P(X)}
\\
P(Y_2 | X) &= \frac{P(X|Y_2)P(Y_2)}{P(X)}
\\
P(Y_3 | X) &= \frac{P(X|Y_3)P(Y_3)}{P(X)}
\end{aligned}
$$

判断分类时可以将P(X)化去

$$
\begin{aligned}
P(Y_1 | X) &\propto P(X|Y_1)P(Y_1)
\\
P(Y_2 | X) &\propto P(X|Y_2)P(Y_2)
\\
P(Y_3 | X) &\propto P(X|Y_3)P(Y_3)
\end{aligned}
$$

很多时候我们无法获取靠谱的先验概率$P(Y_1) \ P(Y_2) \ P(Y_3)$，所以只能假设先验概率相等，在这种情况下只要比较似然函数，因此有最大似然法

### N-gram

即马尔科夫链，这里的n就是马尔科夫链里选择的条件的个数，如

* 1-gram就是独立概率$P(x_1)P(x_2)\cdots P(x_n)$
* 2-gram就是$P(x_1)P(x_2|x_1)P(x_3|x_2) \cdots$
* 3-gram就是$P(x_1) P(x_2|x_1) P(x_3|x_1,x_2) P(x_4|x_2,x_3) \cdots$

在N的选择上，因此实际上就算语料库非常大，N-gram中各词的出现概率随着N增大指数级下降，因此一般2和3常见

### 平滑技术

#### 拉普拉斯平滑

加一平滑法，即在计算N-gram时，分子都加一

$$
P(x_1 | x_2) = \frac{N(x_1,x_2) + 1}{N(x_2) + 所有不重复二元组个数}
$$

当所有不重复二元组个数远大于$N(x_2)$时，即大部分二元组出现次数都为0的情况效果不佳，因为容易喧宾夺主

#### 古德图灵平滑

假设出现r次的N元组个数为$N_r$，从未出现过的N元组个数为$N_0$，这种方法的核心思想是保持所有N元组出现总次数不变，将一部分权重分到未出现过的N元组
