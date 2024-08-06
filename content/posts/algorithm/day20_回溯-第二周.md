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

## N皇后

{{< blockquote link="https://leetcode.cn/problems/n-queens/" author="力扣51" title="N皇后" >}}

按照国际象棋的规则，皇后可以攻击与之处在同一行或同一列或同一斜线上的棋子。

**n 皇后问题** 研究的是如何将 `n` 个皇后放置在 `n×n` 的棋盘上，并且使皇后彼此之间不能相互攻击。

给你一个整数 `n` ，返回所有不同的 **n 皇后问题** 的解决方案。

每一种解法包含一个不同的 **n 皇后问题** 的棋子放置方案，该方案中 `'Q'` 和 `'.'` 分别代表了皇后和空位。

 

**示例 1：**

![img](https://assets.leetcode.com/uploads/2020/11/13/queens.jpg)

```
输入：n = 4
输出：[[".Q..","...Q","Q...","..Q."],["..Q.","Q...","...Q",".Q.."]]
解释：如上图所示，4 皇后问题存在两个不同的解法。
```

**示例 2：**

```
输入：n = 1
输出：[["Q"]]
```

 

**提示：**

- `1 <= n <= 9`

{{< /blockquote >}}

----

对于N皇后问题我们也可以整理为一个树来处理

![51.N皇后](https://code-thinking-1253855093.file.myqcloud.com/pics/20210130182532303.jpg)

对于树的每一层代表某一行，其下的分支表示选择了在某行放置♟️后，后面的行进行放置的过程。

对于判断`isValid`的时候，我们要注意理解剪枝判断是否列中有多个♟️的情况

```c++
for (int i = 0;i < row ; ++i)
```

这里是`i < row`，因为根据我们的回溯过程，超过`row`的行的`i`列根本没被填充所有没有必要检查。

另外，对于斜线的检查分为了45度与135度，分别是`--row-1,--col-1`与`--row-1,++col+1`

#### 代码

```c++
class Solution {
private:
    vector<vector<string>> res;
    void dfs(int level, int n, vector<string>& chessboard)
    {
        if (level == n)
        {
            res.push_back(chessboard);
            return ;
        }
        for (int i = 0; i < n; ++i) {
            if (isValid(level, i, n, chessboard)) {
                chessboard[level][i] = 'Q';
                dfs(level + 1, n, chessboard);
                chessboard[level][i] = '.';
            }
        }
    }
    bool isValid(int row, int col, int n, vector<string>& chessboard)
    {
        for (int i = 0; i < row; ++i) {
            if (chessboard[i][col] == 'Q') return false;
        }
        for (int i = row - 1, j = col - 1; i>=0&&j>=0; --i, --j) {
            if (chessboard[i][j] == 'Q') return false;
        }
        for (int i = row - 1, j = col + 1; i>=0&&j<n; --i, ++j) {
            if (chessboard[i][j] == 'Q') return false;
        }
        return true;
    }
public:
    vector<vector<string>> solveNQueens(int n) {
        std::vector<std::string> chessboard(n, std::string(n, '.'));
        dfs(0, n, chessboard);
        return res;
    }
};
```

- 时间复杂度: O(n!)
- 空间复杂度: O(n)

## 解数独

{{< blockquote link="https://leetcode.cn/problems/sudoku-solver/" author="力扣37" title="解数独" >}}

编写一个程序，通过填充空格来解决数独问题。

数独的解法需 **遵循如下规则**：

1. 数字 `1-9` 在每一行只能出现一次。
2. 数字 `1-9` 在每一列只能出现一次。
3. 数字 `1-9` 在每一个以粗实线分隔的 `3x3` 宫内只能出现一次。（请参考示例图）

数独部分空格内已填入了数字，空白格用 `'.'` 表示。

 

**示例 1：**

![img](https://assets.leetcode-cn.com/aliyun-lc-upload/uploads/2021/04/12/250px-sudoku-by-l2g-20050714svg.png)

```
输入：board = [["5","3",".",".","7",".",".",".","."],["6",".",".","1","9","5",".",".","."],[".","9","8",".",".",".",".","6","."],["8",".",".",".","6",".",".",".","3"],["4",".",".","8",".","3",".",".","1"],["7",".",".",".","2",".",".",".","6"],[".","6",".",".",".",".","2","8","."],[".",".",".","4","1","9",".",".","5"],[".",".",".",".","8",".",".","7","9"]]
输出：[["5","3","4","6","7","8","9","1","2"],["6","7","2","1","9","5","3","4","8"],["1","9","8","3","4","2","5","6","7"],["8","5","9","7","6","1","4","2","3"],["4","2","6","8","5","3","7","9","1"],["7","1","3","9","2","4","8","5","6"],["9","6","1","5","3","7","2","8","4"],["2","8","7","4","1","9","6","3","5"],["3","4","5","2","8","6","1","7","9"]]
解释：输入的数独如上图所示，唯一有效的解决方案如下所示：
```

 

**提示：**

- `board.length == 9`
- `board[i].length == 9`
- `board[i][j]` 是一位数字或者 `'.'`
- 题目数据 **保证** 输入数独仅有一个解

{{< /blockquote >}}

---

对于数独这个题，与之前的 **N皇后**不同，**对于N皇后**，我们一行只放一个♟️，因此可以只用一个for循环来遍历一行中的所有列，然后使用递归来进行下一行。

但是这个 **数独**是每一个位置都要放数字，因此无论如何都需要用两层for循环。此外，这个问题只要找到一个叶子结点，也就是答案就可以返回，因此回溯函数的返回值不是`void`而是`bool`



还有就是判断是否合法，要理解如何判断`3x3`是否合法:

```
int startRow = (row / 3) * 3;
int startCol = (col / 3) * 3;#找到该位置所属的3x3方格的最左上角的位置
for (int i = startRow; i < startRow + 3; i++) { // 判断9方格里是否重复
        for (int j = startCol; j < startCol + 3; j++) {
            if (board[i][j] == val ) {
                return false;
            }
        }
   }
```

