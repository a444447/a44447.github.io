---
title: "Day16_二叉树 层次遍历续"
date: 2024-07-07T10:42:03+08:00
categories: [算法]
---

{{< blockquote author="力扣199" link="https://leetcode.cn/problems/binary-tree-right-side-view/" title="二叉树的右视图" >}}

给定一个二叉树的 **根节点** `root`，想象自己站在它的右侧，按照从顶部到底部的顺序，返回从右侧所能看到的节点值。

 

**示例 1:**

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/tree.jpg)

```
输入: [1,2,3,null,5,null,4]
输出: [1,3,4]
```

**示例 2:**

```
输入: [1,null,3]
输出: [1,3]
```

**示例 3:**

```
输入: []
输出: []
```

 

**提示:**

- 二叉树的节点个数的范围是 `[0,100]`
- `-100 <= Node.val <= 100` 

{{< /blockquote >}}

---

只需记录每次遍历每层时，最后一个元素到`res`中就行

### 代码

```c++
    vector<int> rightSideView(TreeNode* root) {
        queue<TreeNode*> que;
        vector<int> res;
        if (root != nullptr) que.push(root);
        while(!que.empty())
        {
            int siz = que.size();
            for(int i = 0;i < siz; i++)
            {
                auto tmp = que.front();
                que.pop();
                if (i == siz - 1) {

                    res.push_back(tmp->val);
                }
                if(tmp->left) que.push(tmp->left);
                if(tmp->right) que.push(tmp->right);
            }
        }
        return res;
    }
```

## 二叉树的层平均值

{{< blockquote author="力扣637" link="https://leetcode.cn/problems/average-of-levels-in-binary-tree/" title="二叉树的层平均值" >}}

给定一个非空二叉树, 返回一个由每层节点平均值组成的数组。

![637.二叉树的层平均值](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210203151350500.png)

{{< /blockquote >}}

---

就是记录每层的和，然后将平均值保存到`res`中

### 代码

```c++
vector<double> averageOfLevels(TreeNode* root) {
        queue<TreeNode*> que;
        vector<double> res;
        if (root != nullptr) que.push(root);
        while(!que.empty())
        {
            int siz = que.size();
            double sum {0};
            for (int i = 0; i < siz; i++) {
                auto tmp = que.front();
                que.pop();
                sum += tmp->val;
                if (tmp->left) que.push(tmp->left);
                if (tmp->right) que.push(tmp->right);
            }
           
            res.push_back(sum / siz);
        }
        return res;
    }
```



## N叉树的层序遍历

{{< blockquote author="力扣428" link="https://leetcode.cn/problems/n-ary-tree-level-order-traversal/" title="N叉树的层序遍历" >}}

给定一个 N 叉树，返回其节点值的*层序遍历*。（即从左到右，逐层遍历）。

树的序列化输入是用层序遍历，每组子节点都由 null 值分隔（参见示例）。

 

**示例 1：**

![img](https://assets.leetcode.com/uploads/2018/10/12/narytreeexample.png)

```
输入：root = [1,null,3,2,4,null,5,6]
输出：[[1],[3,2,4],[5,6]]
```

**示例 2：**

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/sample_4_964.png)

```
输入：root = [1,null,2,3,4,5,null,null,6,7,null,8,null,9,10,null,null,11,null,12,null,13,null,null,14]
输出：[[1],[2,3,4,5],[6,7,8,9,10],[11,12,13],[14]]
```

 

**提示：**

- 树的高度不会超过 `1000`
- 树的节点总数在 `[0, 104]` 之间

{{< /blockquote >}}

## .在每个树行中找最大值

{{< blockquote author="力扣515" link="https://leetcode.cn/problems/find-largest-value-in-each-tree-row/" title=".在每个树行中找最大值" >}}

{{< /blockquote >}}

## 填充每个节点的下一个右侧节点指针

{{< blockquote author="力扣116" link="https://leetcode.cn/problems/populating-next-right-pointers-in-each-node/" title="填充每个节点的下一个右侧节点指针" >}}

![](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210203152044855.jpg)

{{< /blockquote >}}

---

思路很简单，依然是层次遍历，只不过遇到每层第一个节点要先特殊处理一下。

### 代码

```c++
Node* connect(Node* root) {
        queue<Node*> que;
        Node* res;
        if (root != nullptr) que.push(root);
        while(!que.empty())
        {
            int siz = que.size();
            Node* nodepre;
            Node* node;
            for (int i = 0; i < siz; ++i)
            {
                node = que.front();
                if (i == 0) {
                    nodepre = node;
                    que.pop();
                } else {
                    nodepre->next = node;
                    que.pop();
                    node->next = NULL;
                    nodepre = node;
                }
                if (node->left) que.push(node->left);
                if (node->right) que.push(node->right);
            }
        }
        return root;
    }
```



## 填充每个节点的下一个右侧节点指针II

{{< blockquote author="力扣116" link="https://leetcode.cn/problems/populating-next-right-pointers-in-each-node-ii/" title="填充每个节点的下一个右侧节点指针II" >}}

{{< /blockquote >}}

## 二叉树的最大深度

{{< blockquote author="力扣104" link="https://leetcode.cn/problems/maximum-depth-of-binary-tree/" title="二叉树的最大深度" >}}

给定一个二叉树，找出其最大深度。

二叉树的深度为根节点到最远叶子节点的最长路径上的节点数。

说明: 叶子节点是指没有子节点的节点。

示例：

给定二叉树 [3,9,20,null,null,15,7]，

![104. 二叉树的最大深度](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210203153031914-20230310134849764.png)

返回它的最大深度 3 。

{{< /blockquote >}}

---

使用层次遍历的话就是一道模板题，相当于数有多少层数。

## 二叉树的最小深度

{{< blockquote author="力扣111" link="https://leetcode.cn/problems/minimum-depth-of-binary-tree/" title="二叉树的最小深度" >}}

{{< /blockquote >}}

---

依然可以使用层次遍历，需要考虑的就是什么时候达到最小深度。**显然，当进行层次遍历的时候，第一次遇到一个节点，它的左右孩子都是NULL,此时它就是第一个叶子，也就是最小深度所在层**。
