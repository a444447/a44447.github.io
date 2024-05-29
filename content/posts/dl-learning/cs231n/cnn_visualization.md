---
title: "Cnn_visualization"
date: 2023-08-10T19:27:09+08:00
categories: [深度学习入门, cs231n]
draft: true
---



这部分是在学习 `cs231n`的「net visualization」部分的一些记录。

## 引入

在学习CNN的时候，我们有时候不是只关心最后输出的结果(比如分类问题中，最后将会输出一个输入的类别或者概率)。但是有很多时候，我们也想知道中间的那些卷积层发生了什么，每个卷积核做了什么，或者它们找的特征到底是什么。









## 梯度上升的可视化卷积神经网络

cs231n中，这部分的介绍是:

> By starting with a random noise image and performing gradient ascent on a target class, we can generate an image that the network will recognize as the target class. 

也就是，**从一个随机的噪音图片出发，通过梯度上升的方法来让提升噪音image的分数，直到我们的网络会将它识别为目标类别**。另外如果我们想要查看 *某卷积层某卷积核「想要寻找的特征」*,也可以用梯度上升的方法。

### class visualization

首先，我们来看`cs231n `作业中设计的class visualization.定义 $s_y(I)$, $I$表示生成的噪音Image，我们也是要在这张图上更新它的pixels。我们最终想要得到的就是 $I'$——$I'$输入网络后将在我们的选择的目标类别`target_y`上拥有较高的得分。上面的这段描述可以用下面的公式表示:
$$
I' = arg\underset {I}{max}(s_y(I) - R(I))
$$
其中

---

初始的$I$是一个充满噪音的图像，因此它在目标分类`target_y`上的得分是很低的，为了得分增加，我们应该做的就是更新$I$中每个pixels的值。如何保证我们每次更新pixels的值都会让它朝着使得分增加呢？很容易想到的就是 **梯度**，我们应该顺着梯度的方向更新我们的pixels的值。

{{< notice note >}}

在一般的神经网络中，我们通过loss来衡量参数的好坏，参数应该朝着使loss下降的方向前进，也就是逆着梯度。而在这里，我们可以将在`target_y`上的得分看作我们衡量pixels的好坏的标准，而这次我们不想让它下降，而是让它上升，因此我们要顺着梯度。

{{< /notice >}}

根据上面的思路，就很好进行代码的编写。

首先我们需要初始化一个随机噪音图片。

```python
img = torch.randn(1, 3, 224, 224).mul_(1.0).type(dtype).requires_grad_()
## torch.randn根据输入的shape,返回的是normal distribution,其中mean=0, var=1
## torch.mul_就是torch.mul的原地操作，就是进行乘法
## torch.requires_grad_()也是原地操作，表示需要跟踪梯度
```

![](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202308111438594.png)

#### 预训练的模型

在这篇文章中，我们是想要实验如何可视化卷积神经网络，关心中间的卷积层做了什么工作，卷积核的关注点在哪，因此我们选择一个预训练好了的模型。我们选择的模型是「SqueezeeNet」。

{{< notice note >}}

SqueezeNet ，其精度与 AlexNet 相当，但参数数量和计算复杂度显着减少。使用 SqueezeNet 而不是 AlexNet 或 VGG 或 ResNet 意味着我们可以轻松地在 CPU 上执行所有图像生成实验。

{{< /notice >}}

```python
model = torchvision.models.squeezenet1_1(pretrained=True)
#注意我们是不训练网络的，所以不需要计算梯度
for param in model.parameters():
    param.requires_grad = False
```

#### 开始可视化

现在明确我们的操作流程:

1. 输入我们初始化的image，以及目标分类。
2. 得到image通过网络后，有关目标分类的得分。
3. 根据梯度，更新pixels的值。
4. 重复上面的步骤。



我们给出步骤1到步骤3的，代码如下:

```py
def class_visualization(model, img, target_y, lr, l2_reg):
  ########################################################################
  # img.shape: (N, C, H, W)
  # target_y: 常量
  ###########################################################################
    
    scores = model(img)
    scores = scores[0, target_y]
    scores.backward()
    img_grad = img.grad
    img_grad -= 2 * l2_reg * img
    img.data += lr * img_grad / img_grad.norm()
    img.grad.zero_()
```

