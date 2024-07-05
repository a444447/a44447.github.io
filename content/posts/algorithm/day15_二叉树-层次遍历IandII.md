---
title: "Day15_二叉树 层次遍历IandII"
date: 2024-07-03T20:11:06+08:00
categories: [算法]
---

## 二叉树层次遍历I

{{< blockquote author="力扣102“ link="https://leetcode.cn/problems/binary-tree-level-order-traversal/" title="二叉树的层序遍历" >}}

给你二叉树的根节点 `root` ，返回其节点值的 **层序遍历** 。 （即逐层地，从左到右访问所有节点）。

**示例 1：**

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/tree1.jpg)

```
输入：root = [3,9,20,null,null,15,7]
输出：[[3],[9,20],[15,7]]
```

**示例 2：**

```
输入：root = [1]
输出：[[1]]
```

**示例 3：**

```
输入：root = []
输出：[]
```

 

**提示：**

- 树中节点数目在范围 `[0, 2000]` 内
- `-1000 <= Node.val <= 1000`

{{< /blockquote >}}

---

**迭代法**

二叉树的层次遍历与图论中的一致，因此很适合使用 **队列**,只需要记住的细节时，由于每一层的`size`不相同，因此每次遍历到新的一层都要重新设置`size`大小。

**递归法**

\\TODO

### 代码

```c++
vector<vector<int>> levelOrder(TreeNode* root) {
        queue<TreeNode*> q;
        vector<vector<int>> res;
        if (root != nullptr) q.push(root);
        while(!q.empty())
        {
            int siz = q.size();
            vector<int> tmp;
            for(int i = 0; i < siz; ++i)
            {
                TreeNode* cur = q.front();
                q.pop();
                if(cur->left) q.push(cur->left);
                if(cur->right) q.push(cur->right);
                tmp.push_back(cur->val);
            }
            res.push_back(tmp);
        }
        return res;

    }
```





## 二叉树层次遍历II

{{< blockquote author="力扣107“ link="https://leetcode.cn/problems/binary-tree-level-order-traversal-ii/" title="二叉树的层序遍历II" >}}

给你二叉树的根节点 `root` ，返回其节点值 **自底向上的层序遍历** 。 （即按从叶子节点所在层到根节点所在的层，逐层从左向右遍历）

 

**示例 1：**

![img](https://assets.leetcode.com/uploads/2021/02/19/tree1.jpg)

```
输入：root = [3,9,20,null,null,15,7]
输出：[[15,7],[9,20],[3]]
```

**示例 2：**

```
输入：root = [1]
输出：[[1]]
```

**示例 3：**

```
输入：root = []
输出：[]
```

 

**提示：**

- 树中节点数目在范围 `[0, 2000]` 内
- `-1000 <= Node.val <= 1000`

{{< /blockquote >}}

---

这题实际上就是将`I`中的问题得到的res反转就行



### 代码

```c++
vector<vector<int>> levelOrderBottom(TreeNode* root) {
        queue<TreeNode*> que;
        vector<vector<int>> res;
        if (root != nullptr) que.push(root);

        while(!que.empty())
        {
            int siz = que.size();
            vector<int> vec;
            for(int i = 0; i < siz; i++)
            {
                TreeNode* cur = que.front();
                que.pop();
                vec.push_back(cur->val);
                if(cur->left) que.push(cur->left);
                if(cur->right) que.push(cur->right);
            }
            res.push_back(vec);
        }
        reverse(res.begin(), res.end());
        return res;
    }
```

