# 概念

RPC是远程方法调用，现在RPC已经是分布式系统中重要的通信方式。RPC*允许一个程序调用另一个程序中的函数或方法，而无需了解网络的细节*。因此RPC的出现使得分布式系统中的各个组件之间的通信变得更加简单、高效和可信。

RPC 在很多思想与IPC很像。IPC是进程间通信，常用的技术是 **管道、消息队列、共享内存、信号量、socket等**。

**rpc**最开始的目的就是为了让计算机能够像调用本地方法一样调用远程方法！



![img](E:\githome\a44447.github.io\interview-trivia-question-master\assets\6361e5897dc6985f3898539f63b4ff4b.png)

![image-20250303005124710](E:\githome\a44447.github.io\interview-trivia-question-master\assets\image-20250303005124710.png)

这个图就描述了RPC调用的过程。



## why RPC?

1. 简化远程调用的复杂性

在分布式系统中，服务通常部署在不同的机器和网络中，如果没有RPC，开发人员需要自己实现复杂的网络通信代码：

+ 创建和管理连接
+ 数据的序列化与反序列化
+ 请求和响应的发送与接受
+ 错误处理

RPC对这些底层逻辑进行了封装，开发人员只需要专注于业务逻辑

2. RPC支持微服务架构

微服务架构的目标是，1）将功能拆分为独立的服务，降低模块之间的依赖 2） 各个服务可以独立扩展、部署和更新

但是微服务架构中，不同的服务可能是部署在不同的机器或网络环境，需要高效的通信手段，RPC就提供了：

+ 跨机器调用
+ 通过高效的序列化协议，比如Protobuf优化数据传输
+ 提供可靠性保证，内置重试机制、超时机制，确保服务通信的可靠性。

3. 提升开发效率

+ 抽象通信过程:通过使用RPC、开发人员只需定义远程调用的接口(如方法签名)，无需实现复杂的
  通信逻辑.
+ 统一接口定义:大多数rpc框架(如grpc)提供IDL(接口描述语言)，开发人员只需编写一份接
  口定义，框架会自动生成客户端和服务端代码.
+ 减少出错概率:通过框架封装复杂的通信逻辑，可以减少手动编写网络代码时的错误(如序列化失败、超时处理不当等)

### 实现一个rpc的前置知识



我们实现的rpc框架使用的序列化技术是Protobuf。rpc双方通信时发送的都是固定的结构，发送前用Protobuf把消息结构体二进制序列化，在接收方又把这个消息结构体反序列化。

设计的一个简单消息输出格式是这样的

```protobuf
syntax="proto3";
package Krpc;

message RpcHeader {
	bytes service_name=1;
	bytes method_name=2;
	uint32 args_size=3;
}
```

![image-20250303010841052](E:\githome\a44447.github.io\interview-trivia-question-master\assets\image-20250303010841052.png)

因为参数是可变的，所以不能直接封装进RpcHeader,只能封装参数的数量，然后来解析。



### rpc异步与同步调用

#### 同步

同步调用就是在得到结果之前，一直处于阻塞状态，会一直占用一个工作线程。

![RPC client同步调用](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202503031711202.png)

(1)业务代码发起RPC调用，Result=Add(Obj1,Obj2)

(2) 序列化组件，将对象调用序列化成二进制字节流，可理解为一个待发送的包packet1
(3) 通过连接池组件拿到一个可用的连接connection
(4) 通过连接connection将包packet1发送给RPC-server
(5) 发送包在网络传输，发给RPC-server
(6) 响应包在网络传输，发回给RPC-client
(7) 通过连接connection从RPC-server收取响应包packet2
(8) 通过连接池组件，将conneciont放回连接池
(9) 序列化组件，将packet2反序列化为Result对象返回给调用方
(10) 业务代码获取Result结果，工作线程继续往下走

#### 异步

所谓异步回调，在得到结果之前，不会处于阻塞状态，理论上任何时间都没有任何线程处于阻塞状态，因此异步回调的模型，理论上只需要很少的工作线程与服务连接就能够达到很高的吞吐量。

与同步的RPC设计相比，异步的rpc多了上下文管理器，超时管理器，下游收发队列，下游收发线程结构

