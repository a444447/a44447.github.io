项目说明: 这是参考mit6824的lab，使用c++编写的基于Raft共识算法的分布式K-V数据库。其中使用个人实现的RPC框架和跳表SKipListPro完成的RPC功能和K-V存储功能。

![image-20250304223430770](assets/image-20250304223430770.png)

## Raft梳理

> MapReduce、GFS分布式应用程序出现的问题

Raft是一个分布式算法，为了解决分布式系统中一致性的问题。Raft是作为一个库来提供服务的，其上层运行具体的应用，比如kv存储服务器,redis等等。

在Raft算法中，每个节点有三个状态: follower, candidate,leader。之前提到过类似MapReduce这样的分布式应用会出现**脑裂**行为，而Raft中有一个Term的概念。在一个Term中，只会存在一个leader，这个leader负责处理客户端的请求。

> 

但是Raft算法提出了一个叫多数投票的概念，可以有效防止脑裂现象——任何的决策都必须有过半的节点的同意。这也就是说，无论是选举新的leader,还是一个客户端的请求(比如Get or Put)，都必须得到多数节点的同意才能真正执行。

现在加入有一个Client向一个kv服务器发出一个请求，kv服务器不会马上响应这个请求，而是会发给下层运行的raft集群。在Raft中，这些请求会转换为日志(log)的形式保存，当最后这些请求被多数节点同意了，raft就会告诉上层的状态机，已经commit了，然后状态机就可以执行客户端的请求然后返回给客户端。**Raft算法的核心就是保证各个节点上日志是一致的**。

![image-20250304233252277](assets/image-20250304233252277-1741102374230-7.png)

现在继续来讨论Raft算法中 **选举**机制。每一个raft节点，无论是不是leader,都会有一个内置的选举定时器，当在选举定时器规定时间内都没有收到leader的消息(心跳、AppendEntries)，raft节点就会开启选举，将自己的状态由follower->candidate。这时候它会增加自己的Term(因为一个任期内只能有一个leader,老的任期肯定有一个leader了)，然后向其余的节点开启投票，当它收到过半数的同意票后，就会修改自己状态为leader,然后马上发送心跳消息，通知其余节点说一个新的leader产生了。

> 同意投票并不是无脑同意的，这是有规则的，具体来说”**If votedFor is null or candidateId, and candidate’s log is at least as up-to-date as receiver’s log, grant vote**“，也就是说当raft节点还没有投票的时候，还需要注意这个请求投票的节点的log至少要和当前节点的log一样新。
>
> 我们在实际实现的时候，请求投票的消息中会带有candidate的最后一个log的Term和Index,先比较Term谁大，再比较Index谁大。
>
> 这样可以保证成为leader的节点其日志是大部分节点中最完备的

既然已经说到了投票消息与心跳消息，现在来说一下raft中通信消息，对于心跳与日志请求都是用AppendEntries,对于投票是使用的RequestVote

再来说一下Raft中AppendEntries操作。首先是leader，对于raft节点来说，只有leader中维护了matchIndex[]\(raft node已经复制的日志最高项)与nextIndex[]\(raft bide下一个位置日志位置),它们的长度是节点的数量。所以leader发送AppendEntries时，会带有prevlogIndex、prevlogTerm，还有要添加的entries[]。对于心跳消息,entries[]是空的。这个prevlogIndex、prevlogTerm会成为 **故障恢复**的一个关键。比如当接收方发现prevlogIndex(也就是leader认为它发送消息的这个节点的last log index)其实是空的，就会直接返回false（为了快速回退，还会带有一些其他信息，后面讲）。

现在来说一下**日志恢复**。我们假设在某一个时间点，发送了某些故障，然后触发了选举，经过一段时间后，新的leader当选，**日志恢复的目的就是leader需要把自己的日志强行复制到其他节点**。新的leader当选后，它会把nextIndex[]初始化为自己的日志长度，于是发送的AppendEntries中的prevlogIndex=nextIndex[i] - 1.当接受方发现自己在prevlogindex上有日志项且prevlogterm也吻合，那么说明包括这之前所有的日志项都是匹配的，于是进行追加并返回true.

