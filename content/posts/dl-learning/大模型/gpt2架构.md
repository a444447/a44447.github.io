---
title: "gpt2架构"
date: 2024-09-08T09:38:00+08:00
Categories: [大模型]
---
这部分来看下gpt2的架构是怎么样的

---

我们在讨论 *tokenization*的时候举过一个`language model`的例子，就是手机上的`next word prediction`。GPT2模型也可以看作这样的一个`next word prediction`，只不过他要复杂很多。

---

在提到GPT2的时候我们，再简单说一下其与`BERT`的异同。

**GPT2**

它是利用了transformer的decoder结构，堆叠而成。它的output与传统的language model一样，每次都输出一个token，然后将这个token添加到`input sequence`中，这个新的sequence将作为新的input输入到模型，这个过程是`auto regression`

**BERT**

BERT利用的是transformer中的encoder结构，它



## transformer block的改进

首先提一个对于`transformer decoder`的改进，就是加入`masked self-attention`。这个masked self-attention它会让一个sequences中某个token只能看到自己前面的parted sequence，而future sequence对他而言是不可见的。

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/transformer-decoder-block-self-attention-2.png" alt="img" style="zoom:67%;" />

而原版的self-attention它是能看到所有的tokens的。

---

还有一种叫做`decoder-only block`，它是对transformer的结构的一个改变。使用这个结构的model不需要transformer encoder，只需了decoder,并且将原版的`512 tokens input`扩大到了`4000 tokens input`（并不是说GPT2能处理这么多的tokens）。

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/transformer-decoder-intro.png" alt="img" style="zoom:67%;" />

**GPT2就是用了decoder-only blocks**

## look inside GPT2

假如我们有一个已经训练好的GPT2模型，我们可以让它进行`generating unconditional samples`也就是只是给一个标志开始生成词语的start token`<s>`，然后让模型自己生产单词。

或者`generating *interactive conditional samples`，就是给一个明确的主题。

现在用第一种方式来举例，看看GPT2的运作方式。

我们用最简单的情况讲解，一开始的token是`<s>`，这个`<s>`会一路沿着stacked decoder向上传递，最后输出一个一个词语(实际上是以词语概率形式输出的一个向量，从top-k中随机选择一个。比如top-k=1就是固定选概率最大的单词，top-k=3就是从概率最大前三的单词中随机选一个。*如果仅仅取top-k=1，有可能会造成loop的现象，因此一般会取top-k=n，使得结果更加多样*)。



<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908185038526.png" alt="image-20240908185038526" style="zoom:67%;" />

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908185124050.png" alt="image-20240908185124050" style="zoom:67%;" />

注意，在The向上经过stacked decoder的时候，会需要用到`<s>`的一些信息(q,k)，它们会被模型保留下来而不是重新再计算一次。

### deep look inside

上面是对整个过程的宏观认识，下面来看一下更详细的内容。

1. 输入

和我们之前`tokenization`讨论的一样，为了训练language model，需要把input 转换为model能理解的信息，这就需要之前预训练好的`embedding matrix` ,也叫做`token embedding (wte)`

![image-20240908185534363](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908185534363.png)

除此之外，为了给每个单词加入它在一个句子中的位置的信息，还需要进行position encoding,这也是一个预训练好了的matrix

![image-20240908185813367](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908185813367.png)

所以，比如对于一个`<s>`，它的输入形式就是`embedding <s> + position <s>`

2. 进入stack内部

现在顺着输入的路线向上，接下来的结果是decoder，其内部其实就是`masked self-attention + fn`

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908190051525.png" alt="image-20240908190051525" style="zoom:67%;" />

对于一般的`self-attention`机制，它为了让模型能够理解上下文，需要让`token`计算与其他`tokens`之间的联系，这个联系主要是通过`q`与`k`计算，然后将再乘以`v`，得到的就是对每个token的关联性评分。

> q：q是当前单词的表示，用于对所有其他单词进行评分（使用它们的键）。我们只关心当前正在处理的标记的查询。 
>
> k：k向量就像片段中所有单词的标签。它们是我们在搜索相关单词时匹配的对象。
>
> v：v向量是实际的单词表示，一旦我们对每个单词的相关程度进行了评分，这些就是我们加起来表示当前单词的值。

所以，沿着这些`decoder`一路向上，最后输出的就是一个`output vector`，这个`output vector`与`token embedding matrix`相乘就得到了一个`output token probabilities (logits)`

----

具体有关self-attention怎么计算的就不说了，下面来看GPT2中使用的 **masked self-attention**

对于masked self-attention的实现通常是有一个`attention masked matrix`。想象我们有一个4个单词的输入sequence ("robot must obey orders")。那么在language model训练的时候，这4个单词就会分为4个步骤，每个单词一个步骤。于是由于model都是按batch读入的，我们假设这四个步骤一起被读入到一个batch。

![image-20240908191237487](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908191237487.png)

所以，整个QK得到的结果类似这样:
![image-20240908191448975](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908191448975.png)

有了这个table就可以构建attention mask了,

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908191545237.png" alt="image-20240908191545237" style="zoom:50%;" />

然后使用softmax过后

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908191604779.png" alt="image-20240908191604779" style="zoom:50%;" />

---

之后，由于model在每一轮迭代后，只新增加一个单词，所以为了保持高效，会让model记住之前单词的q,k。

3. fully connected

对于GPT2的全连接部分，它由两个layers组成，第一个layer是`764x764*4`（假设输入是768dims）。为什么乘以4，原因是原本的transformer中是`512x512*4`。

第二个layer`768*4 x 768`

---

![image-20240908192058890](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240908192058890.png)

对于每一个decoder部分，都有自己的权重(GPT2中有12个decoders)

