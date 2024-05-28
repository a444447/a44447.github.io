---
title: "day0_数组-二分查找"
date: 2024-05-28T20:00:00+08:00
categories: [算法]
---

{{< imgcap src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20240521.jpg" title="远山" >}}

# 二分查找

{{< blockquote author="力扣704" link="https://leetcode.cn/problems/binary-search/" title="二分查找" >}}

给定一个 n 个元素有序的（升序）整型数组 nums 和一个目标值 target  ，写一个函数搜索 nums 中的 target，如果目标值存在返回下标，否则返回 -1。

提示：

- 你可以假设 nums 中的所有元素是不重复的。
- n 将在 [1, 10000]之间。
- nums 的每个元素都将在 [-9999, 9999]之间。

{{< /blockquote >}}

```c++
输入: nums = [-1,0,3,5,9,12], target = 9     
输出: 4       
解释: 9 出现在 nums 中并且下标为 4    
    
输入: nums = [-1,0,3,5,9,12], target = 2     
输出: -1        
解释: 2 不存在 nums 中因此返回 -1    
```

---

二分法的关键是理解**区间**这个概念，并且在整个二分查找的过程中要保持**搜索区间的规则**。

比如说如果我们按 **闭区间**的方式搜索:

+ 对于数组array, 我们搜索它的[left, right]
+ 求出middle, 如果`array[middle] > target`，表明了target不可能在[middle, right]这个区间中，因此下一个搜索区间一定是[left, middle-1]
+ 对于`array[middle] < target`，也是同样的，搜索区间更新为[middle+1, right]
+ 只有当`array[middle]=target`的时候，才是找到了目标值

还有一个处理的细节是，对于`while(left <= right)`还是`while(left < right)`

我们只需要思考，当`left==right`时，区间[left,right]依然是有效的，所以可以是`<=`

---

另一个思考的方式就是 **左闭右开**。

沿用我们上面的思考:

+ 首先`while(left < right)`，因为当`left==right`的时候，[left, right)是没有意义的。
+ `array[middle] > target`，更新的下一个区间是[left, middle)，在左闭右开的搜索模式下，上界是`middle`也不会被搜索

## 代码

```c++
int search(std::vector<int>& nums, int target) {
        int middle{0};
        int left{0}, right{static_cast<int>(nums.size()) - 1};
        while(left <= right)
        {
            middle = left + (right - left) / 2 ;
            if (nums[middle] == target)
            {
                return -1;
            } else if (nums[middle] < target) {
                left = middle + 1;
            } else {
                right = middle - 1;
            }
        }
        return -1;
    }
```

上面的代码是 **闭区间搜索**

+ 时间复杂度: $O(\log n)$
+ 空间复杂度: $O(n)$