> `Raft`强制将`Leader`的日志条目覆盖到`Follower`上, 这一机制的根本前提是: **`Leader`的日志是最新和完整的**.我们前面说的投票约束就是保证了这点。
>
> 说一点，我们确保Leader的日志是update的依据是prevLogIndex与prevLogTerm，但是为什么不直接参考Term呢？这个Term是请求投票的candidate的Term。考虑这样一个场景，一个节点变成了 **孤立节点**，意味着它无法与raft集群中其余节点交流。它不断的自增Term(因为它无法收到leader消息，于是触发选举，Term++，但是由于永远无法有多数同意，于是一直无限触发选举机制，一直增加Term)。现在突然它回到了整个集群中，这时候它的Term与集群Term差距很大，故如果我们判断日志是否是最新的依据是按照candidate's Term的话，它一定会当选。但是它已经错过了很多的log了，但是因为它是leader它信任自己，它就会把`log`复制到所有的`follower`上, 这将覆盖已经提交了多个`log`, 导致了错误。

**现在介绍快速恢复**。在之前**日志恢复**的介绍中, 如果有`Follower`的日志不匹配, 每次`RPC`中, `Leader`会将其`nextIndex`自减1来重试, 但其在某些情况下会导致效率很低，其情况为:

1. 某一时刻, 发生了网络分区, 旧的`leader`正好在数量较少的那一个分区, 且这个分区无法满足`commit`过半的要求
2. 另一个大的分区节点数量更多, 能满足投票过半和`commit`过半的要求, 因此选出了`Leader`并追加并`commit`了很多新的`log`
3. 于此同时, 旧的`leader`也在向其分区内的节点追加很多新的`log`, 只是其永远也无法`commit`
4. 某一时刻, 网络恢复正常, 旧的`Leader`被转化为`Follower`, 其需要进行新的`Leader`的日志恢复, 由于其`log数组`差异巨大, 因此将`nextIndex`自减1来重试将耗费大量的时间。

快速恢复的思想在于：**`Follower`返回更多信息给`Leader`，使其可以以`Term`为单位来回退**

![image-20250305001701410](assets/image-20250305001701410-1741105022750-9.png)

下面来讨论 **持久化**，在论文中，需要持久化的为`voteFor`、`currentTerm`、`Log`，

1. ```
   votedFor
   ```

   :

   ```
   votedFor
   ```

   记录了一个节点在某个

   ```
   Term
   ```

   内的投票记录, 因此如果不将这个数据持久化, 可能会导致如下情况:

   1. 在一个`Term`内某个节点向某个`Candidate`投票, 随后故障
   2. 故障重启后, 又收到了另一个`RequestVote RPC`, 由于其没有将`votedFor`持久化, 因此其不知道自己已经投过票, 结果是再次投票, 这将导致同一个`Term`可能出现2个`Leader`

2. `currentTerm`:
   `currentTerm`的作用也是实现一个任期内最多只有一个`Leader`, 因为如果一个几点重启后不知道现在的`Term`时多少, 其无法再进行投票时将`currentTerm`递增到正确的值, 也可能导致有多个`Leader`在同一个`Term`中出现

3. `Log`:
   这个很好理解, 需要用`Log`来恢复自身的状态

这里值得思考的是：**为什么只需要持久化`votedFor`, `currentTerm`, `Log`？**

原因是其他的数据， 包括 `commitIndex`、`lastApplied`、`nextIndex`、`matchIndex`都可以通过心跳的发送和回复逐步被重建, `Leader`会根据回复信息判断出哪些`Log`被`commit`了。

**什么时候持久化**？如果每次修改三个需要持久化的数据: `votedFor`, `currentTerm`, `Log`时, 都进行持久化, 其持久化的开销将会很大， 很容易想到的解决方案是进行批量化操作， 例如只在回复一个`RPC`或者发送一个`RPC`时，才进行持久化操作。

