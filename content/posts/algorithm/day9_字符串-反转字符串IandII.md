---
title: "Day9_字符串 反转字符串and替换数字"
date: 2024-06-17T19:18:09+08:00
categories: [算法]
---

## 反转字符串I

{{< blockquote author="力扣344" link="https://leetcode.cn/problems/reverse-string/" title="反转字符串I" >}}

编写一个函数，其作用是将输入的字符串反转过来。输入字符串以字符数组 `s` 的形式给出。

不要给另外的数组分配额外的空间，你必须**[原地](https://baike.baidu.com/item/原地算法)修改输入数组**、使用 O(1) 的额外空间解决这一问题。

 

**示例 1：**

```
输入：s = ["h","e","l","l","o"]
输出：["o","l","l","e","h"]
```

**示例 2：**

```
输入：s = ["H","a","n","n","a","h"]
输出：["h","a","n","n","a","H"]
```

 

**提示：**

- `1 <= s.length <= 10^5`
- `s[i]` 都是 [ASCII](https://baike.baidu.com/item/ASCII) 码表中的可打印字符

{{< /blockquote >}}

---

双指针，指向字符串的头和尾，然后交换头指针与尾指针指向的元素，再`++i,--j`，直到满足`i < s.size() /2`

### 代码

```c++
 void reverseString(vector<char>& s) {
        for (int i = 0,j = s.size() - 1; i < s.size() / 2; ++i, --j) {
            swap(s[i], s[j]);
        }
    }
```

+ 时间复杂度: $O(n)$
+ 空间复杂度:$O(1)$



## 反转字符串II



{{< blockquote author="力扣541" link="https://leetcode.cn/problems/reverse-string-ii/" title="反转字符串II" >}}

- 给定一个字符串 `s` 和一个整数 `k`，从字符串开头算起，每计数至 `2k` 个字符，就反转这 `2k` 字符中的前 `k` 个字符。

  - 如果剩余字符少于 `k` 个，则将剩余字符全部反转。
  - 如果剩余字符小于 `2k` 但大于或等于 `k` 个，则反转前 `k` 个字符，其余字符保持原样。

   

  **示例 1：**

  ```
  输入：s = "abcdefg", k = 2
  输出："bacdfeg"
  ```

  **示例 2：**

  ```
  输入：s = "abcd", k = 2
  输出："bacd"
  ```

   

  **提示：**

  - `1 <= s.length <= 10^4`
  - `s` 仅由小写英文组成
  - `1 <= k <= 10^4`

{{< /blockquote >}}

---

我们只需要把每`2k`当作一个区间就行，然后在每`2k`的区间中，反转前`k`个字符。

### 代码

```c++
 string reverseStr(string s, int k) {
        for (int i = 0; i < s.size(); i += (2 * k)) {
            if (i + k < s.size()) {
                reverse(s.begin() + i, s.begin() + i + k);
            } else {
                reverse(s.begin() + i, s.end());
            }
        }
        return s;
    }
```

+ 时间复杂度: $O(n)$
+ 空间复杂度: $O(1)$

## 替换数字

{{< blockquote author="卡玛网" link="https://kamacoder.com/problempage.php?pid=1064" title="替换数字" >}}

给定一个字符串 s，它包含小写字母和数字字符，请编写一个函数，将字符串中的字母字符保持不变，而将每个数字字符替换为number。

例如，对于输入字符串 "a1b2c3"，函数应该将其转换为 "anumberbnumbercnumber"。

对于输入字符串 "a5b"，函数应该将其转换为 "anumberb"

输入：一个字符串 s,s 仅包含小写字母和数字字符。

输出：打印一个新的字符串，其中每个数字字符都被替换为了number

样例输入：a1b2c3

样例输出：anumberbnumbercnumber

数据范围：1 <= s.length < 10000。

{{< /blockquote >}}

---

如果不用辅助空间，那么我们要做的就是扩充数组大小到能够装下所有的`number`，思路首先就是 **双指针**。

一个指针指向未扩充前数组的末尾，一个指针指向扩充完毕后的数组末尾，两个指针都从后向前。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20231030173058.png)

### 代码

```c++
string s;
while (cin >> s) {
    int sOldIndex = s.size() - 1;
    int count = 0; // 统计数字的个数
    for (int i = 0; i < s.size(); i++) {
        if (s[i] >= '0' && s[i] <= '9') {
            count++;
        }
    }
    // 扩充字符串s的大小，也就是将每个数字替换成"number"之后的大小
    s.resize(s.size() + count * 5);
    int sNewIndex = s.size() - 1;
    // 从后往前将数字替换为"number"
    while (sOldIndex >= 0) {
        if (s[sOldIndex] >= '0' && s[sOldIndex] <= '9') {
            s[sNewIndex--] = 'r';
            s[sNewIndex--] = 'e';
            s[sNewIndex--] = 'b';
            s[sNewIndex--] = 'm';
            s[sNewIndex--] = 'u';
            s[sNewIndex--] = 'n';
        } else {
            s[sNewIndex--] = s[sOldIndex];
        }
        sOldIndex--;
    }
    cout << s << endl;       
```

