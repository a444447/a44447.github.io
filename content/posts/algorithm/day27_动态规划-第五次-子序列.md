---
title: "Day27_动态规划- 第五次-子序列"
date: 2024-08-20T10:30:04+08:00
Categories: [算法]
---

这部分内容开始讨论有关 子序列的DP问题



## 最长子序列

{{< blockquote link="https://leetcode.cn/problems/longest-increasing-subsequence/" title="最长子序列" author="力扣300" >}}

给你一个整数数组 nums ，找到其中最长严格递增子序列的长度。

子序列是由数组派生而来的序列，删除（或不删除）数组中的元素而不改变其余元素的顺序。例如，[3,6,2,7] 是数组 [0,3,1,6,2,2,7] 的子序列。

示例 1：

- 输入：nums = [10,9,2,5,3,7,101,18]
- 输出：4
- 解释：最长递增子序列是 [2,3,7,101]，因此长度为 4 。

示例 2：

- 输入：nums = [0,1,0,3,2,3]
- 输出：4

示例 3：

- 输入：nums = [7,7,7,7,7,7,7]
- 输出：1

提示：

- 1 <= nums.length <= 2500
- -10^4 <= nums[i] <= 10^4

{{< /blockquote >}}

---

这是一道子序列问题的经典题目。要用dp方法的话，我们先进行DP分析：

+ dp[i]，表示的就是以nums[i]结尾的子序列的最长子序列的长度。
+ 递推公式就是很简单的:`if (nums[i] > nums[j]) dp[i] = max(dp[i], dp[j] + 1)`

+ 初始化，所有的`dp[i]`最初的值都应该是1，因为至少以nums[i]结尾的子序列长度为1(也就是它本身 )
+ 遍历顺序：从前向后遍历。

### 代码

```c++
int lengthOfLIS(vector<int>& nums) {
        if (nums.size() <= 1) return nums.size();
        int n = nums.size();
        vector<int> dp(n + 1, 1);
        int result = 0;
        for (int i = 1; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                if (nums[i] > nums[j]) dp[i] = max(dp[i], dp[j] + 1);
            }
            if (dp[i] > result) result = dp[i];
        }
        return result;

    }
```

## 最长连续递增序列

{{< blockquote link="https://leetcode.cn/problems/longest-continuous-increasing-subsequence/" title="最长连续递增序列" author="力扣674" >}}

给定一个未经排序的整数数组，找到最长且 连续递增的子序列，并返回该序列的长度。

连续递增的子序列 可以由两个下标 l 和 r（l < r）确定，如果对于每个 l <= i < r，都有 nums[i] < nums[i + 1] ，那么子序列 [nums[l], nums[l + 1], ..., nums[r - 1], nums[r]] 就是连续递增子序列。

示例 1：

- 输入：nums = [1,3,5,4,7]
- 输出：3
- 解释：最长连续递增序列是 [1,3,5], 长度为3。尽管 [1,3,5,7] 也是升序的子序列, 但它不是连续的，因为 5 和 7 在原数组里被 4 隔开。

示例 2：

- 输入：nums = [2,2,2,2,2]
- 输出：1
- 解释：最长连续递增序列是 [2], 长度为1。

提示：

- 0 <= nums.length <= 10^4
- -10^9 <= nums[i] <= 10^9

{{< /blockquote >}}

---

这个问题还是很简单的，主要是思考与上一个问题的区别。

**本问题中，要求是递增的子序列是连续的，而上一个题中的子序列是不用连续的**。因此在上一题中，我们需要通过两次for循环来遍历：第一层是`i`，第二层是`j(0~i)`要通过每个dp[j]来找到最大的长度子序列。

而本题中，由于是连续的区间，因此对于`nums[i]`，它的最大连续递增区间的长度，只需要考虑上一个元素`nums[i - 1]`与它的关系就行了。`if nums[i] > nums [i - 1] -> dp[i] = dp[i - 1] + 1`，因此我们只需要一个循环。

### 代码

```c++
    int findLengthOfLCIS(vector<int>& nums) {
        int n = nums.size();
        vector<int> dp(n + 1, 1);
        int res = 1;
        for (int i = 1; i < n; i++) {
            if (nums[i] > nums[i - 1]) {
                dp[i] = dp[i - 1] + 1;
            }
            if (res < dp[i]) res = dp[i];
        }
        return res;
    }
```

## 最长重复子数组

