---
title: "Day13_栈与队列 删除相邻重复and逆波兰and滑动窗口最大值"
date: 2024-07-01T18:56:54+08:00
categories: [算法]
---

## 删除字符串中的所有相邻重复项

{{< blockquote author="力扣1047" link="https://leetcode.cn/problems/remove-all-adjacent-duplicates-in-string/" title="删除字符串中的所有相邻重复项" >}}

给出由小写字母组成的字符串 `S`，**重复项删除操作**会选择两个相邻且相同的字母，并删除它们。

在 S 上反复执行重复项删除操作，直到无法继续删除。

在完成所有重复项删除操作后返回最终的字符串。答案保证唯一。

 

**示例：**

```
输入："abbaca"
输出："ca"
解释：
例如，在 "abbaca" 中，我们可以删除 "bb" 由于两字母相邻且相同，这是此时唯一可以执行删除操作的重复项。之后我们得到字符串 "aaca"，其中又只有 "aa" 可以执行重复项删除操作，所以最后的字符串为 "ca"。
```

 

**提示：**

1. `1 <= S.length <= 20000`
2. `S` 仅由小写英文字母组成。

{{< /blockquote >}}

---

模仿前面的`括号匹配`,这个也可以当作一个匹配问题，匹配的对象是相邻的元素，我们也可以用栈来解决。

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/1047.%25E5%2588%25A0%25E9%2599%25A4%25E5%25AD%2597%25E7%25AC%25A6%25E4%25B8%25B2%25E4%25B8%25AD%25E7%259A%2584%25E6%2589%2580%25E6%259C%2589%25E7%259B%25B8%25E9%2582%25BB%25E9%2587%258D%25E5%25A4%258D%25E9%25A1%25B9.gif" style="zoom:100%"/>

我们从头开始遍历字符串，将每个字符放入栈中，入栈前还要检查该元素与栈顶元素是否匹配(在这道题中，就是是否相等)，如果相等，那么就pop()出栈顶，并且当前字符也不入栈，继续遍历字符串，重复操作。

### 代码

```c++
string removeDuplicates(string S) {
        stack<char> st;
        for (char s : S) {
            if (st.empty() || s != st.top()) {
                st.push(s);
            } else {
                st.pop(); // s 与 st.top()相等的情况
            }
        }
        string result = "";
        while (!st.empty()) { // 将栈中元素放到result字符串汇总
            result += st.top();
            st.pop();
        }
        reverse (result.begin(), result.end()); // 此时字符串需要反转一下
        return result;

    }
```

- 时间复杂度: O(n)
- 空间复杂度: O(n)

也可以把字符串直接当作栈来操作。

# 逆波兰表达式求值

{{< blockquote author="力扣150" link="https://leetcode.cn/problems/evaluate-reverse-polish-notation/" title="逆波兰表达式求值" >}}