(1) 业务代码发起异步RPC调用，Add(Obj1, Obj2, callback)；
(2) 上下文管理器，将请求、回调、上下文存储起来；
(3) 序列化组件，将对象调用序列化为二进制字节流，可以理解为一个待发送的包Packet1；
(4) 下游收发队列，将报文放入"待发送队列"，此时调用返回，不会阻塞工作线程；
(5) 下游收发线程，将报文从待发送队列中取出，通过连接池组件获取一个可用的连接connection；
(6) 通过连接connection将报Packet1发送给RPC server；
(7) 发送包在网络传输，发给RPC server；
(8) 响应包在网络传输，发回给RPC client；
(9) 通过连接connection从RPC server收取响应包Packet2；
(10) 下游收发线程，将报文放入已接收队列，通过连接池组件，将connection放回连接池；
(11) 下游收发队列里，报文取出，此时回调将要开始，不会阻塞工作线程；
(12) 序列化组件，将Packet2反序列化为Result对象；
(13) 上下文管理器，将结果回调，上下文取出；
(14) 通过callback回调业务代码，返回Result结果，工作线程继续往下走；
**如果请求长时间不返回，处理流程为：**
(15) 上下文管理器，请求长时间没有返回；
(16) 超时管理器拿到超时的上下文；
(17) 通过timeout_cb回调业务代码，工作线程继续执行；

---

这个上下文管理器很重要，因为相应包的回调是异步的，甚至不在一个工作线程中，因此要有一个组建来记录请求的上下文，把请求-响应-回调等信息匹配起来

**如何匹配？**

假设通过一条连接向下游服务发送了a,b,c三个请求包，异步的收到了x,y,z三个相应包：

(1) 如何知道哪个请求包对应哪个响应包？
(2) 如何知道哪个响应包与哪个回调函数对应？

方法是有一个请求ID

![异步请求关联](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202503031718472.png)

整个处理流程如上，通过请求id，上下文管理器来对应请求-响应-callback之间的映射关系：
(1) 生成请求id；
(2) 生成请求上下文context，上下文中包含发送时间time，回调函数callback等信息；
(3) 上下文管理器记录req-id与上下文context的映射关系；
(4) 将req-id打在请求包里发给RPC-server；
(5) RPC-server将req-id打在响应包里返回；
(6) 由响应包中的req-id，通过上下文管理器找到原来的上下文context；
(7) 从上下文context中拿到回调函数callback；
(8) callback将Result带回，推动业务的进一步执行；

---

**如何实现超时发送和接受？超时管理器**

![超时管理器](https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/202503031719757.png)

超时管理器，用于实现请求回包超时回调处理。
每一个请求发送给下游RPC server，会在上下文管理器中保存req id与上下文的信息，上下文中保存了请求很多相关信息，例如req id，回包回调，超时回调，发送时间等。**超时管理器启动timer**对上下文管理器中的context进行扫描，看上下文中请求发送时间是否过长，如果过长，就不再等待回包，直接超时回调，推动业务流程继续往下走，并将上下文删除。如果超时回调执行后，正常的回包又到达，通过req id在上下文管理器里找不到上下文，就直接将请求丢弃（因为已经超时处理过了）。

### protobuf学习

protobuf为我们提供了序列化和反序列化的服务，我们只需要按照protobuf的语法定义消息，它序列化出来的数据是二进制格式，所以网络传输的效率很高。

**定义消息类型**

```protobuf
syntax = "proto3";

message SearchRequest {
  string query = 1;
  int32 page_number = 2;
  int32 results_per_page = 3;
}
```

定义的消息中，每个字段都需要指定一个介于 `1` 和 `536,870,911` 之间的数字：

