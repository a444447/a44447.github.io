---
title: "Day6_哈希表 有效字母异位词"
date: 2024-06-06T19:07:00+08:00
categories: [算法]
---

{{< imgcap src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/c05ec9aea86fb8611af0ff0872e55200.jpg" title="某天才牙科医生拍摄😈">}}

## 有效字母异位词

{{< blockquote author="力扣242" link="https://leetcode.cn/problems/valid-anagram/" title="有效的字母异位词">}}

给定两个字符串 `s` 和 `t` ，编写一个函数来判断 `t` 是否是 `s` 的字母异位词。

**注意：**若 `s` 和 `t` 中每个字符出现的次数都相同，则称 `s` 和 `t` 互为字母异位词。

**示例 1:**

```
输入: s = "anagram", t = "nagaram"
输出: true
```

**示例 2:**

```
输入: s = "rat", t = "car"
输出: false
```

**提示:**

- `1 <= s.length, t.length <= 5 * 104`
- `s` 和 `t` 仅包含小写字母

{{< /blockquote >}}



---

这道题的 **暴力做法**就是两次循环，复杂度是$O(n^2)$。

**数组其实就是一个简单哈希表**，而且这道题目中字符串只有小写字符，那么就可以定义一个数组，来记录字符串s里字符出现的次数。

需要定义一个多大的数组呢，定一个数组叫做record，大小为26 就可以了，初始化为0，因为字符a到字符z的ASCII也是26个连续的数值。

再遍历 字符串s的时候，**只需要将 s[i] - ‘a’ 所在的元素做+1 操作即可，并不需要记住字符a的ASCII，只要求出一个相对数值就可以了。** 这样就将字符串s中字符出现的次数，统计出来了。

那看一下如何检查字符串t中是否出现了这些字符，同样在遍历字符串t的时候，对t中出现的字符映射哈希表索引上的数值再做-1的操作。

那么最后检查一下，**record数组如果有的元素不为零0，说明字符串s和t一定是谁多了字符或者谁少了字符，return false。**

### 代码

```c++
bool isAnagram(string s, string t) {
        int record[26];
        for (int i = 0; i < s.size(); i++)
        {
            record[s[i] - 'a']++;
        }
        for (int i = 0; i < t.size(); i++)
        {
            record[t[i] - 'a']--;
        }
        for(int i = 0; i < 26; i++)
        {
            if(record[i])
            {
                return false;
            }
        }
        return true;
    }
```

- 时间复杂度: O(n)
- 空间复杂度: O(1)
