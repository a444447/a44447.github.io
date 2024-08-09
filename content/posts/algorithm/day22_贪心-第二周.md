---
title: "Day22_贪心- 第二周"
date: 2024-08-08T10:15:04+08:00
Categories: [算法]
---
## 加油站

{{< blockquote link="https://leetcode.cn/problems/gas-station/" author="力扣134" title="加油站" >}}

在一条环路上有 `n` 个加油站，其中第 `i` 个加油站有汽油 `gas[i]` 升。

你有一辆油箱容量无限的的汽车，从第 `i` 个加油站开往第 `i+1` 个加油站需要消耗汽油 `cost[i]` 升。你从其中的一个加油站出发，开始时油箱为空。

给定两个整数数组 `gas` 和 `cost` ，如果你可以按顺序绕环路行驶一周，则返回出发时加油站的编号，否则返回 `-1` 。如果存在解，则 **保证** 它是 **唯一** 的。

 

**示例 1:**

```
输入: gas = [1,2,3,4,5], cost = [3,4,5,1,2]
输出: 3
解释:
从 3 号加油站(索引为 3 处)出发，可获得 4 升汽油。此时油箱有 = 0 + 4 = 4 升汽油
开往 4 号加油站，此时油箱有 4 - 1 + 5 = 8 升汽油
开往 0 号加油站，此时油箱有 8 - 2 + 1 = 7 升汽油
开往 1 号加油站，此时油箱有 7 - 3 + 2 = 6 升汽油
开往 2 号加油站，此时油箱有 6 - 4 + 3 = 5 升汽油
开往 3 号加油站，你需要消耗 5 升汽油，正好足够你返回到 3 号加油站。
因此，3 可为起始索引。
```

**示例 2:**

```
输入: gas = [2,3,4], cost = [3,4,3]
输出: -1
解释:
你不能从 0 号或 1 号加油站出发，因为没有足够的汽油可以让你行驶到下一个加油站。
我们从 2 号加油站出发，可以获得 4 升汽油。 此时油箱有 = 0 + 4 = 4 升汽油
开往 0 号加油站，此时油箱有 4 - 3 + 2 = 3 升汽油
开往 1 号加油站，此时油箱有 3 - 3 + 3 = 3 升汽油
你无法返回 2 号加油站，因为返程需要消耗 4 升汽油，但是你的油箱只有 3 升汽油。
因此，无论怎样，你都不可能绕环路行驶一周。
```

 

**提示:**

- `gas.length == n`
- `cost.length == n`
- `1 <= n <= 10^5`
- `0 <= gas[i], cost[i] <= 10^4`

{{< /blockquote >}}

---

首先来看这个问题的暴力求法，也就是使用两个循环，对于每个位置到检查能否回来。

```c++
for (int i = 0; i < cost.size();++i) {
            int index = (i + 1) % cost.size();
            int rest = gas[i] - cost[i];
            while(rest > 0 && index != i) {
                rest += gas[index] - cost[index];
                index = (index + 1) % cost.size();
            }
            if (rest >= 0 && index == i) return i;

        }
        return -1;
    }
```

反正我是超时了。

---

另一个做法就是使用贪心的方法。现在我们有这样一个认识，如果`sum gas`是大于`sum cost`的，那么一定是能够跑完一圈的，我们要做的就是找到一个起始的加油站，它在中途不会使得`rest = gas - cost`为小于零就行。

假设我们从`x`加油站出发，能够达到最远的加油站是`y`，其中`y!=x`，那么很显然，我们从`[x,y]`中任意一个位置再找加油站是没有意义的。

> x要能成功走到x+1,那么其rest += g[x] - cost[x] >= 0，所以到达x+1的时候它的rest >= 0是一定的。所以无论在[x,y]中哪一个地方选一个新的起始加油站（此时起始的rest =0)，都不会比从x开始更好，因为毕竟此前的时候rest > 0都无法走到y+1。

所以，直接选择`y+1`作为下一个起始加油站。

### 代码

```c++
 int canCompleteCircuit(vector<int>& gas, vector<int>& cost) {
        int rest = 0;
        int index = 0;
        int total = 0;
        for(int i = 0; i < cost.size(); ++i)  
        {
            rest += gas[i] - cost[i];
            total += gas[i] - cost[i];
            if (rest < 0 ) {
                index = i + 1;
                rest = 0;
                continue;
            }
        }
        if (total < 0) return -1;
        return index;
    }
```

- 时间复杂度：O(n)
- 空间复杂度：O(1)

## 根据身高重构队列

{{< blockquote link="https://leetcode.cn/problems/queue-reconstruction-by-height/" author="力扣406" title="根据身高重构队列" >}}