- 给定的数字在该消息的所有字段中 **必须唯一**。
- 数字 `19,000` 到 `19,999` 保留给 Protocol Buffers 实现。如果您在消息中使用这些保留的字段编号之一，协议缓冲区编译器将发出警告。
- 不能使用任何先前 [保留](https://protobuf.com.cn/programming-guides/proto3/#fieldreserved) 的字段编号或已分配给 [扩展](https://protobuf.com.cn/programming-guides/proto2#extensions) 的任何字段编号。



**定义服务**

如果我们想让我们的消息类型和RPC系统一起使用，可以在`.proto`文件中定义一个 RPC 服务接口，协议缓冲区编译器将在您选择的语言中生成服务接口代码和存根。

比如我们之前已经定义了`SearchRequest`、 `SearchResponse` 消息，现在想定义一个rpc服务，它是一个接受`SearchRequest`参数返回`SearchResponse`的方法：

```protobuf
service SearchService {
  rpc Search(SearchRequest) returns (SearchResponse);
}
```

### ZooKeeper

ZooKeeper是一个分布式服务框架，为分布式应用提供一致性协调服务的中间件。在这个项目中，callee将【对外提供的服务对象及其方法】以及【网络地址信息】注册在ZooKeeper服务上，caller则通过访问ZooKeeper在整个分布式环境中获取自己想要调用的远端服务对象方法【在哪一台设备上（网络地址信息）】，并向该设备直接发送服务方法调用请求。




## rpc框架实现流程图说明

![image-20250303193909970](E:\githome\a44447.github.io\interview-trivia-question-master\assets\image-20250303193909970.png)

我们以这样的业务场景来分析: Caller调用远端方法Login和Register。Callee中的Login函数接收一个LoginRequest消息体，执行完Login逻辑后将处理结果填写进LoginResponse消息体，再返回给Caller。调用Register函数过程同理。

可以看到，Callee对外提供了远端可调用方法`Login`、`Register`，我们将其在`user.proto`中进行注册。同时我们定义`message LoginRequest`、`message LoginResponse`...

```protobuf
service UserServiceRpc
{
	rpc Login(LoginRequest) returns(LoginResponse);
	rpc Register(RegisterRequest) returns(RegisterResponse);
}
```

然后我们就可以用protoc来编译这个.proto文件

```
protoc user.proto -I ./ -cpp_out=./user
```

最后对于c++会生成`user.cc`与`user.h`

这里面有两个重要的类`UserServiceRpc_Stub`类给caller使用，`UserServiceRpc`给callee使用。caller可以调用`UserServiceRpc_Stub::Login(...)`发起远端调用，而callee则继承`UserServiceRpc`类并重写`UserServiceRpc::Login(...)`函数，实现Login函数的处理逻辑。

---

于是让我们看看业务层逻辑，现在我们假设在caller向callee发起调用Login函数

```c++
MprpcApplication::Init(argc, argv);
//MprpcApplication类提供了解析argc和argv参数的方法，我们在终端执行这个程序的时候，需要通过-i参数给程序提供一个配置文件，这个配置文件里面包含了一些通信地址信息（后面提到）

fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
//这一步操作后面会讲，这里就当是实例化UserServiceRpc_Stub对象吧。UserServiceRpc_Stub是由user.proto生成的类，我们之前在user.proto中注册了Login方法，

fixbug::LoginRequest request;
request.set_name("zhang san");
request.set_pwd("123456");
//回想起我们的user.proto中注册的服务方法：
// rpc Login(LoginRequest) returns(LoginResponse);
// callee的Login函数需要参数LoginRequest数据结构数据

fixbug::LoginResponse response;
// callee的Login函数返回LoginResponse数据结构数据

stub.Login(nullptr, &request, &response, nullptr); 
//caller发起远端调用，将Login的参数request发过去，callee返回的结果放在response中。

if (0 == response.result().errcode()) 
    std::cout << "rpc login response success:" << response.sucess() << std::endl;
else
    std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
//打印response中的内容，别忘了这个result和success之前在user.proto注册过
return 0;

```

现在在callee端，我们假设已经自定义的`UserService`类已经继承了`UserServiceRpc`并且重写了`Login`函数的逻辑。

在callee端，还需要做的

```c++\
MprpcApplication::Init(argc, argv);
//想要用rpc框架就要先初始化

RpcProvider provider;
// provider是一个rpc对象。它的作用是将UserService对象发布到rpc节点上，暂时不理解没关系！！

provider.NotifyService(new UserService());
// 将UserService服务及其中的方法Login发布出去，供远端调用。
// 注意我们的UserService是继承自UserServiceRpc的。远端想要请求UserServiceRpc服务其实请求的就是UserService服务。而UserServiceRpc只是一个虚类而已。

provider.Run();
// 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
return 0;
```

可以看到，我们现在的流程已经到了需要把callee提供的远程调用方法注册到rpcServer的步骤了。RpcServer负责的就是将本地服务方法注册到ZooKeeper上，并接受来自caller的远端服务方法调用，并返回结果给caller。

---

我们需要引入一个`RpcProvider`，这是一个网络对象类，负责的是发布rpc服务方法。这个类对外界提供`NotifyService`和`Run`两个成员方法。`NotifyService`函数可以将`UserService`服务对象及其提供的方法进行**预备发布**。发布完服务对象后再调用`Run()`就**将预备发布的服务对象及方法注册到ZooKeeper上并开启了对远端调用的网络监听**（caller通过tcp向callee请求服务，callee当然要监听).

