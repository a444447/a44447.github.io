---
title: "Day23_动态规划- 第二周"
date: 2024-08-11T14:45:04+08:00
Categories: [算法]
---
这周的动态规划先从0-1背包问题讲起

## 纯0-1背包问题

![E4AE5259A6472FB2CB647B5119EA7E0D](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/E4AE5259A6472FB2CB647B5119EA7E0D.jpg)

![C38CFA43B051D3D25340D5D483715A84](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/C38CFA43B051D3D25340D5D483715A84.png)

## 目标和

{{< blockquote link="https://leetcode.cn/problems/target-sum/" title="目标和" author="力扣494" >}}

给定一个非负整数数组，a1, a2, ..., an, 和一个目标数，S。现在你有两个符号 + 和 -。对于数组中的任意一个整数，你都可以从 + 或 -中选择一个符号添加在前面。

返回可以使最终数组和为目标数 S 的所有添加符号的方法数。

示例：

- 输入：nums: [1, 1, 1, 1, 1], S: 3
- 输出：5

解释：

- -1+1+1+1+1 = 3
- +1-1+1+1+1 = 3
- +1+1-1+1+1 = 3
- +1+1+1-1+1 = 3
- +1+1+1+1-1 = 3

一共有5种方法让最终目标和为3。

提示：

- 数组非空，且长度不会超过 20 。
- 初始的数组的和不会超过 1000 。
- 保证返回的最终结果能被 32 位整数存下。

{{< /blockquote >}}

---

阅读题目，其实也就是找把整个数组分为两部分的方法，我们假设第一部分的和叫做`left`，并且它们都会带上`+`，剩下的部分就是`sum-left`。我们的目的是`left-(sum-left)=target`->`left = (target+sum)/2`

由于`target`与`sum`都是确定的，所以我们的题目就转换成了找到子数组的和为`left`的组合。

---

当然用回溯法是可以做的，但是这里是想讨论如何用`0-1背包完成`

这里，我们的dp[j]表示的含义是`容量为j的背包，其有几种组合方法`。接下来来考虑一下递推的公式,对于`nums[i]`，如果想要装下它那么其所有的可能组合就是`dp[j-nums[i]]`。最终的递归公式就是`dp[j] += dp[j - nums[i]]`

下一个需要考虑的就是如何初始化。dp[0]=1是一定的，因为这是我们所有方法的起源。

遍历顺序还是和之前的单维度的0-1背包一致。

### 代码

```c++
int findTargetSumWays(vector<int>& nums, int target) {
        int n = nums.size();
        int total = 0, left;
        for (auto &e : nums) total += e;
        if ((target + total) % 2 == 1) return 0;
        if (abs(target) > total) return 0;
        left = (target + total) / 2;
        vector<int> dp(left + 1);
        dp[0] = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = left; j >= nums[i]; --j) {
                dp[j] += dp[j - nums[i]];
            }
        }
        return dp[left];
    }
```

- 时间复杂度：O(n × m)，n为正数个数，m为背包容量
- 空间复杂度：O(m)，m为背包容量

## 零和一

{{< blockquote link="https://leetcode.cn/problems/ones-and-zeroes/" title="一和零" author="力扣474" >}}

给你一个二进制字符串数组 strs 和两个整数 m 和 n 。

请你找出并返回 strs 的最大子集的大小，该子集中 最多 有 m 个 0 和 n 个 1 。

如果 x 的所有元素也是 y 的元素，集合 x 是集合 y 的 子集 。

示例 1：

- 输入：strs = ["10", "0001", "111001", "1", "0"], m = 5, n = 3
- 输出：4
- 解释：最多有 5 个 0 和 3 个 1 的最大子集是 {"10","0001","1","0"} ，因此答案是 4 。 其他满足题意但较小的子集包括 {"0001","1"} 和 {"10","1","0"} 。{"111001"} 不满足题意，因为它含 4 个 1 ，大于 n 的值 3 。

示例 2：

- 输入：strs = ["10", "0", "1"], m = 1, n = 1
- 输出：2
- 解释：最大的子集是 {"0", "1"} ，所以答案是 2 。

提示：

