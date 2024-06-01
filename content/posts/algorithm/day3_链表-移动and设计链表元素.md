---
title: "Day3_链表 移动and设计链表元素"
date: 2024-06-01T18:48:39+08:00
categories: [算法]
---

{{< blockquote author="力扣203" link="https://leetcode.cn/problems/remove-linked-list-elements/" title="移除链表元素">}}

题意：删除链表中等于给定值 val 的所有节点。

```
示例 1： 输入：head = [1,2,6,3,4,5,6], val = 6 输出：[1,2,3,4,5]

示例 2： 输入：head = [], val = 1 输出：[]

示例 3： 输入：head = [7,7,7,7], val = 7 输出：[]
```

{{< /blockquote >}}

## 移除链表元素

首先在`c++`中，链表的节点一般是这样定义的

```c++
struct ListNode {
    int val;
    ListNode *next;
 	ListNode(int x): val(x), next(NULL) {}
};
```

对于移除链表中的元素一般是有两种方法，*一种是直接在原链表上进行删除，另一种是添加一个虚拟的头节点*

+ 原链表删除: 对于原链表删除，需要额外处理头节点，也就是如果删除头节点，那么需要把头节点先重新赋予给下一个原本的第二个节点。
+ 虚拟头节点： 添加了一个虚拟的头节点，这样就可以让删除任何节点的操作都是一样，而不需要像 *原链表删除*那样额外处理头节点情况。

### 代码

```c++
//这是原链表删除的方法
ListNode* removeElements(ListNode* head, int val) {
        auto ph = head;
        ListNode* node;

        while(head && head->val == val)
        {
            node = head;
            head = head->next;
            delete node;
        }
        ph = head;
        while(ph != nullptr && ph->next != nullptr)
        {
            if (ph->next->val == val)
            {
                node = ph->next;
                ph->next = node->next;
                delete node;
            } else {
                ph = ph->next;
            }
            
        }
        return head;
}
```

## 设计链表

{{< blockquote author="力扣203" link="https://leetcode.cn/problems/design-linked-list/" title="设计链表">}}

你可以选择使用单链表或者双链表，设计并实现自己的链表。

单链表中的节点应该具备两个属性：`val` 和 `next` 。`val` 是当前节点的值，`next` 是指向下一个节点的指针/引用。

如果是双向链表，则还需要属性 `prev` 以指示链表中的上一个节点。假设链表中的所有节点下标从 **0** 开始。

实现 `MyLinkedList` 类：

- `MyLinkedList()` 初始化 `MyLinkedList` 对象。
- `int get(int index)` 获取链表中下标为 `index` 的节点的值。如果下标无效，则返回 `-1` 。
- `void addAtHead(int val)` 将一个值为 `val` 的节点插入到链表中第一个元素之前。在插入完成后，新节点会成为链表的第一个节点。
- `void addAtTail(int val)` 将一个值为 `val` 的节点追加到链表中作为链表的最后一个元素。
- `void addAtIndex(int index, int val)` 将一个值为 `val` 的节点插入到链表中下标为 `index` 的节点之前。如果 `index` 等于链表的长度，那么该节点会被追加到链表的末尾。如果 `index` 比长度更大，该节点将 **不会插入** 到链表中。
- `void deleteAtIndex(int index)` 如果下标有效，则删除链表中下标为 `index` 的节点。

```
输入
["MyLinkedList", "addAtHead", "addAtTail", "addAtIndex", "get", "deleteAtIndex", "get"]
[[], [1], [3], [1, 2], [1], [1], [1]]
输出
[null, null, null, null, 2, null, 3]

解释
MyLinkedList myLinkedList = new MyLinkedList();
myLinkedList.addAtHead(1);
myLinkedList.addAtTail(3);
myLinkedList.addAtIndex(1, 2);    // 链表变为 1->2->3
myLinkedList.get(1);              // 返回 2
myLinkedList.deleteAtIndex(1);    // 现在，链表变为 1->3
myLinkedList.get(1);              // 返回 3
```

**提示：**

- `0 <= index, val <= 1000`
- 请不要使用内置的 LinkedList 库。
- 调用 `get`、`addAtHead`、`addAtTail`、`addAtIndex` 和 `deleteAtIndex` 的次数不超过 `2000` 。

{{< /blockquote >}}

---

这道题采用 *虚拟头节点的方式*

### 代码

```c++
MyLinkedList() {
        this->size = 0;
        this->head = new ListNode(0);
    }
    
    int get(int index) {
        if (index < 0 || index >= size) {
            return -1;
        }
        ListNode *cur = head;
        for (int i = 0; i <= index; i++) {
            cur = cur->next;
        }
        return cur->val;
    }
    
    void addAtHead(int val) {
        addAtIndex(0, val);
    }
    
    void addAtTail(int val) {
        addAtIndex(size, val);
    }
    
    void addAtIndex(int index, int val) {
        if (index > size) {
            return;
        }
        index = max(0, index);
        size++;
        ListNode *pred = head;
        for (int i = 0; i < index; i++) {
            pred = pred->next;
        }
        ListNode *toAdd = new ListNode(val);
        toAdd->next = pred->next;
        pred->next = toAdd;
    }
    
    void deleteAtIndex(int index) {
        if (index < 0 || index >= size) {
            return;
        }
        size--;
        ListNode *pred = head;
        for (int i = 0; i < index; i++) {
            pred = pred->next;
        }
        ListNode *p = pred->next;
        pred->next = pred->next->next;
        delete p;
    }
private:
    int size;
    ListNode *head;

```