----

**客户端**

在我们的业务中,protobuf生成的`UserServiceRpc_Stub`类是给caller端用的，而且我们在user.proto上注册的rpc方法已经在`UserServiceRpc_Stub`类完全实现。

查看代码

```protobuf
void UserServiceRpc_Stub::Login(::google::protobuf::RpcController* controller,
                              const ::fixbug::LoginRequest* request,
                              ::fixbug::LoginResponse* response, ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(0), controller,
                       request, response, done);
}
```

看到调用了channel_->CallMethod

这个`channel_`是一个`RpcChannel`类，里面提供了一个虚函数CallMethod

```c++
class PROTOBUF_EXPORT RpcChannel {
 public:
  inline RpcChannel() {}
  virtual ~RpcChannel();

  // Call the given method of the remote service.  The signature of this
  // procedure looks the same as Service::CallMethod(), but the requirements
  // are less strict in one important way:  the request and response objects
  // need not be of any specific class as long as their descriptors are
  // method->input_type() and method->output_type().
  virtual void CallMethod(const MethodDescriptor* method,
                          RpcController* controller, const Message* request,
                          Message* response, Closure* done) = 0;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(RpcChannel);
};
```

所以用户就需要自己实现一个继承于`RpcChannel`的派生类，实现`CallMethod`的定义

---

我们再查看`UserServiceRpc`类，这是提供给服务端的类，父类`Service`提供了虚函数`CallMethod`。`UserServiceRpc`也实现了`CallMethod`,

```c++
void UserServiceRpc::CallMethod(
    const ::google::protobuf::MethodDescriptor* method,
    ::google::protobuf::RpcController* controller,
    const ::google::protobuf::Message* request,
    ::google::protobuf::Message* response, ::google::protobuf::Closure* done) {
  ABSL_DCHECK_EQ(method->service(), file_level_service_descriptors_user_2eproto[0]);
  switch (method->index()) {
    case 0:
      Login(controller,
             ::google::protobuf::internal::DownCast<const ::fixbug::LoginRequest*>(request),
             ::google::protobuf::internal::DownCast<::fixbug::LoginResponse*>(response), done);
      break;
    case 1:
      Register(controller,
             ::google::protobuf::internal::DownCast<const ::fixbug::RegisterRequest*>(request),
             ::google::protobuf::internal::DownCast<::fixbug::RegisterResponse*>(response), done);
      break;

    default:
      ABSL_LOG(FATAL) << "Bad method index; this should never happen.";
      break;
  }
}
```

可以看到，他是根据传递进来的方法描述符`method`来选择调用注册在user.proto上的哪一个函数。

在我们自己的服务端派生类`UserService`继承了`UserServiceRpc`并且重写了Login函数的实现。所以当我们调用`service->CallMethod()`的时候，调用的其实是`UserService`中的Login函数。**多漂亮的多态思想啊。**



## 实现

根据我们上面的梳理，我们先实现`Mrpcapplication`类，这是一个单例类，实现rpc框架的初始化操作，包括了读取config等。

通过单例模式实现的。

---



## rpc框架总结

我自己实现的rpc服务框架依赖了protobuf来将进行消息的序列化和反序列化。在`.proto`中定义了消息结构以及某个rpc服务以及其包含的方法后，可以生成c++类。

rpc服务框架主要有两个方法:

+ NotifyService：负责把RPC方法发布出去，就是把本节点提供的服务与方法都注册进`m_serviceMap`
+ Run：启动rpc服务节点，开启rpc远程调用服务

设计了一个`m_serviceMap`，其`key`是某个服务对象，`val`就是这个对象中有哪些服务方法

### NotifyService

protobuf为我们编写的rpc服务生成的c++类，它的基类是`google::protobuf::Service`, 所以以它作为函数的参数，接受一个服务。

我们注意protobuf生成的c++类中，使用`google::protobuf::ServiceDescriptor`类来描述一个服务，可以通过`Service->GetDescriptor()`来得到。

当获得这个服务描述后就可以

```c++
string::name = describe->name();
//查询这个服务有多少个方法
int cnt  = describe->method_count();

```

和服务一样，protobuf也提供了对于方法进行描述的类`google::protobuf::MethodDescriptor` （自己定义的什么Login远程调用方法都是用它来描述）

