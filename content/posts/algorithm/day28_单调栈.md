---
title: "Day27_动态规划- 第五次-子序列"
date: 2024-08-28T17:20:04+08:00
Categories: [算法]
---

**这部分内容开始讨论有关单调栈的问题**

---

首先我们要知道的一般遇到什么样的题目会使用单调栈？

> **通常是一维数组，要寻找任一个元素的右边或者左边第一个比自己大或者小的元素的位置，此时我们就要想到可以用单调栈了**。

## 每日温度

{{< blockquote link="https://leetcode.cn/problems/daily-temperatures/" title="每日温度" author="力扣739" >}}

请根据每日 气温 列表，重新生成一个列表。对应位置的输出为：要想观测到更高的气温，至少需要等待的天数。如果气温在这之后都不会升高，请在该位置用 0 来代替。

例如，给定一个列表 temperatures = [73, 74, 75, 71, 69, 72, 76, 73]，你的输出应该是 [1, 1, 4, 2, 1, 1, 0, 0]。

提示：气温 列表长度的范围是 [1, 30000]。每个气温的值的均为华氏度，都是在 [30, 100] 范围内的整数。

{{< /blockquote >}}

---

单调栈一般都是使用 **空间换时间的思想**，也就是我们会维护一个栈。这个栈的作用是什么呢？可以想一想，如果是暴力方法，我们需要两个for循环——这样的过程并没有对已经遍历过的内容进行记忆，反而进行了重复。所以，定义一个单调栈的目的就是保留遍历过的记忆，在一次遍历的情况下解决问题。

我们讨论 **单调栈**的时候，规定是 **从栈顶到栈顶**进行单调。

现在回到这个问题：很显然，我们需要维护一个 *单调递增的栈，也就是从栈头向栈底元素递增*。为了更好的理解，我们假设数组为`[3,1,2,4]`。最开始我们将`3`放入栈中`[3]`，然后下一个元素`1`与栈顶元素比较。由于题目要求的是右边的第一个比当前值大的元素，而显示`1`不是比`3`大，所以`3`还没找到答案，还需要继续留在栈中。而为了维护我们的单调栈，我们需要把`1`压入栈，现在`[3,1]`，其中`1`变成新的栈顶。面对下一个`2`，它比栈顶元素大,所以`1`得到答案，出栈，而下一个新栈顶`3`又比`2`大了，所以`3`还需要留在栈中，同时`2`入栈成为新的栈顶....

### 代码

```c++
    vector<int> dailyTemperatures(vector<int>& temperatures) {
        vector<int> ans(temperatures.size());
        stack<int> st;
        st.push(0);
        for (int i  = 1; i < temperatures.size(); ++i) {
                
            while (!st.empty() && temperatures[st.top()] < temperatures[i]) {
                ans[st.top()] = i  - st.top(); 
                st.pop();
            }
            st.push(i);
        }
        while(st.empty()) {
            ans[st.top()] = 0;
            st.pop();
        }
        return ans;
    }
```



## 接雨水

{{< blockquote link="https://leetcode.cn/problems/trapping-rain-water/" title="接雨水" author="力扣42" >}}

给定 n 个非负整数表示每个宽度为 1 的柱子的高度图，计算按此排列的柱子，下雨之后能接多少雨水。

示例 1：

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210713205038.png)

- 输入：height = [0,1,0,2,1,0,1,3,2,1,2,1]
- 输出：6
- 解释：上面是由数组 [0,1,0,2,1,0,1,3,2,1,2,1] 表示的高度图，在这种情况下，可以接 6 个单位的雨水（蓝色部分表示雨水）。

示例 2：

- 输入：height = [4,2,0,3,2,5]
- 输出：9

{{< /blockquote >}}

---

首先要理解题目，什么时候情况下能够接住雨水？

![42.接雨水3](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210223092732301.png)

列4 左侧最高的柱子是列3，高度为2（以下用lHeight表示）。

列4 右侧最高的柱子是列7，高度为3（以下用rHeight表示）。

列4 柱子的高度为1（以下用height表示）

那么列4的雨水高度为 列3和列7的高度最小值减列4高度，即： min(lHeight, rHeight) - height。

列4的雨水高度求出来了，宽度为1，相乘就是列4的雨水体积了。

此时求出了列4的雨水体积。

**所以实际上就是由min(leftHeight, rightHeight) - midHeight**求得的

---

现在的问题就是 ，*我们该如何求得某一列的左边第一个比它高的列，右边第一个比它高的列？*

一个想法是，首先用两个数组记录每个数组的左右第一个高的列，然后实际用的时候查询就行。

```c++
 maxLeft[0] = height[0];
        for (int i = 1; i < size; i++) {
            maxLeft[i] = max(height[i], maxLeft[i - 1]);
        }
        // 记录每个柱子右边柱子最大高度
        maxRight[size - 1] = height[size - 1];
        for (int i = size - 2; i >= 0; i--) {
            maxRight[i] = max(height[i], maxRight[i + 1]);
        }
```



另一个方法就是利用单调栈。我们之前已经说过，*单调栈对于要求求某个方向的比它大（或小）的数很有用处*。这里我们可以尝试用单调递增的单调栈。

![42.接雨水4](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/2021022309321229.png)

### 代码

```c++
int trap(vector<int>& height) {
        stack<int> st;
        int sum = 0;
        st.push(0);
        for (int i = 1; i < height.size(); ++i) {
            while(!st.empty() && height[i] > height[st.top()]) {
                int t = st.top();
                st.pop();
                if (!st.empty()) {
                    int h = min(height[st.top()], height[i]) - height[t];
                    int w = i - st.top() - 1;
                    sum += h * w; 
                }
            }
            st.push(i);
        }
        return sum;
    }
```

## 柱状图最大的矩形

{{< blockquote link="https://leetcode.cn/problems/largest-rectangle-in-histogram/" title="每日温度" author="力扣84" >}}

给定 n 个非负整数，用来表示柱状图中各个柱子的高度。每个柱子彼此相邻，且宽度为 1 。

求在该柱状图中，能够勾勒出来的矩形的最大面积。

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210803220437.png)

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210803220506.png)

- 1 <= heights.length <=10^5
- 0 <= heights[i] <= 10^4

{{< /blockquote >}}

---

这个题可以说是 **接雨水问题**的反向。接雨水问题主要是要找到 *左边以及右边第一个比本列高的位置，这样能构成凹槽*；而本题是要找到 *左边以及右边第一个比本列低的位置*。

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/8C1F6FC6C89E5209B67CAE277397EF3D.png" alt="8C1F6FC6C89E5209B67CAE277397EF3D" style="zoom:80%;" />

### 代码

```c++
int largestRectangleArea(vector<int>& heights) {
        stack<int> st;
        int res = 0;
        heights.insert(heights.begin(), 0);
        heights.push_back(0);
        st.push(0);
        for (int i = 0; i < heights.size(); ++i) {
            while (!st.empty() && heights[i] < heights[st.top()]) {
                int mid = st.top();
                st.pop();
                int h = heights[mid];
                int w = i - st.top() - 1;
                res = max(res, h * w);
            }
            st.push(i);
        }
        return res;
    }
```

