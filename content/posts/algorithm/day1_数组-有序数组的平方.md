---
title: "Day1_数组 有序数组的平方"
date: 2024-05-29T19:29:22+08:00
categories: [算法]
---

## 有序数组的平方

{{< blockquote author="力扣977" link="https://leetcode.cn/problems/squares-of-a-sorted-array/description/" title="有序数组的平方">}}

给你一个按 **非递减顺序** 排序的整数数组 `nums`，返回 **每个数字的平方** 组成的新数组，要求也按 **非递减顺序** 排序。

**提示：**

- `1 <= nums.length <= 104`
- `-104 <= nums[i] <= 104`
- `nums` 已按 **非递减顺序** 排序

{{< /blockquote >}}

```c++
示例 1：

输入：nums = [-4,-1,0,3,10]
输出：[0,1,9,16,100]
解释：平方后，数组变为 [16,1,0,9,100]
排序后，数组变为 [0,1,9,16,100]
    
示例 2：

输入：nums = [-7,-3,2,3,11]
输出：[4,9,9,49,121]    
```

---

对于这个问题，如果考虑**暴力解法**，也就是 **先把原始数组的每个数都平方，然后再排序**，这样的时间复杂度是$O(n + nlogn)$​。

再进一步查看题目，提到*nums本身就是非递减顺序*，也就是说由于有负数的存在会使得原元素平方后结果变大，因此平方后最大的元素 **出现在头或者尾**

也就是说，利用双指针可以遍历一次数组就找到答案，额外需要的是一个保存结果的`results`。

具体的思路是： 定义变量`i`,`j`分别指向`nums`的头尾，定义`k`指向`results`的末尾。比较指针所指的元素平方后的大小关系，如果`nums[j] ** 2 > nums[i] ** 2`，那么就将`results[k--] = nums[j--] ** 2`；反之，也是同样的处理。终止的条件是`k < 0`。



### 代码

```c++
vector<int> sortedSquares(vector<int>& nums) {
        int i = 0, j = nums.size() - 1, k = nums.size() - 1;
        vector<int> res(nums.size(), 0);
        while(k >= 0)
        {
            if (nums[i] * nums[i] > nums[j] * nums[j]) {
                res[k] = nums[i] * nums[i];
                i++;
            } else {
                res[k] = nums[j] * nums[j];
                j--;
            }
            k--;
        }
        return res;
    }
```

+ 时间复杂度: $O(n)$
+ 空间复杂度: $O(n)$



### 其他思路

还有一种双指针的思路就是想找到 **临界位置neg**,让`neg`左边（包括`neg`)的部分全部是负数，右边全部是非负数。这样的话`nums[0]~nums[neg]`是 *递减*的，而`nums[neg+1]~nums[n-1]`是 *递增的*。

这样的话相当于将原本的数组分为了两个 **有序子数组**，这样就可以用 **归并排序的思路** : 具体地，使用两个指针分别指向位置`neg`和 `neg+1`，每次比较两个指针对应的数，选择较小的那个放入答案并移动指针。当某一指针移至边界时，将另一指针还未遍历到的数依次放入答案。
