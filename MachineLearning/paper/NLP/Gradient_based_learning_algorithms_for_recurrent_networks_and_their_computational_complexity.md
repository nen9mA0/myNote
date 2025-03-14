[Gradient-Based Learning Algorithms for Recurrent Networks and Their Computational Complexity](https://gwern.net/doc/ai/nn/rnn/1995-williams.pdf)  文章

[[PDF] 13 Gradient-Based Learning Algorithms for Recurrent Networks and Thei r Computational Complexity | Semantic Scholar](https://www.semanticscholar.org/paper/13-Gradient-Based-Learning-Algorithms-for-Recurrent-Williams-Zipser/4983823eb66ed5d8557f20dd5c8a09ed66f05c25)  图

### Formal Assumptions and Definitions

本文使用的网络如下

![](pic/3_1.png)

对应一个3*5的权重矩阵

定义$s_k(t)$为t时刻输入到第k个单元时计算出的中间值

$$
s_k(t+1) = \sum_{t \in U} w_{kl} y_l(t) + \sum_{l \in I} w_{kl} x_l^{net}(t) = \sum_{l \in U \cup I} w_{kl}x_l(t)
\\
y_k(t+1) = f_k(s_k(t+1))
$$



#### Backpropagation Through Time

展开网络

![](pic/3_2.png)

![](pic/3_4.png)
