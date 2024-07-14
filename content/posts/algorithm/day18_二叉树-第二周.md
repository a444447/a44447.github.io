---
title: "Day18_二叉树 第二周"
date: 2024-07-09T20:45:08+08:00
categories: [算法]
---

## 对称二叉树

{{< blockquote author="力扣101" link="https://leetcode.cn/problems/symmetric-tree/" title="对称二叉树" >}}

给你一个二叉树的根节点 `root` ， 检查它是否轴对称。

 

**示例 1：**

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/1698026966-JDYPDU-image.png)

```
输入：root = [1,2,2,3,4,4,3]
输出：true
```

**示例 2：**

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/1698027008-nPFLbM-image.png)

```
输入：root = [1,2,2,null,3,null,3]
输出：false
```

 

**提示：**

- 树中节点数目在范围 `[1, 1000]` 内
- `-100 <= Node.val <= 100`

{{< /blockquote >}}

---

这个题无论是`递归方法`还是`迭代方法`都先要知道如何判断对称。假设我们从root开始，就是要将root分为了其left和right两棵子树，并且判断这两个树是否是对称的。在继续递归、遍历的过程中，就是不断比较当前的node其left和right两棵子树是否是对称的。

当node的left子树与right子树满足对称，且left==right, 那么以这个node为根的整个子树就是对称的，会向上返回true，反之，当出现了某一边不对称的情况就是false。

所以我们的终止条件就是:

- 左右都不为空，比较节点数值，相同返回true;否则返回false

- 左节点为空，右节点不为空，不对称，return false
- 左不为空，右为空，不对称 return false
- 左右都为空，对称，返回true

于是，我们的递归与迭代方法都可以依据这个思路完成了。

---

*值得注意的是迭代法可以用栈或者队列两种数据结构都可以，反正都是先装入left->left and right->right (或者left->right and right->left )*

### 代码

**迭代法**

```c++
bool isSymmetric(TreeNode* root) {
        
        stack<TreeNode*> st;

        st.push(root->left);
        st.push(root->right);

        while(!st.empty())
        {
            auto left = st.top(); st.pop();
            auto right = st.top(); st.pop();

            if (!left && !right) continue;
            if (!left || !right || (left->val != right -> val)) {
                return false;
            }
            st.push(left->left);
            st.push(right->right);
            st.push(left->right);
            st.push(right->left);
        }
        return true;
    }
```

**递归法**

```c++
bool isSymmetric(TreeNode* root) {
        
        return godeep(root->left, root->right);
    }

    bool godeep(TreeNode* left, TreeNode* right)
    {
        if (left == NULL && right != NULL) return false;
        else if (left != NULL && right == NULL) return false;
        else if (left == NULL && right == NULL) return true;
        else if (left->val != right->val) return false;
        else return godeep(left->left, right->right) && godeep(left->right, right->left);

 }
```