下面我们就可以开始iterations。值得注意的是，课程中实现的时候，每轮迭代都给image加上了「抖动」(jitter)。目前还不知道是为了什么（或许是在数据增强中为图像增加一些随机性，从而帮助模型更好地泛化到新的、未见过的数据）。实现jitter的具体方法就是，给定两个参数`ox`,`oy `：

+ 如果 `ox` 不为0，函数会将图像沿着宽度轴移动。具体来说，它会将图像的右侧 `ox` 个像素移动到左侧，并将左侧的其余像素移动到右侧。
+ 如果 `oy` 不为0，函数会将图像沿着高度轴移动。具体来说，它会将图像的底部 `oy` 个像素移动到顶部，并将顶部的其余像素移动到底部。

```python
def jitter(X, ox, oy):
    """
    X: PyTorch Tensor of shape (N, C, H, W)
    ox, oy: Integers giving number of pixels to jitter along W and H axes
    """
    if ox != 0:
        left = X[:, :, :, :-ox]
        right = X[:, :, :, -ox:]
        X = torch.cat([right, left], dim=3)
    if oy != 0:
        top = X[:, :, :-oy]
        bottom = X[:, :, -oy:]
        X = torch.cat([bottom, top], dim=2)
    return X
```

在原课程的iterations步骤中，加入了regularization操作，我这里没有加入，效果是没有他的好

```python
import random
learning_rate = 25
l2_reg = 1e-3
num_iterations = 100
max_jitter = 16
blur_every = 10
show_every = 25

for t in range(num_iterations):
    ox, oy = random.randint(0, max_jitter), random.randint(0, max_jitter)
    img.data.copy_(jitter(img.data, ox, oy))
    class_visualization(model, img , 76, learning_rate, l2_reg)
    img.data.copy_(jitter(img.data, -ox, -oy)) # undo jitter

     # As regularizer, clamp and periodically blur the image
    # for c in range(3):
    #     lo = float(-SQUEEZENET_MEAN[c] / SQUEEZENET_STD[c])
    #     hi = float((1.0 - SQUEEZENET_MEAN[c]) / SQUEEZENET_STD[c])
    #     img.data[:, c].clamp_(min=lo, max=hi)
    # if t % blur_every == 0:
    #     blur_image(img.data, sigma=0.5)
    #每show_every次显示一次
    if t == 0 or (t + 1) % show_every == 0 or t == num_iterations - 1:
        preprocessed_img = img.data.clone().cpu()
        preprocessed_img = preprocessed_img[0].numpy().transpose(1, 2, 0)
        plt.imshow(preprocessed_img)
        plt.show()

```



👇是迭代100次的结果



![image-20230811173838304](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202308111738347.png)

### 可视化特定卷积核寻找的特征

假设我们现在有一个生成的可视化卷积核图$x$ ,我们想要知道第$i$层的某一个卷积核$j$ 「正在寻找的特征」，那么我们就想要当$x$传递到它时，能让它有最大的激活值。

{{< notice note >}}

为什么有最大的激活值可以说明当前的$x$是目标卷积核寻找的「特征」呢？考虑二维的情况，我们都知道两个向量作点乘$v_1 ·v_2=|v1||v2|cos\theta$ ，当两个向量共线时有最大的值。我们以此为启发，我们认为如果$x$拥有目标卷积核想要的特征的话，它们在空间维度上应该呈现共线或者趋近共线，因此它们作点乘的值越大越好，**也就是激活值越大，表示$x$与卷积核寻找的特征越接近。**

{{< /notice >}}



接下来，我们依然从一个噪音图片出发，将它输入网络中进行向前传播，我们得到第$i$层的某一卷积核$j$ 的激活值$a_{ij}$，然后反向计算$\frac{da_{ij}}{dx}$，并用这个梯度来更新$x$,也就是更新pixels的值。
$$
x =x + \eta \frac{da_{ij}}{dx}
$$


## 第二类可视化方法:非参数化

可视化工作分为两大类，一类是非参数化方法:这种方法不分析卷积核具体的参数，而是先选取图片库，然后将图片在已有模型中进行一次前向传播，对某个卷积核，我们使用对其响应最大的图片块来对之可视化;而另一类方法着重分析卷积核中的参数，使用参数重构出图像。

我们之前讲解了第一类，现在来讲第二类非参数化方法。

### pytorch中的hook