再来说说**快照** 。log实际上是描述了上层状态机操作，像KV数据库，随着时间推移，可能log会变得很长，但是KV数据库实际上可能数据不多，因为很多都是赋值取值操作。而且对于一个上层应用来说，保存它的状态代价要比维护一个很长的log要小。

于是Raft要求上层应用在某个时间节点对于自己状态做一个快照，这样它就可以丢掉快照之前的Log。引入快照后，`Raft`启动时需要检查是否有之前创建的快照, 并迫使应用程序应用这个快照。

快照会造成 **日志短缺**问题，来源就是假设有一个follwer它的日志很短，在某一时候，leader已经做了快照决定并且丢弃了前面的log（这个follwer的log index在这之前），于是缺失的log永远补不回来。

Raft引入了`InstallSnapshot RPC`来补全丢失的`Log`, 具体来说过程如下:

1. `Follower`通过`AppendEntries`发现自己的`Log`更短, 强制`Leader`回退自己的`Log`
2. 回退到在某个点时，`Leader`不能再回退，因为它已经到了自己`Log`的起点, 更早的`Log`已经由于快照而被丢弃
3. `Leader`将自己的快照发给`Follower`
4. `Leader`稍后通过`AppendEntries`发送快照后的`Log`

## Raft-cpp实现梳理

### 选举

#### electionTimeoutTicker设计

#### doEelection

#### sendRequestVote

#### Request

### 日志复制与心跳机制

#### 

### kvServer

#### kvServer如何保证线性一致性

线性一致性： 线性一致性是指一个系统表现得就像只有一个服务器。在线性一致性系统中，执行历史是一系列的客户端请求，可以按照一个顺序排列，并且排列顺序与客户端请求的实际时间相符合。**每一个读操作都看到的是最近一次写入的值**。

kvServer就是接受客户端请求，然后协调内部运行的raft与kvDB如何处理这个请求，最看下关键的成员变量:

+ `Raft m_raftNode`：这就是下层的Raft
+ `LockQueue<ApplyMsg> applyChan`： 实现了一个`LockQueue<>`，是一个并发的安全队列，来充当golang里面的channel结构。Raft实现中，也有一个`applyChan`，kvServer与下层的Raft就是通过这个channel进行通信的。
+ `Op`类：它是kvServer传给raft的command，包括了这些信息:

```c++
std::string Operation;  // "Get" "Put" "Append"
std::string Key;
std::string Value;
std::string ClientId;  //客户端号码
int RequestId;         //客户端号码请求的Request的序列号，为了保证线性一致性
```



对于通信部分，我们是使用的自己实现的rpc服务框架，它为客户端提供rpc服务，方法有:

```protobuf
rpc PutAppend(PutAppendArgs) returns(PutAppendReply);
rpc Get (GetArgs) returns (GetReply);
```

再来看下当某个command被raft同意提交，于是需要apply到kvDB中，提供了

+ `ExecuteGetOpOnKVDB`:

```c++
//跳表中的接口
m_skipList.search_element(op.Key, *value)
如果成功找到了标记一个exist=true
//更新kvServer中对应的client的最新请求完成编号
m_lastRequestId[op.ClientId] = op.RequestId;
```

+ `ExecutePutOpOnKVDB`

```c++
//调用接口
 m_skipList.insert_set_element(op.Key, op.Value);
//更新kvServer中对应的client的最新请求完成编号
m_lastRequestId[op.ClientId] = op.RequestId;
```

#### RPC 服务架构

我们使用protobuf定义了RPC通信消息结构，以及rpc服务与方法，然后使用其提供的protoc的工具生成了与服务相关的c++代码。

我们的自己实现的服务框架提供了:

+ `Provider`: 服务发布端只需要继承protobuf生成的c++代码中对应的服务类，然后实现它提供的方法即可。然后就调用Notify()把服务进行预发布。最后调用Run，监听服务请求。我们的`Provider`会自动处理到来的连接，自动解析头部，然后并根据相应的事件（不同的服务方法请求），调用对应的服务提供的方法，并且负责传回给请求方。——因此在这个框架下，服务提供方只需要写自己的业务逻辑就行。
+ `Channel`: protobuf生成的c++代码中，有专门给客户端调用的Stub类，这个Stub类实现了服务的所有方法。它实现的代码是调用_channel->CallMethod()。因此我们的实现了一个protobuf::Channel的继承类，实现了它的CallMethod()方法。我们实现的CallMethod()方法包含了与方法提供端建立连接，并且添加自定义的头部（防止TCP粘包拆包），与参数一起序列化后传给服务提供端，等待返回的数据并反序列化到response结构体中。——因此在这个框架下，用户只需要创建一个Channel对象，然后以该Channel对象构造一个Stub类就行，然后就能直接用Stub调用所有方法。



----

所以KvServer的初始化流程就是：

1. 创建KvServer的编号、与下游raft沟通的channel、Raft节点
2. 开启kvServer的rpc服务，这是为客户端提供Get,PutAppend。注意我们的KVserver就是继承了对应的rpc服务类，并实现了Get、PutAppend方法。
3. 启动一个raft节点的rpc服务，为与其他raft通信提供方法。注意我们的Raft类本身就是继承了对应的rpc服务类
4. 与其他的raft节点建立连接，从配置文件中读取每个raft node的ip port,然后分别创建Channel，然后用这个Channel分别创建对各个raft节点的Stub对象，就能与它们通信了
5. 等到所有连接完成后，再调用raft->init，启动raft
6. 初始化kvDB，需要从persist部分中检查是否有快照，如果有的话要读入快照。
7. kvServer进入训话，不断地等待applyChan中的内容，如果有内容就开始执行应用command到kvDB的逻辑。

#### ApplyChan有内容后的流程

当ApplyChan有内容后，kvServer从阻塞中恢复，如果message中应用command是true，那么就可以进行执行流程：

1. 如果这个commandIndex比最新的快照索引还要小，说明已经在快照中了，于是就放弃这次提交。
2. 否则,判断这个command是否是重复command。如果是就放弃这个提交，并返回消息给client
3. 如果不是，则根据对应的Op执行到状态机中
4. 我们之前设定了`maxRaftLog`，需要检查当前的log是否太大，如果太大需要制作快照。
5. 传回消息给client

#### kvServer提供的rpc方法

以`PutAppend`为例：

1. 把args中的参数放到Op中，这个Op是我们KvServer传给Raft的command
2. 调用m_raft->Start
3. 如果不是Leader,会直接返回。PutAppend也返回消息给客户端，并设置自己不是Leader
4. 向`waitApplyCh`中添加这个raft日志，等待这个raft日志被同意commit。
5. 如果在规定时间内没有等待到，就设置超时错误，但是如果本身是一个重复的请求，就还是返回ok
6. 否则返回成功。

### Clerk

clerk的成员变量如下:

+ `std::vector<std::shared_ptr<raftServerRpcUtil>> m_servers`: 保存所有kvServer的信息
+ `m_clientID`： 唯一标识自己client号
+ `int m_requestId`: 一个递增的请求编号，它与<m_clientID,m_requestId>标识了一个唯一的请求
+ `int m_recentLeaderId`: 记录最近的leader节点编号

其Init主要做了：

1. 从配置参数中读取所有kvServer的ip port
2. 通过我们的rpc服务框架，分别建立Channel(ip,port)对象，再用这些对象分别建立Stub对象

以`PutAppend`为例看下Clerk的请求

1. `++m_requestID`
2. `server = m_recentLeader`，注意，不一定依然是leader
3. 接下来是一个while，先把request,response都创建好，然后调用rpc方法PutAppend。
4. 根据reply中的err来判断是否调用成功
5. 对于ErrorWrongLeader，,通过`(server+1)%m_servers.size()`直到找到leader

6. 如果成功，保存当前的`m_recentLeaderId=server`

