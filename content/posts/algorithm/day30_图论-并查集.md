---
title: "Day30_图论 并查集"
date: 2024-09-11T18:57:40+08:00
Categories: [算法] 
---

并查集的主要使用场景有两点：

+ 把两个元素添加到一个集合里面
+ 判断两个元素是否是一个集合

---

首先为什么我们需要`并查集`来查询元素，为什么不用数组？设想一下，如果你有很多的类别，意味着你要建立若干个数组来存储每个类别。

这时也许会想到如果是一个二维的数组呢？但是当我们想执行 **查询**操作的时候，比如想知道`A`与`B`是否实在一个类别，就需要遍历整个二维数组一次，很不方便。

所以设计了并查集这样一个数据结构，它的思想有点借鉴连通图，也就是说同一个类别的元素它们之间一定是连通的(也就是在一个类中的任何`u`与`v`，它们之间一定是可达的)。

在最开始的时候，每个加入的元素它的根节点都是自己`father[e] = e`，然后当执行`查询`,`合并`操作的时候：

+ 查询: 调用`isSame(a,b)`——执行`find(a)`,`find(b)`函数，返回分别是`root of a`和`root of b`，通过判断它们的根是否一致判断是否是同一类别。
+ 合并: 调用`join(a,b)`——首先会调用`find(a)`与`find(b)`，然后如果它们的root不一致，就会让`root of b`->`root of a` （当然反向也可以）。这样的话a与b就连通了，就是一个类别了。

---

为什么无论是查询还是合并都会调用find? 这不仅仅是为了知道元素的根是谁，也是为了效率考虑。

在`find`中会执行**路径压缩**。如果是没有路径压缩的版本的find:

```c++
elem find(elem a)
{
    if (a == father[a]) return a; //只有根节点的父节点是自己
    return find[father[a]];
}
```

可以看到，这样是可以通过递归最后返回根的，但是问题是每次调用`find`都会重复整个过程，但是实际上我们可以在经历一次这样的递归 **长路径**后，直接让当前元素的father[a]指向根，这样就压缩和缩短了路径。

```c++
elem find(elem a)
{
    if (a == father[a]) return a;
    return father[a] = find(father[a]);
}
```

也许有人会说，我想问问了，如果当前的root节点被join到另一个类中了怎么办？根据代码就可以知道，假如当前的`root`有了父节点了，那么在`if(a == father[a])`就不会直接返回了，而是会沿着路径递归上去，最后我们的`a`节点的路径压缩又会重新更新。



---

总结一下并查集的代码

```c++
elme node_nums = 1000;
vector<elem> father = vector<elem> (n);

void init()
{
    for (int i = 0; i < node_nums; ++i) {
        father[i] = i;
    }
}

elem find(elem u)
{
    if (u == father[u]) return u;
    return father[u] = find(father[u]);
}

void join(elem a, elem b)
{
    auto u = father[a];
    auto v = father[b];
    if (u == v) return ;
    father[v] = u;
}

bool isSame(elem a, elem b)
{
    auto u = find(a);
    auto v = find(b);
    return u == v;
}
```

并查集的效率是O(n)