给你一个字符串数组 `tokens` ，表示一个根据 [逆波兰表示法](https://baike.baidu.com/item/逆波兰式/128437) 表示的算术表达式。

> 一个[表达式](https://baike.baidu.com/item/表达式/7655228?fromModule=lemma_inlink)E的后缀形式可以如下定义：
>
> （1）如果E是一个变量或常量，则E的[后缀式](https://baike.baidu.com/item/后缀式/3198638?fromModule=lemma_inlink)是E本身。
>
> （2）如果E是E1 op E2形式的表达式，这里op是任何二元[操作符](https://baike.baidu.com/item/操作符/8978896?fromModule=lemma_inlink)，则E的后缀式为E1'E2' op，这里E1'和E2'分别为E1和E2的后缀式。
>
> （3）如果E是（E1）形式的表达式，则E1的后缀式就是E的后缀式。
>
> 如：我们平时写a+b，这是[中缀表达式](https://baike.baidu.com/item/中缀表达式/2725244?fromModule=lemma_inlink)，写成后缀表达式就是：ab+
>
> (a+b)*c-(a+b)/e的后缀表达式为：
>
> (a+b)*c-(a+b)/e
>
> →((a+b)*c)((a+b)/e)-
>
> →((a+b)c*)((a+b)e/)-
>
> →(ab+c*)(ab+e/)-
>
> →ab+c*ab+e/-

请你计算该表达式。返回一个表示表达式值的整数。

**注意：**

- 有效的算符为 `'+'`、`'-'`、`'*'` 和 `'/'` 。
- 每个操作数（运算对象）都可以是一个整数或者另一个表达式。
- 两个整数之间的除法总是 **向零截断** 。
- 表达式中不含除零运算。
- 输入是一个根据逆波兰表示法表示的算术表达式。
- 答案及所有中间计算结果可以用 **32 位** 整数表示。

 

**示例 1：**

```
输入：tokens = ["2","1","+","3","*"]
输出：9
解释：该算式转化为常见的中缀算术表达式为：((2 + 1) * 3) = 9
```

**示例 2：**

```
输入：tokens = ["4","13","5","/","+"]
输出：6
解释：该算式转化为常见的中缀算术表达式为：(4 + (13 / 5)) = 6
```

**示例 3：**

```
输入：tokens = ["10","6","9","3","+","-11","*","/","*","17","+","5","+"]
输出：22
解释：该算式转化为常见的中缀算术表达式为：
  ((10 * (6 / ((9 + 3) * -11))) + 17) + 5
= ((10 * (6 / (12 * -11))) + 17) + 5
= ((10 * (6 / -132)) + 17) + 5
= ((10 * 0) + 17) + 5
= (0 + 17) + 5
= 17 + 5
= 22
```

 

**提示：**

- `1 <= tokens.length <= 10^4`
- `tokens[i]` 是一个算符（`"+"`、`"-"`、`"*"` 或 `"/"`），或是在范围 `[-200, 200]` 内的一个整数

 

**逆波兰表达式：**

逆波兰表达式是一种后缀表达式，所谓后缀就是指算符写在后面。

- 平常使用的算式则是一种中缀表达式，如 `( 1 + 2 ) * ( 3 + 4 )` 。
- 该算式的逆波兰表达式写法为 `( ( 1 2 + ) ( 3 4 + ) * )` 。

逆波兰表达式主要有以下两个优点：

- 去掉括号后表达式无歧义，上式即便写成 `1 2 + 3 4 + * `也可以依据次序计算出正确结果。
- 适合用栈操作运算：遇到数字则入栈；遇到算符则取出栈顶两个数字进行计算，并将结果压入栈中

{{< /blockquote >}}

---

只要理解了逆波兰的思想，就是操作数推入栈，遇到op就pop两个操作数出来，然后将结果再压回栈中。

### 代码

```c++
int evalRPN(vector<string>& tokens) {
        stack<int> st;
        for(auto &e: tokens)
        {
            if (e==("+") || e==("-") || e==("*") || e==("/")) {
                int nums1, nums2;
                nums1 = st.top();
                st.pop();
                nums2 = st.top();
                st.pop();
                if (e==("+")) st.push(nums1 + nums2);
                else if(e==("-")) st.push(nums2 - nums1);
                else if(e==("*")) st.push(nums2 * nums1);
                else {
                    st.push(nums2 / nums1);
                }
            } else {
                st.push(stoi(e));
            }
        }
        return st.top();
    }
```

- 时间复杂度: O(n)
- 空间复杂度: O(n)

## 滑动窗口最大值

{{< blockquote author="力扣239" link="https://leetcode.cn/problems/sliding-window-maximum/" title="滑动窗口最大值" >}}

1. 给你一个整数数组 `nums`，有一个大小为 `k` 的滑动窗口从数组的最左侧移动到数组的最右侧。你只可以看到在滑动窗口内的 `k` 个数字。滑动窗口每次只向右移动一位。

   返回 *滑动窗口中的最大值* 。

    

   **示例 1：**

   ```
   输入：nums = [1,3,-1,-3,5,3,6,7], k = 3
   输出：[3,3,5,5,6,7]
   解释：
   滑动窗口的位置                最大值
   ---------------               -----
   [1  3  -1] -3  5  3  6  7       3
    1 [3  -1  -3] 5  3  6  7       3
    1  3 [-1  -3  5] 3  6  7       5
    1  3  -1 [-3  5  3] 6  7       5
    1  3  -1  -3 [5  3  6] 7       6
    1  3  -1  -3  5 [3  6  7]      7
   ```

   **示例 2：**

   ```
   输入：nums = [1], k = 1
   输出：[1]
   ```

    

   **提示：**

   - `1 <= nums.length <= 10^5`
   - `-10^4 <= nums[i] <= 10^4`
   - `1 <= k <= nums.length`

{{< /blockquote >}}

---

这个题目的暴力做法就是对于每个滑动窗口的位置都找最大值，时间复杂度是$o(n*k)$。

但是这个问题常见的解法是 **单调队列**。也就是维护一个队列，它里面元素按从大到小组织。我们不必在队列排序并保留 **该位置的滑动窗口的所有元素**，我们只需要按下面的原则维护： 

1. pop(value)：如果窗口移除的元素value等于单调队列的出口元素，那么队列弹出元素，否则不用任何操作
2. push(value)：如果push的元素value大于入口元素的数值，那么就将队列入口的元素弹出，直到push元素的数值小于等于队列入口元素的数值为止

>  感觉解释得不好，TODO

### 代码

```c++
vector<int> maxSlidingWindow(vector<int>& nums, int k) {
        int n = nums.size();
        deque<int> q;
        for (int i = 0; i < k; ++i) {
            while (!q.empty() && nums[i] >= nums[q.back()]) {
                q.pop_back();
            }
            q.push_back(i);
        }

        vector<int> ans = {nums[q.front()]};
        for (int i = k; i < n; ++i) {
            while (!q.empty() && nums[i] >= nums[q.back()]) {
                q.pop_back();
            }
            q.push_back(i);
            while (q.front() <= i - k) {
                q.pop_front();
            }
            ans.push_back(nums[q.front()]);
        }
        return ans;
    }

作者：力扣官方题解
链接：https://leetcode.cn/problems/sliding-window-maximum/solutions/543426/hua-dong-chuang-kou-zui-da-zhi-by-leetco-ki6m/

```

- 时间复杂度: O(n)
- 空间复杂度: O(k)
