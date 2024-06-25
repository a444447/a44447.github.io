---
title: "Day11_字符串 实现KMPand重复子字符串"
date: 2024-06-19T20:54:11+08:00
categories: [算法]
---

## 实现substr

{{< blockquote author="力扣28" link="https://leetcode.cn/problems/find-the-index-of-the-first-occurrence-in-a-string/" title="找出字符串中第一个匹配项的下标" >}}

给你两个字符串 `haystack` 和 `needle` ，请你在 `haystack` 字符串中找出 `needle` 字符串的第一个匹配项的下标（下标从 0 开始）。如果 `needle` 不是 `haystack` 的一部分，则返回 `-1` 。

 

**示例 1：**

```
输入：haystack = "sadbutsad", needle = "sad"
输出：0
解释："sad" 在下标 0 和 6 处匹配。
第一个匹配项的下标是 0 ，所以返回 0 。
```

**示例 2：**

```
输入：haystack = "leetcode", needle = "leeto"
输出：-1
解释："leeto" 没有在 "leetcode" 中出现，所以返回 -1 。
```

 

**提示：**

- `1 <= haystack.length, needle.length <= 10^4`
- `haystack` 和 `needle` 仅由小写英文字符组成

{{< /blockquote >}}

---

对于从一个字符串`s`中找到子串`s1`是kmp算法的经典题目，所以我们首先需要掌握kmp是如何操作的。

首先，对于「最暴力」的解决模式匹配的方法，就是两个字符串同时从前向后遍历，一一比较它们每个字符是否相同，如果模式串能够走到结尾，那么就找到了一个匹配的下标；如果中途遇到了不一样的字符串，那么就将主串的起始位置向后移动一位，模式串又回到开头，然后再次进行上面的逐一比较过程。

而与这样的暴力法不同，KMP进行模式匹配的根本特点就是: **当出现字符串不匹配时，可以知道一部分之前已经匹配的文本内容，可以利用这些信息避免从头再去做匹配了。**

### next数组

kmp算法中有一个next数组，也叫做前缀表，它的作用就是在每次匹配失败后，**指示模式串应该从哪儿开始重新匹配**。

> 假如我们现在不研究next数组是如何构建的，把它当作一个黑箱，现在我们模式串在`index=5`的位置匹配失败，通过`new_index=next[index]`就可以知道回退的位置，而不是直接`new_index=0`重头开始匹配

![KMP精讲1](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202406251723065.png)

观察上面的图片，如果模式串在`index=5`匹配失败，我们能过回退到的最远位置一定是「**最长相等的前缀和后缀**」。

> 假如当前
> s: aabaabaafa
>
> t: aabaaf
> 很显然当index=5时,也就是s[5]=b 与 t[5]=f不相等，匹配失败的位置是 **后缀子串的后面**，因此我们只需要找到一个相同的前缀子串，然后回退到它的后面即可。这也就是为什么寻找「**最长相等的前缀和后缀**」。

### 计算前缀表

接下来就要说一说怎么计算前缀表。

如图：

![KMP精讲5](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/KMP%25E7%25B2%25BE%25E8%25AE%25B25.png)

长度为前1个字符的子串`a`，最长相同前后缀的长度为0。（注意字符串的**前缀是指不包含最后一个字符的所有以第一个字符开头的连续子串**；**后缀是指不包含第一个字符的所有以最后一个字符结尾的连续子串**。）

![KMP精讲6](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/KMP%25E7%25B2%25BE%25E8%25AE%25B26.png)

长度为前2个字符的子串`aa`，最长相同前后缀的长度为1。

![KMP精讲7](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/KMP%25E7%25B2%25BE%25E8%25AE%25B27.png)

长度为前3个字符的子串`aab`，最长相同前后缀的长度为0。

以此类推： 长度为前4个字符的子串`aaba`，最长相同前后缀的长度为1。 长度为前5个字符的子串`aabaa`，最长相同前后缀的长度为2。 长度为前6个字符的子串`aabaaf`，最长相同前后缀的长度为0。

那么把求得的最长相同前后缀的长度就是对应前缀表的元素，如图： ![KMP精讲8](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/KMP%25E7%25B2%25BE%25E8%25AE%25B28.png)

可以看出模式串与前缀表对应位置的数字表示的就是：**下标i之前（包括i）的字符串中，有多大长度的相同前缀后缀。**



注意找到不匹配的位置，我们看的是该位置的前一个字符的前缀表。这是因为我们想要找的是前面匹配成功的字符串的「**最长相等的前缀和后缀**」。

### 两种做法

KMP算法对于前缀表或者说next数组的表示有两种方法

+ next数组=前缀表
+ next数组=**前缀表统一减一（右移一位，初始位置为-1）。**



## 重复的子字符串

{{< blockquote author="力扣459" link="https://leetcode.cn/problems/repeated-substring-pattern/" title="重复的子字符串" >}}

给定一个非空的字符串 `s` ，检查是否可以通过由它的一个子串重复多次构成。

 

**示例 1:**

```
输入: s = "abab"
输出: true
解释: 可由子串 "ab" 重复两次构成。
```

**示例 2:**

```
输入: s = "aba"
输出: false
```

**示例 3:**

```
输入: s = "abcabcabcabc"
输出: true
解释: 可由子串 "abc" 重复四次构成。 (或子串 "abcabc" 重复两次构成。)
```

 

**提示：**



- `1 <= s.length <= 10^4`
- `s` 由小写英文字母组成

{{< /blockquote >}}
