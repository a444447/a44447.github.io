# blender-sim

## **水塘抽样（Reservoir Sampling）**

**水塘抽样**（Reservoir Sampling）是一种 **在线随机抽样算法**，用于在 **无法提前确定数据总量，且数据量很大** 时，从数据流中 **等概率地选取 k 个样本**。

假设我们希望从一个 **未知长度** 的数据流 `stream` 中，随机选取 `k` 个样本：

1. **前 k 个元素直接存入“水塘”**（reservoir）。

2. 从第 k+1 个元素开始，以概率 `k/i` 进行替换

   ：

   - **如果被选中**，随机替换掉水塘中的一个已有元素。
   - **如果未被选中**，丢弃该元素。

通过这种方式，**可以保证最终每个元素被选入水塘的概率是相同的**。

## grpc通信设计

项目定义的通信消息以及提供的服务于方法尽量与我们在原本真实场景规定的驱动一致。

### 消息结构

项目定义了一个Emulate服务，用于处理渲染任务和节点。它为客户端提供了发布任务的方法，也为master提供了注册下游渲染节点节点、心跳检测、注册渲染驱动的方法。

首先是为 **User**提供的方法

```
service Emulate{
	//让物体移动
	rpc Move (MoveReq) returns (MoveRsp) {}
	// 进行渲染任务，指定光源和任务数
  	rpc Render (RenderReq) returns (RenderRsp) {}
}
```

渲染响应的消息结构是这样的:

```
// 渲染请求
message RenderReq {
    Serial id              = 1;  // 任务唯一标识
    uint32 task_count      = 2;  // 任务数量
    uint32 light_count     = 3;  // 光源数量
    repeated uint32 lights = 4;  // 光源 ID 列表
}

// 渲染响应
message RenderRsp {
    Serial id              = 1;  // 任务唯一标识
    Status status          = 2;  // 任务状态
    uint32 task_count      = 3;  // 任务数量
    uint32 width           = 4;  // 渲染图像宽度
    uint32 height          = 5;  // 渲染图像高度
    bytes imgs             = 6;  // 渲染图像数据
}
```

而移动的消息结构是这样的:

```
// 物体移动请求
message MoveReq {
    Serial id         = 1;  // 物体唯一标识
    Position position = 2;  // 目标位置
}

// 物体移动响应
message MoveRsp {
    Serial id     = 1;  // 物体唯一标识
    Status status = 2;  // 操作状态
}
```

客户端连接master的方法

```
rpc RegisterDriver(DriverInfo) returns (RegisteDriverRsp) {}
// UnRegister Driver
rpc UnRegisterDriver(Serial) returns (Status) {}

// RegisterDriver message
message DriverInfo
{
    string version = 1;
}

message RegisteDriverRsp
{
    Status status = 1;
    Serial serial = 2;
}
```

这个方法是通过客户端提供一个`version`发给master，然后master允许客户端连接成功后，会返回给客户端一个属于它自己的唯一标识。

## 模块分析

### driver

#### BlenderSimDriver

这是客户端连接服务器，发起请求的，发送心跳的核心模块

##### try_connect()

1. try_connect()提供了连接Master并且注册Driver的功能

在gRPC中，通过`Stub`（存根）是**客户端用来调用远程 gRPC 服务器方法的对象**。它相当于 **客户端的代理**，封装了所有的 gRPC 方法，让客户端可以像调用本地函数一样调用远程方法。

于是我们为了调用远程服务器提供的方法，就需要创建一个Stub对象，用于调用Master服务器。_channel是gRPC连接对象，**`_emulate` 是 `Stub`，它可以调用 `Render()`、`Move()`、`HeartBeatUser()` 等远程方法**

使用stub调用方法的时候需要传入，**grpc::ClientContext context**，它是在 gRPC 客户端调用远程方法时，`grpc::ClientContext` **用于管理 RPC 调用的上下文信息**。

2. 调用`RegisterDriver`来进行注册，然后等待返回的结果，获得自己的ID。

##### led_update_batch()

要发起渲染任务，就要设置好任务的信息

```c++
render_req.mutable_id()->set_serial(_driver_id);
  render_req.set_task_count(task_count);
  render_req.set_light_count(light_count);
  for(const auto& val : leds) {
      uint32_t rgb = (val.Intensity << 24) | (val.R << 16) | (val.G << 8) | val.B;
      render_req.add_lights(rgb);
  }
```

+ 设置了id,任务数量,灯数量，每个灯的参数(亮度+RGB)