{{< blockquote link="https://leetcode.cn/problems/maximum-length-of-repeated-subarray/" title="最长重复子数组" author="力扣718" >}}

给两个整数数组 A 和 B ，返回两个数组中公共的、长度最长的子数组的长度。

示例：

输入：

- A: [1,2,3,2,1]
- B: [3,2,1,4,7]
- 输出：3
- 解释：长度最长的公共子数组是 [3, 2, 1] 。

提示：

- 1 <= len(A), len(B) <= 1000
- 0 <= A[i], B[i] < 100

{{< /blockquote >}}

---

注意，题目说的是`子数组`而不是`子序列`。`子数组`就等于`连续的子序列`。

对于DP问题，设计一个合理的dp数组是很重要的，只要能够想到一个合适的dp数组，很多问题就迎刃而解了。

对于这个问题，我们有两个数组，可以考虑这样设计: dp\[i][j],表示以A[i - 1]与以B[j-1]结尾，它们的最长子序列长度。

### 代码

```c++
    int findLength(vector<int>& nums1, vector<int>& nums2) {
        int n = nums1.size();
        int m = nums2.size();
        vector<vector<int>> dp(n + 1, vector<int>(m + 1));
        int res = 0;
        for (int i = 1; i <= n; ++i) {
            for (int j = 1; j <= m; ++j) {
                if (nums1[i - 1] == nums2[j - 1]) {
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                }
                if (dp[i][j] > res) res = dp[i][j];
            }
        }
        return res;
    }
```

当然，由于`dp[i][j]`完全由`dp[i - 1][j-  1]`推出，因此也可以用滚动数组来优化。

## 不相交的线

{{< blockquote link="https://leetcode.cn/problems/uncrossed-lines/" title="不相交的线" author="力扣1035" >}}

我们在两条独立的水平线上按给定的顺序写下 A 和 B 中的整数。

现在，我们可以绘制一些连接两个数字 A[i] 和 B[j] 的直线，只要 A[i] == B[j]，且我们绘制的直线不与任何其他连线（非水平线）相交。

以这种方法绘制线条，并返回我们可以绘制的最大连线数。

![1035.不相交的线](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/2021032116363533.png)

{{< /blockquote >}}

---

分析题目，说到`只要 A[i] == B[j]，且我们绘制的直线不与任何其他连线（非水平线）相交。`我们重点要分析的是什么样的序列会发生连线交汇？

很显然，只要找到公共的子序列就一定不会发生相交(子序列不会改变元素的排列顺序，只是中间的元素可以略过)。因此这个问题也就变成了求最大公共子序列。

### 代码

```c++
 int maxUncrossedLines(vector<int>& nums1, vector<int>& nums2) {
        int n = nums1.size(), m = nums2.size();
        vector<vector<int>> dp(n+1, vector<int>(m + 1));
        int res = 0;
        for (int i = 1; i <= n; ++i) {
            for (int j = 1; j <= m; ++j) {
                if (nums1[i - 1] == nums2[j - 1]) dp[i][j] = dp[i - 1][j - 1] + 1;
                else {
                    dp[i][j] = max(dp[i - 1][j], dp[i][j - 1]);
                }
                if (dp[i][j] > res) res = dp[i][j];
            }
        }
        return res; 
    }
```

## 最大子序和

{{< blockquote link="https://leetcode.cn/problems/maximum-subarray/" title="最大子序和" author="力扣53" >}}

给定一个整数数组 nums ，找到一个具有最大和的连续子数组（子数组最少包含一个元素），返回其最大和。

示例:

- 输入: [-2,1,-3,4,-1,2,1,-5,4]
- 输出: 6
- 解释: 连续子数组 [4,-1,2,1] 的和最大，为 6。

{{< /blockquote >}}

---

定义dp[i]: 以nums[i]结尾的数组的最大子序列后。

初始化: 由于dp[i]是由dp[i - 1] + nums[i]或者nums\[i](也就是说从当前nums[i]开始再找新的子序和)，dp[0]是一定要初始化的，dp[0]=nums[0]

### 代码

```c++
int maxSubArray(vector<int>& nums) {
       int n = nums.size();
       vector<int> dp(n+1);
       dp[0] = nums[0];
       int res =dp[0];
       for (int i = 1; i < n; i++) {
        dp[i] = max(dp[i - 1] + nums[i], nums[i]);
        if (dp[i] > res) res = dp[i];
       }
       return res;
    }
```

