---
title: "Day20_回溯 第二周"
date: 2024-08-02T15:12:04+08:00
Categories: [算法]
---

## 组合总和 II

{{< blockquote link="https://leetcode.cn/problems/combination-sum-ii/" author="力扣40" title="组合总和II" >}}

给定一个候选人编号的集合 `candidates` 和一个目标数 `target` ，找出 `candidates` 中所有可以使数字和为 `target` 的组合。

`candidates` 中的每个数字在每个组合中只能使用 **一次** 。

**注意：**解集不能包含重复的组合。 

 

**示例 1:**

```
输入: candidates = [10,1,2,7,6,1,5], target = 8,
输出:
[
[1,1,6],
[1,2,5],
[1,7],
[2,6]
]
```

**示例 2:**

```
输入: candidates = [2,5,2,1,2], target = 5,
输出:
[
[1,2,2],
[5]
]
```

 

**提示:**

- `1 <= candidates.length <= 100`
- `1 <= candidates[i] <= 50`
- `1 <= target <= 30`

{{< /blockquote >}}

---

这道题和之前的那些组合求和的核心区别在于： *如何在搜索的过程中就把重复的结果跳过*。

在这道题目中，假设我们的`candiates=[1,1,2,1], target=3`：

+ 对于`[1,1,1]`、`[1_1,2]`是允许的
+ 但是如果是`[1_2],2`与`[1_1,2]`就不能同时出现

还是按照我们之前的对于组合问题以 *树的角度思考*的方法，也就是同一个分支下是可以重复元素的，但是同一层中的重复元素是不允许的。

保证了上面这个条件，我们就能够做到在搜索的过程中同时去除掉重复的组合。

为了实现这个规则，可以引入一个`used[]`数组，我们首先会将整个`candiates`排序，然后每次准备进入下一个分支之前都会先检查`if candiates[i] == candiates[i - 1]`， 如果是`true`，就需要判断这个相同的元素是对于同层还是同一树支:

+ 如果`used[i - 1] == true`，表示同一树枝上`candidates[i - 1]`使用过
+ 如果`used[i - 1] == false`，表示同一层上`candidates[i - 1]`使用过

![40.组合总和II](https://code-thinking-1253855093.file.myqcloud.com/pics/20230310000918.png)

#### 代码

```c++
class Solution {
private:
    vector<vector<int>> res;
    vector<int> path;
   
    void dfs(int idx, int target, int sum, vector<int>& path, vector<int>& candidates, vector<bool>& used)
    {
        if (target == sum)
        {
            res.push_back(path);
            return;
        }
        for (int i = idx; i < candidates.size() && sum + candidates[i] <= target; i++)
        {
            if (i > 0 && candidates[i - 1] == candidates[i] && used[i - 1] == false) {
                continue;
            }
            sum += candidates[i];
            path.push_back(candidates[i]);
            used[i] = true;
            dfs(i + 1, target, sum, path, candidates, used);
            used[i] = false;
            sum -= candidates[i];
            path.pop_back();
        }
        return ;
    }
public:
    vector<vector<int>> combinationSum2(vector<int>& candidates, int target) {
        vector<bool> used(candidates.size(), false);
        sort(candidates.begin(), candidates.end());
        dfs(0, target, 0, path, candidates, used);
        return res;
    }
};
```