```c++
//通过服务，来获得方法的描述
const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
//获得方法的名称
std::string method_name = pmethodDesc->name();
```

然后就是把服务方法都保存

```c++
service_info.service = service;
service_map.emplace(service_name, service_info);
```



### Run

调用Run，就是把`service_map`中的服务以及它们的方法都注册到zk上。我们的项目中zk的作用就是让提供rpc服务的节点将【对外提供的服务对象及其方法】以及【网络地址信息】注册到上面。caller则通过访问ZooKeeper在整个分布式环境中获取自己想要调用的远端服务对象方法【在哪一台设备上（网络地址信息）】，并向该设备直接发送服务方法调用请求。

于是首先，我们封装了Zookeeper的一些api，创建了一个名为`ZKClient`的类，提供了`Start`（zkclient连接zkserver),`create`在zk中创建一个节点,以及`GetData`。

使用的是`zookeeper_mt`多线程库，使用了三个线程:

+ 主线程： 负责用户调用api
+ IO线程： 负责网络通信
+ completion线程：对于异步请求（Zookeeper中提供的异步API，一般都是以zoo_a开头的api）以及**watcher的响应回调**，io线程会发送给completion线程完成处理。关于watcher机制后面会介绍。

设计的是： 把服务对象名以永久节点的形式在ZK中保存，所以当rpcServer与ZK断开连接后，还是会保存服务对象名(比如叫UserServiceRpc)。服务提供的方法对象名是临时节点，然后节点的数据就是提供该服务方法的RpcServer的地址(ip+port).

**Start**

Start方法负责的是连接上ZK。

```c++
// 使用zookeeper_init初始化一个zk对象，异步建立rpcserver和zkclient之间的连接
//connstr:"ip:port"
//global_watcher：当ZK返回给client一个事件通知时，调用这个watch回调。
//通过这个保证Start执行完前已经建立了连接。
m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 6000, nullptr, nullptr, 0);
```

**watcher机制就是ZooKeeper客户端对某个znode建立一个watcher事件，当该znode发生变化时，这些ZK客户端会收到ZK服务端的通知，然后ZK客户端根据znode的变化来做出业务上的改变。**

从下面的代码中要先知道一个事情，我们在ZkClient::Start()函数中调用了zookeeper_init(...)函数，并且把全局函数global_watcher(...)传了进去。zookeeper_init(...)函数的功能是【异步】建立rpcserver和zookeeper连接，并返回一个句柄赋给m_zhandle（客户端通过该句柄和服务端交互）。如何理解异步建立，就是说当程序在ZkClient::Start()函数中获得了zookeeper_init(..)函数返回的句柄后，连接还不一定已经建立好。因为发起连接建立的函数和负责建立连接的任务不在同一个线程里完成。（之前说过ZooKeeper有三个线程）
 所以调用完zookeeper_init函数之后，下面还定义了一个同步信号量sem，并且调用sem_wait(&sem)阻塞当前主线程，等ZooKeeper服务端收到来自客户端callee的连接请求后，服务端为节点创建会话（此时这个节点状态发生改变），服务端会返回给客户端callee一个事件通知，然后触发watcher回调（执行global_watcher函数）

**GetData**

核心就是把服务名与方法名组织成路径通过ZK提供的api与ZK服务器交互。

---

在实现了ZKClient后，我们可以把服务对象与方法发布到ZK上了，然后就利用muduo库提供的网络模块开启rpcServer的(ip, port)监听。我们已经获得了`ip,port`

```c++
//创建address对象，这是对套接字的封装
muduo::net::InetAddress address(ip, port);
//// 创建TcpServer对象
std::shared_ptr<muduo::net::TcpServer> server = std::make_shared<muduo::net::TcpServer>(&event_loop, address, "KrpcProvider");

```

muduo是Reactor模型，所以是根据到达的事件来调用响应的回调函数

```c++
server->setConnectionCallback(std::bind(&KrpcProvider::OnConnection, this, std::placeholders::_1));
server->setMessageCallback(std::bind(&KrpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
```

可以看到分别为连接事件和消息事件设置了回调。

后面就是启动`ZKClient.Start`，然后为`service_map`中的服务与方法注册节点到ZK中。

然后开启监听，并且`event_loop.loop()`。

现在来看看回调函数是怎么设计的

**OnConnection:**

```c++
void KrpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    { // 如果连接关闭则断开连接即可。
        conn->shutdown();
    }
}
```