- 1 <= strs.length <= 600
- 1 <= strs[i].length <= 100
- strs[i] 仅由 '0' 和 '1' 组成
- 1 <= m, n <= 100

{{< /blockquote >}}

---

这个问题和我们之前的0-1背包问题的不同之处在于，我们之前的0-1背包问题可以使用dp[j]这样的一维数组，原因是我们的背包只需要考虑`重量`这个单一属性，所以通过不断刷新一维数组来达到二维数组的效果。

> 比如，原本二维的单考虑重量的0-1背包问题，其二维数组是dp\[i][j]，要想递推得到它是由max(dp\[i-1][j],dp\[i-1][j-nums[i]] +val)。而max(dp\[i-1][j],dp\[i-1][j-nums[i]] +val)又可以直接在原地更新dp\[i][j] =max(dp\[i][j],dp\[i][j-nums[i]] +val)。从而，可以直接压缩为一维。

但是当背包的属性不只是单一的`重量`的时候，比如有`重量1、重量2`，或者像本体一样有`1数量，0数量`，我们还是需要用二维的dp数组。

在考虑好这点后，我们的递推公式也很简单了:
$$
dp[i][j] = max(dp[i][j], dp[i - onenumber][j-twonumber] + 1)
$$
至于初始化和遍历顺序还是和传统的0-1一维dp数组一样，从后向前遍历.

### 代码

```c++
int findMaxForm(vector<string>& strs, int m, int n) {
        vector<vector<int>> dp(m + 1, vector<int>(n+1, 0));
        for (int i = 0; i < strs.size(); ++i)
        {
            auto s = strs[i];
            int zeroNum{0}, oneNum{0};
            for (auto &e : s) {
                if (e == '0') ++zeroNum;
                else ++oneNum;
            }
            for (int i = m; i >= zeroNum; --i) {
                for (int j = n; j >= oneNum; --j) {
                    dp[i][j] = max(dp[i][j], dp[i - zeroNum][j - oneNum] + 1);
                }
            }
        }
        return dp[m][n];
    }
```

- 时间复杂度: O(kmn)，k 为strs的长度
- 空间复杂度: O(mn)



## 零钱兑换

{{< blockquote link="https://leetcode.cn/problems/coin-change-ii/" title="零钱兑换II" author="力扣518" >}}

给定不同面额的硬币和一个总金额。写出函数来计算可以凑成总金额的硬币组合数。假设每一种面额的硬币有无限个。

示例 1:

- 输入: amount = 5, coins = [1, 2, 5]
- 输出: 4

解释: 有四种方式可以凑成总金额:

- 5=5
- 5=2+2+1
- 5=2+1+1+1
- 5=1+1+1+1+1

示例 2:

- 输入: amount = 3, coins = [2]
- 输出: 0
- 解释: 只用面额2的硬币不能凑成总金额3。

示例 3:

- 输入: amount = 10, coins = [10]
- 输出: 1

注意，你可以假设：

- 0 <= amount (总金额) <= 5000
- 1 <= coin (硬币面额) <= 5000
- 硬币种类不超过 500 种
- 结果符合 32 位符号整数

{{< /blockquote >}}

---

本题目是一个 **完全背包问题**，主要想讨论的点是，在遍历顺序上，是否还能任意？

需要区别的是排列数与组合数 

> 排列数对于不同顺序认定是不同的： {1，5}和{5，1}是两种答案
>
> 而组合数:{1,5}与{5,1}认为是一个答案.

如果将for最外层是背包容量，那么就会出现排序数的情况，比如{1,5}与{5,1}看作不同情况

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/4315860B10AAC1FB3E7C145AA9A24452.png" alt="4315860B10AAC1FB3E7C145AA9A24452" style="zoom:20%;" />

如果for最外层是物品种类，那么就是组合数。

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210120181331461.jpg" alt="518.零钱兑换II" style="zoom:67%;" />

### 代码

```c++
 int change(int amount, vector<int>& coins) {
        vector<int> dp(amount + 1, 0);
        int n = coins.size();
        dp[0] = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = coins[i]; j <= amount; ++j) {
                dp[j] += dp[j - coins[i]];
            }
        }
        return dp[amount];
    }
```

