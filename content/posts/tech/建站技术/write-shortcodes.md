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

```
{{</* mark text="这是一个重点标记" */>}}
```



{{< mark text="这是一个重点标记" >}}

#### 文本居中

短代码`align`

```
{{</* align center "文字居中" */>}}
```



{{< align center "文字居中" >}}

#### 块引用

短代码`blockquote `

```
{{</* blockquote author="电影" link="https://irithys.com" title="《寻梦环游记》" */>}}
死亡不是一切的终点，遗忘才是
{{</* /blockquote */>}}
```



{{< blockquote author="电影" link="https://irithys.com" title="《寻梦环游记》" >}}
死亡不是一切的终点，遗忘才是
{{< /blockquote >}}

#### 隐藏

短代码`detail`

```
{{</* detail "点下我呀🎁" */>}}
对看到这行文字的人报以深切的祝福！🥰
{{</* /detail */>}}
```



{{< detail "点下我呀🎁" >}}
对看到这行文字的人报以深切的祝福！🥰
{{< /detail >}}

#### notice

```
{{</* notice warning */>}}
这是告诫! 请注意!
{{</* /notice */>}}
```

短代码`notice`

{{< notice warning >}}
这是告诫! 请注意!
{{< /notice >}}

{{< notice info >}}
这是引言
{{< /notice >}}

{{< notice tip >}}
这是小贴示
{{< /notice >}}

{{< notice note >}}
这是注释
{{< /notice >}}



#### imgcap

```
{{</* imgcap src="https://w.wallhaven.cc/full/nr/wallhaven-nrq3pq.jpg" title="bird and sea" */>}}
```

{{< imgcap src="https://w.wallhaven.cc/full/nr/wallhaven-nrq3pq.jpg" title="bird and sea" >}}

其中, `src` 和 `title` 为必填项, `alt` 和 `width` 为选填项, `alt` 默认与 `title` 保持一致, `width` 默认值为: 95% .	
