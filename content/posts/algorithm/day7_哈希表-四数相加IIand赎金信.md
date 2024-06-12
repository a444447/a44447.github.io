---
title: "Day7_哈希表 四数相加IIand赎金信"
date: 2024-06-12T19:56:25+08:00
categories: [算法]
---

## 四数相加II

{{< blockquote author="力扣454" link="https://leetcode.cn/problems/4sum-ii/" title="四数相加II">}}

给你四个整数数组 `nums1`、`nums2`、`nums3` 和 `nums4` ，数组长度都是 `n` ，请你计算有多少个元组 `(i, j, k, l)` 能满足：

- `0 <= i, j, k, l < n`
- `nums1[i] + nums2[j] + nums3[k] + nums4[l] == 0`

**示例 1：**

```
输入：nums1 = [1,2], nums2 = [-2,-1], nums3 = [-1,2], nums4 = [0,2]
输出：2
解释：
两个元组如下：
1. (0, 0, 0, 1) -> nums1[0] + nums2[0] + nums3[0] + nums4[1] = 1 + (-2) + (-1) + 2 = 0
2. (1, 1, 0, 0) -> nums1[1] + nums2[1] + nums3[0] + nums4[0] = 2 + (-1) + (-1) + 0 = 0
```

**示例 2：**

```
输入：nums1 = [0], nums2 = [0], nums3 = [0], nums4 = [0]
输出：1
```

- `n == nums1.length`
- `n == nums2.length`
- `n == nums3.length`
- `n == nums4.length`
- `1 <= n <= 200`
- `-2^28 <= nums1[i], nums2[i], nums3[i], nums4[i] <= 2^28`

{{< /blockquote >}}

---

这个题目只要求给出有多少组元组满足条件，而不需要给出具体得元组，我们可以想到使用 **哈希表**。

思路就是： 使用一个哈希表记录数组`A`与数组`B`每个元素的和，还有出现的次数；然后再遍历C与D的和，看是否在哈希表中已经存在，如果存在说明这是一个满足条件的元组。

### 代码

```c++
int fourSumCount(vector<int>& nums1, vector<int>& nums2, vector<int>& nums3, vector<int>& nums4) {
        unordered_map<int,int> record;
        int res{0};
        for(auto a: nums1) {
            for (auto b: nums2) {
                record[a+b] += 1;
            }
        }
        for(auto c: nums3) {
            for(auto d: nums4) {
                if (record.find(0 - (c + d)) != record.end()) {
                    res += record[0-(c+d)];
                }
            }
        }
        return res;
    }
```

+ 时间复杂度: $O(n^2)$
+ 空间复杂度: $O(n^2)$

## 赎金信

{{< blockquote author="力扣383" link="https://leetcode.cn/problems/ransom-note/" title="赎金信">}}

给你两个字符串：`ransomNote` 和 `magazine` ，判断 `ransomNote` 能不能由 `magazine` 里面的字符构成。

如果可以，返回 `true` ；否则返回 `false` 。

`magazine` 中的每个字符只能在 `ransomNote` 中使用一次。

**示例 1：**

```
输入：ransomNote = "a", magazine = "b"
输出：false
```

**示例 2：**

```
输入：ransomNote = "aa", magazine = "ab"
输出：false
```

**示例 3：**

```
输入：ransomNote = "aa", magazine = "aab"
输出：true
```

 

**提示：**

- `1 <= ransomNote.length, magazine.length <= 105`
- `ransomNote` 和 `magazine` 由小写英文字母组成

{{< /blockquote >}}

---

使用`哈希表`是比较容易想到的。

### 代码

```c++
bool canConstruct(string ransomNote, string magazine) {
        int record[26];
        for(auto &e: magazine)
        {
            record[e - 'a'] += 1;
        }
        for(auto &e: ransomNote)
        {
            if (record[e - 'a'] > 0) {
                record[e - 'a'] -= 1;
            } else {
                return false;
            }
        }
        return true;
    }
```

+ 时间复杂度: $O(n)$
+ 空间复杂度: $O(1)$
