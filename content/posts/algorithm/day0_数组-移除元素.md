---
title: "day0_数组-移除元素"
date: 2024-05-28T21:30:00+08:00
categories: [算法]
---

# 移除元素

{{< blockquote author="力扣27" link="https://leetcode.cn/problems/remove-element/description/" title="移除元素" >}}

给你一个数组 `nums` 和一个值 `val`，你需要 **[原地](https://baike.baidu.com/item/原地算法)** 移除所有数值等于 `val` 的元素。元素的顺序可能发生改变。然后返回 `nums` 中与 `val` 不同的元素的数量。

假设 `nums` 中不等于 `val` 的元素数量为 `k`，要通过此题，您需要执行以下操作：

- 更改 `nums` 数组，使 `nums` 的前 `k` 个元素包含不等于 `val` 的元素。`nums` 的其余元素和 `nums` 的大小并不重要。
- 返回 `k`

{{< /blockquote >}}

```c++
输入：nums = [3,2,2,3], val = 3
输出：2, nums = [2,2,_,_]
解释：你的函数函数应该返回 k = 2, 并且 nums 中的前两个元素均为 2。
你在返回的 k 个元素之外留下了什么并不重要（因此它们并不计入评测）。

输入：nums = [0,1,2,2,3,0,4,2], val = 2
输出：5, nums = [0,1,4,0,3,_,_,_]
解释：你的函数应该返回 k = 5，并且 nums 中的前五个元素为 0,0,1,3,4。
注意这五个元素可以任意顺序返回。
你在返回的 k 个元素之外留下了什么并不重要（因此它们并不计入评测）。   
```

---

这个问题如果是**暴力的解法**就是使用两个循环，第一个`for`负责找到等于`val`的位置，然后第二个`for`将数组后面的部分全部移动一位。

这个时间复杂度显然是$O(n^2)$，空间复杂度$O(1)$。

---

对于数组，链表的操作问题，要经常记住 **双指针甚至更多**

这个问题可以使用快慢指针求解。其核心的思路就是，定义`fast_ptr`与`slow_ptr`，当没有遇到`nums[index]==val`的时候，`fast_ptr`与`slow_ptr`同步前进，当遇到一个`val`时，`slow_ptr`会停止在原地，而`fast_ptr`负责继续向前寻找一个`!=val`的值用于覆盖`slow_ptr`所指的内容, `slow_ptr`不再停留。最后当`fast_ptr`遍历到末尾，整个过程结束，`slow_ptr`所对应的index就是答案。

## 代码

```c++
int removeElement(vector<int>& nums, int val) {
        int slow_ptr{0};
        for (int fast_ptr = 0; fast_ptr < nums.size(); ++fast_ptr)
        {
            if (nums[fast_ptr] != val)
            {
                nums[slow_ptr++] = nums[fast_ptr];
            }
        }
        return slow_ptr;
}
```

+ 时间复杂度: $O(n)$
+ 空间复杂度: $O(1)$



## 双指针优化

前面的双指针方法中，最坏情况下会走接近`2n`的数组长度，比如`[1,2,3,4,5], val=1`。

另一个优化方法就是让`fast_ptr`与`slow_ptr`变成分别位于数组头尾的两个指针，它们向中间移动遍历整个数组，最坏情况下合起来只遍历一次。

具体思路就是: `left`指向的元素如果等于`val`，那么就将`right`指向的元素复制给`left`,此时`right-1`, `left`不会改变（因为有可能`right`给的值依然是`val`，这样可以继续重复前面的过程，直到`nums[left]!=val`）。当`left==right`，返回`left`

```c++
int left = 0, right = nums.size();
       while (left < right)
       {
        if(nums[left] == val)
        {
            nums[left] = nums[right - 1];
            right--;
        } else {
            left++;
        }
       }
       return left;
```