在实现之前，我们先来介绍一下pytorch中的hook的作用。和很多其他地方的hook作用一样，它是在特定事件后自动执行的函数。PyTorch 为每个张量或 nn.Module 对象注册 hook。hook 由对象的向前或向后传播触发。它们具有以下函数签名:

```python
from torch import nn, Tensor

def module_hook(module: nn.Module, input: Tensor, output: Tensor):
    # For nn.Module objects only.
    
def tensor_hook(grad: Tensor):
    # For Tensor objects only.
    # Only executed during the *backward* pass!
```

### 正式开始

这次，我们以训练好的`ResNet50`为模型，输入一个准备好的图片，查看每个卷积层提取到的特征。

```python
img_path = 'example.jpg'
img = Image.open(img_path)

#利用transforms将图片转换为可以处理的形式
transform = transforms.Compose([
    transforms.Resize([224, 224]),
    transforms.ToTensor(),
    transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225])
])
input_image = transform(img).unsqueeze(0)#(1,3, 224,224)
```

然后，我们就需要定义我们的hook函数。我们用`get_activation(name)`函数将`hook`包起来，这样方便为任意我们想要观测的卷积层加上hook：

```python
activation  = {}
def get_activation(name):
    def hook(model, input, output):
        activation[name] = output.detach()
    return hook
```

我们打印我们的模型，观察到网络结构如下:

> ResNet(
>   (conv1): Conv2d(3, 64, kernel_size=(7, 7), stride=(2, 2), padding=(3, 3), bias=False)
>   (bn1): BatchNorm2d(64, eps=1e-05, momentum=0.1, affine=True, track_running_stats=True)
>   (relu): ReLU(inplace=True)
>   (maxpool): MaxPool2d(kernel_size=3, stride=2, padding=1, dilation=1, ceil_mode=False)
>   (layer1): Sequential(
>     (0): Bottleneck(
>       (conv1): Conv2d(64, 64, kernel_size=(1, 1), stride=(1, 1), bias=False)
>       (bn1): BatchNorm2d(64, eps=1e-05, momentum=0.1, affine=True, track_running_stats=True)
>       (conv2): Conv2d(64, 64, kernel_size=(3, 3), stride=(1, 1), padding=(1, 1), bias=False)
>       (bn2): BatchNorm2d(64, eps=1e-05, momentum=0.1, affine=True, track_running_stats=True)
>       (conv3): Conv2d(64, 256, kernel_size=(1, 1), stride=(1, 1), bias=False)
>       (bn3): BatchNorm2d(256, eps=1e-05, momentum=0.1, affine=True, track_running_stats=True)
>       (relu): ReLU(inplace=True)
>       (downsample): Sequential(
>         (0): Conv2d(64, 256, kernel_size=(1, 1), stride=(1, 1), bias=False)
>         (1): BatchNorm2d(256, eps=1e-05, momentum=0.1, affine=True, track_running_stats=True)
>       )
>     )
>     (1): Bottleneck(
>       (conv1): Conv2d(256, 64, kernel_size=(1, 1), stride=(1, 1), bias=False)
>       (bn1): BatchNorm2d(64, eps=1e-05, momentum=0.1, affine=True, track_running_stats=True)
>       (conv2): Conv2d(64, 64, kernel_size=(3, 3), stride=(1, 1), padding=(1, 1), bias=False)
>       (bn2): BatchNorm2d(64, eps=1e-05, momentum=0.1, affine=True, track_running_stats=True)
>       (conv3): Conv2d(64, 256, kernel_size=(1, 1), stride=(1, 1), bias=False)
>       (bn3): BatchNorm2d(256, eps=1e-05, momentum=0.1, affine=True, track_running_stats=True)
>       (relu): ReLU(inplace=True)
>     )
>
> ....
>
> )

假如我们想要知道，将image输入后，layer2中的，第一个bottleneck的conv3关注了哪些特征，我们就可以这样注册:

```python
model.layer2[0].conv3.register_forward_hook(get_activation('layer2_conv3'))
```

之后将`input_image`输入模型中，我们的`activation`字典中由于hook自动触发，就拥有了`layer2_bottleneck1_conv3`的激活值，我们选择一些打印出来，就可以进行观察:

```python
plt.figure(figsize=(12,12))
for i in range(64):
    plt.subplot(8,8,i+1)
    plt.imshow(layer2_conv3[0,i,:,:], cmap='gray')
    plt.axis('off')
plt.show()
```

![image-20230812171432054](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202308121714097.png)
