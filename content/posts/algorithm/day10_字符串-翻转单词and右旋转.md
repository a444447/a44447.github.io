---
title: "Day10_字符串 翻转单词and右旋转"
date: 2024-06-18T19:52:04+08:00
categories: [算法]
---

## 翻转字符串里的单词

{{< blockquote author="力扣151" link="https://leetcode.cn/problems/reverse-words-in-a-string/" title="翻转字符串里的单词" >}}

给你一个字符串 `s` ，请你反转字符串中 **单词** 的顺序。

**单词** 是由非空格字符组成的字符串。`s` 中使用至少一个空格将字符串中的 **单词** 分隔开。

返回 **单词** 顺序颠倒且 **单词** 之间用单个空格连接的结果字符串。

**注意：**输入字符串 `s`中可能会存在前导空格、尾随空格或者单词间的多个空格。返回的结果字符串中，单词间应当仅用单个空格分隔，且不包含任何额外的空格。

 

**示例 1：**

```
输入：s = "the sky is blue"
输出："blue is sky the"
```

**示例 2：**

```
输入：s = "  hello world  "
输出："world hello"
解释：反转后的字符串中不能存在前导空格和尾随空格。
```

**示例 3：**

```
输入：s = "a good   example"
输出："example good a"
解释：如果两个单词间有多余的空格，反转后的字符串需要将单词间的空格减少到仅有一个。
```

 

**提示：**

- `1 <= s.length <= 10^4`
- `s` 包含英文大小写字母、数字和空格 `' '`
- `s` 中 **至少存在一个** 单词

{{< /blockquote >}}

---

这个问题的如果使用这样的思路: `根据空格split为几个单词，然后将单词以倒序拼接为新的string`。这样的思路会用额外的空间。

我们如果考虑要在$O(1)$的空间复杂度完成这个问题，那么思路分为了下面的步骤:

+ 清除不必要的空格
+ 将整个`s`的字符串颠倒
+ 再将单个单词的字符顺序颠倒。

---

对于 `清除不必要的空格`,可以使用快慢指针的思路，也就是：快指针始终向前寻找不为空格的元素，当找到后，慢指针开始与快指针同步移动，直到遇到空格为止。



对于`反转字符串`与`反转字符子区间`在day9中已经提到。

### 代码

```c++
 void reverse(string& s, int start, int end){ 
        for (int i = start, j = end; i < j; i++, j--) {
            swap(s[i], s[j]);
        }
    }

    void delSpace(string& s) {
        int slow{0};
        for (int i = 0; i < s.size(); ++i)
        {
            if (s[i] != ' ') {
                if (slow != 0) s[slow++] = ' ';
                while(i < s.size() && s[i] != ' ') {
                    s[slow++] = s[i++];
                }
            }
        }
        s.resize(slow); 
    }

    string reverseWords(string s) {
        delSpace(s);
        reverse(s, 0, s.size() - 1);
        int start = 0; //removeExtraSpaces后保证第一个单词的开始下标一定是0。
        for (int i = 0; i <= s.size(); ++i) {
            if (i == s.size() || s[i] == ' ') { //到达空格或者串尾，说明一个单词结束。进行翻转。
                reverse(s, start, i - 1); //翻转，注意是左闭右闭 []的翻转。
                start = i + 1; //更新下一个单词的开始下标start
            }
        }
        return s;
    }
```

- 时间复杂度: O(n)
- 空间复杂度: O(1) 或 O(n)，取决于语言中字符串是否可变

---

还可以使用`双端队列`,只不过有额外的空间开销$O(n)$

## 右旋字符串

{{< blockquote author="卡吗网" link="https://kamacoder.com/problempage.php?pid=1065" title="右旋字符串" >}}

字符串的右旋转操作是把字符串尾部的若干个字符转移到字符串的前面。给定一个字符串 s 和一个正整数 k，请编写一个函数，将字符串中的后面 k 个字符移到字符串的前面，实现字符串的右旋转操作。

例如，对于输入字符串 "abcdefg" 和整数 2，函数应该将其转换为 "fgabcde"。

输入：输入共包含两行，第一行为一个正整数 k，代表右旋转的位数。第二行为字符串 s，代表需要旋转的字符串。

输出：输出共一行，为进行了右旋转操作后的字符串。

样例输入：

```text
2
abcdefg 
```

样例输出：

```text
fgabcde
```

数据范围：1 <= k < 10000, 1 <= s.length < 10000;

{{< /blockquote >}}

---

这个题的思路和前面的反转单词几乎一样，我们只需要把整个字符串看作`length-k`与`k`两个部分，然后选整体反转，再单独反转这两个子字符串就达到了要求。

这样的操作也是不需要申请额外空间的。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20231106172058.png)

### 代码

```c++
// 版本一
#include<iostream>
#include<algorithm>
using namespace std;
int main() {
    int n;
    string s;
    cin >> n;
    cin >> s;
    int len = s.size(); //获取长度

    reverse(s.begin(), s.end()); // 整体反转
    reverse(s.begin(), s.begin() + n); // 先反转前一段，长度n
    reverse(s.begin() + n, s.end()); // 再反转后一段

    cout << s << endl;

} 
```



