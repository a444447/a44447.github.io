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
