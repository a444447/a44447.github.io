---
title: "Day6_哈希表 数组交集and快乐数and两数之和"
date: 2024-06-11T19:12:42+08:00
categories: [算法]
---

## 两个数组的交集

{{< blockquote author="力扣349" link="https://leetcode.cn/problems/intersection-of-two-arrays/" title="两个数组的交集" >}}

给定两个数组 `nums1` 和 `nums2` ，返回 它们的 交集。输出结果中的每个元素一定是 **唯一** 的。我们可以 **不考虑输出结果的顺序** 。



**示例 1：**

```
输入：nums1 = [1,2,2,1], nums2 = [2,2]
输出：[2]
```

**示例 2：**

```
输入：nums1 = [4,9,5], nums2 = [9,4,9,8,4]
输出：[9,4]
解释：[4,9] 也是可通过的
```

**提示：**

- `1 <= nums1.length, nums2.length <= 1000`
- `0 <= nums1[i], nums2[i] <= 1000`

{{< /blockquote >}}

---

对于本题，因为题目给出了数据的范围`0 <= nums1[i], nums2[i] <= 1000`,并且不是很大，所以可以使用数组作哈希表。

```c++
 vector<int> intersection(vector<int>& nums1, vector<int>& nums2) {
        int record[1001];
        vector<int> res;
        for(auto &e: nums1)
        {
            record[e] = 1;
        }
        for(auto &e: nums2)
        {
            if (record[e] == -1) continue;
            if (record[e] > 0)
            {
                record[e] = -1;
                res.push_back(e);
            }
        }
        return res;
    }
```

另一种做法是使用`unordered_set`。

{{< notice info >}}

unordered_set会维护一个哈希表，它并不对数据进行排序，但是会对进去其中的数据自动进行去重操作。

{< /notice >}

### 代码

下面的代码是使用`unordered_set`的版本

```c++
vector<int> intersection(vector<int>& nums1, vector<int>& nums2) {
        unordered_set<int> res;
        unordered_set<int> num_set(nums1.begin(), nums1.end());

        for(auto e : nums2)
        {
            if (num_set.find(e) != num_set.end()) {
                res.insert(e);
            }
        }
        return vector<int>(res.begin(), res.end());
    }
```

+ 时间复杂度: $O(n + m)$,其中m是将set转换为vector的额外用时
+ 空间复杂度: $O(N)$

## 快乐数

{{< blockquote author="力扣202" link="https://leetcode.cn/problems/happy-number/" title="快乐数" >}}

编写一个算法来判断一个数 `n` 是不是快乐数。

**「快乐数」** 定义为：

- 对于一个正整数，每一次将该数替换为它每个位置上的数字的平方和。
- 然后重复这个过程直到这个数变为 1，也可能是 **无限循环** 但始终变不到 1。
- 如果这个过程 **结果为** 1，那么这个数就是快乐数。

如果 `n` 是 *快乐数* 就返回 `true` ；不是，则返回 `false` 。

**示例 1：**

```
输入：n = 19
输出：true
解释：
12 + 92 = 82
82 + 22 = 68
62 + 82 = 100
12 + 02 + 02 = 1
```

**示例 2：**

```
输入：n = 2
输出：false
```

 

- `1 <= n <= $2^31$ - 1`

{{< /blockquote >}}

这个题目说了，如果出现 **无限循环**，说明不是快乐数。这就说明了， *在求n的各位平方和的过程中，会出现重复的sum*。因此我们使用一个`unordered_set`记录计算的每次sum结果，如果出现重复，那么就说明会出现无限循环。

### 代码

```c++
bool isHappy(int n) {
    unordered_set<int> record;
    int tmp;
    while(record.find(n) == record.end())
    {
        record.insert(n);
        while(n)
        {
            tmp += (n % 10) *  (n % 10);
            n /= 10;
        }
        if (tmp == 1)
        {
            return true;
        }
        n = tmp;
        tmp = 0;
    }
    return false;
}
```

- 时间复杂度: $O(logn)$
- 空间复杂度: $O(logn)$

## 两数之和

{{< blockquote author="力扣1" link="https://leetcode.cn/problems/two-sum/" title="两数之和" >}}

- 给定一个整数数组 `nums` 和一个整数目标值 `target`，请你在该数组中找出 **和为目标值** *`target`* 的那 **两个** 整数，并返回它们的数组下标。

  你可以假设每种输入只会对应一个答案。但是，数组中同一个元素在答案里不能重复出现。

  你可以按任意顺序返回答案

  **示例 1：**

  ```
  输入：nums = [2,7,11,15], target = 9
  输出：[0,1]
  解释：因为 nums[0] + nums[1] == 9 ，返回 [0, 1] 。
  ```

  **示例 2：**

  ```
  输入：nums = [3,2,4], target = 6
  输出：[1,2]
  ```

  **示例 3：**

  ```
  输入：nums = [3,3], target = 6
  输出：[0,1]
  ```

- `2 <= nums.length <= 10^4`
- `-10^9 <= nums[i] <= 10^9`
- `-10^9 <= target <= 10^9`

{{< /blockquote >}}

---

这个题也可以使用 **哈希表**的思路

{{< notice notice >}}

当我们需要记录一个元素是否已经出现过的时候，就可以想到哈希表的方法

{{< /notice >}}

这里我们不能使用`set`，因为set只能记录`key`，而现在题目中我们除了要记录元素`x`是否出现过之外，还需要记录`index of x`,因此选择`unordered_map`

### 代码

```c++
 vector<int> twoSum(vector<int>& nums, int target) {
        unordered_map<int,int> record;
        int ant;
        for (int i = 0; i < nums.size(); i++)
        {
            ant = target - nums[i];
            if (record.find(ant) != record.end())
            {
                return vector<int>{i, record[ant]};
            }
            record[nums[i]] = i;
        }
        return {0,0};
    }
```