然后就是通过Stub来调用远程方法`auto status = _emulate->Render(&context, render_req, &render_rsp);`

然后解析返回的

+ width, height ，计算得到img_size=w*h
+ 读取后续的图片字节。

##### 发送心跳

很简单，就是一个定时器，每隔一秒调用stub提供的发送心跳方法

```
void DoHeartBeat() {
    while(m_bRuning) {
        grpc::ClientContext context;
        blendersim::Serial oSerial;
        blendersim::Status oStatus;
        oSerial.set_serial(_driver_id);
        _emulate->HeartBeatUser(&context, oSerial, &oStatus);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
```



### mainnode

我们的mainnode主要由三个核心模块

+ **`EmulateService.h`**（核心 gRPC 服务器，处理设备和渲染任务）
+ **`ActiveConnectionManager.h`**（管理设备和 Worker、Master 节点的连接）

#### ActiveConnectionManager.h

`ActiveConnectionManager`是个模板类，通过传入`CDeviceInfo`类或者`CNodeInfo`类标明这是一个管理客户端连接或者渲染节点连接。

它提供了下面几个方法:

**注册新连接**

**检查连接是否存活**

**连接心跳更新**

**删除超时连接**

**启动后台线程定期检查超时设备**

---

先看下定义了哪些以及作用，成员变量

首先定义了`ConnctionInfo`，它是由`std::pair<int, std::unique_ptr<CTX>>`封装来的：

+ `first`：连接存活时间（heartbeat 超时计数）

+ `second`：`CTX` 连接信息的指针

然后就是

```c++
std::unordered_map<uint32_t, ConnectionInfo> connections;
std::function<void(uint32_t)> DeleteCB;
std::shared_mutex mtx;
std::thread monitor_thread;
std::atomic<bool> running;
int interval;
int initial_value;
```

**`connections`**：存储当前活跃的连接 (`connId` -> `ConnectionInfo`)

**`DeleteCB`**：连接超时后的回调函数

**`mtx`**：**读写锁**，用于保护 `connections`

**`monitor_thread`**：定期检查超时连接的线程

**`running`**：**控制心跳监测线程** 是否继续运行

**`interval`**：心跳监测的 **时间间隔**（单位：秒）

**`initial_value`**：连接 **初始存活时间**（心跳计数）

构造函数就是见到那的初始化`interval`,`initial_value`,`running`,`DeleteCB`

##### `registerConnection()` - 设备注册

```
void registerConnection(uint32_t connId, std::unique_ptr<CTX> ctx = nullptr) {
    std::unique_lock<std::shared_mutex> lock(mtx);
    connections[connId] = std::make_pair(initial_value, std::move(ctx));
}
```

**功能**

- **新建设备**，并初始化存活时间 `initial_value`
- **线程安全**，加写锁 `std::unique_lock<std::shared_mutex> lock(mtx)`

> 注意unique_lock是主要用于管理mutex的，它是RAII机制。（构造时获得资源，离开作用域的时候自动解锁析构）
>
> 它可以指定延迟锁定`std::unique_lock<std::mutex> lock(mtx, std::defer_lock);`,在需要用锁的地方调用`lock.lock()`
>
> 它可以配合`std::condition_variable`，比如在某个地方调用`std::unique_lock<std::mutex> lock(mtx);cv.wait(lock, [] { return ready; });`

> 另一个管理mutex的RAII机制，叫做`std::lock_guard`,它比起`unique_lock`更加轻量的机制。它适合简单的临界区保护。注意它不能配合condition_variable,std::condition_variable` 需要手动释放 `mutex.
>
> 在 `std::condition_variable::wait()` 时，要求：
>
> 1. 线程必须**持有互斥锁**（否则 `wait()` 会报错）。
> 2. **在等待时必须释放互斥锁**，以便其他线程能获取锁并改变条件变量。
> 3. **被唤醒后重新持有互斥锁**，以继续执行后续逻辑。
>
> 所以总结起来,lock_guard是更轻量的为锁提供RAII服务的机制，它 不能主动lock/unlock,也不提供延迟/定时的服务。

##### `CheckAlive()` - 设备存活检查

```c++
bool CheckAlive(uint32_t connId) {
    std::shared_lock lock(mtx);
    return connections.find(connId) != connections.end();
}
```

**检查设备是否在线**

**线程安全**，使用 **读锁 `std::shared_lock`**

如果设备在 `connections` 里，返回 `true`，否则 `false`

