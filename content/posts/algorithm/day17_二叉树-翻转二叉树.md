---
title: "Day17_二叉树 翻转二叉树"
date: 2024-07-08T20:29:05+08:00
categories: [算法]
---

{{< blockquote author="力扣226" link="https://leetcode.cn/problems/invert-binary-tree/" title="翻转二叉树" >}}

给你一棵二叉树的根节点 `root` ，翻转这棵二叉树，并返回其根节点。

 

**示例 1：**

![img](https://assets.leetcode.com/uploads/2021/03/14/invert1-tree.jpg)

```
输入：root = [4,2,7,1,3,6,9]
输出：[4,7,2,9,6,3,1]
```

**示例 2：**

![img](https://assets.leetcode.com/uploads/2021/03/14/invert2-tree.jpg)

```
输入：root = [2,1,3]
输出：[2,3,1]
```

**示例 3：**

```
输入：root = []
输出：[]
```

 

**提示：**

- 树中节点数目范围在 `[0, 100]` 内
- `-100 <= Node.val <= 100`

{{< /blockquote >}}

---

思路很简单，就是对每个节点的左右节点都进行翻转，只是要决定以哪种顺序遍历节点

+ 前序、后序、层级：都可以
+ 中序: 中序有点问题,因为可能对某个节点反转两次

我们使用递归法很容易就可以写出代码，迭代法当然也能做。

### 代码

```c++
class Solution {
public:
    TreeNode* invertTree(TreeNode* root) {
        if (root == NULL) return root;
        swap(root->left, root->right);  // 中
        invertTree(root->left);         // 左
        invertTree(root->right);        // 右
        return root;
    }
};
```

