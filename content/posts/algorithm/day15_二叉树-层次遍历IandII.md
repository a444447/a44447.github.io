---
title: "Day15_二叉树 层次遍历IandII"
date: 2024-07-03T20:11:06+08:00
categories: [算法]
---

## 二叉树层次遍历I
![image-20240706194312111](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240706194312111.png)

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

![image-20240706194401991](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240706194401991.png)

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
