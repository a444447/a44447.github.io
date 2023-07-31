---
title: "Write Shortcodes"
date: 2023-07-30T23:43:33+08:00
categories: [技术,建站技术]
---
**shortcodes**是hugo中一个很重要内容，短代码是快速使用我们之前预定义的模板的桥梁。hugo中短代码通过来使用。

> 需要注意的是，我们每个定义的短代码，都需要在根目录的layout/shortcodes/中，创建对应短代码名称的html.比如，我们创建一个名为mark的短代码，那么就需要创建`layout/shortcodes/mark.html`



### 部分使用的短代码

#### 高亮

短代码`mark`

{{< mark text="这是一个重点标记" >}}

#### 文本居中

短代码`align`

{{< align center "文字居中" >}}

#### 块引用

短代码`blockquote `

{{< blockquote author="电影" link="https://irithys.com" title="《寻梦环游记》" >}}
死亡不是一切的终点，遗忘才是
{{< /blockquote >}}

#### 隐藏

短代码`detail`

{{< detail "点下我呀🎁" >}}
对看到这行文字的人报以深切的祝福！🥰
{{< /detail >}}

#### notice

短代码`notice`

{{< notice notice-warning >}}
我是警告⚠️
{{< /notice >}}

{{< notice notice-note >}}

我是提示

{{< /notice >}}

{{< notice notice-info >}}

我是info

{{< /notice >}}

{{< notice notice-tip >}}

我是tip

{{< /notice >}}

这是第一个标签，将 `notice-warning` 分别修改为 `notice-note`、`notice-info`、`notice-tip` 就可以得到其他三个。



