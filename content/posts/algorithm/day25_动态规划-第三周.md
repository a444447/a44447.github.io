---
title: "Day23_动态规划- 第三周"
date: 2024-08-13T09:45:04+08:00
Categories: [算法] 
---

继续来看有关DP的问题

## 单词拆分

{{< blockquote link="https://leetcode.cn/problems/word-break/" title="单词拆分" author="力扣139" >}}

给定一个非空字符串 s 和一个包含非空单词的列表 wordDict，判定 s 是否可以被空格拆分为一个或多个在字典中出现的单词。

说明：

拆分时可以重复使用字典中的单词。

你可以假设字典中没有重复的单词。

示例 1：

- 输入: s = "leetcode", wordDict = ["leet", "code"]
- 输出: true
- 解释: 返回 true 因为 "leetcode" 可以被拆分成 "leet code"。

示例 2：

- 输入: s = "applepenapple", wordDict = ["apple", "pen"]
- 输出: true
- 解释: 返回 true 因为 "applepenapple" 可以被拆分成 "apple pen apple"。
- 注意你可以重复使用字典中的单词。

示例 3：

- 输入: s = "catsandog", wordDict = ["cats", "dog", "sand", "and", "cat"]
- 输出: false

{{< /blockquote >}}

---

这个题当然可以使用回溯法解决，但是我们主要先来考虑的是DP的做法。

我们可以把字符串`s`当作背包容量，而`wordDict`相当于就是物品，那么定义的`dp[i]`，表示的就是长度为`i`的字符串能不能被拆分，如果能那么是`true`。

我们的递推公式也就是: 如果s[j~i]字串在wordDict中出现，且dp[j]=true,那么dp[i] = true。

另外这个题是求一个排列数的问题，也就是顺序是关键的，所以我们遍历的时候要先遍历背包容量。然后为了让整个递推能够进行下去,dp[0]是基础，所以dp[0]= true。

### 代码

```c++
bool wordBreak(string s, vector<string>& wordDict) {
        unordered_set<string> wordSet(wordDict.begin(), wordDict.end());
        int vol = s.size();
        int n = wordDict.size();
        vector<bool> dp(vol + 1, false);
        dp[0] = true;
        for (int i = 1; i <= vol; i++) {
            for (int j = 0; j < i; ++j) {
                auto subs = s.substr(j, i - j);
                if (wordSet.find(subs) != wordSet.end() && dp[j]) {
                    dp[i] = true;
                }
            }
        }
        return dp[vol];
    }
```

**处理完全背包问题的时候要记住排列数与组合数的区别**

## 多重背包

多重背包： 有N种物品和一个容量为V 的背包。第i种物品最多有Mi件可用，每件耗费的空间是Ci ，价值是Wi 。求解将哪些物品装入背包可使这些物品的耗费的空间 总和不超过背包容量，且价值总和最大。

实际上多重背包问题可以看作`0-1背包问题`,这是为什么呢？我们实际是就是把$M_i$件可用全部拆分了，就变成了0-1背包问题了。

比如下面是未拆分前的完全背包

| 重量  | 价值 | 数量 |      |
| ----- | ---- | ---- | ---- |
| 物品0 | 1    | 15   | 2    |
| 物品1 | 3    | 20   | 3    |
| 物品2 | 4    | 30   | 2    |

接下来是拆分后的0-1背包：

| 物品0 | 1    | 15   | 1    |
| ----- | ---- | ---- | ---- |
| 物品0 | 1    | 15   | 1    |
| 物品1 | 3    | 20   | 1    |
| 物品1 | 3    | 20   | 1    |
| 物品1 | 3    | 20   | 1    |
| 物品2 | 4    | 30   | 1    |
| 物品2 | 4    | 30   | 1    |



**从下面的题开始就不一定是背包问题了，是DP问题**

## 打家劫舍

{{< blockquote link="https://leetcode.cn/problems/house-robber/" title="打家劫舍" author="力扣198" >}}

你是一个专业的小偷，计划偷窃沿街的房屋。每间房内都藏有一定的现金，影响你偷窃的唯一制约因素就是相邻的房屋装有相互连通的防盗系统，如果两间相邻的房屋在同一晚上被小偷闯入，系统会自动报警。

给定一个代表每个房屋存放金额的非负整数数组，计算你 不触动警报装置的情况下 ，一夜之内能够偷窃到的最高金额。

- 示例 1：
- 输入：[1,2,3,1]
- 输出：4

解释：偷窃 1 号房屋 (金额 = 1) ，然后偷窃 3 号房屋 (金额 = 3)。  偷窃到的最高金额 = 1 + 3 = 4 。

- 示例 2：
- 输入：[2,7,9,3,1]
- 输出：12 解释：偷窃 1 号房屋 (金额 = 2), 偷窃 3 号房屋 (金额 = 9)，接着偷窃 5 号房屋 (金额 = 1)。  偷窃到的最高金额 = 2 + 9 + 1 = 12 。

提示：