> `std::shared_lock` 是 **C++17** 引入的 **共享互斥锁管理器**，用于管理 **`std::shared_mutex`**（共享互斥量），允许多个**读取线程**同时获取锁，但写入线程必须独占访问。



##### monitorFunction()- 连接心跳检测

```c++
void monitorFunction() {
    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(interval));
        std::set<uint32_t> erased;
        {
            std::unique_lock<std::shared_mutex> lock(mtx);
            for (auto it = connections.begin(); it != connections.end();) {
                LogDebug << "[ActiveConnectionManager] Receive HeartBeat From " << it->first 
                         << " life: " << it->second.first << moveai::endl;
                if (--(it->second.first) <= 0) {
                    erased.insert(it->first);
                    it = connections.erase(it);
                } else {
                    ++it;
                }
            }
        }
        for(auto& serial : erased)
        {
            DeleteCB(serial);
        }
    }
}

```



#### EmulateService.h

`AsyncEmulateService` 类是 `Master` 端的 gRPC 服务器，负责管理 `Driver` 和 `Worker` 设备的注册、心跳、渲染任务等操作。它是一个 **异步 gRPC 服务器**，意味着它不会阻塞主线程，而是使用 **事件驱动** 处理请求，提高性能和扩展性。

需要重点关注的是`completionQueue`（简称 `CQ`）是 **核心组件**，用于**管理和触发异步事件**。
 它主要用于： ✅ **监听客户端请求**（RPC 方法调用）
 ✅ **等待服务器完成 RPC 处理**（发送响应）
 ✅ **通知 gRPC 服务器执行下一个任务**

看下我们的成员变量负责了什么:

声明了两个`ActiveConnectionManager`帮助服务器管理连接。

```c++
ActiveConnectionManager<CDeviceInfo>& m_roDriverConnections;
ActiveConnectionManager<CNodeInfo>& m_roNodeConnections;
```



首先是构造函数:
```c++
AsyncEmulateService(const std::string& strUrl, uint32_t dwMaxUser, uint32_t dwMaxNode) 
    : m_strUrl(strUrl),
      m_oDriverConnections(3, 1, [this](uint32_t serial){ 
          SerialDistributor::GetInstance()->ReleaseId("driver", serial);
          DevicePositionMap::Instance()->DeleteDevicePosition(serial); 
      }), 
      m_oNodeConnections(3,1, [this](uint32_t serial){
          SerialDistributor::GetInstance()->ReleaseId("node", serial);
      }) 
{
    SerialDistributor::GetInstance()->RegisterToken("driver", 10);
    SerialDistributor::GetInstance()->RegisterToken("node", 20);
    m_oDriverConnections.startMonitoring();
    m_oNodeConnections.startMonitoring();
}
```

传入的`strURL`标识的是Master监听的地址(ip:port), 然后定义两个连接管理器分别负责管理master与客户端,master与服务端，并让它们开启心跳监听。



##### Run

Run方法负责启动gRPC服务器

```
void Run() {
    std::string server_address(m_strUrl);
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    LogInfo << "Server listening on " << server_address << moveai::endl;

    HandleRpcs();
}
```

gRPC的API 是这样的

**创建gRPC服务器**

```c++ 
grpc::ServerBuilder builder;
builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
```

`AddListeningPort()` 监听 `IP:Port`

`InsecureServerCredentials()` **不加密通信**

如果要启用 TLS，需要 `SslServerCredentials()`

**注册 gRPC 服务**

```
builder.RegisterService(&service_);
```

- `service_` 是 **异步 gRPC 服务**，用于处理请求

**创建 `CompletionQueue` 事件队列**

```
cq_ = builder.AddCompletionQueue();
```

- **gRPC 异步模型** 需要 **事件队列**，用于管理 RPC 请求和响应

**启动服务器**

```
server_ = builder.BuildAndStart();
```

然后调用`handleRpcs()`监听请求

##### `HandleRpcs()`（注册 gRPC 处理器）

```c++
void HandleRpcs() {
    new CallDataImpl<blendersim::MoveReq, blendersim::MoveRsp, static_cast<int>(RPCS::MOVE)>(&service_, cq_.get(), ...);
    new CallDataImpl<blendersim::RenderReq, blendersim::RenderRsp, static_cast<int>(RPCS::RENDER)>(&service_, cq_.get(), ...);
    new CallDataImpl<blendersim::GetPositionReq, blendersim::GetPositionRsp, static_cast<int>(RPCS::GETPOSITION)>(&service_, cq_.get(), ...);
}
```

- **创建 `CallDataImpl` 实例**，用于监听 gRPC 方法
- **支持异步请求处理**

