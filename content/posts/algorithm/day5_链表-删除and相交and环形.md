---
title: "Day5_链表 删除and相交and环形"
date: 2024-06-04T15:25:27+08:00
draft: true
---

##  删除链表的倒数第N个节点

{{< blockquote author="力扣19" link="https://leetcode.cn/problems/remove-nth-node-from-end-of-list/" title="删除链表的倒数第N个节点">}}

给你一个链表，删除链表的倒数第 `n` 个结点，并且返回链表的头结点。

- 链表中结点的数目为 `sz`
- `1 <= sz <= 30`
- `0 <= Node.val <= 100`
- `1 <= n <= sz`

{{< /blockquote >}}

例1:

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202406041529972.jpg)

```
输入：head = [1,2,3,4,5], n = 2
输出：[1,2,3,5]
```

例2:

```
输入：head = [1], n = 1
输出：[]
```

例3:

```
输入：head = [1,2], n = 1
输出：[1]
```

---

这是一个很典型的双指针应用题，也就是定义`fast`与`slow`两个指针，先让`fast`移动`n`步，然后同时移动`fast`与`slow`，直到`fast`指向尾部，这时候`slow`指向的就是待删除的节点。

为了简单处理头结点删除的逻辑，我们还是可以使用`虚拟头结点处理`。注意我们实际处理的时候，先让`fast`走`n+1`步，因为这样当`fast`指向尾部的时候，`slow`指向的是要删除的节点的 **前一个节点**，这样方便删除.