**OnMessage:**

一旦进入了消息回调函数，首先就是把rpc请求字符流获得

```
std::string recv_buf = buffer->retrieveAllAsString();
```

使用protobuf提供的反序列化方法`CodedInputStream`

```c++
google::protobuf::io::ArrayInputStream raw_input(recv_buf.data(), recv_buf.size());
google::protobuf::io::CodedInputStream coded_input(&raw_input);
```

首先是要把`header_size`读了

```
coded_input.ReadVarint32(&header_size);// 解析header_size
```

然后设置一下最大允许读取的数量， 防止读多了

```c++
google::protobuf::io::CodedInputStream::Limit msg_limit = coded_input.PushLimit(header_size);
coded_input.ReadString(&rpc_header_str, header_size);
// 恢复之前的限制，以便安全地继续读取其他数据
coded_input.PopLimit(msg_limit);
```

现在有了rpc_header_str，就可以先把这个解析了

```c++
if(krpcHeader.ParseFromString(rpc_header_str)){
        service_name = krpcHeader.service_name();
        method_name = krpcHeader.method_name();
        args_size=krpcHeader.args_size();
    }else{
        KrpcLogger::ERROR("krpcHeader parse error");
        return;
    }
```

然后是根据args_size读取参数的步骤(我们之所有有rpcheader，是为了防止TCP拆包粘包)

```c++
 bool read_args_success=coded_input.ReadString(&args_str,args_size);
```

现在就是从`service_map`中根据我们已经提取的`service_name`和`method_name`取出对应的`Service`与`Method`

```c++
auto it = service_map.find(service_name);
    if (it == service_map.end())
    {
        std::cout << service_name << "is not exist!" << std::endl;
        return;
    }
    auto mit = it->second.method_map.find(method_name);
    if (mit == it->second.method_map.end())
    {
        std::cout << service_name << "." << method_name << "is not exist!" << std::endl;
        return;
    }
    google::protobuf::Service *service = it->second.service;        // 获取服务对象
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取方法对象
```

现在就可以创建request,`GetRequestPrototype`

```c++
google::protobuf::Message *request = service->GetRequestPrototype(method).New();
//反序列化生成request
request->ParseFromString(args_str);
```

对于response也是同样的

```c++
 google::protobuf::Message *response = service->GetResponsePrototype(method).New();
```

现在我们需要创建一个闭包(callback),done，它负责当我们最后的`CallMethod`执行完成后，它会被触发。

```c++
google::protobuf::Closure *done = google::protobuf::NewCallback<KrpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &KrpcProvider::SendRpcResponse,
                                                                                                 conn, response);
```

`NewCallback`:

+ param1: 类的类型
+ param2: 第一个参数的类型
+ param3: 第二个参数的类型
+ param4: 绑定的对象(KrpcProvider的示例)
+ param5: 绑定的成员函数
+ param6: `TcpConnectionPtr`，表示 RPC 连接
+ param7: `Message*`，表示 RPC 方法的响应对象

最后执行一个`service->CallMethod`,根据远端的请求，调用当前rpc节点上发布的方法。

有关send,其实就是把response反序列化了

```c++
void KrpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        // 序列化成功，通过网络把rpc方法执行的结果返回给rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize error!" << std::endl;
    }
   // conn->shutdown(); // 模拟http短链接，由rpcprovider主动断开连接
}
```

### RpcChannel

我们上面的RpcProvider是服务于rpc方法提供节点的，它们能够发布服务与方法，并且Run。

对于客户端，我们实现了一个RpcChannel类，它是继承于`google::protobuf::RpcChannel`，目的是为了给客户端方法调用的时候，统一接收。

在protobuf生成的给caller的类`Stub`中，我们在`.proto`文件注册的的rpc方法都已经实现了，怎么实现的？

```c++
void UserServiceRpc_Stub::Login(::PROTOBUF_NAMESPACE_ID::RpcController* controller,
                              const ::fixbug::LoginRequest* request,
                              ::fixbug::LoginResponse* response,
                              ::google::protobuf::Closure* done) {
  channel_->CallMethod(descriptor()->method(0),
                       controller, request, response, done);
}

```

调用了一个`channel_->CallMethod`，这个`channel_`就是一个`google::protobuf::RpcChannel`类,它是一个虚类，也就是说caller需要自己实现一个继承了`google::protobuf::RpcChannel`的类，并且实现`CallMethod`。

---

