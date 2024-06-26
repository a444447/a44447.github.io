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

---

首先以 **前缀表**来讲解如何构建next数组。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/2174350-20201010221728853-1683608557.png)

看上面的例子，next[8]是多少？对于求解next[8]，我们只需要nexrt[7]与array[8]就可以了,next[8]表示的是array[0-7]的最长前后缀长度：**我们通过next[7]可以找到包括该位置的之前字符串( 不包括该位置)的最长前后缀长度是多少，我们只需要讨论的是array[next[7]]与array[7]是否相等**，如果相等则next[8] = next[7]+1;

如果不相等就需要进入一个递归的过程：

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/2174350-20201010230822374-1301464430.png)

比如求next[10]，因为array[next[9]] != array[9]，所以我们需要做的就是尝试查询 **更短的相同前后缀**，这个更短的相同前后缀该如何得到？自然就是next[next[9]]，直到array[next[next...next[9]]] = array[9]。 *如果总是找不到怎么办？由于next[0]=0,所以如果当回退到next[0]还是没有满足，递归就会一直进行，这里就可以设置条件退出递归。**也可以将next[0]=-1，自然检查到-1就必要继续嵌套了***

所以也就出现了**前缀表统一减一（右移一位，初始位置为-1**的next数组表示方法

### 代码

```c++
//前缀表统一减1
void getNext(int *next, const string &s)
{
    int j = -1;
    next[0] = j;
    for(int i = 1; i < s.size(); i++) {
        while(j >= 0 && s[i] != s[j + 1]) {
            j = next[j];
        }
        if (s[i] == s[j + 1]) {
            j++;
        }
        next[i] = j;
    }
}
//使用next数组进行模式匹配
int j = -1; // 因为next数组里记录的起始位置为-1
for (int i = 0; i < s.size(); i++) { // 注意i就从0开始
    while(j >= 0 && s[i] != t[j + 1]) { // 不匹配
        j = next[j]; // j 寻找之前匹配的位置
    }
    if (s[i] == t[j + 1]) { // 匹配，j和i同时向后移动
        j++; // i的增加在for循环里
    }
    if (j == (t.size() - 1) ) { // 文本串s里出现了模式串t
        return (i - t.size() + 1);
    }
}
```

- 时间复杂度: O(n + m)
- 空间复杂度: O(m), 只需要保存字符串needle的前缀表

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

---

此问题第一种想法是,如果一个字符串s是由它的一个子串重复多次构成的,那么把两个字符串拼在一起`s+s`,那么这个拼接字符串中一定有找得到一个字符串s.

当然，我们在判断 s + s 拼接的字符串里是否出现一个s的的时候，**要刨除 s + s 的首字符和尾字符**，这样避免在s+s中搜索出原来的s，我们要搜索的是中间拼接出来的s。

---

另一种做法是kmp.==TODO还没有看懂==

### 代码

```c++
bool repeatedSubstringPattern(string s) {
        string new_s {s + s};
        new_s.erase(new_s.begin());
        new_s.erase(new_s.end() - 1);
        if (new_s.find(s) != std::string::npos) return true;
        return false;
    }
```

- 时间复杂度: O(n)
- 空间复杂度: O(1)

注意find的过程可能包括了我们之前提到的kmp方法.

> npos是一个常数，表示size_t的最大值（Maximum value for size_t）。许多容器都提供这个东西，用来表示不存在的位置.
