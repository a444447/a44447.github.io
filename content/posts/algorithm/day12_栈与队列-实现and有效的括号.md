---
title: "Day12_栈与队列 实现and有效的括号"
date: 2024-06-27T19:07:24+08:00
categories: [算法]
---

## 栈与队列的实现

对于栈与队列，在`c++`中一般是STL中的两种标准数据结构，其内部的实现往往也与版本相关。

### 栈

无论是什么版本，栈都要满足先入先出的原则，并且不会提供index访问的功能，也不会实现迭代器来遍历所有元素。

**栈是以底层容器完成其所有的工作，对外提供统一的接口，底层容器是可插拔的（也就是说我们可以控制使用哪种容器来实现栈的功能）。**

所以在STL中，栈往往不会被给归纳为容器，*而是container adapter*

<img src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/20210104235459376.png" alt="栈与队列理论3" style="zoom:80%;" />

所以栈是以其他的容器做底层实现的。

在 **SGI STL**中，如果没有指定底层实现，默认以deque作底层容器

deque就是一个双向队列，所以我们只需要封住一边就是一个stack了。

### 队列

队列的情况与栈类似，它也是 *container adapter*

### 使用栈实现队列

实际上就是用两个栈实现队列的功能

```c++
stack<int> s1;
    stack<int> s2;
    MyQueue() {

    }
    
    void push(int x) {
        s1.push(x);
    }
    
    int pop() {
        if (s2.empty()) {
            while(!s1.empty()) {
                int item = s1.top();
                s1.pop();
                s2.push(item);
            }
        }
        int res = s2.top();
        s2.pop();
        return res;
    }
    
    int peek() {
        int top = this->pop();
        s2.push(top);
        return top;
    }
    
    bool empty() {
        return s1.empty() && s2.empty();
    }
```

### 使用单向队列实现栈

第一种思路，实际上也是和前面的栈的实现一样，使用两个单项队列即可实现。假设两个队列分别是`q`和`copy_q`，这里面另一个队列的作用完全是备份，也就是说我们将`q`中所有的元素弹出，直到最后一个元素是我们想要的，其余元素暂时放在`copy_q`，然后之前要将他们全部copy回`q`。

所以实际上 **更优化的方法是，将队列的元素弹出直到最后一个元素期间，弹出的那些元素直接再次添加到队列的尾巴就好。**



## 有效的括号

{{< blockquote author="力扣20" link="https://leetcode.cn/problems/valid-parentheses/" title="有效的括号" >}}

给定一个只包括 `'('`，`')'`，`'{'`，`'}'`，`'['`，`']'` 的字符串 `s` ，判断字符串是否有效。

有效字符串需满足：

1. 左括号必须用相同类型的右括号闭合。
2. 左括号必须以正确的顺序闭合。
3. 每个右括号都有一个对应的相同类型的左括号。

 

**示例 1：**

```
输入：s = "()"
输出：true
```

**示例 2：**

```
输入：s = "()[]{}"
输出：true
```

**示例 3：**

```
输入：s = "(]"
输出：false
```

 

**提示：**

- `1 <= s.length <= 10^4`
- `s` 仅由括号 `'()[]{}'` 组成

{{< /blockquote >}}

---

这个就是最经典的 **括号匹配**

在做的时候先思路，有哪几种匹配错误的情况

1. 左括号多余
2. 括号没有多余，但是括号类型无法匹配上
3. 右方向的括号多余

我们使用 **栈**来解决这个问题，从左到右遍历括号串，遇到一个左括号，就将它需要匹配的右括号压入栈中；每当遇到一个右括号，就pop(),比较这个栈中pop出来的括号是否与该右括号相同，相同的话就成功匹配，反之匹配失败，返回false。

当栈为空了，还是有右括号，也就是右括号多余了

当遍历完了字符串，但是还是栈不为空，说明左括号多了。

### 代码

```c++
bool isValid(string s) {
        stack<int> st;
        if (s.size() % 2 != 0) return false;
        for (int i = 0; i < s.size(); ++i)
        {
            if (s[i] == '(') st.push(')');
            else if (s[i] == '{') st.push('}');
            else if (s[i] == '[') st.push(']');

            else if (st.empty() || s[i] != st.top()) return false;
            else st.pop();
        }
        return st.empty();
    }
```

+ 时间复杂度: $O(n)$
+ 空间复杂度: $O(n)$

