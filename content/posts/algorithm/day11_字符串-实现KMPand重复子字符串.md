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
