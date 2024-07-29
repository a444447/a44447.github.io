---
title: "Day19_回溯 第一周"
date: 2024-07-25T15:33:04+08:00
Categories: [算法]
---

这是回溯算法的第一周。有关回溯算法，它大部分时候是用来解决 **组合、排列等**问题，也就是很多时候我们找不到特别好的思路，回溯可以穷举所有的选项。

我们可以把所有的回溯问题都想象为一个 **高度有限的N叉树**。

{{< imgcap src="https://w.wallhaven.cc/full/nr/wallhaven-nrq3pq.jpg" title="bird and sea" >}}

## 组合问题

### 基础组合1

{{< blockquote author="力扣77" link="https://leetcode.cn/problems/combinations/" title="基础组合" >}}

给定两个整数 `n` 和 `k`，返回范围 `[1, n]` 中所有可能的 `k` 个数的组合。

你可以按 **任何顺序** 返回答案。

 

**示例 1：**

```
输入：n = 4, k = 2
输出：
[
  [2,4],
  [3,4],
  [2,3],
  [1,2],
  [1,3],
  [1,4],
]
```

**示例 2：**

```
输入：n = 1, k = 1
输出：[[1]]
```

 

**提示：**

- `1 <= n <= 20`
- `1 <= k <= n`

{{< /blockquote >}}

---

这是一个很经典的回溯的题，我们想象有一棵树，树的分支是由当前节点状态能够选择继续选择的数还有哪些决定的。

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202407251610632.png" alt="77.组合" style="zoom:67%;" />

在定义dfs函数的时候，可以传入一个`startIndex`用来记录当前层遍历的范围[startIndex, n]

对于回溯问题，有时候是可以剪枝的，比如这个问题中，如果`n=4,k=4`，那么第一层中`startIndex=2,3,4`的那些分支都可以被删掉，因为他们已经无法满足。

对于这个问题，我们假设`k - path.size()`是还需要的元素数量, `n - (k - path.size()) + 1`指的就是`startIndex`必须要小于这个值，才能满足`path.size()`的要求

#### 代码

```c++
vector<vector<int>> combine(int n, int k) {
        vector<int> path;
        vector<vector<int>> res;
        dfs(res, path, 1, n, k);
        return res;
    }
    void dfs(vector<vector<int>> &res, vector<int> &path, int startIndex, int n, int k) {
        if (path.size() == k) {
            res.push_back(path);
            return ;
        }
        for (int i = startIndex; i <= (n-(k-path.size()) + 1) i++) {
            path.push_back(i);
            dfs(res, path, i + 1, n, k);
            path.pop_back();
        }
    }
```

- 时间复杂度: O(n * 2^n)
- 空间复杂度: O(n)

### 基础组合2

{{< blockquote author="力扣216" link="https://leetcode.cn/problems/combination-sum-iii/" title="基础组合2" >}}

找出所有相加之和为 `n` 的 `k` 个数的组合，且满足下列条件：

- 只使用数字1到9
- 每个数字 **最多使用一次** 

返回 *所有可能的有效组合的列表* 。该列表不能包含相同的组合两次，组合可以以任何顺序返回。

 

**示例 1:**

```
输入: k = 3, n = 7
输出: [[1,2,4]]
解释:
1 + 2 + 4 = 7
没有其他符合的组合了。
```

**示例 2:**

```
输入: k = 3, n = 9
输出: [[1,2,6], [1,3,5], [2,3,4]]
解释:
1 + 2 + 6 = 9
1 + 3 + 5 = 9
2 + 3 + 4 = 9
没有其他符合的组合了。
```

**示例 3:**

```
输入: k = 4, n = 1
输出: []
解释: 不存在有效的组合。
在[1,9]范围内使用4个不同的数字，我们可以得到的最小和是1+2+3+4 = 10，因为10 > 1，没有有效的组合。
```

 

**提示:**

- `2 <= k <= 9`
- `1 <= n <= 60`

{{< /blockquote >}}

---

整体的思路和第一题差不多，只不过多了一个判断条件，也就是是否所有累计的和达到了`targetSum`。

剪枝操作就是在每次进入dfs函数一开头，`sum`是否已经大于了`targetSum`。

#### 代码

```c++
private:
    vector<vector<int>> res;
    vector<int> path;
public:
    vector<vector<int>> combinationSum3(int k, int n) {
        dfs(n, k, 0, 1);
        return res;
    }
    void dfs(int n, int k, int sum, int startIndex)
    {
        if (sum > n) return ;
        if (path.size() == k) {
            if (sum == n) res.push_back(path);
            return ;
        }
        for (int i = startIndex; i <= 9 - (k - path.size()) + 1; i++) {
            sum += i;
            path.push_back(i);
            dfs(n, k, sum, i + 1);
            sum -= i;
            path.pop_back();
        }
    }
```



