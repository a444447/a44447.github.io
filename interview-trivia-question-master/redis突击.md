# 认识Redis

**什么是redis?**

Redis是基于内存的数据库，对数据的读写操作都在内存中完成，所以读写速度非常快，常用于缓存、消息队列、分布式锁场景。

Redis提供了很多数据类型来支持不同的业务场景——String、Hash、ZSet、Bitmaps...对这些数据结构的操作都是原子性的， **因为执行命令由单线程负责，不存在并发竞争的问题。**

> 与memcached区别与共同点:
>
> 1. 两个都是基于内存的数据库，一般都用做缓存
> 2. 都有过期策略
> 3. 性能都很高
>
> 区别:
>
> 1. Redis支持的数据类型更丰富，而Memcached只支持最简单的key-value类型
> 2. Redis支持持久化，把内存中数据保持在磁盘，重启的时候加载使用。而memcached没做持久化处理
> 3. redis原生支持集群模式，而memcached没有原生支持。

**为什么用Redis作用MySQL缓存**

1. Redis高性能
2. Redis高并发

​	Redis的QPS，单机能达到10W，MySQL单机很难破1W。

## Redis常见的数据结构

### String

String底层数据结构是 *SDS(简单动态字符串*:

+ 它不仅保存文本数据，还可以保存二进制数据

### List

List的底层数据结构是双向链表或压缩列表，主要取决于： 如果列表的元素个数小于512,每个元素的大小小于64字节，那么会使用压缩列表作用List类型底层结构。否则会使用双向链表。

**注意Redis3.2后，List底层数据结构都是quicklist**



### Hash

哈希表

### ZSet

跳表实现

## Redis 线程模型

我们常说Redis是单线程，因为它“接受客户端请求-> 解析->进行数据读写操作->发送数据给客户端”这个过程是由一个线程来完成的。所以说它是单线程

但是实际上，Redis程序并不是单线程的，而是会后台启动线程BIO.这些后台线程负责 **关闭文件、AOF刷盘、异步释放redis内存**

之所以为这些操作创建后台内存，是因为它们是耗时的操作，如果把这些任务都放在主线程处理，那么很容易就阻塞了。无法处理后续请求。

![img](https://cdn.xiaolincoding.com/gh/xiaolincoder/redis/%E5%85%AB%E8%82%A1%E6%96%87/%E5%90%8E%E5%8F%B0%E7%BA%BF%E7%A8%8B.jpg)

**Redis的单线程模式**

Redis的单线程模式是基于Reactor模式，具体I/O多路复用机制是epoll(linux)



![img](https://cdn.xiaolincoding.com/gh/xiaolincoder/redis/%E5%85%AB%E8%82%A1%E6%96%87/redis%E5%8D%95%E7%BA%BF%E7%A8%8B%E6%A8%A1%E5%9E%8B.drawio.png)

![image-20250307132634340](D:/GitHome/a44447.github.io/interview-trivia-question-master/img/image-20250307132634340.png)

为什么Redis是单线程还这么快？

1. Redis大部分都是内存操作，并且数据结构高效，所以Redis的瓶颈可能可能是机器内存或者网络带宽，而并不是CPU，既然不是CPU瓶颈那么自然可以用单线程
2. 单线程模型可以避免多线程之间的竞争，省去了多线程切换带来的时间和性能上的开销，也没有死锁问题。
3. Redis采用I/O多路复用，一个线程可以监听多个连接。

---

在Redis 6.0之前，采用单线程（这是无法利用服务器多核CPU的）是因为上面提到的CPU并不是瓶颈所在，想要利用多核CPU可以开启多个节点或者采用**分片集群。**而且单线程可维护性高。

