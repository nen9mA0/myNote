[1301.3781](https://arxiv.org/pdf/1301.3781)  word2vec

### Introduction

本文提出的方案可以有效地在庞大的数据集上进行训练，且有下列几个特性

* 每个词可以有多个维度的相似性，反映到自然语言中比如一个名词可能对应多种意思

* 词义相近的单词向量也相近

* 还有一个惊喜的发现，部分词向量表现出加和性，如`vector("king") + vector("woman") - vector("man")`的结果与`vector("queen")`很相近

这里主要列了两个previous work

* NNLM（Neural network language model）  使用一个线性输入层和一个非线性隐含层组成前馈神经网络，学习如何将语言统计模型与词向量表示结合

* 另一种NNLM架构是word2vec的基础，词语会先由只含一个隐含层的简单神经网络学习，再使用输出的词向量训练另一个模型。word2vec扩展了这个工作的第一步：即如何通过一个简单网络学习词向量

### Model Architectures

这里首先定义了模型的复杂度

$$
O = E \times T \times Q
$$

* E：epoch数量

* T：训练集中的单词总数

* Q：与模型架构相关的常量


