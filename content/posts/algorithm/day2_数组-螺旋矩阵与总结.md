---
title: "Day2_数组 螺旋矩阵II与总结"
date: 2024-05-30T19:59:59+08:00
categories: [算法]
---



{{< imgcap src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/DALL%C2%B7E%202024-05-30%2020.47.20%20-%20An%20abstract%2C%20grand%2C%20and%20somber%20historical%20scene.%20The%20composition%20features%20sharp%2C%20angular%20lines%20representing%20an%20impossible%20blade%2C%20juxtaposed%20with%20swirl.webp" title="刀的锋刃难以逾越，因此智者说救赎的道路难寻" >}}



## 螺旋矩阵II

{{< blockquote author="力扣59" link="https://leetcode.cn/problems/spiral-matrix-ii/" title="螺旋矩阵II">}}

给你一个正整数 `n` ，生成一个包含 `1` 到 $n^2$ 所有元素，且元素按顺时针顺序螺旋排列的 `n x n` 正方形矩阵 `matrix` 。

**提示：**

- `1 <= n <= 20`

{{< /blockquote >}}

*示例1*

![img](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/spiraln.jpg)

```
输入：n = 3
输出：[[1,2,3],[8,9,4],[7,6,5]]
```

*示例2*

```
输入：n = 1
输出：[[1]]
```



---

这是另一个类型的数组题目，它属于 **模拟过程**，也就是我们需要明白题目描述的过程是怎么样的。最简单的办法就是拿个纸笔，多写几种情况去理解一下。

比如，本题中，当`n`是偶数与`n`是奇数时，转圈几轮后剩下的情况不一样。`n=3`时，`n / 2`是只转一圈，中间剩余一个单独的元素, `n=5`时，转两圈中间剩余一个单独的元素；而对于`n=2`时，`n / 2`转一圈后完全用完$1到n^2$。对于偶数情况，还需要注意到，每转一次圈，剩下的部分相当于完成一个 **更小的偶数螺旋矩阵`n-2 x n-2`**

另外对于转圈的概念也还是理解。我们将转一圈分解为四个填充模式:

+ 最上行填充左到右(在最右上边拐角前停止)
+ 最右列从上到下(在最右下拐角前停止)
+ 最下行填充右边(在最左下拐角前停止)
+ 最左列从下到上(在最左上拐角前停止)

![image-20240530202612525](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/image-20240530202612525.png)

可见，**每个拐角处将完成转换方向的操作**。

于是经过这样的模拟，整个过程也就清晰了

### 代码

```c++
vector<vector<int>> generateMatrix(int n) {
        int startx = 0, starty = 0;
        int loop = n / 2; 
        int mid = n / 2;
        int offset = 1;
        int val = 1;
        vector<vector<int>> mat(n, vector<int>(n, 0));
        while(loop--)
        {
            int i = startx, j = starty;
            for (j; j < n - offset; ++j)
            {
                mat[i][j] = val++; 
            }
            for (i; i < n - offset; ++i)
            {
                mat[i][j] = val++; 
            }
            for (j; j >= offset; --j)
            {
                mat[i][j] = val++; 
            }
            for (i; i >= offset; --i)
            {
                mat[i][j] = val++; 
            }
            startx++;
            starty++;
            offset++;
        }
        if (n % 2)
        {
            mat[mid][mid] = val;
        }
        return mat;
    }
```

+ 时间复杂度: $O(n^2)$
+ 空间复杂度: $O(1)$

## 数组总结