假设有打乱顺序的一群人站成一个队列，数组 `people` 表示队列中一些人的属性（不一定按顺序）。每个 `people[i] = [hi, ki]` 表示第 `i` 个人的身高为 `hi` ，前面 **正好** 有 `ki` 个身高大于或等于 `hi` 的人。

请你重新构造并返回输入数组 `people` 所表示的队列。返回的队列应该格式化为数组 `queue` ，其中 `queue[j] = [hj, kj]` 是队列中第 `j` 个人的属性（`queue[0]` 是排在队列前面的人）。

 

**示例 1：**

```
输入：people = [[7,0],[4,4],[7,1],[5,0],[6,1],[5,2]]
输出：[[5,0],[7,0],[5,2],[6,1],[4,4],[7,1]]
解释：
编号为 0 的人身高为 5 ，没有身高更高或者相同的人排在他前面。
编号为 1 的人身高为 7 ，没有身高更高或者相同的人排在他前面。
编号为 2 的人身高为 5 ，有 2 个身高更高或者相同的人排在他前面，即编号为 0 和 1 的人。
编号为 3 的人身高为 6 ，有 1 个身高更高或者相同的人排在他前面，即编号为 1 的人。
编号为 4 的人身高为 4 ，有 4 个身高更高或者相同的人排在他前面，即编号为 0、1、2、3 的人。
编号为 5 的人身高为 7 ，有 1 个身高更高或者相同的人排在他前面，即编号为 1 的人。
因此 [[5,0],[7,0],[5,2],[6,1],[4,4],[7,1]] 是重新构造后的队列。
```

**示例 2：**

```
输入：people = [[6,0],[5,0],[4,0],[3,2],[2,2],[1,4]]
输出：[[4,0],[5,0],[2,2],[3,2],[1,4],[6,0]]
```

 

**提示：**

- `1 <= people.length <= 2000`
- `0 <= hi <= 10^6`
- `0 <= ki < people.length`
- 题目数据确保队列可以被重建

{{< /blockquote >}}

----

这个问题我们凭直觉觉得就要先排序，毕竟初始的输入的顺序不关键，而最后要求的输出是某个序列。

题目中有两个维度 `身高`与`前面大于该身高的维度`。我们遇到这种问题，需要先确立一个维度排序，然后再在这个排好序的维度上，根据另一个属性再排序。两个一起排序很容易混乱。

如果按`k`维度排序，其实是没有意义的，因为排序后的数组即使你想交换顺序，或者进行插入操作，因为不知道具体的身高信息，你的插入操作也是无效的。

所以，我们应该按照`身高`维度排序，且 **从大到小**，遇到身高相同的，以`k`小的在前，为什么？ 因为高身高在前，可以让我们排序后放心的按照`k`进行插入，插入后的位置一定是满足k的。

```
比如，排序后为[[7,0],[7,1],[5,1]]。我们只需要将[5,1]根据k=1插入到index=1的位置上就满足了。
```

### 代码

```c++
 vector<vector<int>> reconstructQueue(vector<vector<int>>& people) {
        vector<vector<int>> res;
        sort(people.begin(),people.end(), [](vector<int>&a, vector<int>& b) {
            if (a[0] == b[0]) return a[1] < b[1];
            return a[0] > b[0];
        });
        for (auto &e: people) {
            std::cout << e[0] << ',' << e[1]<< std::endl;
        }
        for (int i = 0; i < people.size() ; ++i) {
            auto pos = people[i][1];
            res.insert(res.begin() + pos, people[i]);
        }
        return res;
    }
```

上面这个代码可以看到有一个问题，就是我们用了`vector.insert`，当我们不断向一个空的向量添加的时候，它也会不断扩容、复制，这是很耗时的。而且向量的insert接近$O(n^2)$

所以如果我们将容器实现改为`list`，也就是底层用链表实现的数据结构，其插入要更加高效。

```c++
class Solution {
public:
    // 身高从大到小排（身高相同k小的站前面）
    static bool cmp(const vector<int>& a, const vector<int>& b) {
        if (a[0] == b[0]) return a[1] < b[1];
        return a[0] > b[0];
    }
    vector<vector<int>> reconstructQueue(vector<vector<int>>& people) {
        sort (people.begin(), people.end(), cmp);
        list<vector<int>> que; // list底层是链表实现，插入效率比vector高的多
        for (int i = 0; i < people.size(); i++) {
            int position = people[i][1]; // 插入到下标为position的位置
            std::list<vector<int>>::iterator it = que.begin();
            while (position--) { // 寻找在插入位置
                it++;
            }
            que.insert(it, people[i]);
        }
        return vector<vector<int>>(que.begin(), que.end());
    }
};
```