- 0 <= nums.length <= 100
- 0 <= nums[i] <= 400

{{< /blockquote >}}

---

### 代码

```c++
int rob(vector<int>& nums) {
        vector<int> dp(nums.size() + 1, 0 );
        if (nums.size() == 1) return nums[0];
        dp[0] =  0;
        dp[1] = nums[0];
        for (int i = 2; i <= nums.size(); ++i) {
            dp[i] = max(dp[i - 2] + nums[i - 1], dp[i - 1]);
        }
        return dp[nums.size()];
    }
```



### 打家劫舍II

上面的题目改变一下，就变成这样：
> 你是一个专业的小偷，计划偷窃沿街的房屋，每间房内都藏有一定的现金。**这个地方所有的房屋都 围成一圈** ，这意味着第一个房屋和最后一个房屋是紧挨着的。同时，相邻的房屋装有相互连通的防盗系统，如果两间相邻的房屋在同一晚上被小偷闯入，系统会自动报警 。
>
> 给定一个代表每个房屋存放金额的非负整数数组，计算你 在不触动警报装置的情况下 ，能够偷窃到的最高金额。

也就是说，**如果偷盗了第一家，那么最后一家就无法偷盗；偷盗了最后一家，第一家就无法偷盗**

于是我们把整个问题分为：

+ 偷盗第一家，不偷盗最后一家
+ 偷盗最后一家，不偷盗第一家。

分别讨论这两种情况，最后哪种情况的结果金额最大，于是就输出谁。单独的一种情况其实就是上面的问题。

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210129160821374-20230310134003961.jpg" alt="213.打家劫舍II1" style="zoom:67%;" />

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210129160842491-20230310134008133.jpg" alt="213.打家劫舍II2" style="zoom:67%;" />

```c++
int rob(vector<int>& nums) {
        int n = nums.size();
        if (n == 0) return 0;
        if (n == 1) return nums[0];
        int res1 = subrob(nums,0, n - 2);
        int res2 = subrob(nums,1, n - 1);
        return max(res1, res2);
    }

    int subrob(vector<int>& nums, int start, int end)
    {
        if (start == end)  return nums[start];
        
        vector<int> dp(nums.size(), 0);
        dp[start] = nums[start];
        dp[start + 1] = max(nums[start], nums[start + 1]);
        for (int i = start + 2; i <= end; ++i) {
            dp[i] = max(dp[i - 2] + nums[i], dp[i - 1]);
        }
        return dp[end];
    }
```

### 打家劫舍III

还有一种变体，就是树形DP

> 在上次打劫完一条街道之后和一圈房屋后，小偷又发现了一个新的可行窃的地区。这个地区只有一个入口，我们称之为“根”。 除了“根”之外，每栋房子有且只有一个“父“房子与之相连。一番侦察之后，聪明的小偷意识到“这个地方的所有房屋的排列类似于一棵二叉树”。 如果两个直接相连的房子在同一天晚上被打劫，房屋将自动报警。
>
> 计算在不触动警报的情况下，小偷一晚能够盗取的最高金额。
>
> <img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210223173849619.png" alt="337.打家劫舍III" style="zoom:50%;" />

分析这个题目，对于一个节点来说，它只有取与不取两种情况，我们需要知道的是这两种情况哪种会得到更高的价值。

我们之前的状态转移都是在数组dp[]上的，在这里，我们需要变成在树的节点上。因此回想一下树的递归是怎么样的。

```c++
//比如后序
void 后序遍历(treenode* node)
{
    if (node == nullptr) return;
    后序遍历(node->left);
    后序遍历(node->right);
    do something
    return;
}
```

现在，**我们要求的是一个节点偷与不偷两个状态获得的金钱**。我们只需要一个dp[2]= {不偷, 偷},通过不断的后序遍历最后返回得到当前节点的dp[2]，然后继续向上返回。最终求max(dp[0], dp[1])。

在这个后序遍历的过程中，我们具体的逻辑代码，也就是do something应该是这样的

```c++

//首先这是前面的后序遍历 下标0：不偷，下标1：偷
vector<int> left = robTree(cur->left); // 左
vector<int> right = robTree(cur->right); // 右

// 偷cur
int val1 = cur->val + left[0] + right[0]; //当前node偷，那么node->left与node->right只有不偷了，因此只能取left[0],right[0]
// 不偷cur
int val2 = max(left[0], left[1]) + max(right[0], right[1]);//当前node不偷，left可以偷也可以不偷选择max(left[0],left[1]);right同理
return {val2, val1};
```

---

总体代码

```c++
int rob(TreeNode* root) {
    auto res = robTree(root);
    return max(res[0], res[1]);

}
vector<int> robTree(TreeNode* node) {
    if (node == nullptr) return {0, 0};
    auto left = robTree(node->left);
    auto right = robTree(node->right);

    int res1 = left[0] + right[0] + node->val;
    int res2 = max(left[0] , left[1]) + max(right[0], right[1]);
    return {res2, res1};
}
```

