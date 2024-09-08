# Efficient Estimation of Word Representations in Vector Space

word2vec原始论文

[1301.3781 (arxiv.org)](https://arxiv.org/pdf/1301.3781)

## 2 Model Architectures

这里提出了模型的训练复杂度计算公式

$$
O = E \times T \times Q
$$

* E  训练时的epoch数量，一般为3~50

* T  训练集的词数

* Q  与模型本身相关的参数

模型优化的目的在于最大化准确率，并最小化计算复杂度

### 2.1 Feedforward Neural Net Language Model(NNLM)

前馈神经网络语言模型，结构为

* 输入层：N个单词被编码为独热码，即输入为`N*V`

* 隐含层：隐含层矩阵宽度H一般为500~1000，即矩阵大小为`V*H`，隐含层输出为`N*H`

* 输出层：输出为N*D的矩阵

因此该模型的常数Q

$$
Q = N \times D + N \times D \times H + H \times V
$$

使用一些优化方法，如使用二叉树编码单词表可以显著降低最后一项的开销

### 2.2 Recurrent Neural Net Language Model(RNNLM)

递归神经网络语言模型

# Distributed Representations of Words and Phrases and their Compositionality