#### 核心`CallData`

我们的gRPC服务器是一个异步服务器，在 gRPC 的 **异步 API** 里，每个 RPC 调用需要手动管理请求的生命周期。所以我们定义了一个`CallData`基类，它 **封装了 gRPC 事件处理逻辑**。

首先理解一下gRPC服务端异步处理的逻辑：gRPC让我们像流水线一样操作，先准备一个**CallData对象作为一个容器**，然后 gRPC 会通过 ServerCompletionQueue 将各种事件发送到 CallData 对象中，让这个对象根据自身的状态进行处理。然后处理完毕当前的事件之后还需要手动再创建一个 CallData 对象，这个对象是为下个 Client 请求准备的，整个过程就像流水线一样。

![grpc3](https://img.luozhiyun.com/20220501172555.png)

- CallData 对象刚创建的时候会从 CREATE 状态扭转为 PROCESS 状态，表示等待接收请求；
- 请求过来之后，首先会创建一个 CallData 对象，然后处理完后扭转为 FINISH 状态，等待给 Client 回包结束；
- 回包结束之后将 CallData 对象自身删除。

整个 Server 的流程应该如下：

1. 启动服务时，预分配 *一个* CallData 实例供未来客户端使用。
2. 该 CallData 对象构造时，`service_->RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this)` 将被调用，通知 gRPC 开始准备接收 *恰好一个* `SayHello` 请求。 这时候我们还不知道请求会由谁发出，何时到达，我们只是告诉 gRPC 说我们已经准备好接收了，让 gRPC 在真的接收到时通知我们。 提供给 `RequestSayHello` 的参数告诉了 gRPC 将上下文信息、请求体以及回复器放在哪里、使用哪个完成队列来通知、以及通知的时候，用于鉴别请求的 tag（在这个例子中，`this` 被作为 tag 使用）。
3. `HandleRpcs()` 运行到 `cq->Next()` 并阻塞。等待下一个事件发生。

**一段时间后….**

1. 客户端发送一个 `SayHello` 请求到服务器，gRPC 开始接收并解码该请求（IO 操作）

**一段时间后….**

1. gRPC 接收请求完成了。它将请求体放入 CallData 对象的 `request_` 成员中（通过我们之前提供的指针），然后创建一个事件（使用`指向 CallData 对象的指针` 作为 tag），并 **将该事件放到完成队列** `**cq_**` **中**.
2. `HandleRpcs()` 中的循环接收到了该事件（之前阻塞住的 `cq->Next()` 调用此时也返回），并调用 `CallData::Proceed()` 来处理请求。
3. CallData 的 `status_` 属性此时是 `PROCESS`，它做了如下事情： 6.1. 创建一个新的 CallData 对象，这样在这个请求后的新请求才能被新对象处理。 6.2. 生成当前请求的回复，告诉 gRPC 我们处理完成了，将该回复发送回客户端 6.3. gRPC 开始回复的传输 （IO 操作） 6.4. `HandleRpcs()` 中的循环迭代一次，再次阻塞在 `cq->Next()`，等待新事件的发生。

**一段时间后….**

1. gRPC 完成了回复的传输，再次通过在完成队列里放入一个以 CallData 指针为 tag 的事件的方式通知我们。
2. `cq->Next()` 接收到该事件并返回，`CallData::Proceed()` 将 CallData 对象释放（使用 `delete this;`）。`HandleRpcs()` 循环并重新阻塞在 `cq->Next()` 上，等待新事件的发生。

---

上面描述的是服务端只提供单个方法的情况，实际上，我们服务端有多个方法，为了根据适应这点，根据客户端不同的请求响应不同的处理方法，我们将CallData作为抽象类，然后实现创建一个模板类`CallDataImpl`，它继承了这个抽象类，并且根据传入模板中的参数不同，调用不同的方法，因此就相当于可以制作适用于不同方法的容器。

```c++
class CallData {
public:
    virtual void Proceed() = 0;
};
```

可以看到它是一个抽象基类，它有一个纯虚函数`Proceed`，用于请求gRPC请求的生命周期。我们后面的继承类必须实现它。

```c++
template <typename RequestType, typename ResponseType, int RPC>
class CallDataImpl : public CallData {
public:
    CallDataImpl(blendersim::Emulate::AsyncService* service, grpc::ServerCompletionQueue* cq, 
                 std::function<void(grpc::ServerContext*, RequestType*, grpc::ServerAsyncResponseWriter<ResponseType>*, grpc::ServerCompletionQueue*, grpc::ServerCompletionQueue*, void*)> request_func, 
                 ActiveConnectionManager<CDeviceInfo>& roDeviceConnections, ActiveConnectionManager<CNodeInfo>& roNodeConnections) : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), request_func_(request_func), m_roDriverConnections(roDeviceConnections), m_roNodeConnections(roNodeConnections)
    {
        Proceed();
    }
```

这是继承类，值得注意的是，我们需要根据模板来传入对应的Request方法，这样才能起到一个充当”对应方法接受容器“的作用。

可以看到，我们需要的函数对象是

```c++
std::function<void(grpc::ServerContext*, RequestType*, grpc::ServerAsyncResponseWriter<ResponseType>*, grpc::ServerCompletionQueue*, grpc::ServerCompletionQueue*, void*)>
```

也就是形如`RequestSayHello(&ctx_, &request_, &responder_, cq_, cq_, this)`

我们构造它的方法是使用std::bind()

```
std::bind(&blendersim::Emulate::AsyncService::RequestMove, &service_,
              std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
              std::placeholders::_4, std::placeholders::_5, std::placeholders::_6),
    m_oDriverConnections, m_oNodeConnections);
```

bind一般的参数是,bind(f, arg1,arg2,...)，也就是第一个参数是我们想要构建的函数对象的函数指针。但是由于这个函数是一个成员函数，所以我们还需要提供一个对象指针给它，也就是bind(f, p, arg1,arg2)，这样bind生成的形式最终就是

```c++
service_->RequestMove(&ctx_, &request_, &responder_, cq_, cq_, this);
```

---

现在回头看一下我们的`HandleRpcs()`，我们一来就创建了不同方法`CallDataImpl`对象，相当于给cq中装入了这些方法的容器，等到客户端发起了一个对某个方法的请求后，就会取出它，调用它的`Process`——我们传入了`this`作为标识的`tag`，因此可以直接用`static_cast<CallData*>tag->Process()`调用。

##### 方法处理逻辑

**MOVE**

我们以MOVE为例，当一个Move请求到达，并且cq相应了它，并取出了对应的CallDataImpl后，首先再创建一个同样的CallDataImpl为后面的请求，然后进入业务流程。

首先根据RPC判断到底进入哪一个分支

```c++
if constexpr (RPC == static_cast<int>(RPCS::MOVE))
```

然后检验客户端`Drive`是否注册

```c++
if(!Authentication(roMoveReq, roMoveRsp))
{
LogInfo << "[mainnode] " << "Serial<" << roMoveReq.id().serial() << "> UnRegister Call Move." << moveai::endl;
break;
}
```

更新Driver的位置

```c++
DevicePositionMap::Instance()->UpdateDevicePosition(
    roMoveReq.id().serial(),
    std::make_tuple(roMoveReq.position().x(), roMoveReq.position().y(), roMoveReq.position().z())
);
```

注意`DevicePositionMap`是一个单例类，它里面有一个map，存储了不同用户，它当前的物体的坐标。

然后就是`Respond`消息的各种参数。

> 在 gRPC 生成的 **C++ Protobuf 代码** 中，`mutable_id()` 是 **Protobuf 生成的 setter 方法**，用于获取一个可修改的对象引用。
>
> 在 **Protobuf 生成的 gRPC 代码** 中，每个 `message` 字段都有 **getter 和 setter 方法**：
>
> `const Serial& id() const;`, `Serial* mutable_id();`

注意我们实现的逻辑中，Move相关的方法都不需要实际去渲染节点渲染。

**RENDER**

依然是首先检查客户端是否以及注册了，然后生成一个给渲染节点的请求参数

```c++
auto poSlaveRequest = std::make_shared<blendersim::SlaveRenderReq>();
auto& [x, y, z] = DevicePositionMap::Instance()->GetDevicePosition(roRenderReq.id().serial());

poSlaveRequest->mutable_position()->set_x(x);
poSlaveRequest->mutable_position()->set_y(y);
poSlaveRequest->mutable_position()->set_z(z);
poSlaveRequest->set_task_count(roRenderReq.task_count());
poSlaveRequest->set_light_count(roRenderReq.light_count());
for (const auto& light : roRenderReq.lights()) {
    poSlaveRequest->add_lights(light);
}
```

然后我们将任务放入线程池，通过:

```c++
m_oTaskProcessPool.AddTask([this, poSlaveRequest]{
```

+ 注意lambda函数中，如果是捕获 this那么对于里面的成员对象都是按引用，`poSlaveRequest`是按值传递的

`m_roNodeConnections.getConnections()` **返回所有可用 `Worker`**

如果没有可用 `Worker`，返回 `OVERLOAD` 错误

**随机选择一个 `Worker`**

- 遍历 `Worker`，收集 `Worker` 的 `serial`
- 随机选择 `Worker` **分担计算任务**

选择好Worker后，就调用远端worker提供的grpc方法，Render，然后等待Rsp

```c++
LogDebug << "[mainnode] Select Node: " << poConnection->m_strAccessUrl << moveai::endl;
grpc::ChannelArguments oChannelArgs;
oChannelArgs.SetMaxReceiveMessageSize(-1);
auto channel = grpc::CreateCustomChannel(poConnection->m_strAccessUrl, grpc::InsecureChannelCredentials(), oChannelArgs);
std::unique_ptr<blendersim::SlaveEmulate::Stub> stub = blendersim::SlaveEmulate::NewStub(channel);
blendersim::SlaveRenderRsp oSlaveRsp;

grpc::ClientContext oClientContext;
LogDebug << "[mainnode] Call Slave Render RPC." << moveai::endl;
grpc::Status status = stub->Render(&oClientContext, *poSlaveRequest.get(), &oSlaveRsp);
```

这里，我们的mainnode是作了客户端向渲染节点发出了请求。

放入线程池中的任务执行完毕了到最后该如何给gRPC说已经完成了呢？

```c++
//标记 Render 任务完成
//告诉 CompletionQueue 这个 CallDataImpl 处理完了
m_bFinished = true;
//创建 gRPC Alarm（定时器）
//m_poAlarm 是 std::unique_ptr<grpc::Alarm> 类型
//重置 m_poAlarm，确保它是新的 Alarm 实例
m_poAlarm.reset(new grpc::Alarm());
//告诉 CompletionQueue：“在 gpr_now(GPR_CLOCK_REALTIME) 这个时间，触发 this 事件。”
//在 CompletionQueue 里注册 tag = this，稍后 Proceed() 会处理这个事件
m_poAlarm->Set(cq_, gpr_now(GPR_CLOCK_REALTIME), this);
```

所以当gRPC Alarm触发的时候

1️⃣ `cq_->Next()` 取出 `tag = this`（`CallDataImpl`）
 2️⃣ `static_cast<CallData*>(tag)->Proceed();` 执行 `Proceed()`
 3️⃣ **`Proceed()` 进入 `FINISH` 状态，清理自己，准备下一个 `Render` 请求**

简单来说就是，服务端向渲染节点发出渲染请求的这个过程被交给了线程池来决定什么时候执行。当把这个任务放入线程池后，线程池选择这个任务调用，然后当resp接收完毕后，就设置Alarm，`Alarm` 触发的条件是 `gpr_now(GPR_CLOCK_REALTIME)` 到达，也就是立刻触发。

这一次执行 **`Proceed()` 进入 `FINISH` 状态，执行Finish,清理自己**

> 在 gRPC 的 **异步服务器** 里，每个 RPC 方法对应一个 `grpc::ServerAsyncResponseWriter<T>`，用于**异步发送响应**。
>
> 其中，`Finish()` 是 `grpc::ServerAsyncResponseWriter<T>` 的方法：
>
> ```c++
> class grpc::ServerAsyncResponseWriter<T> {
> public:
>     void Finish(const T& response, const grpc::Status& status, void* tag);
> };
> ```
>
> ✅ **`Finish()` 结束 gRPC 请求，返回 `response` 给客户端**
>  ✅ **`CompletionQueue` 触发 `tag`，`Proceed()` 进入 `FINISH` 状态**

**REGISTER**

对于渲染节点的注册，它在启动的时候调用RegisterNode，向Master服务器注册。

主要流程是这样的：

1. 生成唯一 ID

```
auto Id = SerialDistributor::GetInstance()->GenerateId("node");
serial->set_serial(Id);
```

> 之前没提到我们的唯一ID生成器，它也是一个单例类: `SerialDistributor`
>
> 它提供：
>
> + `RegisterToken(token, maxId)`--**注册一个 ID 生成器**，`token` 作为唯一标识
> + `GenerateId(token)`--生成唯一 ID
> + `ReleaseId(const std::string& token, uint32_t id)`
>
> 所谓的token是一个标识，标识特定的ID生成器。当调用`RegisterToken(token, maxId)`,我们发现没有对应token的generator的时候就创建一个该token的generator。
>
> 我们在`SerialDistributor`类中定义了一个` SequenceGenerator`,根据token的不同，我们为其分配不同的 SequenceGenerator，这样生成的ID就不会互相冲突。

2. 在 `ActiveConnectionManager` 里注册 `Worker`

```c++
m_roNodeConnections.registerConnection(Id, std::make_unique<CNodeInfo>(roNodeInfo.access_url(), std::to_string(roNodeInfo.access_port())));
```

注意`CNodeInfo`类是一个提供渲染节点信息的类,。里面有渲染节点的`ip:port`信息

**HEARTBEATNode-渲染节点心跳**

渲染节点定期发送心跳给Master

```c++
if(!m_roNodeConnections.CheckAlive(roSerial.serial()))
```

如果 `Worker` **不存在**，返回 `UNAUTHORIZED` 错误

`Master` 可能已经删除了它的连接

然后更新这个渲染节点的存活状态

```c++
m_roNodeConnections.updateConnection(roSerial.serial());
```

**`updateConnection()` 重置 `Worker` 的超时时间**

`Worker` 继续存活，`Master` 仍然可以分配任务给它

**REGISTERDEVICE**

这是客户端向master发起连接的请求，这和Register node类似，只是注册到客户端连接管理器中。

**HEARTBEAT-客户端心跳**

和Worker的心跳处理一样。

### server

简单介绍一下server中重要的部分:

+ `SlaveEmulateServiceImpl.h`：这个和master中的`Emulate`一样，是渲染节点的异步RPC服务端
+ `CLoadBalaencer.h`:这个是负责提供负载均衡服务的一个单例类（这个负载均衡主要是指的是渲染节点下运行着多个blender实例，对每个blender实例分配任务的负载均衡）
+ `CGTaskTable.h`： 这是一个负责控制渲染执行的模块
+ `SCB.h`: 定义一个blender进程的属性
+ `SCBController.h`: 提供对blender进程的管理方法，包括了启动，关闭，获得进程的pid
+ `Router.h`: 

✅ **管理 `Worker` 内部的组件交互**
 ✅ **负责 `gRPC` 服务器 (`SlaveEmulateServiceImpl`) 的启动**
 ✅ **初始化 `负载均衡 (CLoadBalancer)`，任务调度 (`CGTaskTable`)**
 ✅ **管理 `SCBControler`，用于控制 `Blender` 进程**

#### `SlaveEmulateServiceImpl.h`

这部分的异步RPC服务端的实现和Master里面一样，区别是渲染节点只提供`Render`这一个服务，因此我们不需要`CallData`作为基类，我们只需要CallData绑定`Render`这个请求事件，在每次收到一个Render请求后处理前再生成一个CallData对象，形成形成这样的流水线模式。

**RENDER**

下面来说明一下`Proceed`执行的时候业务逻辑是如何的。

`CallData`中有一个`std::unique_ptr<Message>`，这个`Message`类是我们实现了很多运算符重载，方便我们把`request`参数拼接成一个能发送给blender进程消息。可以这么理解，我们的渲染节点与`blender进程`之间的通信类型，就是这个`Message`。

1. 首先调用`Reset`，清空Message
2. 然后调用负载均衡器分配任务

```C++
auto assignedTasksMap = CLoadBalancer::Instance()->TasksDispath(request_.task_count());
//注意assignedTaskMap是一个std::unordered_map<pid_t, uint32_t>，pid_t是分配的blender进程号,uint32_t是任务数
```

3. 拆分任务

```c++
auto mapTasks = SplitTasks(assignedTasksMap, request_.light_count());
```

SplitTasks做的工作就是，根据assignTasksMap中对不同blender进程分配的任务数，各自生成发送给它们的`Message`

说一下Message的结构，这个是和我们项目的任务有关的

```c++
任务号|position(x,y,z)|任务数|led灯数量|灯的参数(RGB和brightness,数量是task_count*led_count)
```



4. 把任务插入到`CGTaskTable`中，`CGTaskTable`是一个单例类，它就像线程池那样，维护了一系列线程与待处理任务，一有进程空就把任务交予线程池处理。它也维护了一个任务队列，当某个任务的目标Blender有空闲就会自动调度一个属于该blender进程的任务去完成。

```C++
CGTaskTable::Instance()->InsertTask(pid, std::move(poMsg), [this, &vecTasksRsp, &dwReceviceCount](std::unique_ptr<Message> poMsg, ErrCode eStatus){
    std::unique_lock<std::mutex> lock(mutex_);
    dwReceviceCount++;
    LogDebug << "[node] Receive Rsp Count: " << dwReceviceCount << "/" << vecTasksRsp.size() << moveai::endl;
    if(eStatus != ErrCode::OK)
    {
        LogError << "[node] Blender Server Error: " << (int)eStatus << moveai::endl;
    }
    else
    {
        uint32_t serial{std::numeric_limits<uint32_t>::max()};

        (*poMsg) >> serial;
        LogDebug << "[node] Receive Task Rsp Serail: " << serial << moveai::endl;
        vecTasksRsp[serial] = std::make_pair(eStatus, std::move(poMsg));
    }
    if(dwReceviceCount == vecTasksRsp.size())
    {
        LogDebug << "[node] Receive Rsp Finished." << moveai::endl;
        finished = true;
        cv_.notify_one();
    }
});
```

+ InsertTask的参数是`任务属于的blender进程的pid`、`要传递的消息` 、`当这个任务完成时，需要执行逻辑`。
+ 这个任务完成的回调函数，会首先不断累积完成的任务的数量` dwReceviceCount++`,直到完成了所有的任务。

5. 当收到所有任务已经渲染完毕，就把每个`blender进程`传回的Message重新反序列化为Resp

#### `CGTaskTable.h`

`CGTaskTable.h`负责控制渲染任务与blender进程之间的交流，它有：

+ `TCB`类:  任务控制块 - Task Control Block

+ `CRenderTaskQueue`类: 一个存储渲染任务的类
+ `CGTaskTable`任务管理器，负责初始化数据结构、插入新任务、删除已完成的任务



##### TCB

TCB是一个任务控制块，它存储了一个任务所有的信息，包括了:

+ blender进程号pid
+ 渲染任务号task id
+ 渲染任务信息Message
+ 内部通知任务完成的回调,FinishedCB
+ 内部通知任务异常的回调，ExceptionCB
+ 外部通知任务运行情况的回调，TaskRuningCB

我们实现时删除了复制与赋值，它只能够被移动

我们还定义了一个`using TCBDict = std::unordered_map<uint32_t, std::unique_ptr<TCB>>;`，表示任务号与TCB的映射关系。

##### CRenderTaskQueue

维护了一个TCB队列,`std::deque<TCB*> m_oTaskQueue;`，提供了下面的方法:

+ `Enque`：入队
+ `Run`： 根据设置的进程数，启动相应数量的进程，执行`Process方法`
+ `Process`: 处理渲染任务的主逻辑

我们直接看Process

1. 首先就用`unique_lock`+`cv`的方式，让进程阻塞，直到`!m_bRuning || !m_oTaskQueue.empty()`
2. 从任务队列中取出一个任务（TCB）
3. 通过`SCBController`(blender进程管理器)，获得对应blender进程所拥有的`jbus::CChannel`,然后调用这个channel提供的`Write`方法

> jbus是我们任务中引入的一个内部通信总线（消息总线）**，用于在不同的 `Worker` 与 `Blender` 进程之间进行 **消息传递。
>
> 我们每个blender进程都有一个Channel，我们可以通过调用实现的`Write`接口，直接把数据写给channel的对端。在我们的项目中channel的一端就是SCB,另一端就是blender python脚本。这个blender python脚本通过jbus和我们沟通，获得Message数据，然后解析参数，最后通过bpy接口把参数传给blender服务进行渲染。

4. 调用channel的`Read`的方法等待渲染结果的返回，收到后把数据装入`Message`
5. 调用内部任务完成的回调` poTcb->m_pFinishedCB(std::move(poRecvMsg));`

##### CGTaskTable

提供下面几个方法:

+ Init，初始化各种结构，比如:
  + `m_oTaskTable`：它是blender进程pid与其所拥有的任务的映射
  + `CRenderTaskQueue`: 渲染任务队列，并在Init中，直接`Run`，等待任务到达。
+ InsertTask，插入任务
+ FinishedTask，内部任务完成后的回调函数。

#### CLoadBalancer.h

它的负载均衡逻辑是：

+ 记录当前 `Blender` 进程已有的任务数,`totalTasksMap `
+ `assignedTasksMap` 记录新的任务分配情况，初始值为 0

+ `taskToPidMap-->std::multimap<uint32_t, pid_t>` 按 `任务数` 排序 pid
+ 然后持续循环这样一个流程，直到任务被分配完: 取出拥有任务最少的PID，然后从,`taskToPidMap `移除；分配一个任务给它，记录数量变化，然后又插入回taskToPidMap .

