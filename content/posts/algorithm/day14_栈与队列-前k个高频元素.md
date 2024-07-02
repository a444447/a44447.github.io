---
title: "Day14_栈与队列 前k个高频元素"
date: 2024-07-02T19:19:24+08:00
categories: [算法]
---

## 前K个高频元素

{{< blockquote author="力扣347" link="https://leetcode.cn/problems/top-k-frequent-elements/" title="前K个高频元素" >}}

给你一个整数数组 `nums` 和一个整数 `k` ，请你返回其中出现频率前 `k` 高的元素。你可以按 **任意顺序** 返回答案。

 

**示例 1:**

```
输入: nums = [1,1,1,2,2,3], k = 2
输出: [1,2]
```

**示例 2:**

```
输入: nums = [1], k = 1
输出: [1]
```

 

**提示：**

- `1 <= nums.length <= 10^5`
- `k` 的取值范围是 `[1, 数组中不相同的元素的个数]`
- 题目数据保证答案唯一，换句话说，数组中前 `k` 个高频元素的集合是唯一的

 

**进阶：**你所设计算法的时间复杂度 **必须** 优于 `O(n log n)` ，其中 `n` 是数组大小

{{< /blockquote >}}

---

这个题只需要整理好思路，我们处理的步骤如下:

+ 首先遍历一次容器，并用map记录下频率
+ 维护一个小顶堆，当堆中的size() > k时，就pop()
+ 然后将堆中的元素输出。

只介绍一下`priority_queue`

> template<
>
>   class T,
>   class Container = [std::vector](http://en.cppreference.com/w/cpp/container/vector)<T>,
>   class Compare = [std::less](http://en.cppreference.com/w/cpp/utility/functional/less)<typename Container::value_type>
>
> \> class priority_queue;
>
> 如果我们要自定义比较函数，可以用lambda表达式如下
>
> ```
> // Using lambda to compare elements.
>     auto cmp = [](int left, int right) { return (left ^ 1) < (right ^ 1); };
>     std::priority_queue<int, std::vector<int>, decltype(cmp)> lambda_priority_queue(cmp);
> ```



### 代码

```c++
    vector<int> topKFrequent(vector<int>& nums, int k) {
        unordered_map<int, int> map;
        for(auto i: nums)
        {
            map[i] += 1;
        }
        auto cmp = [](const pair<int,int>& lhs, const pair<int,int>& rhs) {return lhs.second > rhs.second;};
        priority_queue<pair<int,int>, vector<pair<int,int>>, decltype(cmp)> pri_que;
        for (unordered_map<int,int>::iterator it=map.begin(); it != map.end(); it++)
        {
            pri_que.push(*it);
            if (pri_que.size() > k)
            {
                pri_que.pop();
            }
        }
        vector<int> res;
        while(!pri_que.empty())
        {
            auto tmp = pri_que.top();
            res.push_back(tmp.first);
            pri_que.pop();
        }
        return res;
    }
```

+ 时间复杂度: $O(nlogk)$
+ 空间复杂度: $O(n)$