![img](https://code-thinking.cdn.bcebos.com/pics/19.%E5%88%A0%E9%99%A4%E9%93%BE%E8%A1%A8%E7%9A%84%E5%80%92%E6%95%B0%E7%AC%ACN%E4%B8%AA%E8%8A%82%E7%82%B9.png)

![img](https://code-thinking.cdn.bcebos.com/pics/19.%E5%88%A0%E9%99%A4%E9%93%BE%E8%A1%A8%E7%9A%84%E5%80%92%E6%95%B0%E7%AC%ACN%E4%B8%AA%E8%8A%82%E7%82%B91.png)

![img](https://code-thinking.cdn.bcebos.com/pics/19.%E5%88%A0%E9%99%A4%E9%93%BE%E8%A1%A8%E7%9A%84%E5%80%92%E6%95%B0%E7%AC%ACN%E4%B8%AA%E8%8A%82%E7%82%B93.png)

### 代码

```c++
ListNode* removeNthFromEnd(ListNode* head, int n) {
        ListNode* dummyNode = new ListNode(0);
        dummyNode->next = head;
        auto fast = dummyNode;
        auto slow = dummyNode;

        int cnt = n+1;
        while(fast && cnt--)
        {
            fast=fast->next;
        }

        while(fast)
        {
            fast = fast->next;
            slow = slow->next;
        }

        auto rm = slow->next;
        slow->next = rm->next;
        delete rm;
        return dummyNode->next;
    }
```

- 时间复杂度：$O(L)$，其中 L 是链表的长度。
- 空间复杂度：$O(1)$。

### 另一种思路: 栈

我们也可以在遍历链表的同时将所有节点依次入栈。根据栈「先进后出」的原则，我们弹出栈的第 n个节点就是需要删除的节点，并且**目前栈顶的节点就是待删除节点的前驱节点**。这样一来，删除操作就变得十分方便了。

```c++
ListNode* removeNthFromEnd(ListNode* head, int n) {
        ListNode* dummy = new ListNode(0, head);
        stack<ListNode*> stk;
        ListNode* cur = dummy;
        while (cur) {
            stk.push(cur);
            cur = cur->next;
        }
        for (int i = 0; i < n; ++i) {
            stk.pop();
        }
        ListNode* prev = stk.top();
        prev->next = prev->next->next;
        ListNode* ans = dummy->next;
        delete dummy;
        return ans;
    }

```

+ 时间复杂度：$O(L)$
+ 空间复杂度：$O(L)$

## 链表相交

{{< blockquote author="力扣160" link="https://leetcode.cn/problems/intersection-of-two-linked-lists-lcci/" title="链表相交">}}

- 给你两个单链表的头节点 `headA` 和 `headB` ，请你找出并返回两个单链表相交的起始节点。如果两个链表没有交点，返回 `null` 。

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202406041622191.png" alt="image-20240604162224151" style="zoom:50%;" />

- `listA` 中节点数目为 `m`
- `listB` 中节点数目为 `n`
- `0 <= m, n <= 3 * 104`
- `1 <= Node.val <= 105`
- `0 <= skipA <= m`
- `0 <= skipB <= n`
- 如果 `listA` 和 `listB` 没有交点，`intersectVal` 为 `0`
- 如果 `listA` 和 `listB` 有交点，`intersectVal == listA[skipA + 1] == listB[skipB + 1]`

{{< /blockquote >}}

例1:

![img](https://assets.leetcode-cn.com/aliyun-lc-upload/uploads/2018/12/14/160_example_1.png)

```
输入：intersectVal = 8, listA = [4,1,8,4,5], listB = [5,0,1,8,4,5], skipA = 2, skipB = 3
输出：Intersected at '8'
解释：相交节点的值为 8 （注意，如果两个链表相交则不能为 0）。
从各自的表头开始算起，链表 A 为 [4,1,8,4,5]，链表 B 为 [5,0,1,8,4,5]。
在 A 中，相交节点前有 2 个节点；在 B 中，相交节点前有 3 个节点。
```

例2:

![img](https://assets.leetcode-cn.com/aliyun-lc-upload/uploads/2018/12/14/160_example_2.png)

```
输入：intersectVal = 2, listA = [0,9,1,2,4], listB = [3,2,4], skipA = 3, skipB = 1
输出：Intersected at '2'
解释：相交节点的值为 2 （注意，如果两个链表相交则不能为 0）。
从各自的表头开始算起，链表 A 为 [0,9,1,2,4]，链表 B 为 [3,2,4]。
在 A 中，相交节点前有 3 个节点；在 B 中，相交节点前有 1 个节点。
```

例3:

![img](https://assets.leetcode-cn.com/aliyun-lc-upload/uploads/2018/12/14/160_example_3.png)

```
输入：intersectVal = 0, listA = [2,6,4], listB = [1,5], skipA = 3, skipB = 2
输出：null
解释：从各自的表头开始算起，链表 A 为 [2,6,4]，链表 B 为 [1,5]。
由于这两个链表不相交，所以 intersectVal 必须为 0，而 skipA 和 skipB 可以是任意值。
这两个链表不相交，因此返回 null 。
```

---

首先由于A与B长度不一定相等，因此需要先将它们两个对齐。然后再通过`curA`与`curB`两个指针比较它们所指的是否是同一个节点。

### 代码

```c++
ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
        int la{0}, lb{0};
        ListNode* curA = headA, *curB = headB;
        while(curA)
        {
            curA = curA->next;
            la++;
        }
        while(curB)
        {
            curB = curB->next;
            lb++;
        }
        curA = headA;
        curB = headB;
        if (la > lb) {
            int diff = la - lb;
            while(diff--)
            {
                curA = curA->next;
            }
        } else {
            int diff = lb - la;
            while(diff--)
            {
                curB = curB->next;
            }
        }
        while(curA)
        {
            if (curA == curB) {
                return curA;
            }
            curA = curA->next;
            curB = curB->next;
        }
        return NULL;
    }
```

+ 时间复杂度：$O(n+m)$
+ 空间复杂度: $O(1)$

## 环形链表II

{{< blockquote author="力扣142" link="https://leetcode.cn/problems/linked-list-cycle-ii/" title="环形链表II">}}

给定一个链表的头节点  `head` ，返回链表开始入环的第一个节点。 *如果链表无环，则返回 `null`。*

如果链表中有某个节点，可以通过连续跟踪 `next` 指针再次到达，则链表中存在环。 为了表示给定链表中的环，评测系统内部使用整数 `pos` 来表示链表尾连接到链表中的位置（**索引从 0 开始**）。如果 `pos` 是 `-1`，则在该链表中没有环。**注意：`pos` 不作为参数进行传递**，仅仅是为了标识链表的实际情况。

**不允许修改** 链表。

{{< /blockquote >}}

例1:

![img](https://assets.leetcode.com/uploads/2018/12/07/circularlinkedlist.png)

```
输入：head = [3,2,0,-4], pos = 1
输出：返回索引为 1 的链表节点
解释：链表中有一个环，其尾部连接到第二个节点。
```

例2:

![img](https://assets.leetcode-cn.com/aliyun-lc-upload/uploads/2018/12/07/circularlinkedlist_test3.png)

```
输入：head = [1], pos = -1
输出：返回 null
解释：链表中没有环。
```

---

首先如果这个题目只是简单的判断链表是否有环，那么我们采用的就是常规的 **快慢指针**，也就是`fast`一次移动2步,`slow`一次移动一步。

那么如果有环， **我们如何找到环的入口**呢？

假设从头结点到环形入口节点 的节点数为x。 环形入口节点到 fast指针与slow指针相遇节点 节点数为y。 从相遇节点 再到环形入口节点节点数为 z。 如图所示：

![img](https://code-thinking-1253855093.file.myqcloud.com/pics/20220925103433.png)

那么相遇时： slow指针走过的节点数为: `x + y`， fast指针走过的节点数：`x + y + n (y + z)`，n为fast指针在环内走了n圈才遇到slow指针， （y+z）为 一圈内节点的个数A。

因为fast指针是一步走两个节点，slow指针一步走一个节点， 所以 fast指针走过的节点数 = slow指针走过的节点数 * 2：

```
(x + y) * 2 = x + y + n (y + z)
```

两边消掉一个（x+y）: `x + y = n (y + z)`

因为要找环形的入口，那么要求的是x，因为x表示 头结点到 环形入口节点的的距离。

所以要求x ，将x单独放在左面：`x = n (y + z) - y` ,

再从n(y+z)中提出一个 （y+z）来，整理公式之后为如下公式：`x = (n - 1) (y + z) + z` 注意这里n一定是大于等于1的，因为 fast指针至少要多走一圈才能相遇slow指针。

这个公式说明什么呢？

先拿n为1的情况来举例，意味着fast指针在环形里转了一圈之后，就遇到了 slow指针了。

当 n为1的时候，公式就化解为 `x = z`，

这就意味着，**从头结点出发一个指针，从相遇节点 也出发一个指针，这两个指针每次只走一个节点， 那么当这两个指针相遇的时候就是 环形入口的节点**。

也就是在相遇节点处，定义一个指针index1，在头结点处定一个指针index2。**让index1和index2同时移动，每次移动一个节点， 那么他们相遇的地方就是 环形入口的节点。**

### 代码

```c++
  ListNode *detectCycle(ListNode *head) {
      ListNode* fast{head}, *slow{head};
      while(fast && fast->next)
      {
          slow = slow->next;
          fast = fast->next->next;
          if(slow == fast)
          {
              ListNode* index1 = fast;
              ListNode* index2 = head;
              while(index1 != index2)
              {
                  index1 = index1->next;
                  index2 = index2->next;
              }
              return index1;
          }
      }
      return NULL;
  }
```

+ 时间复杂度：$O(n)$,快慢指针相遇前，指针走的次数小于链表长度，快慢指针相遇后，两个index指针走的次数也小于链表长度，总体为走的次数小于 2n
+ 空间复杂度: $O(1)$
