---
title: "tokenization"
date: 2024-09-05T10:14:00+08:00
Categories: [大模型]
---
在训练模型的时候，我们还有重要的一个步骤是如何把数据转换为模型能够训练的格式，这也是tokenization的意义所在。

{{< blockquote >}}

付诸笑谈中

{{< /blockquote >}}

## 引入

我们用一个实际的例子来引入`word embedding`方法。我们都知道心理学测试可以将人们划分为不同的个性, 比如`MBTI`测试。现在，假设有一个简单的心理学测试，它仅仅根据测试者的回答得到一个`0-100`的分值，其中`0`表示极度内向，`100`表示极度外向。那么表示一个人的心里状态就可以用这样一个数值表示`jame:[30]`。但是很明显一个维度是不充分的，我们通常会从多个维度来评价一个事物，于是就有了`jame:[30,20,15,...]`，其维度分别是`[维度1，维度2，维度3,...]`。于是我们就说这样的`vector`表示了这个人的`性格`。

现在又来了另一个人`peter:[50,32,11,...]`，我们如何衡量它们之间性格的相似呢？由于它们都是向量表示的，我们可以用一些计算的方式，比如`cosine_similarity`
$$
cos\_similarity(jame,peter)=score
$$

```
所谓的cosine_similarity就是求两个向量之间的cos值。当两个向量完全重合的时候，cos=1；而完全相反的时候cos=-1。所以向量有很高维度的时候，我们就用这样的方法衡量向量之间的相似度。
```



这个`score`就表示了两者的性格相似程度。

## word embedding

把上面我们举出的例子放在单词中，也就是一个单词`word`也是可以由`vector`构成，而`vector`包含了很多的维度，不同的词语在不同的维度上有不同的得分，而每个词语之间的联系(相似、不同) 也都是通过考虑表示它们的向量之间的距离来计算的。`word_embedding`指的就是这个。

//TODO 可视化展示



## language modeling

一个NLP最经典的应用就是`next word prediction`，比如，我们的输入法中，打出一个字后的下一个字联想功能。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409062041235.png)

一个`language model`，它可以接受一连串words作为输入，然后输出一个`prediction word`。注意在实际模型中，输出的还是概率。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409062043943.png)

可以看到1)中的查找`word embedding`就和我们之前讨论的内容差不多，实际上在训练完成后，除了语言模型外，还有一个所有词汇表的`word embedding matrix`，在每次预测下一个词语的时候，都会从这个matrix中查询`input words`的embedding，然后计算预测词。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409062046420.png)

---

下面讨论一下language model 如何训练的，首先要明白我们如何得到word embedding:

1. 获得很多很多很多的文本数据
2. 定义一个window,它在所有文本上滑动。
3. 通过这个滑动窗口，我们获得模型的training samples。



这是最开始的情况，数据集为空

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409062052407.png" alt="img" style="zoom:67%;" />

之后，将前两个词语作为inputs(相当于两个features)，然后第三个词语作为label(output)

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409062052821.png" alt="img" style="zoom:67%;" />

不断重复这个过程，我们就获得了我们的数据集。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409062054422.png)

*在实际训练中，获得数据集与训练可能是同时进行的*

### skipgram

我们上面讨论的得到word embedding的方式，是考虑了一个方向，类似`jay was hit by a _`，让模型去填空`_`，但是如果情况变成这样呢`jay was hit by a _ bus`？

很符合我们直觉的思考的是，相比于只考虑target word的前面几个单词，为何不同样考虑它的后面几个单词呢？这会不会得到更好的word embedding呢？答案是会的。

假如我们现在的target word是`red`，那么考虑它前后两个单词的话就是`[by a {red} bus in]`,最后数据集中表示如下：

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409062102230.png" alt="img" style="zoom:67%;" />

这个是叫做`**Continuous Bag of Words**`

---

另外一个考虑前后方向的方式叫做`skipgram`。它使用`current word`来猜测邻居单词.

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409070058473.png)

我们以`red`作为input,那么可以产生4个train sample

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409070059792.png)

于是不断移动window就可以产生很多很多train samples,最终

### 训练过程

训练的过程和一般神经网络差不多，都是梯度下降、反向传播。loss是模型预测的单词的vector与label word embedding之间的差距

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409070107483.png" alt="img" style="zoom:80%;" />

---

但是要注意的是，由于文本的数据量很大，我们如果每个sample都经历这样一个过程，那么耗时、计算量就太大了，因此把任务分成两件:

+ 生成高质量的word embedding任务
+ 使用这个高质量的word embedding来训练language model(由于已经训练好了word embedding，在训练过程中就不需要考虑学习这部分参数了)。



于是对于`生成高质量word embedding`这个任务，我们就把任务从预测word,变成了判断两个词语是否是邻居。这就是从一个神经网络训练变成了一个logistic回归问题

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409070124248.png)

我们的数据集就可以改变了，只要是input是`word1,word2`，output是`0 or 1`。但是需要考虑的是，我们的数据集不能只有相邻的元素，因为这样学习出来的模型就会是，无论输入any, 输出always 1。

于是我们之前通过skipgram创造的数据集就需要加入一个`0值`，也就是非相邻的单词，这些单词的选取是随机从词汇表选择的。这叫做`Negative Sampling`

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409070127895.png)

组合起来就叫做skipgram with negative sampling.

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409070130559.png)

---

有了这样的数据集后，来具体看看`word embedding`任务怎么训练

首先要决定`vocab_size`也就是词汇表的容量，然后还要决定`embedding_size`，也就是一个word vector有多少维度。

在训练的开始，定义`embedding matrix`与`context matrix`，它们都赋予随机的参数一开始。然后开始训练，在一开始，选择一个positive example,以及与它关联的negative examples：

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409070137301.png)

如图，input `not`，然后output word是`thou(true neighbor), aaron,taco`。现在需要查找它们的embedding(即使一开始embedding_matrix,context_matrix都是随机的值)。注意`input`从`embedding matrix`查找，而`output`从`context matrix`中查找。

当找到对应的embedding vector后，就用input对output分别作点乘操作计算值，这个值有正有负，我们想要把它们都限制在0-1之间，于是使用`sigmoid()`。

我们把`sigmoid()`后的值当作这次模型的结果，我们需要把它和真实label相减，计算loss。然后执行反向传播来不断更新调整两个matrix.

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202409070142119.png)

当不断的训练后，我们最后丢弃`context matrix`，把`embedding matrix`作为我们预训练的结果。

至此，我们得到了一个`良好的word embedding`

