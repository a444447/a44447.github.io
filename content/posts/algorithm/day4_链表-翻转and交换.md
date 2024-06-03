---
title: "Day4_链表 翻转and交换"
date: 2024-06-03T19:45:53+08:00
draft: true
categories: [算法]
---

## 翻转链表

{{< blockquote author="力扣206" link="https://leetcode.cn/problems/reverse-linked-list/" title="反转链表">}}

给你单链表的头节点 head ，请你反转链表，并返回反转后的链表。

![image-20240603195939734](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240603195939734.png)

> 输入：head = [1,2,3,4,5]
> 输出：[5,4,3,2,1]

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/rev1ex2.jpg)

> 输入：head = [1,2]
> 输出：[2,1]

**提示：**

- 链表中节点的数目范围是 `[0, 5000]`
- `-5000 <= Node.val <= 5000`

{{< /blockquote >}}

---



翻转链表的最常见方法就是 **双指针**，具体思路是这样的： 定义一个`pre`以及`cur`。`pre`初始化指向`null`,`cur`指向翻转前的`head`。然后调转`cur`指向的每个节点的方向，比如对于`cur`指向`head`的情况：

```c++
auto tmp = cur;
cur->next=pre;
pre=cur;
cur=tmp->next;
```

持续这个过程，直到`cur`到达末尾，也就是`cur==nullptr`

### 代码

```c++
ListNode* reverseList(ListNode* head) {
        auto cur = head;
        ListNode* tmp = nullptr;
        ListNode* pre = nullptr;
        while(cur)
        {
            tmp = cur->next;
            cur->next = pre;
            pre = cur;
            cur = tmp;
        }
        return pre;
    }
```



## 两两交换链表中的节点

{{< blockquote author="力扣24" link="https://leetcode.cn/problems/swap-nodes-in-pairs/" title="两两交换链表中的节点">}}

给你一个链表，两两交换其中相邻的节点，并返回交换后链表的头节点。你必须在不修改节点内部的值的情况下完成本题（即，只能进行节点交换）。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/swap_ex1.jpg)

> 输入：head = [1,2,3,4]
> 输出：[2,1,4,3]

> 输入：head = []
> 输出：[]

> 输入：head = [1]
> 输出：[1]

{{< /blockquote >}}

---

题目要求说的很清楚，不能只是对值进行交换，也就是不能简单修改`node1->val`与`node2->val`的值，而是要交换`node1`与`node2`。

因此这个题也可以看作是一个模拟题，增加一个`虚拟头节点`

![image-20240603211152988](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240603211152988.png)

整个过程可以这样模拟: `cur`起始是一个虚拟头节点，它指向`head`，然后为了交换前两个`node1`与`node2`，忽略中间的一些temp变量，我们最终要让这第一组的链表指向为`cur->node2->node1->node3->node4->....`，然后将cur指向`node1`，然后重复上面的过程。

### 代码

```c++
ListNode* swapPairs(ListNode* head) {
        ListNode* dummy = new ListNode(0);
        dummy->next = head;
        ListNode* cur = dummy;
        while(cur->next && cur->next->next)
        {
            auto tmp1 = cur->next;
            auto tmp2 = cur->next->next;
            auto tmp3 = cur->next->next->next;
            cur->next = tmp2;
            cur->next->next = tmp1;
            cur->next->next->next = tmp3;
            cur = cur->next->next;

        }
        auto res = dummy->next;
        delete dummy;
        return res;
}
```

- 时间复杂度：O(n)
- 空间复杂度：O(1)