这就是为什么我们有一个`RpcChannel`类。我们自己的实现的CallMethod中，主要做的事情就是:

1. 通过`service_name`和`method_name`从ZK服务器获得对应的`ip:port`。（所以需要调用`ZKClient.Start()`）
2. 获得参数的序列化字符串长度

```c++
uint32_t args_size{};
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
```

3. 定义我们rpc header的service_name, method_name, args_size

```c++
Krpc::RpcHeader krpcheader;
krpcheader.set_service_name(service_name);
krpcheader.set_method_name(method_name);
krpcheader.set_args_size(args_size);
```

4. 计算整个rpc header序列化为字符串后的长度。

```c++
uint32_t header_size = 0;
std::string rpc_header_str;
if (krpcheader.SerializeToString(&rpc_header_str))
{
    header_size = rpc_header_str.size();
}
```

5. 生成整个发送的protobuf流

```c++
std::string send_rpc_str;
    {
        google::protobuf::io::StringOutputStream string_output(&send_rpc_str);
        google::protobuf::io::CodedOutputStream coded_output(&string_output);
        coded_output.WriteVarint32(static_cast<uint32_t>(header_size));
        coded_output.WriteString(rpc_header_str);
    }
send_rpc_str += args_str;
```

6. 调用send()发送
7. 调用recv()接受
8. 调用`response->ParseFromArray(recv_buf, recv_size)`反序列化

---

当框架完成后，rpc提供方只需要在`.proto`中定义好自己的rpc服务与方法，然后继承一下自己的rpc类，并在整个类中实现自己服务的方法。要发布它就实例化provider,然后调用`provider.NotifyService(new UserService())`

以`UserService为例`:

```c++
class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端（rpc服务提供者）
{
public:
    bool Login(std::string name, std::string pwd)
    {
        /**** 业务层代码 ****/
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return false;
    }
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
		/**** callee要继承UserServiceRpc并重写它的Login函数 ****/
        
        std::string name = request->name();
        std::string pwd = request->pwd();
        //request存着caller发来的Login函数需要的参数
        
        bool login_result = Login(name, pwd);
        //处理Login函数的逻辑，这部分逻辑单独写了一个函数。处于简化目的，就只是打印一下name和pwd。
        
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);
        //将逻辑处理结果写入到response中。
        
        done->Run();
        //将结果发送回去
    }
};

int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);
	//想要用rpc框架就要先初始化
    
    RpcProvider provider;
    // provider是一个rpc对象。它的作用是将UserService对象发布到rpc节点上，暂时不理解没关系！！
    
    provider.NotifyService(new UserService());
    // 将UserService服务及其中的方法Login发布出去，供远端调用。
    // 注意我们的UserService是继承自UserServiceRpc的。远端想要请求UserServiceRpc服务其实请求的就是UserService服务。而UserServiceRpc只是一个虚类而已。

    provider.Run();
	// 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    return 0;
}
```



而对于caller方，由于protoc对于`Stub`类实现了注册的方法，通过调用channel_->CallMethod的方式。因此只需要直接实例化`Stub(new Channel())`，然后创建request，调用方法就行。

```c++
/****
文件注释：
文件名： calluserservice.cc
caller端代码：caller向callee发起远端调用，即caller想要调用处于callee中的Login函数。
****/
#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"
int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);
	//MprpcApplication类提供了解析argc和argv参数的方法，我们在终端执行这个程序的时候，需要通过-i参数给程序提供一个配置文件，这个配置文件里面包含了一些通信地址信息（后面提到）

    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    //这一步操作后面会讲，这里就当是实例化UserServiceRpc_Stub对象吧。UserServiceRpc_Stub是由user.proto生成的类，我们之前在user.proto中注册了Login方法，
    
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
  	//回想起我们的user.proto中注册的服务方法：
    // rpc Login(LoginRequest) returns(LoginResponse);
    // callee的Login函数需要参数LoginRequest数据结构数据
    
    fixbug::LoginResponse response;
    // callee的Login函数返回LoginResponse数据结构数据
    
    stub.Login(nullptr, &request, &response, nullptr); 
    //caller发起远端调用，将Login的参数request发过去，callee返回的结果放在response中。

    if (0 == response.result().errcode()) 
        std::cout << "rpc login response success:" << response.sucess() << std::endl;
    else
        std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
    //打印response中的内容，别忘了这个result和success之前在user.proto注册过
    return 0;
}

```

