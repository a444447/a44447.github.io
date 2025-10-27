# 场景问题

## 如何查看内存？如何定位哪个进程发生了内存泄漏？如何在不重新编译运行程序的情况下详细分析内存泄漏的情况？

### 查看内存

free：可以显示系统的总内存、已使用内存、空闲内存以及用于缓存的内存

top：动态显示内存和 CPU 使用情况，还可以查看按内存排序的进程。

### 如何定位

当系统中某个进程发生了内存泄漏时，可以通过分析 **进程的内存占用趋势** 来定位问题。以下是详细的步骤：

+ top:查看哪个进程内存使用最大，并且有持续增加的趋势

### **在不重新编译程序的情况下分析内存泄漏**

+ 使用valgrind

```c++
valgrind --leak-check=full ./my_program
```

+ gdb attach PID,设置 `break malloc` 或 `break mmap`。

## 程序CPU占用高如何排查？

1. 找到进程: top -c,看看哪个进程占用率高
2. 找到进程中的线程: top -H -p pid,也就是查看pid进程中的线程
3. 找到线程后，查看代码位置，可以通过gdb -p pid，也就是附加到这个正在运行的线程中，查看堆栈信息

可能的原因包括了:

1. 代码问题：出现了死循环等等
2. 多线程开发的时候，用了自旋锁
3. 频繁的系统调用，可以考虑一下池化方法，比如线程池内存池。

# C11——C17编程技巧





## socket相关

### 判断socket fd是否关闭，开启的事件

**判断 socket fd 是否已关闭**：

- 可读事件触发后读取数据，`read()` 返回 `0` 表示对端关闭。
- 检测异常事件如 `EPOLLHUP`、`POLLHUP` 或通过 `getsockopt()` 检查错误状态。

通过 `getsockopt()` 获取套接字的状态信息：

- 使用选项 `SO_ERROR` 来查看 `socket fd` 上是否有错误。如果返回错误值（如 `ECONNRESET`），可以判断 `socket fd` 已经无法再使用。

```c++
if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
    // 检测失败
}
```

### UDP调用connect与不调用connect

1. 不调用： 不调用connect的话，其实就是每次调用`sendto`发送的时候都需要提供目标源地址，并且调用`recvfrom`的时候可以获得数据包来源的`IP`和端口号

比较适用于：广播多播场景（需要给多个目标发送数据包）

多个客户端与服务端通信（UDP服务器可以接受来自多个客户端的数据，因此不需要对目标地址进程绑定）

2. 调用 **connect()**的UDP

虽然UDP是无连接协议，但是调用`connect`后，会对`socket`行为作如下设置:**绑定固定目标地址：**

- 调用 `connect()` 后，UDP 的 `socket` 会将目标地址（`IP` 和 **端口号**）绑定到套接字：

```c++
connect(socket_fd, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
```

+ 可以直接调用`send`与`recv`

```c++
send(socket_fd, buffer, length, 0);
recv(socket_fd, buffer, sizeof(buffer), 0);
```

+ **丢弃非目标地址的数据包：**一旦调用 `connect()` 绑定了目标地址的 `socket`，接收到的数据包如果不是来自绑定的目标地址，内核会自动丢弃这些数据包，而不传递到应用层。

> 注意使用普通`sendto`，如果目标地址不可达，错误通常被内核忽略；
>
> 而调用了 `connect()` 后，`send() / recv()` 会直接报告通信失败等错误，这使得错误处理更加及时。

**判断 socket fd 可读**：

- 检测 `EPOLLIN`、`POLLIN` 或 `FD_ISSET` 可读事件。
- 调用 `read()` 或 `recv()` 检查数据是否到达，返回大于 `0` 表明有数据到达。



## 设计模式

### c++单例模式

> 定义：单例模式是创建型设计模式，指的是在系统的生命周期中只能产生一个实例(对象)，确保该类的唯一性。
>
> 一般遇到的写进程池类、日志类、内存池（用来缓存数据的结构，在一处写多出读或者多处写多处读）的话都会用到单例模式

**实现方法：**全局只有一个实例也就意味着不能用new调用构造函数来创建对象，因此构造函数必须是虚有的。但是由于不能new出对象，所以类的内部必须提供一个函数来获取对象，而且由于不能外部构造对象，因此这个函数不能是通过对象调出来，换句话说这个函数应该是属于对象的，很自然我们就想到了用static。由于静态成员函数属于整个类，在类实例化对象之前就已经分配了空间，而类的非静态成员函数必须在类实例化后才能有内存空间。

单例模式的要点总结：

1. 全局只有一个实例，用static特性实现，构造函数设为私有
2. 通过公有接口获得实例
3. 线程安全
4. 禁止拷贝和赋值

单例模式可以**分为懒汉式和饿汉式**，两者之间的区别在于创建实例的时间不同：懒汉式指系统运行中，实例并不存在，只有当需要使用该实例时，才会去创建并使用实例(这种方式要考虑线程安全)。饿汉式指系统一运行，就初始化创建实例，当需要时，直接调用即可。（本身就线程安全，没有多线程的问题）

#### 懒汉式

- 普通懒汉式会让线程不安全

  因为不加锁的话当线程并发时会产生多个实例，导致线程不安全

如果是 **传统的双重检查锁**，需要使用双重判空，确保单例对象只能被初始化一次。

1. 第一次检查 `if (instance == nullptr)` 是为了避免每次都加锁，提高效率。
2. 如果发现 `instance` 为空，进入临界区加锁，并再次检查 `instance` 是否为空（第二次检查）。这是因为在第一次检查之后，可能有另一个线程也在尝试创建实例。

```c++
class Singleton {
private:
    static Singleton* instance;
    static std::mutex mtx;

    Singleton() {} // 私有构造函数
    ~Singleton() {}

public:
    static Singleton* getInstance() {
        if (instance == nullptr) { // 第一次检查
            std::lock_guard<std::mutex> lock(mtx);
            if (instance == nullptr) { // 第二次检查
                instance = new Singleton();
            }
        }
        return instance;
    }
};

// 静态成员变量初始化
Singleton* Singleton::instance = nullptr;
std::mutex Singleton::mtx;
```

c++11后，引入了线程安全的局部静态变量初始化，局部静态变量在初始化时由编译器保证是线程安全的。这意味着在多线程环境下，局部静态变量只会被初始化一次，不需要手动加锁。

```c++
class Singleton {
public:
    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }

private:
    Singleton() {} // 私有构造函数
    ~Singleton() {}

    Singleton(const Singleton&) = delete; // 禁止拷贝构造
    Singleton& operator=(const Singleton&) = delete; // 禁止赋值运算
};
```



#### 饿汉式

### 观察者模式

观察者模式主要是用于这样的场景：常用于实现一对多依赖关系，让多个对象观察一个对象，当被观察者发生变化时会自动通知所有观察者。

> 你订阅了某个微信公众号（被观察者）。当公众号发新消息时，它会推送给你（观察者），你就能“被动接收”更新。

在观察者模式（Observer Pattern）中，主要有以下**四个核心角色**，每个角色职责清晰，协作实现“发布-订阅”机制：

1. subject
    提供注册（attach）、注销（detach）观察者的方法；
    提供通知方法（notify），在自身状态变化时通知所有观察者。

2. concreteSubject(具体主题)
    实现subject接口，维护自己的内部状态；
    状态发生变化时调用 notify()；
    通常提供 getState() 方法供观察者读取状态。
3. observer
    声明一个 update() 接口方法，被 Subject 调用；
    可以设计成“拉模式”或“推模式”（是否传数据给 update）。
4. concreteObserver(具体观察者)
    保存对主题的引用（可选）；
    在 update() 中拉取数据或根据推送数据做出动作；
    更新自己的状态或执行逻辑（如 UI 刷新、日志记录等）。

### 工厂模式

它的核心思想是：**将对象的创建过程封装起来，使调用者无需关心具体类如何实例化，只需“向工厂要一个产品”即可**。

工厂类常见的三种模式

| 模式名                           | 简述                                   |
| -------------------------------- | -------------------------------------- |
| **简单工厂（Simple Factory）**   | 一个工厂类，创建所有产品               |
| **工厂方法（Factory Method）**   | 每种产品一个工厂子类，工厂类做“抽象化” |
| **抽象工厂（Abstract Factory）** | 创建“产品族”（多个相关产品）           |

#### 简单工厂模式

简单工厂模式通常包括一个工厂类和多个产品类。工厂类负责根据客户端的请求，返回对应的产品类实例。就是用户申请一个产品，由工厂负责创建对象。而不是用户自己创建对象。在简单工厂模式中，客户端只需要通过调用工厂类的方法，并传入相应的参数，而无需直接实例化产品类。工厂类**根据客户端传入的参数**决定创建哪个产品类的实例，并将实例返回给客户端。

简单工厂模式可能的问题就是，当需要创建多种类型的产品实例时，工厂类的代码可能会变得复杂，并且随着产品类型的增加，工厂类的责任也会越来越大。

```c++
class Product {
public:
    virtual void operation() = 0;
};
class ConcreteProductA : public Product {
public:
    void operation() override {
        // 具体产品A的操作实现
    }
};
class ConcreteProductB : public Product {
public:
    void operation() override {
        // 具体产品B的操作实现
    }
};
class SimpleFactory {
public:
    static Product* createProduct(const std::string& type) {
        if (type == "A") {
            return new ConcreteProductA();
        } else if (type == "B") {
            return new ConcreteProductB();
        } else {
            return nullptr; // 可以添加默认处理逻辑或抛出异常
        }
    }
};
```

#### 工厂方法模式

将工厂也抽象化，每个产品由其对应的具体工厂创建。这样的话，新增产品无需修改原工厂类，只需新增一个工厂子类。



### Command模式

也就是把请求封装为对象，比如我的blender项目中，工作节点会把主节点发来的任务都构造为一个TCB对象。

### State 模式

**状态模式**允许一个对象在其内部状态发生改变时，**行为也随之改变**，就好像这个对象换了一个类一样。所以，当一个对象的行为依赖于它的状态，并且它必须在运行时根据状态改变其行为时，使用状态模式是最合适的。

一般来说有下面三个角色

1. **Context（上下文/环境类）**

> 持有当前状态对象，并将请求委托给当前状态对象处理。

📌 职责：

- 维护一个 `State` 类型的实例；
- 提供设置状态的接口（`setState()`）；
- 对外暴露操作接口（`request()`），但不自己处理，而是转发给当前状态对象。

✅ 示例：

```
class Elevator {
    State* state;
public:
    void setState(State* s) { state = s; }
    void request() { state->handle(); }
};
```

✅ 2. **State（抽象状态类）**

> 定义所有状态类共有的接口，通常是一个或多个行为方法，如 `handle()`。

📌 职责：

- 声明接口，例如 `handle(Context*)`；
- 可根据需要将 `Context` 传入状态对象，支持状态内部切换。

✅ 示例：

```
class State {
public:
    virtual void handle(Context* ctx) = 0;
    virtual ~State() {}
};
```

✅ 3. **ConcreteState（具体状态类）**

> 实现 `State` 接口，定义在该状态下 `Context` 应该如何行为，并**决定是否切换状态**。

📌 职责：

- 实现 `handle()`，定义该状态下的操作；
- 可以调用 `ctx->setState()` 实现状态切换。

✅ 示例：

```
class OpenState : public State {
public:
    void handle(Context* ctx) override {
        std::cout << "开门状态，禁止运行\n";
        ctx->setState(new ClosedState());  // 状态转换
    }
};
```



## 类的大小

class的大小主要由这几点:

1. 非静态成员变量的大小（静态成员存储在全局区）
2. 内存对齐（详见内存对齐规则）
3. 虚指针（有虚函数的话，或者虚继承）



## atomic

c11引入。`std::atomic` 支持原子地读写变量，还支持原子加减、交换、比较并交换（CAS）、加载/存储等操作，适合构建无锁算法或优化临界区。

```c++
std::atomic<int> x = 0;
x.store(1);
int val = x.load();
x.fetch_add(1);
x.compare_exchange_strong(expected, desired); //CAS操作

std::atomic<int> a = 10;
int expected = 10;
int desired = 20;
if (a.compare_exchange_strong(expected, desired)) {
    // 成功将 a 从 10 改为 20
}
//compare_exchange_weak 允许因为硬件的原因而 偶发性失败，即使 expected == current，也可能返回 false。它适合在循环重试场景使用，因为性能更高
```

| 函数                      | 特点                               | 适用场景                           |
| ------------------------- | ---------------------------------- | ---------------------------------- |
| `compare_exchange_strong` | 失败仅因 `expected != current`     | 推荐用于单次尝试                   |
| `compare_exchange_weak`   | 可能会偶发失败（spurious failure） | 推荐用于循环重试（`while (!cas)`） |

### 内存序

`std::atomic` 支持不同的内存序语义，如 `memory_order_relaxed`、`acquire/release`、`seq_cst`。默认是最强的 `seq_cst`，提供全序一致性，但有时我们会选用更弱的内存序来优化性能。

内存序控制的是原子操作与其他普通操作的执行顺序。默认的 `seq_cst` 最强，适用于大多数并发场景。如果性能瓶颈严重，可以考虑 `acquire/release` 建立 happens-before 关系。只有在非常确定的非同步场景才会考虑 `relaxed`

| 内存序                 | 含义                                   | 用途                         |
| ---------------------- | -------------------------------------- | ---------------------------- |
| `memory_order_relaxed` | 最弱，**不保证同步和顺序性**，仅原子性 | 高性能计数器、非同步统计数据 |
| `memory_order_acquire` | 读屏障，**后续读取不能被提前**         | 保证读依赖不会乱序           |
| `memory_order_release` | 写屏障，**之前的写不能被延后**         | 保证写操作先于其他线程看到   |
| `memory_order_acq_rel` | 同时具备 acquire + release             | 读写合并操作（如 CAS）       |
| `memory_order_seq_cst` | 默认值，全序一致性，**最安全但最慢**   | 简单并发场景，推荐默认使用   |

### CAS的汇编指令

`CMPXCHG` .CAS 是通过 `CMPXCHG` 指令实现的，这条指令结合 `LOCK` 前缀实现原子性。它会比较目标地址的值与 EAX 寄存器中的值，如果相等则写入新值，否则将目标地址的值加载到 EAX。

```
mov eax, expected       ; 把期望值放入 EAX
mov ebx, desired        ; 把目标值放入 EBX
lock cmpxchg [x], ebx   ; 如果 [x] == EAX，就把 EBX 写入 [x]，否则把 [x] 加载进 EAX
```



## 内存对齐

在 C++ 中，**内存对齐是指变量或结构体成员在内存中的起始地址必须是其类型对齐大小的整数倍**。这是出于 CPU 访问效率的考虑

> 假设 CPU 总线一次能取 4 字节（32bit），如果一个 `int`（4字节）恰好落在 **4 的倍数地址**上，CPU 只需一次取数。
>  如果它落在地址 `0x0003` 上（非对齐），这个 int 会跨越两个总线周期（一次取低 1 字节 + 再取高 3 字节），CPU 要拼接两次，效率大幅下降。
>
> **对齐访问**：一次内存访问搞定。
>
> **未对齐访问**：可能需要 2 次或更多访问 + 硬件拼接。

一般结构体内是这样放的：成员按声明顺序放，编译器在成员间与末尾插入 padding 使整体保持最大成员的对齐倍数；所以 `sizeof` 往往大于所有成员之和。

> 我们说的最大对齐倍数就是，所有成员类型对齐值中的最大值。要知道一个类型的对齐值是多少可以用alignof

```c++
struct A {
    char a;     // 1 字节
    int b;      // 4 字节
    char c;     // 1 字节
};
```

默认对齐方式下，编译器会在 `a` 后面插入 3 个字节的 padding，使得 `b` 对齐到 4 字节边界

**使用 `#pragma pack(n)` 或 `alignas` 控制对齐**：

`alignas(n)` 可以显式指定变量或结构体的对齐方式。

## 内存泄漏

内存泄漏指的是，程序运行过程中一些不再使用的对象没有被正确释放，从而导致程序使用的内存不断增加

 ### 原因

+  忘记释放
+ 逻辑不当
+ 循环引用

### 怎么避免

+ 使用智能指针等RAII机制来管理对象

+ 正确捕获异常

### 怎么定位

+ clang-tidy

+ valgrind是一个

## new和malloc区别

+ 首先new是c++中的操作符，malloc是c的一个库函数。
+ new会先调用operator new，申请空间后，调用类型的构造函数，最后返回类型的指针，调用析构函数，再调用operator delete。
+ malloc是在堆上分配，new是在自由存储区分配(自由存储区可以是堆、全局/静态存储区等)
+ 如果malloc分配失败返回NULL，而new分配失败返回bad_alloc异常——如果不捕捉这个异常会直接异常退出。

## 面向对象三大特性

### **一、封装（Encapsulation）**

- **核心思想**：把数据和操作数据的方法绑定在一起，对外只暴露必要接口，隐藏内部实现细节。
- **关键点**：
  - C++ 用 `public / protected / private` 控制访问权限。
  - 好处：降低耦合、保证安全性、提高可维护性。
- **一句话总结**：封装就是“对外只说我要做什么，不说我是怎么做的”。

------

### **二、继承（Inheritance）**

- **核心思想**：子类复用父类的成员和方法，同时可以扩展或重写功能。
- **关键点**：
  - 支持 `public / protected / private` 继承方式。
  - 实现代码复用，建立层次化关系。
  - 但过度继承会导致耦合过高，通常**组合优于继承**。
- **一句话总结**：继承就是“在已有的基础上加东西”。

------

### **三、多态（Polymorphism）**

- **核心思想**：同一接口，不同对象，表现出不同的行为。
- **关键点**：
  - **编译期多态**：函数重载、运算符重载、模板。
  - **运行期多态**：虚函数 + 动态绑定，通过基类指针或引用调用子类方法。
  - 运行期多态需要虚函数表，调用有轻微开销。
- **一句话总结**：多态就是“让调用者不用关心具体对象，但行为能自动切换”。

## C实现c++面向对象

主要是要实现三大特性： **封装、继承、多态**

封装

封装是面向对象编程的一个核心概念，它允许我们将数据和操作这些数据的函数封装在一起。在C语言中，我们可以通过**结构体和函数指针**来实现封装。

+ 把数据成员封装在一个结构体中，然后使用函数指针把操作数据的函数和这个结构体绑定起来

```c++
typedef struct {
    int id;
    char name[50];
    void (*print)(struct Student*);
} Student;
void printStudent(Student* student) {
    printf("ID: %d, Name: %sn", student->id, student->name);
}
```

继承

继承是OOP的另一个重要特性，它允许我们创建一个新的结构体，该结构体可以继承另一个结构体的属性和方法。在C语言中，我们通过**嵌套结构体**来实现继承。

```c++
//比如这是一个基类
typedef struct {
    int id;
    char name[50];
} Person;
//定义一个继承了persion的结构体(在子类中定义一个基类的对象即可实现对父类的继承)
typedef struct {
    Person base;
    float gpa;
} Student;
//所以要访问基类元素
Student student;
student.base.id = 1;
```

多态

多态依然用函数指针来实现



## explicit

explicit关键词，能防止隐式转换的发生

```c++
class A {
    explicit A();
}
```

+ 像是几乎所有STL容器的构造函数都加了`explicit`，因为vector, map这样的数据结构都有单参数构造，如果不加explicit，就容易在赋值、初始化的时候发送隐式类型转换，引发逻辑错误。
+ 如果给默认构造函数加了explicit的话，就没有用`T obj = {}`这样隐式初始化，就必须用`T obj{}或者T obj()`



## extern和inline

### extern

c++程序执行到汇编的时候，会生成.o文件，然后链接阶段会被这些.o文件都组合成二进制可执行文件或者静态库动态库。

假如两个.o文件包含了相同的符号，链接的时候就会出现符号重定义

一种解决方法是，在任意的一个源代码中，声明`extern void func()`，表示承诺链接的时候只需要在其他文件去找就行。

### inline

第二个解决方法就是，使用inline修饰——它表示允许一个函数、变量在多个文件中重复定义。

inline修饰过的这些重复定义的符号，它们会生成软连接，在链接器链接的时候会找到这些同名的软链接，然后生成一份独一无二的函数定义。**但是使用这种方式的话就要求这多个定义都要相同，否则可能会报错或者随机选择这个作为唯一的定义。**





## 野指针与悬空指针

**野指针（Wild Pointer）**

- **定义**：指向一个**未知内存地址**的指针（未初始化，或者随意赋值）。
- **成因**：
  - 定义时没有初始化：`int* p; // 未初始化`
  - 指针变量指向一块不可预期的内存。
- **危害**：可能导致访问非法内存 → 程序崩溃或产生不可预期行为。
- **预防方法**：
  - 定义指针时及时初始化：`int* p = nullptr;`
  - 使用智能指针（`std::unique_ptr` / `std::shared_ptr`）。

**悬空指针（Dangling Pointer）**

- **定义**：指向一块**已经被释放或超出作用域**的内存的指针。
- **成因**：
  - `delete` 或 `free` 释放内存后，没有把指针置空。
  - 指向局部变量的指针，在函数返回后继续使用。
- **危害**：看似合法的指针，实际上已经指向无效区域，访问会出错。
- **预防方法**：
  - 内存释放后，将指针置为 `nullptr`。
  - 避免返回局部变量的地址。
  - 同样推荐使用智能指针管理生命周期。

## extern C

extern C 就是告诉编译器按照C语言的方式编译和链接函数或变量，以解决C++名称修饰带来的兼容性问题。

### 名称修饰问题

c++存在名称修饰问题：

+ C++支持函数重载，所以编译器会在编译时修改函数名，加入参数类型信息
+ c语言不支持重载，其函数名在编译后不会被修饰。

所以如果c++直接调用c语言的函数，链接时找不到匹配的名称，就会报`undefined reference`错误。

---

常用于：

+ c++调用c库（opencv, ffmpeg)
+ 封装C接口，提供给python,rust语言使用
+ 一些嵌入式开发

### 注意

+ `extern "C"` 只能用于全局作用域，不能嵌套在局部变量或者函数内部
+ 用#ifdef __cpluscplus,保证头文件可以在C与C++两端兼容
+ 不能修饰类，也不能修饰重载函数

## RVO

"返回值优化"（Return Value Optimization，简称RVO）是一种优化技术，可以消除创建的临时对象，这个临时对象用来保存函数的返回值。**这种技术允许编译器直接在将保存函数结果的内存位置中构造返回值，从而避免调用移动构造函数**。

注意：RVO只在构造类的时候出现，对类进行赋值的时候不会被使用例如

## mutable

1. 允许在const成员函数中或者const对象中修改该成员变量

```c++
private:
    mutable int count = 0;
public:
    void printA() const {
        std::cout << count++ << std::endl;
    }
```

2. 修改lambda捕获的变量。因为如果一般`[capture]()->void {}`的话，是不允许改变被捕获的值，只能通过`[&capture]`，但是这样更改后，外部被捕获的值也会改变。要是想要既不改变外部的值，又要在lambda函数体能修改这个被捕获值，就需要mutable

```c+=
auto b = [count]() mutable ->void  {
        count++;
        std::cout << count++ << std::endl;
    };
```



**适用于类的成员变量，常用于缓存、延迟计算、日志记录等场景**。

## static关键字的作用和常见的场景

1. **静态局部变量**：在函数内部声明的静态局部变量，其生命周期超过了函数的作用域。也就是说，当函数结束时，静态局部变量并不会被销毁，而是会保留其值，直到下一次函数调用时再次使用。这对于需要在**多次函数调用之间保留状态的场景**非常有用。

   ```cpp
   void count() {
       static int counter = 0;
       counter++;
       std::cout << counter << std::endl;
   }
   ```

2. **静态类成员变量**：在类中声明的**静态成员变量是所有该类的对象共享的**。也就是说，无论创建多少个对象，静态成员变量只有一个副本。这对于需要所有对象共享某个值的场景非常有用。

   ```cpp
   class MyClass {
   public:
       static int staticVar;
   };
   
   int MyClass::staticVar = 0;  // 静态成员变量需要在类外初始化
   ```

3. **静态类成员函数**：在类中声明的**静态成员函数也是所有该类的对象共享的**。**静态成员函数只能访问静态成员变量**，不能访问类的非静态成员变量。这对于需要不依赖于特定对象就能执行的函数非常有用。

   ```cpp
   class MyClass {
   public:
       static int staticVar;
       static void staticFunction() {
           // 只能访问静态成员变量
           std::cout << staticVar << std::endl;
       }
   };
   ```

4. **静态全局变量和函数**：在文件内部声明的静态全局变量和函数，**其作用域仅限于该文件**。这对于需要限制变量或函数的可见性，防止其他文件访问的场景非常有用。

```c++
static int staticVar = 0;  // 只能在当前文件中访问

static void staticFunction() {  // 只能在当前文件中访问
    std::cout << staticVar << std::endl;
}
```

> 所以在这种限制的情况下

## const用法

1. 修饰函数传参，`print(const string& s)`可以防止函数内修改变量
2. 修饰函数返回值,`const string Getstring()`
3. 修饰指针（左定值，右定指针）

const int * pv1 = &addr，这样是保护的是&addr里面的内容不能被修改，但是pv1还是可以指向其他的指针

int* const pv1 = &addr, addr里面的内容可以修改，但是pv1无法修改指向。**所以引用就可以当作指针常量，this指针也是指针常量**

5. 修饰引用,`const int& pv1 = x`,相当于是`const int* const pv1 =1`

### const 与define区别

+ `#define` 是 **预处理器指令**，用于定义宏，在编译前由预处理器进行 **文本替换**。`const` 是一个 **关键字**，用于声明一个变量为只读，仍参与类型检查和作用域规则。
+ `#define` 没有类型，纯粹是字符串替换，因此不会进行类型检查。`const` 有类型，编译器会进行类型检查，能避免许多错误。
+ `#define` 没有作用域概念，一经定义在整个文件中都生效。`const` 遵循 C++ 的作用域规则，可以在类中、命名空间中、函数中使用。

## constexpr

constexpr修饰的式编译器常量，值是在编译期计算并确定的，适用于常量表达式。

> 像const的含义主要是表示变量是只读的，其值不一定在编译时已知，可以进行运行时计算。

## 函数指针与指针函数

### 函数指针：

**定义**：指向函数的指针，可以通过它来调用函数。

- 本质：一个变量，存储的是“函数的入口地址”。
- 例子：

```
int add(int a, int b) {
    return a + b;
}

// 定义一个函数指针
int (*fp)(int, int);

// 让它指向 add
fp = add;

// 用函数指针调用
int result = fp(3, 4);   // 等价于 add(3, 4)
```

📌 **关键点**：**函数指针是指针**，只是指向函数。

### 指针函数

**定义**：返回值是“指针”的函数。

- 本质：一个函数，它的返回值类型是指针。
- 例子：

```
int* func(int x, int y);   // func 是一个函数，返回 int* 类型
```

调用：

```
int a = 10, b = 20;
int* p = func(a, b);  // p 接收一个 int* 类型的结果
```

📌 **关键点**：**指针函数是函数**，只是返回类型是指针。

## 大端小端

大端模式：是指**数据的高字节保存在内存的低地址**中，而数据的低字节保存在内存的高地址端

小端模式，是指**数据的高字节保存在内存的高地址**中，低位字节保存在在内存的低地址端

---

*可以使用union来检测本机是大端还是小端*

```c++
union test{
  uint32_t value;
  uint8_t bytes[4];
};

test t;
t.value=0x12345678;
if (t.bytes[0] == 0x12) std::cout << "大端" << std::endl;
if (t.bytes[0] == 0x78) std::cout << "小端" << std::endl;
```

*另一个方法就是指针*

我们可以把一个整数写入内存，然后用 `char*` 指针读第一个字节。若第一个字节是最低有效位（比如0x78），说明本机是小端；若是最高有效位（0x12），说明是大端。这种方法简单直接，几乎所有教材都会用。

```c++
unsigned int x = 0x12345678;     // 一个 4 字节整数
unsigned char *p = (unsigned char*)&x;

if (*p == 0x78) {
    std::cout << "Little Endian\n";   // 低地址存低位
} else if (*p == 0x12) {
    std::cout << "Big Endian\n";      // 低地址存高位
} else {
    std::cout << "Unknown Endian\n";  // 理论上很少见
}

return 0;
```



**网络字节序：**为了保证网络中不同计算机之间传输数据的时候，能统一数据的存储顺序，采用大端序。

```
htonl
htons
ntohl
ntols
```



## STL

C++ STL从广义来讲包括了三类:算法，容器和迭代器。

### allocator

stl容器都有一个allocator的模版参数，它使得我们可以自定义内存的分配方式。

### 容器总结

1. 序列容器 sequence containers

   - array: 固定大小数组。支持快速随机访问，不能添加或者删除元素
   - vector: 底层数据结构为数组 ，支持快速随机访问。
   - deque: 底层数据结构为一个中央控制器和多个缓冲区，支持首尾（中间不能）快速增删，也支持随机访问。
   - list: 底层数据结构为双向链表，支持快速增删。

2. 关联容器 associative containers

   （红黑树实现）

   - set
   - multiset
   - map
   - multimap

   (hash表实现）

   - hash_set
   - hash_multiset
   - hash_map
   - hash_multimap

3. 无序容器

   - unordered_map: 存储键值对 <key, value> 类型的元素，其中各个键值对键的值不允许重复，且该容器中存储的键值对是无序的。
   - unordered_multimap: 和 unordered_map 唯一的区别在于，该容器允许存储多个键相同的键值对。
   - unordered_set
   - unordered_multiset



### unordered_map

- 基于**哈希表**。
- 每个键值通过哈希函数映射到哈希表中的某个槽位（bucket）,同一个槽位可能会存储多个键值对,然后用开链法处理冲突（使用的是链表或者红黑树结构）

```
1.插入
计算哈希值 → 定位桶 i
在堆上用 allocator 配置一个 node，把 <key, value> 放进去
将其 next 指向当前桶头，更新桶数组[i] 指向新节点
2.查找 / erase
同样定位桶 i
顺着链表 next 依次比较 key_equal
3.rehash（扩容）
只重新分配更大的桶数组并重新分布“桶指针”，节点本身地址不变；因此迭代器仍有效。
```

什么时候触发rehash

+ 插入新元素的时候检查，size() > max_load_factor() × bucket_count()
+ reserve(n)
+ rehash(n),强制把桶数设为 **ꞵ ≥ n / max_load_factor()**

#### 迭代器和指针区别

迭代器实际上是对“遍历容器”这一操作进行了封装。迭代器不是指针，是类模板。重载了指针的一些操作符如：，->, * , ++, --等。

在编程中我们往往会用到各种各样的容器，但由于这些容器的底层实现各不相同，所以对他们进行遍历的方法也是不同的。例如，数组使用指针算数就可以遍历，但链表就要在不同节点直接进行跳转。

#### 什么时候会使得迭代器失效

插入一个元素后，只要没有调用rehash，也就不会重新散列，这样的话先前的迭代器依然有效。

> 一般当负载因子过大的时候就会进行rehash-->load_factor = 元素总数 / bucket 总数

#### map

map插入一个元素后，原来的迭代器依然有效。`std::map` 是基于**红黑树**实现的有序关联容器，红黑树是一种平衡二叉搜索树。红黑树节点的插入和删除不会改变其他节点的内存地址，而是通过旋转和重新链接来保持平衡。



### vector

vector底层是一个**动态数组**，包含三个迭代器:

+ start指向已经被使用的空间的头，finish指向已经被使用的空间的尾，end_of_storage是整块连续空间包括备用空间的尾部。

vector中的对象必须是能够复制与赋值的。不能使用引用作为vector元素，因为容器开辟的时候还没有初始化，而引用必须一开始就初始化。

#### vector内存增长机制

**当空间不够装下数据（vec.push_back(val)）时，会自动申请另一片更大的空间**，**然后把原来的数据拷贝到新的内存空间，接着释放原来的那片空间**。当释放或者删除（vec.clear()）里面的数据时，其存储空间不释放，仅仅是清空了里面的数据。

*对vector的任何操作一旦引起了空间的重新配置，指向原vector的所有迭代器会都失效了。*

#### vector中的reserve和resize的区别

+ reserve(n)是增加了vector的cap，但是size没有变，也不会真正创建对象，也就是说一个没有加入过元素的v,调用v.reserve(n)后，依然是空的不能用v[x]，智能用push_back添加了元素后，才能引用元素。
+ resize()，改变容器的cap与size，同时也会创建对象，因此就算是一个空的v,再resize后也可以进行引用。

#### vector在栈还是堆，能开10万个元素的vector吗，怎么扩容的，怎么开辟内存

```c++
vector<int*> *p = new vector<int*>
vector<int*> p
```

上面两个都是在堆上分配的。STL库中的容器虽没有经过这两个关键字创建，但同样是存放在堆区。容器均调用了`allocator`来创建。

#### emplace_back和push_back

**相同**

emplace_back和push_back都支持左值和右值的传入。

我们这里就说类元素，不说内置和基本类型了。

传入左值的时候，会调用拷贝构造函数构造出一个匿名对象，然后将该对象存储到vector中

传入右值的时候，调用的是两个函数的移动构造函数构。

**不同**

emplace_back 还支持另一种调用方式，原地构造（in-place construction）！

即emplace_back的参数是可变的，传入的参数可以是vector类型的构造函数的参数，直接原地构造

比如emplace_back(10, “test”)可以只调用一次constructor

而push_back(MyClass(10, “test”))中MyClass(10, “test”)调用了一次构造函数，同时值传递又调用拷贝构造函数。

#### vector作为函数返回值用法

在C++11中提供了RVO/NRVO机制可以防止这种重复拷贝开销，RVO机制使用父栈帧（或任意内存块）来分配返回值的空间，来避免对返回值的复制。也就是将Base fun();改为void fun(Base &x);。

#### vector如何释放空间

vector的内存占用空间只增不减，比如你首先分配了10,000个字节，然后erase掉后面9,999个，留下一个有效元素，但是内存占用仍为10,000个。

**所有的内存空间是在vector析构时候才能被系统回收。**

empty()用来检测容器是否为空的，clear()可以清空所有元素。但是即使clear()，vector所占用的内存空间依然如故，无法保证内存的回收。

如果需要空间动态缩小，可以考虑使用deque。如果vector，可以用swap()来帮助你释放内存。

```
vector(Vec).swap(Vec); //将Vec的内存清除； 
vector().swap(Vec); //清空Vec的内存；
```

## c++ 类型转换

### static_cast

static_cast 是c++提供的编译时静态类型转换，用于已知的安全的类型转转。也就是说你在编译的时候就知道你的类型转换时安全的

所以一般是基本的数据类型转换、基类与派生类之间的向上转换。

> 派生类向父类的向上转换一般不会有什么问题。但是父类转换为子类时，如果存在父类没有的成员，并且还访问它就会出现未定义的行为，这个是 **向下转换**会出现的问题。

### dynamic_cast

c++运行时转换，用于基类和派生类之间的转换，特别是用于安全的向下转换。如果基类没有虚函数，则dynamic_cast无法正确进行运行时检查，结果是UB

### const_cast

去除const/volatile限定符。

注意它是 **去除原本不是const变量的const属性的**

```c++
void modify(const char* str) {
  char *p = const_cast<char*>(str);
}
char str[] = "hello";
modify(str);
```

但是如果本身是const,如果使用const_cast修改会出现UB

```c++
const int a = 1;
int *p = const_cast<int*>(&a); //错误，a本身是const属性
```

### reinterpret_cast

只要保证地址数据，地址内存大小能对上，就能进行转换，比如指针和整数之间的转换。

**作用**：**位级别的强制转换**，重新解释内存二进制。

**常见用途**：

- 在不同指针类型之间转换（比如 `int* -> char*`）。
- 把指针转成整数/整数转回指针。
- 用于底层系统编程，操作硬件寄存器、网络字节流等。

**危险点**：非常不安全，容易破坏类型系统。

## function

在 C++ 中，`std::function` 是一个**通用的函数封装器**，可以存储、复制和调用任何可调用对象(函数、lambda、函数对象、成员函数等).

```c++
//基本语法
std::function<返回类型(参数类型...)> 函数变量;

 std::function<int(int, int)> add = [](int a, int b) {
        return a + b;
 };
```

function经常和bind一起封装回调函数。

**bind**：生成一个**可调用对象（callable）**，你可以把它赋值给 `std::function`

```c++
void greet(const std::string& name, int age) {
    std::cout << "Hello, " << name << "! You are " << age << " years old." << std::endl;
}

// 绑定前两个参数，顺序为 name, age
auto boundFunc = std::bind(greet, "Alice", 25);
std::function<void()> func = boundFunc;

auto add10 = std::bind(add, 10, std::placeholders::_1);//占位符
MyClass obj;
// 绑定成员函数：obj + 参数 a = 3，b = 占位
auto boundFunc = std::bind(&MyClass::printSum, &obj, 3, std::placeholders::_1);
```





## 右值引用

 区分左值和右值的方法主要是看能不能对表达式取地址，如果可以是左值，否则是右值。

对右值又可以分为纯右值与将亡值。

+ 纯右值： 像是非引用返回的临时变量，运算表达式产生的临时变量、lambda表达式，这些都是纯右值

+ 将亡值： std::move的返回值，T&&函数返回值。

**什么是右值引用**

- **定义**：右值引用是 C++11 引入的新类型声明，语法是 `T&&`。

- **本质**：它能**绑定到右值（临时对象）**，而普通的 `T&`（左值引用）只能绑定到左值。

- **例子**：

  ```
  int x = 10;
  int& lref = x;          // 左值引用
  int&& rref = 20;        // 右值引用，绑定到临时量 20
  // int&& bad = x;       // 错误，x 是左值
  ```

**为什么需要右值引用（解决了什么问题）**

> **核心：避免不必要的拷贝，提高性能**

1. **之前的问题**
    在 C++98/03 里，函数参数只能以值传递（拷贝）或 `const&` 传递。
   - 如果传的是临时对象，往往会产生**额外拷贝**。
   - 对于 `std::vector` 这样的容器，在扩容或返回值时可能触发大量拷贝。
2. **右值引用的引入**
    右值引用可以**区分临时对象**（右值）和已有对象（左值），从而允许编译器/开发者在处理右值时**直接窃取资源**，避免拷贝。

**右值引用的主要应用**

1. **移动语义（Move Semantics）**

   - 用 `T(T&&)` 构造函数 或 `T& operator=(T&&)`，可以实现“搬迁”而不是“拷贝”。

   - 核心思路：把右值里的资源“偷走”，留下一个有效但资源为空的对象。

   - 例子：

     ```
     struct Buffer {
         int* data;
         size_t size;
         Buffer(size_t n) : data(new int[n]), size(n) {}
         ~Buffer() { delete[] data; }
     
         // 移动构造
         Buffer(Buffer&& other) noexcept
             : data(other.data), size(other.size) {
             other.data = nullptr;
             other.size = 0;
         }
     };
     ```

     > 用右值引用避免了在拷贝构造里分配+拷贝一份数组。

---

之前也说过,T&&在发生自动类型推断的时候,是个未定的引用类型, 如果接受的是左值,那么就是一个左值;如果接受的是右值就是右值.但是这样没能按照参数本身的类型进行初始化.

### 完美转发

C++11 引入右值引用后，右值可以触发移动语义避免拷贝。但在模板函数里，右值一旦传给有名字的形参，就会退化成左值，失去右值特性。
 解决方法有两种：写重载，或者使用完美转发。重载方式代码量爆炸，而完美转发通过 `T&&`（转发引用）、引用折叠和 `std::forward`，就能在一个模板里保留参数原本的值类别。
 标准库里的 `emplace_back`、`make_unique`、`thread` 等都依赖完美转发。和 `std::move` 的区别在于：`move` 总是右值，而 `forward` 会根据类型推导选择左值或右值。



**完美转发**是指在C++中，**将一个参数以其原本的类型（左值或右值）完整地传递到另一个函数中**，而不改变它的值类别（value category）。
 通常用在**模板函数**中，以保证高效转发（避免不必要的拷贝或移动）。

实现完美转发需要用到两样东西：

1. **右值引用模板参数**：`T&&`
2. **`std::forward<T>(arg)`**：根据参数实际的值类别，决定是拷贝、移动，还是继续传递为引用。

C++11引入了完美转发：在函数模板中，完全依照模板的参数的类型（即保持参数的左值、右值特征），将参数传递给函数模板中调用的另外一个函数。C++11中的std::forward正是做这个事情的，他会按照参数的实际类型进行转发。看下面的例子：

```c++
void processValue(int& a){ cout << "lvalue" << endl; }
void processValue(int&& a){ cout << "rvalue" << endl; }
template <typename T>
void forwardValue(T&& val)
{
    processValue(std::forward<T>(val)); //照参数本来的类型进行转发。
}
void Testdelcl()
{
    int i = 0;
    forwardValue(i); //传入左值 
    forwardValue(0);//传入右值 
}
```

右值引用T&&是一个universal references，可以接受左值或者右值，正是这个特性让他适合作为一个参数的路由，然后再通过std::forward按照参数的实际类型去匹配对应的重载函数，最终实现完美转发。



## 多态机制的实现

### 静态多态

静态多态的方式是函数重载，它是由编译器确定的。其规则是：

1. 允许同一个作用域下声明多个同名的函数
2. 这些函数的参数列表、参数个数或者参数顺序不一样
3. 不能通过返回值不同来区别重载

**具体原理**

```
主要是通过函数名修饰，会给函数名加上一些修饰

1.预编译， 把头文件中的函数声明拷贝到源文件，以免编译的时候找不到函数定义
2.编译，语法分析
3.汇编，会生成函数名到函数地址的映射
4.链接，将多个文件的符号表汇总
```

### 动态多态

动态多态是依赖虚函数重写的。

### 动态多态

## 完美转发

在c++11之前，泛型函数在传递参数的时候无法保持参数的原始类型（左值或右值），导致额外的拷贝和移动，完美转发能保持参数的原始特征，避免额外开销。

**完美转发**就是在 **泛型函数**中，以参数的原始形式（左值或右值）传递给目标函数，从而避免不必要的拷贝或移动操作。

```c++
void process(int & x) {std::cout << "lvalue:" << x << std::endl;}
void process(int&& x) {std::cout << "rvalue:" << x << std::endl;}


template <typename T>
void forwardExample(T&& arg) {
    process(std::forward<T>(arg)); //std::forward保持数据类型
}

int main()
{
    int a = 1;
    forwardExample(a);
    forwardExample(1);
}
```

forward会进行引用折叠和类型推到来决定参数是否应该保留右值的特性。



 

## volatile 关键字

```c++
volatile int i = 10; 
```

* volatile 关键字是一种类型修饰符，用它声明的类型变量表示可以被某些编译器未知的因素（操作系统、硬件、其它线程等）更改。所以使用 volatile 告诉编译器不应对这样的对象进行优化。
* volatile 关键字声明的变量，**每次访问时都必须从内存中取出值**（没有被 volatile 修饰的变量，可能由于编译器的优化，从 CPU 寄存器中取值）
* const 可以是 volatile （如只读的状态寄存器）
* 指针可以是 volatile

总结来说，`volatile`变量修饰符就是告诉编译器不要对该变量作优化

> 常见的场景比如硬件寄存器访问: **当程序需要直接访问硬件设备的寄存器（如嵌入式开发中），这些寄存器的值可能会被外部硬件修改。在这种情况下，`volatile` 确保每次都读取最新值。**
>
> ***注意，volatile依然不能代替线程同步，它只是保证了变量的值是从内存中读取，但是不能保证操作是原子的。***





## 哈希表解决冲突

### 链地址法

每个哈希桶中存储的不仅仅是一个单值，而是一个链表，当发送冲突的时候，具有相同哈希值的键值对存储在一个链表里面：

处理过程

1. 哈希函数计算键的哈希值，找到对应的桶。
2. 如果桶空，则直接将数据插入该桶。
3. 如果桶已占用，则将冲突的数据以链表节点的形式添加到对应的链表或树中。

在现代方法 中，一般使用更复杂的数据结构来代替单纯的链表，比如`unordered_map`就是用的红黑树。

### 开放地址法

开放地址法的思路是，当哈希表发送冲突的时候，尝试从表中找到一个空的桶来储存。

**线性探查 (Linear Probing):**

- 按固定步长（比如+1）寻找下一个空桶。
- 当插入时，如果哈希桶已被占用，则依次检查之后的桶，直到找到一个空桶。
- 查找时，同样按照插入时的方式探查，找到对应数据。

**二次探查 (Quadratic Probing):**

- 按二次方步长（+k²，其中 k 是探查次数）寻找下一个空桶。
- 比线性探查更分散，能避免多次冲突时的聚集现象（称为“主群”问题）。

**双重哈希 (Double Hashing):**

- 使用两个不同的哈希函数 `h1(key)` 和 `h2(key)`。
- 当冲突发生时，用第二个哈希函数计算步长，从而找到下一个空桶。
- 比线性和二次探查更灵活，无主群问题。

### 总结

冲突比较少的时候，链地址法性能很好；如果冲突多且哈希表空间有限，开放地址法是合适的。

## 静态链接、动态链接

### 编译过程

C++编译过程通常包括四个主要步骤：预处理、编译、汇编和链接。

1. **预处理(Preprocessing)**：这是编译过程的第一步。预处理器接收源代码文件并处理它们。预处理器会处理源代码中的预处理指令，如`#include`，`#define`和`#ifdef`等。例如，`#include`指令会导致预处理器将相关文件的内容直接插入到指令的位置。预处理器还会处理条件编译指令和宏。

2. **编译(Compilation)**：编译器接收预处理后的文件，并进行词法分析、语法分析、语义分析和优化等步骤，然后生成相应的汇编代码。这个阶段会检查代码的语法错误，并进行一些优化。

3. **汇编(Assembly)**：汇编器将编译器生成的汇编代码转换为机器语言代码，也就是目标文件。这个过程基本上是一对一的转换，将汇编指令转换为机器指令。

4. **链接(Linking)**：链接器将一个或多个目标文件以及库合并到一起，生成一个可执行文件。链接器会解决符号引用问题，例如当你在一个文件中调用另一个文件中定义的函数时，链接器会找到这个函数并将调用指向它。链接器还会将需要的库文件链接到可执行文件中。

**如果头文件定义了函数，源文件不实现，会在哪个环节报错？如果构建的是静态库，会报错吗，为什么？**

```c++
如果头文件中定义了函数，但是在源文件中没有实现，那么在链接阶段（Linking）会报错。链接器会试图找到所有函数的实现，如果找不到，就会报出 "undefined reference" 的错误。
  
  如果你正在构建静态库，那么在构建静态库的过程中并不会报错。静态库是一组预编译的目标文件的集合，它们在库创建时并不需要全部解析所有的符号引用。只有在这个静态库被链接到某个具体的应用程序时，链接器才会尝试解析所有的符号引用，如果找不到某个函数的实现，就会报错。
	这是因为静态库的创建只是将一组目标文件打包到一起，而并不进行链接。只有在链接阶段，链接器才会检查所有的函数和变量是否都有对应的定义。所以，如果你的静态库中缺少了某个函数的实现，那么在创建静态库的时候并不会报错，只有在后续链接静态库的时候才会报错。
  
 如果你正在构建动态库（也称为共享库或DLL），并且头文件中定义了函数，但源文件没有实现，那么在动态库的构建过程中就会报错。
动态库不同于静态库，它在构建时进行了部分链接，所有的未解析的符号（如未实现的函数）都需要在构建动态库时解析。这是因为动态库在运行时被加载到程序中，所以它需要在构建时解析所有的符号引用，以确定需要的运行时重定位。
因此，如果你的动态库中缺少了某个函数的实现，那么在构建动态库的时候就会报错，提示 "undefined reference"。
```

**静态库**：
静态库是一种包含了预编译的目标代码的文件，它在编译时被链接到程序中。当你编译一个使用了静态库的程序时，静态库中的代码会被复制到最终的可执行文件中。这意味着，如果有多个程序使用了同一个静态库，那么这个库的代码会在每一个程序中都有一份拷贝。静态库的优点是它简化了程序的部署，因为所有的代码都包含在单个可执行文件中。然而，它的缺点是可能会导致程序体积变大，且如果库的代码更新，所有使用该库的程序都需要重新编译和链接。

**动态库**：
动态库（也被称为共享库或DLL）和静态库不同，它在程序运行时被加载到内存中，而不是在编译时被链接到程序中。这意味着，如果有多个程序使用了同一个动态库，那么这个库的代码在内存中只需要有一份拷贝。动态库的优点是它可以减小程序的体积，并允许多个程序共享同一份库代码，节省内存。此外，如果库的代码更新，只需要替换库文件，而不需要重新编译和链接使用该库的程序。然而，它的缺点是需要确保在程序运行的环境中有相应的库文件，否则程序无法运行。



1. 静态链接

在编译时，所有**库代码（.a 或 .lib）**都会被复制到最终的可执行文件中。

可执行文件包含了所有依赖的库，不需要额外的库文件。

2. 动态链接

在编译时，程序只会记录库的符号信息，真正的库代码不会被包含。

在程序运行时，**动态库（.so 或 .dll）**会被操作系统加载，程序再去查找和调用库函数。

```C++
ldd my_program //查看动态库依赖
```

```c++
g++ main.cpp -o program -static（静态）
g++ main.cpp -o program -ldl（动态）
```

```c++
//假设我们有一个math.h, math.cpp
//先用它构建静态库
g++ -c math.cpp //-c表示只编译不链接
ar rcs libmath.a math.o //生成一个静态库
g++ main.cpp -L. -lmath -o main//-L.表示在当前文件目录搜索
---
//构建动态库
g++ -fPIC -c math.cpp//-fPIC 表示生成位置无关的代码
g++ -shared -o libmath.so math.o//生成动态库
    
```





##s

## 指针和引用的区别

1. 指针是一个变量，它有自己的内存地址，其内容是它指向的对象的地址。而引用是一个别名，它不需要
2. 指针不必一定要初始化，引用必须要初始化
3. Sizeof(指针)返回的是指针本身的大小，sizeof(引用)返回的是引用对象的大小。
4. 引用还必须绑定到一个合法的内存地址上，不能是未初始化的指针。

所以如果是优化前的编译器（比如-g命令）可以看到符号表中，引用在符号表中对应记录的内存地址和被应用变量是同一个地址；而指针的话就是指针变量的内存地址。（比如编译器开了O2这样的优化就看不到了）。

指针和引用都是一种内存地址的概念，区别呢，**指针是一个实体，引用只是一个别名**。在程序编译的时候，**将指针和引用添加到符号表中。指针它指向一块内存，指针的内容是所指向的内存的地址，在编译的时候，则是将“指针变量名-指针变量的地址”添加到符号表中**，所以说，指针包含的内容是可以改变的，允许拷⻉和赋值，有 const 和非 const 区别，甚至可以为空，**sizeof 指针得到的是指针类型的大小**。

**而对于引用来说，它只是一块内存的别名，在添加到符号表的时候，是将"引用变量名-引用对象的地址"添加到符号表中**，符号表一经完成不能改变，所以引用必须而且只能在定义时被绑定到一块内存上，后续不能更改，也不能为空，也没有 const 和非 const 区别。**sizeof 引用得到代表对象的大小**。而 sizeof 指针得到的是指针本身的大小。另外在参数传递中，指针需要被解引用后才可以对对象进行操作，而直接对引用进行的修改会直接作用到引用对象上。

-----

### 函数中传引用和指针的区别

指针和引用都可以传递变量地址，让函数内部修改外部变量。引用更安全、更简洁，但不能为 null、不能重新绑定，通常用于函数参数或操作符重载。指针更灵活，能用于动态分配、数据结构等，但更容易出错。

## 虚函数

虚函数是 C++ 实现运行时多态的关键机制。当基类中的成员函数被声明为 `virtual`，并被子类重写时，通过基类指针或引用调用函数时，会在运行时动态绑定调用的函数版本。

C++ 使用 **虚函数表（vtable）** 来实现动态多态。每个包含虚函数的类在编译时会生成一张 vtable，表中保存该类所有虚函数的地址。每个对象实例中会有一个指向 vtable 的指针（通常叫 vptr），在调用虚函数时，程序通过 vptr 找到对应函数并调用。

当通过基类指针调用一个虚函数时，程序会先通过对象的 vptr 找到它的 vtable，再在 vtable 中查找对应函数的地址，最后通过该地址跳转到函数体执行。这一过程发生在运行时，从而实现了基于实际对象类型的动态分发

### 纯虚函数

**纯虚函数**是没有实现的虚函数，在基类中用 `= 0` 来表示，是实现 **抽象接口** 的关键。

+ 强制派生类必须重写该函数；

+ **含有纯虚函数的类称为抽象类，不能实例化**；

+ 用来定义接口/规范，类似 Java 中的 interface。

### 虚继承

虚继承是 C++ 为了解决**多重继承中“菱形继承”导致的重复继承问题**而引入的一种继承方式。

```c++
class A { public: int x; };
class B : virtual public A {};
class C : virtual public A {};
class D : public B, public C {};  // 如果不虚继承，D 会有两份 A::x！
```

虚继承让 **最上层的基类（A）只保留一份子对象**；

编译器会让 `D` 只继承一份 `A`，并通过指针修正来避免歧义；

虚继承内部通过 **虚基表（vtable）和指针偏移**来访问唯一的 A。

### 构造/析构函数能否是虚函数

构造函数不能是虚函数，因为虚函数调用的时候需要虚函数表，但是构造函数是在虚函数表建立之前调用的。

### 构造函数在执行过程中能不能调用虚函数



## NULL 与nullptr

NULL来自C语言，一般由宏定义实现，而 nullptr 则是C++11的新增关键字。C语言中，NULL是`(void*)0`，C++中NULL是`0`，编译器一般定义是：

```c++
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
```

主要是NULL在c++中是0的话就会无法区分整数0，特别是在函数重载的情况

```c++
void fun(char* p) {
 cout << "char*" << endl;
}
void fun(int p) {
 cout << "int" << endl;
}
int main()
{
 fun(NULL);
 return 0;
}
//输出结果：int
```

而nullptr是明确的nullptr_t类型。



## 内存池和线程池这样的池化技术有了解吗？它们的原理和使用场景

首先，**池化技术**的核心思想都是「资源重用」，通过提前分配资源并在用完后归还的方式来避免重复的资源分配和释放操作。——*比如线程池就是预先分配一些线程用来处理任务，内存池就是提前分配一块较大的内存，当有需要的时候直接从池子中去。*

### 内存池

内存池是一种优化内存管理的机制，它提前分配一块较大的内存空间（或者实例化一批对象），并将其分成若干小块以备程序运行时动态使用，避免频繁的内存分配和回收操作（如`malloc`/`free`或`new`/`delete`）。当需要内存时直接从池中分配，当不再需要时归还池中供后续复用。

内存池的操作主要分为以下几个步骤：

1. **初始化阶段（池的创建）：**
   - 预先分配一块较大的内存。
   - 将内存分割成一系列固定大小的**内存块（Block）**，并组织成一个结构（如链表）。
   - 空闲的内存块会按照一定的策略（如链表、栈等）进行管理。
2. **分配（Allocate）：**
   - 当请求内存时，从已经分配的空闲内存块中取出一个（比如取链表头部或栈顶的块）。
   - 如果内存池中的块已经耗尽，则可以动态扩展池的大小（有些实现可能不支持扩展）。
3. **释放（Free）：**
   - 将释放的内存块重新放回内存池中的空闲块列表中，供下次复用。
4. **销毁阶段：**
   - 当内存池不再使用时，统一释放所有的内存块。

*内存池这种管理已经分配的内存块，不直接调用malloc/free，实现了高效的多次分配和释放。*

**使用场景**

- **频繁申请和释放小对象的场景**：比如游戏场景中的粒子对象、数据库连接存储结构等。
- **实时性要求高的场景**：减少调用`malloc`和`free`带来的系统开销，提高实时性。
- **固定大小对象**：当分配的对象大小固定时，内存池能够更高效。

```c++
#include <iostream>
#include <vector>

class MemoryPool {
private:
    struct Block {
        Block* next;  // 指向下一个空闲块
    };

    Block* freeList; // 空闲链表的头指针
    std::vector<void*> chunks; // 记录已分配的内存块（方便释放）
    size_t blockSize;  // 每个对象的大小
    size_t chunkSize;  // 每次分配多少个对象
public:
    MemoryPool(size_t blockSize, size_t chunkSize = 32)
        : freeList(nullptr), blockSize(blockSize), chunkSize(chunkSize) {}

    ~MemoryPool() {
        // 释放所有分配的内存
        for (void* chunk : chunks) {
            free(chunk);
        }
    }

    void* allocate() {
        if (!freeList) {
            allocateChunk();
        }
        Block* block = freeList;
        freeList = freeList->next; // 从空闲链表取出一个块
        return block;
    }

    void deallocate(void* ptr) {
        Block* block = static_cast<Block*>(ptr);
        block->next = freeList;
        freeList = block; // 回收内存块到空闲链表
    }

private:
    void allocateChunk() {
        size_t chunkBytes = blockSize * chunkSize;
        void* chunk = malloc(chunkBytes);
        chunks.push_back(chunk);

        // 将新分配的 chunk 分割成小块，并链接到 freeList
      //转换为char*是因为原始的void*不支持指针偏移操作，而char*适合字节偏移操作
        char* start = static_cast<char*>(chunk);
        for (size_t i = 0; i < chunkSize; ++i) {
            Block* block = reinterpret_cast<Block*>(start + i * blockSize);
            block->next = freeList;
            freeList = block;
        }
    }
};

// 示例使用
struct TestObject {
    int x, y, z;
};

int main() {
    MemoryPool pool(sizeof(TestObject), 10);

    TestObject* obj1 = static_cast<TestObject*>(pool.allocate());
    obj1->x = 10; obj1->y = 20; obj1->z = 30;

    std::cout << "Allocated object: " << obj1->x << ", " << obj1->y << ", " << obj1->z << std::endl;

    pool.deallocate(obj1);

    return 0;
}

```



### 线程池

线程池是为了解决线程的频繁创建和销毁开销问题。线程池预先创建和维护一定数量的线程（通常是若干空闲线程），当需要线程处理任务时，从池中获取线程执行任务，任务完成后线程被归还池中继续等待下一个任务。适用于像后台任务调度系统。

**工作原理**

线程池的整体流程如下：

1. **初始化阶段：**
   - 创建若干固定数量的线程（称为工作线程）。
   - 工作线程处于**空闲状态**，等待处理任务。
2. **任务提交：**
   - 主线程将任务提交给线程池，并将任务放入**任务队列**中。
   - 空闲线程会从任务队列中取出任务并执行。
   - 如果任务数量超过线程池的线程容量，则任务会被阻塞或存储在队列中等待处理。
3. **线程归还：**
   - 线程执行完任务后不会退出，而是重新进入空闲状态并准备接收下一个任务。
4. **关闭或销毁阶段：**
   - 当需要销毁线程池时，所有正在执行的任务会被强行退出，线程资源被释放。

```c++
#pragma once

#include <vector>
#include <thread>
#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace blendersim
{

class ThreadPool
{
public:
    ThreadPool(int n)
    {
        _runing = true;
        for(int i = 0; i < n; ++i)
        {
            _thread_pool.emplace_back(std::thread(&ThreadPool::process, this));
        }
    }

    void AddTask(const std::function<void(void)>& task)
    {
        {
            std::lock_guard lk(_m);
            _task_queue.emplace_back(task);
        }
        _cv.notify_one();
    }

    ~ThreadPool()
    {
        _runing = false;
        _cv.notify_all(); // Notify all threads to exit.
        for(int i = 0; i < _thread_pool.size(); ++i)
        {
            if(_thread_pool[i].joinable())
                _thread_pool[i].join();
        }
    }
private:
    void process()
    {
        while(_runing)
        {
            std::unique_lock lk(_m);
            _cv.wait(lk, [this] { return !_runing || !_task_queue.empty(); });
            if(!_runing && _task_queue.empty())
                break;
            auto func = std::move(_task_queue.front());
            _task_queue.pop_front();
            lk.unlock();
            func();
        }
    }
    std::deque<std::function<void(void)>> _task_queue; //Task queue
    std::vector<std::thread> _thread_pool;  // Save subthreads
    std::mutex _m;
    std::condition_variable _cv;
    std::atomic<bool> _runing;
};

} // namespace blendersim

```

### 线程的数量如何确定

我们调整线程池中的线程数量的最主要的目的是为了充分并合理地使用 CPU 和内存等资源，从而最大限度地提高程序的性能。

所以一般会针对任务类型来设计不同的策略，比如：

**CPU密集型任务**

这类任务会执行很多计算，比如加密、解密、压缩等需要耗费大量CPU资源。CPU密集型任务最佳的线程数为 CPU 核心数的 1~2 倍。如果设置过多的线程数，实际上并不会起到很好的效果。

假设我们设置的线程数量是 CPU 核心数的 2 倍以上，因为计算任务非常重，会占用大量的 CPU 资源，所以这时 CPU 的每个核心工作基本都是满负荷的。

我们又设置了过多的线程，每个线程都想去利用 CPU 资源来执行自己的任务，这就会造成不必要的上下文切换，此时线程数的增多并没有让性能提升，反而由于线程数量过多会导致性能下降。

**IO密集型**

比如数据库、文件的读写，网络通信等任务。并不会特别消耗 CPU 资源，但是 IO 操作很耗时，总体会占用比较多的时间。IO密集型任务最大线程数一般会大于 CPU 核心数很多倍，因为 IO 读写速度相比于 CPU 的速度而言是比较慢的，如果我们设置过少的线程数，就可能导致 CPU 资源的浪费。

如果我们设置更多的线程数，那么当一部分线程正在等待 IO 的时候，它们此时并不需要 CPU 来计算，那么另外的线程便可以利用 CPU 去执行其他的任务，互不影响，这样的话在工作队列中等待的任务就会减少，可以更好地利用资源。

《Java并发编程实战》的作者 Brain Goetz 推荐的计算方法：**线程数 = CPU 核心数 \*（1+平均等待时间/平均工作时间）**

## auto关键字

C++11标准赋予了auto新的含义：

- 声明变量时根据初始化表达式**自动推断该变量的类型**。
- 声明函数时**函数返回值的占位符**。

### 尽可能的使用auto关键字

1. 声明局部变量时使用auto

   ```c++
   auto i = 42;		//int
   auto d = 42.5;		//double
   auto s = "text";	//const char*
   auto v = {1,2,3};	//std::initializer_list<int>
   ```

2. lambda表达式

   ```c++
   auto upper = [](const char c) { return toupper(c); }
   ```

3. 函数返回值（存在多个return语句时，返回类型必须完全相同）

   ```c++
   template<typename F, typename T>
   auto apply(F&& f, T value)
   {
       return f(value);
   }
   ```

使用auto的优点：

1. 避免未初始化的变量

2. 不会出现隐式转换

   ```c++
   auto v = std::vector<int>{1,2,3};
   int size1 = v.size(); //存在隐式转换,可能会导致数据丢失
   auto size2 = v.size(); //这里的size2不存在隐式转换，为size_t类型，不会出现数据丢失
   ```

### 使用auto的一些注意事项

auto的一些缺点：

1. 无法使用const/volatile修饰，甚至**引用也可能无法正常推断**。

   ```c++
   struct Foo
   {
       int x_;
       int& getX() { return x_; }
   };
   
   auto x = getX(); //这里的x变量不会被识别成引用，修改x不会改变foo中x_d值
   int& x = getx(); //这样就是引用
   ```

3. auto不能用于多单词关键字

   ```c++
   auto l1 = long long{42}; //error
   auto l2 = llong{42};     //ok
   ```

4. 对于问题1的解决方案使用**decltype**运算符

   ```c++
   struct Foo
   {
       int x_;
       int& getX() { return x_; }
   };
   
   decltype(auto) x = getX(); //x将会是引用类型
   ```

5. 当用一个auto关键字声明多个变量的时候，编译器遵从由左 往右的推导规则，以最左边的表达式推断auto的具体类型

   ```c++
   int n = 5;
   auto *pn = &n, m = 10;
   //auto *pn = &n, m = 10.0; //编译失败
   ```

   这里先根据`&n`确定auto的类型为int。进而m的类型为int

6. 当使用条件表达式初始化auto声明的变量时，编译器总是使 用表达能力更强的类型：

   ```c++
   auto i = true ? 5 : 8.0; // i的数据类型为double
   ```

7. 按照C++20之前的标准，无法在函数形参列表中使用auto声 明形参（注意，在C++14中，auto可以为lambda表达式声明形参）

   ```c++
   void echo(auto str) {…} 							// C++20之前编译失败，C++20编译成功
   auto add_lambda = [](const auto a, const auto b) 	//c++14开始
   {
       return a + b;
   };
   ```



### auto推导规则

​		如果**auto声明的变量是按值初始化，则推导出的类型会忽略 cv限定符**。进一步解释为，在使用auto声明变量时，既没有使用引用，也没有使用指针，那么编译器在推导的时候会忽略const和 volatile限定符。

```c++
const int i = 5;
auto j = i; // auto推导类型为int，而非const int
auto &m = i; // auto推导类型为const int，m推导类型为const int&
auto *k = i; // auto推导类型为const int，k推导类型为const int*
const auto n = j; // auto推导类型为int，n的类型为const int
```

上述规则作用于多态时的情况

```c++
class Base {
    public:
        virtual void f()
        {
            std::cout << "Base::f()" << std::endl;
        };
};

class Derived : public Base {
    public:
        virtual void f() override
        {
            std::cout << "Derived::f()" << std::endl;
        };
};


int main(int argc, char** argv) {
    Base* d = new Derived();
    auto b = *d;	//这里为直接赋值，因此b的类型为Base
    auto& b = *d;	//这里b的类型为Derived
    b.f();
    return 0;
}
```

​	使用auto声明变量初始化时，**目标对象如果是引用，则引用属性会被忽略**

```c++
int i = 5;
int &j = i;
auto m = j; // auto推导类型为int，而非int&
```

另外如果是一个右值(&&),auto也不会推断

```c++
int&& getx();

auto ax = getx(); //ax是int，而不是int&&，如果要得到int&&类型，使用decltype(auto)
//注意，即使是auto*/auto& 也只是推断的时候保留*与&的性质，而对于右值&&并不会保留。
```



## decltype关键字

decltype的推导规则：

1. 如果e是一个未加括号的**标识符表达式**（结构化绑定除外）或 者未加括号的**类成员访问**，则decltype(e)推断出的类型是e的类型 T。如果并不存在这样的类型，或者e是一组重载函数，则无法进行推导。 
2. 如果e是一个函数调用或者仿函数调用，那么decltype(e) 推断出的类型是其返回值的类型。 
3. 如果e是一个类型为T的左值，则decltype(e)是T&。
4. 如果e是一个类型为T的将亡值，则decltype(e)是T&&。 
5. 除去以上情况，则decltype(e)是T

一些推导的例子：

```c++
const int&& foo();
int i;
struct A {
	double x;
};
const A* a = new A();

decltype(foo()); // decltype(foo())推导类型为const int&&（条件2）
decltype(i); // decltype(i)推导类型为int（条件1）
decltype(a->x); // decltype(a->x)推导类型为double（条件1）
decltype((a->x)); // decltype((a->x))推导类型为const double&
```

注意最后一种情况：加了括号不满足条件1，而x是一个左值因此推导出`double &`，又因为a有const限定符，因此类型推导为：`const double&`



### decltype推导示例

```c++
int i;
int *j;
int n[10];
decltype(i=0); // decltype(i=0)推导类型为int&
decltype(0,i); // decltype(0,i)推导类型为int&
decltype(i,0); // decltype(i,0)推导类型为int
decltype(n[5]); // decltype(n[5])推导类型为int&
decltype(*j); // decltype(*j)推导类型为int&
decltype(static_cast<int&&>(i)); // decltype(static_cast<int&&>
(i))推导类
// 型为int&&
decltype(i++); // decltype(i++)推导类型为int
decltype(++i); // decltype(++i)推导类型为int&
decltype("hello world"); // decltype("hello world")推导类型为
// const char(&)[12]
```

1．可以确认以上例子中的表达式都不是标识符表达式，这样就排 除了规则1。 

2．i=0和0,i表达式都返回左值i，所以推导类型为int&。 

3．i,0表达式返回0，所以推导类型为int。 

4．n[5]返回的是数组n中的第6个元素，也是左值，所以推导类 型为int&。 

5．*j很明显也是一个左值，所以推导类型也为int&。 

6．static_cast(i)被转换为一个将亡值类型，所以 其推导类型为int&&。 

7．**i++和++i分别返回右值和左值，所以推导类型分别为int和 int&**。 

​	i++返回右值的原因是因为**后置++操作中编译器首先会生成 一份x值的临时复制**，然后才对x递增，最后返回临时复制内容。

8．hello world是一个常量数组的左值，其推导类型为const char(&)[12]。



### decltype的CV限定符推导

通常情况下，decltype(e)所推导的类型会同步e的cv限定符， 比如：

```c++
const int i = 0;
decltype(i); // decltype(i)推导类型为const int
```

当e是未加括号的成员变量时，父对象表达式的cv限定符会被忽略，不能同步到推导结果：

```c++
struct A {
double x;
};
const A* a = new A();
decltype(a->x); // decltype(a->x)推导类型为double, const属性被忽略
```

如果需要推导为`const double`

```c++
decltype((a->x))
```



### decltype(auto)

​		在C++14标准中出现了decltype和auto两个关键字的结合体： `decltype(auto)`  。它的作用简单来说，就是告诉编译器**用 decltype的推导表达式规则来推导auto**。另外需要注意的是， **decltype(auto)必须单独声明**，也就是它不能结合指针、引用以及 cv限定符。

一些例子：

```c++
int i;
int&& f();
auto x1a = i; // x1a推导类型为int
decltype(auto) x1d = i; // x1d推导类型为int
auto x2a = (i); // x2a推导类型为int
decltype(auto) x2d = (i); // x2d推导类型为int&
auto x3a = f(); // x3a推导类型为int
decltype(auto) x3d = f(); // x3d推导类型为int&&
auto x4a = { 1, 2 }; // x4a推导类型为std::initializer_list<int>
decltype(auto) x4d = { 1, 2 }; // 编译失败, {1, 2}不是表达式
auto *x5a = &i; // x5a推导类型为int*
decltype(auto)*x5d = &i; // 编译失败，decltype(auto)必须单独声明
```



## inline说明符

​	inline 说明符，在用于函数的声明说明符序列时，将函数声明为一个 *内联（inline）函数*。完全在 `class/struct/union`的定义之内定义的函数，无论它是成员函数还是非成员 friend 函数，均为隐式的内联函数。

### 特点

* 相当于宏，却比宏多了类型检查，真正具有函数特性；
* 编译器一般不内联包含循环、递归、switch 等复杂操作的内联函数；
* 在类声明中定义的函数，**除了虚函数的其他函数**都会自动隐式地当成内联函数。



### 关于虚函数的内联性

* **虚函数可以是内联函数，内联是可以修饰虚函数的**，但是**当虚函数表现多态性的时候不能内联**。

* 内联是在编译期建议编译器内联，而虚函数的多态性在运行期，编译器无法知道运行期调用哪个代码，因此虚函数表现为多态性时（运行期）不可以内联。

* `inline virtual` 唯一可以内联的时候是：编译器知道所调用的对象是哪个类（如 `Base::who()`），这只有在编译器具有实际对象而不是对象的指针或引用时才会发生。

  ```c++
  #include <iostream>  
  using namespace std;
  class Base
  {
  public:
  	inline virtual void who()
  	{
  		cout << "I am Base\n";
  	}
  	virtual ~Base() {}
  };
  class Derived : public Base
  {
  public:
  	inline void who()  // 不写inline时隐式内联
  	{
  		cout << "I am Derived\n";
  	}
  };
  
  int main()
  {
  	// 此处的虚函数 who()，是通过类（Base）的具体对象（b）来调用的，编译期间就能确定了，所以它可以是内联的，但最终是否内联取决于编译器。 
  	Base b;
  	b.who();
  
  	// 此处的虚函数是通过指针调用的，呈现多态性，需要在运行时期间才能确定，所以不能为内联。  
  	Base *ptr = new Derived();
  	ptr->who();
  
  	// 因为Base有虚析构函数（virtual ~Base() {}），所以 delete 时，会先调用派生类（Derived）析构函数，再调用基类（Base）析构函数，防止内存泄漏。
  	delete ptr;
  	ptr = nullptr;
  
  	system("pause");
  	return 0;
  } 
  ```

  

### C++17对inline的扩展

在C++17标准之前，定义类的非常量静态成员变量是一件让人头 痛的事情，因为变量的声明和定义必须分开进行：

```c++
#include <iostream>
#include <string>
class X {
public:
	static std::string text; //c++17以前，如果你想在声明的时候定义这个非常量静态变量是不行的
  //static std::string text {"hello"}; 错误
};
std::string X::text{ "hello" };
int main()
{
	X::text += " world";
	std::cout << X::text << std::endl;
}
```

为了保证代码能够顺利地编译，我们必须**保证静态成员变量的 定义有且只有一份**，稍有不慎就会引发错误，比较常见的错误是为了 方便将静态成员变量的定义放在头文件中：

```c++
#ifndef X_H
#define X_H
class X {
public:
	static std::string text;
};
std::string X::text{ "hello" };	//将定义放在头文件中
#endif
```

​		将上面的代码包含到多个CPP文件中会引发一个链接错误，因为 include是单纯的宏替换，所以会存在多份X::text的定义导致链接失败。为了解决上面这些问题，C++17标准中增强了inline说明符的能 力，它允许我们内联定义静态变量：

```c++
#include <iostream>
#include <string>
class X {
public:
	inline static std::string text{"hello"};
};
int main()
{
	X::text += " world";
	std::cout << X::text << std::endl;
}
```

​		上面的代码可以成功编译和运行，而且即使将类X的定义作为头文件包含在多个CPP中也不会有任何问题。在这种情况下，编译器会 在类X的定义首次出现时对内联静态成员变量进行定义和初始化。



## 函数返回类型后置

C++11标准中的新语法特性：函数返回类型后置

```c++
auto foo2()->int(*)(int)
{
	return bar_impl;
}
```

上述函数实际返回类型为一个函数指针。如果不使用函数返回类型后置的方法，只能先typedef函数指针类型，例如：

```c++
typedef int(*bar)(int);
bar foo1()
{
	return bar_impl;
}

```

### 推导函数模板返回类型

函数返回类型后置的作用之一是推导函数模板的返 回类型，当然前提是**需要用到decltype说明符**：

```c++
template<class T1, class T2>
auto sum1(T1 t1, T2 t2)->decltype(t1 + t2)
{
	return t1 + t2;
}
int main() {
	auto x1 = sum1(4, 2);
}

```

请注意，decltype(t1 + t2)不能写在函数声明前，编译器在解析返回类型的时候还没解析到参数部分，所以它对t1和t2一无所知，导致编译失败

```c++
decltype(t1 + t2) auto sum1(T1 t1, T2 t2) {…} // 编译失败，无法识别t1和t2
```



## using创建类型别名

语法格式`using identifier = type-id`

```c++
using byte = unsigned char;
using pbyte = unsigned char *;
using fn = void(byte, double); //建议采用using fn = std::function<void(byte, double)>;更具有普适性

void func(byte b, double d);
fn* f = func;					//注意这里的‘*’是必须的等价于typedef void (*)(byte, double);
```

> 如果使用`using fn = void(*)(byte, double);`则不需要再添加指针，或者使用`std::function<void(byte, double)>` 也不需要添加指针。

语法格式`template<template-params-list> identifier = type-id`创建模板别名

```c++
template<typename T>
class custom_allocator {};

template<typename T>
using vec_t = std::vector<T, custom_allocator<T>>;

vec_t<int> v1;
```

注意：创建的别名在`std::is_same`中被认为是相同类型。



## 关于long long类型

长整型`long long`虽然之前很早就在使用，但是其在C++11标准中才被正式承认。C++标准中定义，long long是一个**至少**为64位的整数类型（虽然目前还没有大于64位的）。C++标准还提供了`ULL`和`LL`字面量后缀。分别表示无符号长整型和有符号长整型。例如：

```c++
long long x = 65536LL;
```

字面量后缀的使用场景：

```c++
long long x1 = 65536 << 16; // 计算得到的x1值为0
std::cout << "x1 = " << x1 << std::endl;
long long x2 = 65536LL << 16; // 计算得到的x2值为
4294967296（0x100000000）
std::cout << "x2 = " << x2 << std::endl;
```

**注意**：在没有字面量后缀的情况下，这里的65536被当作32位整型操作。



## 新字符类型char16_t和char32_t

### 字符集和编码方式

​		通常我们所说的字符集是指系统支持的所有抽象字符的集合，通 常一个字符集的字符是稳定的。而编码方法是利用数字和字符集建立 对应关系的一套方法。比如Unicode字符集就 有UTF-8、UTF-16和UTF-32这3种编码方法。除了Unicode字符集，我 们常见的字符集还包括ASCII字符集、GB2312字符集、BIG5字符集 等，它们都有各自的编码方法。



### 使用新类型来存储Unicode字符集的不同编码方式

除了新类型`char16_t`和`char32_t`外，C++11标准还为3种编码提供了新前缀用于声明 3种编码字符和字符串的字面量，它们分别是UTF-8的前缀`u8`、UTF-16 的前缀`u`和UTF-32的前缀`U`

```c++
char utf8c = u8'a'; // C++17标准
//char utf8c = u8'好';
char16_t utf16c = u'好';
char32_t utf32c = U'好';
char utf8[] = u8"你好世界";
char16_t utf16[] = u"你好世界";
char32_t utf32[] = U"你好世界";
```

注意：`char utf8c = u8'好'`是无法通过编译的，因为存储“好”需要 3字节，显然utf8c只能存储1字节，所以会编译失败。



## 使用alignas和alignof进行对齐

### alignof查看类型的对齐字节

```c++
int main(int argc, char** argv) {
    std::cout << "char: " << alignof(char) << ", " << "int: " << alignof(int) << ", double: " << alignof(double) << std::endl;
    return 0;
}
//输出: char: 1, int: 4, double: 8
```



### 结构体对齐规则

1. **结构体中元素按照定义顺序依次置于内存中，但并不是紧密排列。从结构体首地址开始依次将元素放入内存时，元素会被放置在其自身对齐大小的整数倍地址上。**这里说的地址是元素在结构体中的偏移量，结构体首地址偏移量为0。
2. **如果结构体大小不是所有元素中最大对齐大小的整数倍，则结构体对齐到最大元素对齐大小的整数倍，填充空间放置到结构体末尾**
3. **基本数据类型的对齐大小为其自身的大小，结构体数据类型的对齐大小为其元素中最大对齐大小元素的对齐大小**

例如分析如下结构体的大小：

```c++
struct test
{
    char c;
    double d;
    int i;
};
```

首先根据规则1：d应该对齐在8字节地址，c的偏移地址为0且占1字节，其后需要填充7byte，i的对齐字节为4，刚好i的偏移在4的整数倍故d后面不需要填充，此时共计：`1+7+8+4 = 20 Byte`，结构体的大小应该是最大元素double（8字节）的整数倍，故需要在i后面添加4字节，最终结构体大小为24字节。

>  总结一下:
>
> + 对齐值一般指的是该类型的大小。
>
> + 结构体中的成员变量的初始地址需要是对齐值的整数倍
> + 整个结构体的大小必须是 **最大对齐值**的大小。
> + 所以，为了符合以上要求，编译器可能会在成员间/结构体末尾插入额外填充字节。





### 使用alignas说明符

使class或struct对象的内部元素按照给定字节对齐

```c++
struct alignas(4) foo //表示foo类型按照4字节对齐，也就是说sizeof(foo)的大小是4的倍数
{
    char a;
    char b;
};

sizeof(foo); //4byte
//注意：这里是指的foo类型按照4字节对齐，而不是其中的元素按照4字节对齐
```

![](./img/3.JPG)

```c++
struct X
{
 	char a;
    alignas(4) char b;
};

sizeof(X); //8byte
```

这里设置struct的成员变量b为4字节对齐。也就是说b的地址应该为4的倍数。因此需要在成员变量a之后填补3字节，此时共5字节，由于最大对齐为4，故需要将结构体填充到4点倍数（8Byte）



## 使用带作用域的枚举类型

当使用`enum`声明时为不带作用域的枚举类型，使用`enum class`或`enum struct`声明的是带作用域的枚举类型（`enum class`与`enum struct`完全等价）

```c++
enum class Status
{
    Unknown,
    Created,
    Connected
};
```

指定enum中元素的类型

```c++
enum class Codes : unsigned int
{
    OK = 0,
    Failure = 1
};

//注意：不能直接将Codes::OK赋值给整型变量，将会导致编译错误
Codes c1 = Codes::OK; 		//OK
int c2 = Codes::Failure;	//error
int c3 = static_cast<int>(Codes::OK); //OK
```



## 对虚方法使用override、final修饰

1. 始终使用`virtual`标记虚方法，无论是在派生类还是在父类中。
2. 在子类中使用`override`标记重载的父类方法
3. 使用`final`表示不能够被重载或继承

```c++
class Derived2 : public Derived1
{
	virtual void foo() final{} // 该虚方法不可被重载
};
class Derived3 : public Derived1
{
	virtual void foo() override{}
};
```

```c++
class Base final // 指定该类不可被继承
{
    
};
```

`override`帮助编译器确定一个虚方法是否重载其父类方法，例如：

```c++
class Base
{
public:
    virtual void foo() {}
    virtual void bar() {}
};

class Derived1 : public Base
{
public:
   virtual void foo() override {}
   virtual void bar(const char t) override() {} //这里，编译报错，override需要函数标识符与父类完全一致
};
```



## 继承构造函数

### 继承关系中构造函数的困局

​		假设现在有一个 类Base提供了很多不同的构造函数。某一天，你发现Base无法满足 未来业务需求，需要把Base作为基类派生出一个新类Derived并且 对某些函数进行改造以满足未来新的业务需求，比如下面的代码：

```c++
class Base {
public:
	Base() : x_(0), y_(0.) {};
	Base(int x, double y) : x_(x), y_(y) {}
	Base(int x) : x_(x), y_(0.) {}
	Base(double y) : x_(0), y_(y) {}
	void SomeFunc() {}
private:
	int x_;
	double y_;
};
class Derived : public Base {
public:
	Derived() {};
	Derived(int x, double y) : Base(x, y) {}
	Derived(int x) : Base(x) {}
	Derived(double y) : Base(y) {}
	void SomeFunc() {}
};
```

面对Base中大量的构造函数，我们不得不在Derived中定义同样多的构造函数，目的仅仅是转发构造参数，因为派生类本身并没有需要初始化的数据成员。

### 使用继承构造函数

C++中可以使用using关键字将基类的函数引入派生 类，比如：

```c++
class Base {
public:
	void foo(int) {}
};
class Derived : public Base {
public:
	using Base::foo;
	void foo(char*) {}
};
int main()
{
	Derived d;
	d.foo(5);
}
```

C++11的继承构造函数正是利用了这一点，将using关键字的能力进行了扩展，使其能够引入基类的构造函数：

```c++
class Base {
public:
	Base() : x_(0), y_(0.) {};
	Base(int x, double y) : x_(x), y_(y) {}
	Base(int x) : x_(x), y_(0.) {}
	Base(double y) : x_(0), y_(y) {}
private:
	int x_;
	double y_;
};
class Derived : public Base {
public:
	using Base::Base;
};
```

在上面的代码中，派生类Derived使用`using Base::Base`让 编译器为自己生成转发到基类的构造函数。

### 使用继承构造函数的注意事项

1. 派生类是隐式继承基类的构造函数，所以只有在程序中使用了 这些构造函数，编译器才会为派生类生成继承构造函数的代码
2. 派生类不会继承基类的默认构造函数和复制构造函数
3. 继承构造函数不会影响派生类默认构造函数的隐式声明，也就是说**对于继承基类构造函数的派生类，编译器依然会为其自动生成默 认构造函数的代码**。
4. 在派生类中声明签名相同的构造函数会禁止继承相应的构造函数。
5. 派生类继承多个签名相同的构造函数会导致编译失败（即多重继承中存在签名相同的构造函数）



## This指针

1. `this` 指针是一个隐含于每一个**非静态成员函数中的特殊指针**。它指向调用该成员函数的那个对象。
2. `this` 指针被隐含地声明为: `ClassName *const this`，这意味着不能给 `this` 指针赋值；在 `ClassName` 类的 `const` 成员函数中，`this` 指针的类型为：`const ClassName* const`，这说明不能对 `this` 指针所指向的这种对象是不可修改的（即不能对这种对象的数据成员进行赋值操作）；
3. `this` 并不是一个常规变量，而是个**右值**，所以不能取得 `this` 的地址（不能 `&this`）。



## 使用range-based for loop

语法格式：`for ( init-statement(optional) range-declaration : range-expression ) loop-statement`

```c++
std::vector<int> getRates()
{
        return std::vector<int>{1,2,3,4,5,6};
}

std::multimap<int, bool> getRates2()
{
        return std::multimap<int, bool> {
                {1, true},
                {2, false},
        };
}

for(int rate : getRates())
{
    std::cout << rate << std::endl;
}

for(auto&& [rate, flag] : getRates2()) //c++17
{
    std::cout << rate << ":" << flag << std::endl;
}
```

编译器生成的代码类似于：

```c++
{
    auto&& _range = range-expression;
    for(auto _begin = begin_expr, _end = end_expr; _begin != _end; ++_begin)
    {
        range_declaration = *_begin;
        loop_statement;
    }
}
```

1. 对于C风格数组`begin_expr`和`end_expr`依赖于数组元素的个数
2. 对于具有`begin()`和`end()`成员函数的类有`begin_expr = _range.begin()`和`end_expr = _range.end()`
3. 其余类型`begin_expr = begin(_range)`和`end_expr = end(_range)`

**注意**：任何一个类如果有`begin`和`end`成员函数，都会被赋值给`begin_expr`和`end_expr`，因此对于这类对象可能无法使用这种方式的loop。

## 使自定义类型支持range-based for loop

为了使用户自定义类型支持基于范围的`for`用户需要定义如下结构：

1. 定义`可变`和`不可变`**迭代器**迭代器应该实现如下方法：
   1. `operator++`：用于增加迭代器
   2. `operator*`：用于解引用迭代器，获得元素的值
   3. `operator!=`：判断两个迭代器不相等
2. 用户自定义类型还需要定义`begin()`和`end()`方法

下面详细实现`FooArray`；

```c++
//迭代器类
//T：将会在解引用的时候返回的值的类型
//C：迭代器的迭代对象，这里是FooArray
//Size：迭代器的下标
template <typename T, typename C, const size_t Size>
class FooArrayIterator
{
public:
    //获得一个管理对象的引用用于修改。
	FooArrayIterator(C& collection, size_t index) : _index(index), _datas(collection) {}
    //实现!=方法
	bool operator!=(const FooArrayIterator& it) const { return _index != it._index; }
    //实现operator++
	const FooArrayIterator& operator++() { _index++; return *this; } //声明返回类型为const的主要原因是防止++++
    //实现operator*
	const T& operator*() { return _datas.GetAt(_index); }
private:
	size_t _index;
	C& _datas;
};

//FooArray类
template <typename T, const size_t size>
class FooArray
{
public:
	const T& GetAt(int index)
	{
		if (index >= size)
			throw std::out_of_range("index out of range");
		return datas[index];
	}
	void SetAt(const int& index, const int& value)
	{
		if (index >= size)
			throw std::out_of_range("index out of range");
		datas[index] = value;
	}
	size_t GetSize(void) { return size; }
	FooArrayIterator<T, FooArray, size> begin()
	{
		return FooArrayIterator<T, FooArray, size>(*this, 0);
	}
	FooArrayIterator<T, FooArray, size> end()
	{
		return FooArrayIterator<T, FooArray, size>(*this, size);
	}
private:
	T datas[size];
};

//之后我们便可以使用基于范围的for来遍历FooArray
for (int& value : arr)
{
	std::cout << value << std::endl;
}
```



## 实现自己的统一初始化构造

使用 `std::initializer_list<T>`可以实现统一初始化构造，定义于头文件 `<initializer_list>`

```c++
std::vector<int> vec{1,2,3};
int t{3};
```

一个例子：

```c++
void func(std::initializer_list<int> const l) 
{ 
	for (auto const & e : l) 
	std::cout << e << std::endl; 
}
```

使用 `initializer_list` 的注意事项

1. 所有初始化元素应该相同类型
2. 禁止 `narrowing conversion`
   - double -> float
   - float->int
   - ......



## 名称空间

### 考虑使用匿名名称空间解决函数作用于文件域问题

例如：假设`file1.cpp`和`file2.cpp`都具有函数`void print(std::string message)`，一般的解决方案是使用`static`来修饰，使得函数作用域在本文件以内（将外部链接改为内部链接）。现在介绍**匿名名称空间方法**

```c++
//file1.cpp
namespace
{
    void print(std::string message)
    {
        std::cout << message << std::endl;
    }
}
void file1_run()
{
    print("file1 run");
}

//file2.cpp
namespace
{
    void print(std::string message)
    {
        std::cout << "file2" << message << std::endl;
    }
}
void file2_run()
{
    print("file2 run");
}
```



### 嵌套名称空间的简单定义方式（C++17）

```c++
namespace A::B::C {
int foo() { return 5; }
}
```

上述定义等价于：

```c++
namespace A {
namespace B {
namespace C {
int foo() { return 5; }
}
}
}
```



## Pairs和Tuples

### Pairs

#### Meta program补充

​	`std::pair`定义了 `first_type` 和 `second_type` 的类型预定义，用于分辨第一和第二个元素的类型。



#### Piars的比较

`std::pair` 提供比较运算符

| 运算符   | 描述                                                   |
| -------- | ------------------------------------------------------ |
| p1 == p2 | 若p1与p2的所有元素相同则比较结果为真                   |
| p1 != p2 | return !(p1 == p2)                                     |
| p1 < p2  | 从第一个元素开始比较，若第一个元素相等则比较第二个元素 |
| p1 > p2  | return (p2 < p1)                                       |
| p1 <= p2 | return !(p2 < p1)                                      |
| p1 >= p2 | return !(p1 < p2)                                      |

**可见对于比较运算，只需要提供：`==` 运算符和 `<` 就足够了**



#### tuple风格的接口

- 通过`std::get<N>(p)`来获得内容

  ```c++
  std::get<0>(p); //p.first
  ```

- `std::tuple_size<pair_type>::value` 来获得参数个数

  ```c++
  typedef std::pair<int, float> IntFloatPair;
  std::tuple_size<IntFloatPair>::value; //2
  ```

- `std::tuple_element<N, pair_type>::type` 来获得第`N`个参数的类型

  ```c++
  std::tuple_element<0, IntFloatPair>::type; //int
  ```



### Tuples

`std::tuple`是`std::pair`的扩展，`tuple`允许任意数量的元素。

#### tuple的创建

- 使用 `std::make_tuple`

  ```c++
  auto t = std::make_tuple(22,44,"nico");
  ```

#### tuple的修改

- 使用`std::get<N>(t)`

  ```c++
  std::get<0>(t) = "hello"; //注意新复制的类型必须与原类型匹配
  auto number = std::get<int>(tuple); //通过类型来访问tuple，这只在tuple中指定的类型只有一种时有效
  ```

- 在创建时使用引用语义

  ```c++
  string s;
  std::make_tuple(std::ref(s));
  ```

#### tuples解包

- `std::tie`

  ```c++
  std::tuple<int, float, std::string> t(77, 1.1, "more light");
  int i;
  std::string s;
  std::tie(i, std::ignore, s) = t; //使用tie可以对元组进行部分解包
  ```

  > tie解包的原理：通过`std::tie`创建一个引用元组，然后对该引用元组进行赋值。

- structured bindings

**注意**：

- 你不能通过赋值初始化列表来初始化元组

  ```c++
  std::tupel<int, double> t3 = {42, 3.14}; //error
  std::tupel<int, double> t3{42, 3.14};  //OK
  std::tupel<int, double> t3(42, 3.14); //OK
  ```

- 同样你不能使用初始化列表来初始化一个期望tuple的参数

  ```c++
  std::vector<std::tuple<int, float>> v{{1,1.0}, {2,2.0}}; //ERROR
  ```

  但是你可以用上述方式初始化 `std::pair`

  对于`tuple`可以使用`std::make_tuple`来替代初始化列表

  ```c++
  std::vector<std::tuple<int, float>> v{std::make_tuple(1,1.0), std::make_tuple(2,2.0)};
  ```

#### tuple的操作

- tuple_size

- tuple_element

- tuple_cat：拼接多个tuple，到一个tuple

  ```c++
  int n;
  auto tt = std::tuple_cat(std::make_tuple(42, 7.7, "hello"), std::tie(n)); //将 n 作为一引用语义绑定进tuple
  ```

#### tuple的遍历

使用编译时的嵌套递归生成tuple的遍历代码

```c++
#include <iostream>

template<size_t index, typename Tuple, typename Functor>
void tuple_at(const Tuple& tuple, const Functor& func) //在tuple的指定下标对每个元素执行函数func
{
        const auto& v = std::get<index>(tuple);
        func(v);
}

template<typename Tuple, typename Functor, size_t index = 0>
void tuple_for_each(const Tuple& tuple, const Functor& func) //对tuple的每一个元素执行func
{
        constexpr auto size = std::tuple_size<Tuple>::value;
        if constexpr ( index < size )
        {
                tuple_at<index, Tuple, Functor>(tuple, func);
                tuple_for_each<Tuple, Functor, index + 1>(tuple, func);
        }

}

int main(void)
{
        auto tuple = std::make_tuple(1,"ljr", 1.0);
        tuple_for_each(tuple, [](const auto& v){std::cout << v;});
        std::cout << std::endl;
}
```



### 在tuples和pairs之间转换

​	你可以使用`std::pair`初始化双值`std::tuple`,  你也可以将`std::pair`赋值给双值`std::tuple`。



## 使用structured bindings来处理多返回值函数

当使用`std::pair`或`std::tuple`从函数中返回多值时。对于`std::pair`需要使用`first`和`second`来解包，而使用`std::tuple`需要使用`std::tie`来解包。都不够直观。下面介绍使用c++17的structured bindings来解包

```c++
std::tuple<int, std::string, double> find()
{
    return std::make_tuple(1, "marius", 1234.5);
}

//structured bindings
auto [id, name, score] = find();
```

### 结构化绑定支持的3种类型

结构化绑定可以作用于3种类型，包括原生数组、结构体和类对象、元组和类元组的对象。



#### 绑定到原生数组

所需条件要求别名的数量与数组元素 的个数一致，比如：

```c++
#include <iostream>
int main()
{
	int a[3]{ 1, 3, 5 };
	auto[x, y, z] = a;
	std::cout << "[x, y, z]=["
	<< x << ", "
	<< y << ", "
	<< z << "]" << std::endl;
}
```

#### 绑定到结构体和类对象

所需类和结构体的条件：

1. 首先，类或者结构体中的非静态数据成员个数必须和标识符列表中的别名的**个数相同**；
2. 其次，这些数据成员必须是公有的（C++20 标准修改了此项规则）；
3. 这些**数据成员必须是在同一 个类或者基类中**；
4. 最后，绑定的类和结构体中**不能存在匿名联合体**。

```c++
class BindBase1 {
public:
	int a = 42;
	double b = 11.7;
};
class BindTest1 : public BindBase1 {};
class BindBase2 {};
class BindTest2 : public BindBase2 {
public:
	int a = 42;
	double b = 11.7;
};
class BindBase3 {
public:
	int a = 42;
};
class BindTest3 : public BindBase3 {
public:
	double b = 11.7;
};
int main()
{
	BindTest1 bt1;
	BindTest2 bt2;
	BindTest3 bt3;
	auto[x1, y1] = bt1; // 编译成功
	auto[x2, y2] = bt2; // 编译成功
	auto[x3, y3] = bt3; // 编译错误
}

```



## 数值的取值范围

`numeric_limits<T>`类定义在`<limits>`头文件中

可以使用`min()`和`max()`静态方法来获得数值的最小和最大值

```c++
std::numeric_limits<int>::max(); //获得int表示的最大值
```



## 随机数生成器

为了生成随机数你需要完成以下几个步骤：

1. 包含`<random>`头文件

2. 使用实例化`std::random_device`对象，作为种子

3. 实例化随机数生成算法引擎

   1. `linear_congruential_engine`：实现线性同余算法
   2. `mersenne_twister_engine`：实现梅森缠绕器
   3. `subtract_with_carry_engine`：实现带进位减（一种[延迟斐波那契](https://en.wikipedia.org/wiki/Lagged_Fibonacci_generator)）算法

4. 选择随机数的分布

   1. 均匀分布

      ```c++
      uniform_int_distribution  //产生在一个范围上均匀的整数值
      uniform_real_distribution //产生在一个范围上均匀的实数值
      ```

      

   2. 伯努利分布

   3. 正太分布

   4. 采样分布

   5. 泊松分布

使用实例：

```c++
// 生成 1 与 6 间的随机数
std::random_device r;                                            //生成种子
std::default_random_engine e1(r());                              //用种子初始化随机数引擎
std::uniform_int_distribution<int> uniform_dist(1, 6);           //选择正态分布，生成范围[1,6]
int mean = uniform_dist(e1);									 //将引擎传入分布器，生成一个随机数
std::cout << "Randomly-chosen mean: " << mean << '\n';
```



## 使用string_view替代const string&

程序中许多临时变量都只是需要把一个数据拷贝到另一个空间里面，而不需要对它进行修改，通常可以使用`const string&`来节省开销，但C++17提出了string_view来替代const string&

```c++
std::string_view get_filename(std::string_view str)
{
    auto const pos1{str.find_last_of('')};
    auto const pos2{str.find_last_of('.')};
    return str.substr(pos1 + 1, pos2-pos1 - 1);
}
```

注意：std::string_view相当于一个const std::string&，因此要注意原字符的生命周期问题。

### string_view相对于const string&的优势

- 对于const string&并不能保证不复制原始数据尤其是使用`c-style`风格字符串时

  ```c++
  void foo( std::string_view bob ) {
    std::cout << bob << "\n";
  }
  int main(int argc, char const*const* argv) {
    foo( "This is a string long enough to avoid the std::string SBO" );
    if (argc > 1)
      foo( argv[1] );
  }
  ```

  上述例子中，传入`foo`的字符串长度已经大于`std::string`默认的栈空间（SBO：当字符串过短时string不会在堆上存储）。此时如果使用`const std::string&`作为入参类型将导致拷贝

### string_view失去的东西

使用`string_view`意味着你将**失去`null`终止符**。



## Defaulted和deleted函数

1. 使用default代替默认构造函数的函数体

   ```c++
   struct foo
   {
     foo() = default;  
   };
   ```

   

2. 使用`delete`删除不需要的函数

   ```c++
   struct foo
   {
       foo(const foo&) = delete; //删除拷贝构造函数
   };
   ```

### 显示删除的应用举例

#### 禁止类从堆上创建

通过显示删除类的`new`运算符实现。

```c++
struct type
{
	void * operator new(std::size_t) = delete;
};
type global_var;
int main()
{
	static type static_var;
	type auto_var;
	type *var_ptr = new type; // 编译失败，该类的new已被删除
}
```



## placement new

​	`placement new `是 new 关键字的一种进阶用法，**既可以在栈（stack）上生成对象，也可以在堆（heap）上生成对象**。相对应地，我们把常见的 new 的用法称为 operator new，它只能在 heap 上生成对象。

语法格式：

```c++
new(address) ClassConstruct(…)
```

address 表示已有内存的地址，该内存可以在栈上，也可以在堆上；ClassConstruct(…) 表示调用类的构造函数，如果构造函数没有参数，也可以省略括号。

```c++
class Foo
{
public:
    Foo(int x) : _x(x) {}
    int _x;
};

int main()
{
    Foo foo_stack(10);
    Foo* foo_pointer = new(&foo_stack) Foo(20);
    std::cout << foo_pointer->_x << std::endl; // 20
    std::cout << foo_stack._x << std::endl; // 20
}
```

> placement new 利用已经申请好的内存来生成对象，它不再为对象分配新的内存，而是将对象数据放在 address 指定的内存中。在本例中，placement new 使用的是 foo_stack 的内存空间



## lambda函数

lambda的完整声明如下：

```c++
[ captures ] ( params ) specifiers exception -> ret { body }
```

**捕获范围**：

​	只能捕获作用域内非静态局部变量。

**lambda本质**：

​	匿名lambda是一个匿名函数对象

```c++
struct __lambda+name__
{
	xxx operator()(xxx) {...}
};
```

**捕获的原理**：

​	被捕获的参数以构造函数参数的形式传入lambda。

**尽可能的指定 `lambda` 的返回类型**

```c++
std::function<int*(void)> get_callback()
{
    	int* value = new int;
        return [value]{ return value; };
}
```

这样可能会编译报错：类型不匹配。lambda没有推断出正确的`int*`类型。因此建议如下改写

```c++
std::function<int*(void)> get_callback()
{
    	int* value = new int;
        return [value]()->int*{ return value; };
}
```

### lambda捕获使用：

#### 捕获的类型：

- 按值捕获：`[=]`

- 按引用捕获：`[&]`

- 在类中，按引用捕获类的成员变量：`[this]`

- 在类中，按值捕获类的成员变量：`[*this]`

  **注意**：这里的按值捕获并不意味着你可以修改类的成员变量，更准确的说这里是用const捕获类的成员变量。

- 全局按引用捕获，对特定变量按引用捕获：`[&, v]`

- 全局按值捕获，对特定变量按引用捕获：`=, &v`



#### 按值捕获并修改捕获变量

需要使用`mutable`限定符（允许我们在lambda表达式函数体内改变按值捕获的变量，或者调用非const 的成员函数）

```c++
int x{0};
auto callable = [x]() mutable { x++; std::cout << x << std::endl;  };
callable();
```

**注意**：如果使用了限定符，那么形参列表不能省略（也就是说`()`不能省略）。



### 将lambda对象赋值给C风格的函数指针

```c++
void call_callback(void(*func)(void))
{
	func();
}

int main(void)
{
	call_callback(+[] {std::cout << "Hello World" << std::endl; });
}
```

只需要在常规的lambda前面加一个`+`号。

**注意**：此时的lambda表达式不能有任何的捕获语句



### Polymorphic lambda（多态lambda）

所谓的Polymorphic lambda就是指：入参由`auto`说明的lambda，此时lambda可以接受不同类型的参数进行调用。

```c++
int v = 3;
auto lambda = [v](auto v0, auto v1) {v + v0 * v1;}
```

注意：这里有个限制就是**捕获类型必须指定**。为了使得捕获类型也可以自动推断，可以使用模板函数来创建lambda：

```c++
template<typename T>
auto create_lambda(const T& v)
{
    return [v](auto v0, auto v1){return v  + v0 * v1;};
}
```



### 从lambda函数中返回引用类型

```c++
auto l = [](int &i)->auto& { return i; };
auto x1 = 5;
auto &x2 = l(x1);
assert(&x1 == &x2); // 有相同的内存地址
```



### 广义捕获

C++14标准中定义了广义捕获，所谓广义捕获实际上是两种捕获方式：

1. **简单捕获**，这种捕获就是我们在前文中提到的捕获方法，即[identifier]、[&identifier]以及[this]等。

2. **初始化捕获**，这种捕获方式是在C++14标准中引入的，它解决了简单捕获的一 个重要问题，即只能捕获lambda表达式定义上下文的变量，而无法捕获表达式结果以及自定义捕获变量名。比如：

   ```c++
   int main()
   {
   	int x = 5;
   	auto foo = [x = x + 1]{ return x; }; //c++14之前编译错误
   }
   ```

   注意：这里的`x`跨越了两个作用域，等号左边的`x`是lambda作用域，而等号右边的x存在于main作用域。例如你可以写成：

   ```c++
   auto foo = [r = x + 1]{ return r; }
   ```



#### 初始化捕获的应用场景

1. 使用移动操作减少代码运行的开销

   ```c++
   #include <string>
   int main()
   {
   	std::string x = "hello c++ ";
   	auto foo = [x = std::move(x)]{ return x + "world"; };
   }
   ```

2. 第二个场景是在**异步调用时复制this对象**，防止lambda表达式被调用时因原始this对象被析构造成未定义的行为

   ```c++
   #include <iostream>
   #include <future>
   class Work
   {
   private:
   	int value;
   public:
   	Work() : value(42) {}
   	std::future<int> spawn()
   	{
   		return std::async([=]() -> int { return value; });
   	}
   };
   std::future<int> foo()
   {
   	Work tmp;
   	return tmp.spawn();
   }
   int main()
   {
   	std::future<int> f = foo();
   	f.wait();
   	std::cout << "f.get() = " << f.get() << std::endl;
   }
   //注意这里，f.get()的输出是未定义的，因为tmp对象以及被析构了。
   ```

   为了避免上述问题，我们可以考虑将`std::async`的调用修改如下

   ```c++
   std::async([=, tmp = *this]() -> int { return tmp.value; }); //复制一份类实例
   ```

   上述代码在`C++17`中可以简化为：

   ```c++
   std::async([=, *this]() -> int { return value; }); //在lambda中复制一份类实例
   ```

   

## 移动语义

c++中移动语义是一种优化手段，防止不必要的资源复制，主要是通过移动构造函数与移动赋值运算符来实现的。

比如传统的拷贝操作把一个对象的内容复制到另一个对象，这可能设计很多内存分配、复制数据的操作，对于大型对象或者动态分配的对象来说，这样的操作带来的开销比较大。而移动语义就是在不复制对象的内存情况下，把对象的资源（动态分配的内存、文件句柄）从一个对象转移到另一个对象。

### 移动语义补充

- 对于**移动构造/移动赋值**函数都应尽量声明为`noexcept`修饰符，否则STL可能在特定情况下仍然采用复制的方式。

  ```c++
  A(A&&) noexcept; //移动构造
  A& operator=(A&&) noexcept;  //移动赋值
  ```

  

- 还可以对类的成员函数使用`&&`修饰符，强制只有右值能够调用该成员函数：

  ```c++
  struct Foo{
    auto func() && {} //右值修饰符  
  };
  
  auto a = Foo{};
  a.func(); //非法
  std::move(a).func(); //合法
  Foo{}.func(); //合法
  ```


### 在成员函数名称后加入&或&&参数的意义

一个例子：

```c++
class Test{
public:
    void foo(int)&;
    void foo(int)&&;
    void foo(int)const;
};
```

这里的`&`和`&&`主要作用是：函数重载。任何类成员函数的调用都隐含需要一个`this`指针，因此一个类的成员函数调用过程如下：

```c++
Test test;
test.foo(1);
//实际上的调用形式: 
foo(*this, 1);
```

上述成员函数的完整声明如下：

```c++
class Test{
public:
    void foo(Test&, int);
    void foo(Test&&, int);
    void foo(const Test&, int);
}
```

因此：对于使用左值调用`foo`的实例会使用`&`修饰的成员函数，而使用右值调用`foo`的实例会使用`&&`修饰的成员函数。

在`libtorch`中有一段使用示例：

```c++
  template <typename TransformType>
  MapDataset<Self, TransformType> map(TransformType transform) & {
    return datasets::map(static_cast<Self&>(*this), std::move(transform));
  }

  /// Creates a `MapDataset` that applies the given `transform` to this dataset.
  template <typename TransformType>
  MapDataset<Self, TransformType> map(TransformType transform) && {
    return datasets::map(std::move(static_cast<Self&>(*this)), std::move(transform));
  }
```

在使用右值实例调用`map`时可以直接`move`增加效率。



## 关于noexcept的补充

​		`noexcept`只是告诉编译器不会抛出异常， 但函数不一定真的不会抛出异常。这相当于对编译器的一种承诺，当 我们在声明了noexcept的函数中抛出异常时，程序会调用 `std::terminate`去结束程序的生命周期。

​		另外，`noexcept`还能接受一个返回布尔的常量表达式，当**表达式评估为true的时候，表示函数不会抛出异常**。反之，**当表达式评估为false的时候，则表示该函数有可能会抛出异常**。这个特性广泛应用于模板当中，例如：

```c++
template <class T>
T copy(const T & o) noexcept {
}
```

​		以上代码想实现一个复制函数，并且希望使用noexcept优化不 抛出异常时的代码。但问题是如果T是一个复杂类型，那么调用其复制构造函数是有可能发生异常的。直接声明noexcept会导致当函数 遇到异常的时候程序被终止，而不给我们处理异常的机会。我们可以修改如下：

```c++
template <class T>
T copy(const T &o) noexcept(std::is_fundamental<T>::value) {
}
```

只有T为基本类型的时候才使用noexcept标签。



## 可变参数模板

举一个可变参数模板的例子：

```c++
template<class ...Args>
void foo(Args ...args) {}
template<class ...Args>
class bar {
public：
    bar(Args ...args) {      
    	foo(args...);  
	}
};
```

- `class ...Args`：类型模板形参包，它可以接受零个或者多个类型的模板实参。
- `Args ...args`：函数形参包，出现在函数的形参列表中，可以接受零个或者多个函数实参
- `args...`：是形参包展开，形参包展开为零个或者多个**模式**的列表，这个过程称为解包。这里所谓的模式是指实参展开的方法，形参包的名称必须出现在这个方法中作为实参展开的依据，最简单的情况为解包后就是实参本身。



### 变参函数模板

```c++
template<class ...Args>
void foo(Args ...args) {}

int main()
{
    unsigned int x = 8;    
    foo();          // foo()  
    foo(1);           // foo<int>(int)  
    foo(1, 11.7);     // foo<int,double>(int,double)  
    foo(1, 11.7, x);  // foo<int,double,unsigned int>(int,double,unsigned int)}
}
```

以上是一个变参函数模板，它可以接受任意多个实参，**编译器会根据实参的类型和个数推导出形参包的内容**。



### 变参类模板

变参类模板虽然不能通过推导得出形参包的具体内容，但是我们可以直接指定它：

```c++
template<class ...Args>
class bar {};
int main()
{  
    bar<> b1;  
    bar<int> b2;  
    bar<int, double> b3;  
    bar<int, double, unsigned int> b4;
}
```



### 变参函数模板和变参类模板之间的比较

1. 在类模板中，模板**形参包必须是模板形参列表的最后一个形参**

   ```c++
   template<class ...Args, class T>
   class bar {};
   bar<int, double, unsigned int> b1;     // 编译失败，形参包并非最后一个
   template<class T, class ...Args>
   class baz {};
   baz<int, double, unsigned int> b1;     // 编译成功
   ```

   但是对于函数模板而言，模板形参包不必出现在最后，只要保证后续的形参类型能够通过实参推导或者具有默认参数即可，例如：

   ```c++
   template<class ...Args, class T, class U = double>
   void foo(T, U, Args ...args) {}
   foo(1, 2, 11.7);    // 编译成功
   ```



### 非类型形参包

```c++
template<int ...Args>
void foo1() {};
template<int ...Args>
class bar1 {};
int main()
{  
    foo1<1, 2, 5, 7, 11>();  
    bar1<1, 5, 8> b;
}
```



### 形参包展开

允许包展开的场景：

1．表达式列表。

2．初始化列表。

3．基类描述。

4．成员初始化列表。

5．函数参数列表。

6．模板参数列表。

7．动态异常列表（C++17已经不再使用）。

8．lambda表达式捕获列表。

9．Sizeof...运算符。

10．对齐运算符。

11．属性列表。

包展开举例：

```c++
#include <iostream>

template<class T, class U>
T baz(T t, U u){  
    std::cout << t << ":" << u << std::endl;  
    return t;
}

template<class ...Args>
void foo(Args ...args) {}

template<class ...Args>
class bar 
{
public:  
    bar(Args ...args)  
    {       
        foo(baz(&args, args) ...);  
    }
};

int main()
{  
    bar<int, double, unsigned int> b(1, 5.0, 8);
}
```

首先bar是一个普通的变参函数模板。值得注意的是`baz(&args, args) ...`称为包展开，其中`baz(&args, args)`称为模式。包展开的过程就是保持模式中非args部分不变，然后一个个展开args。上述包展开为：

```c++
class bar 
{
public:  
    bar(int a1, double a2, unsigned int a3)  
    {
        //将模式中的args依次替换
        foo(baz(&a1, a1), baz(&a2, a2), baz(&a3, a3));  
    }
};
```

在调用foo()时对baz求值进而可能的输出结果，如下：

```c++
0x7fff1d58e5b0:8
0x7fff1d58e5a8:5
0x7fff1d58e5b4:1
//这里的输出顺序是由函数参数求值顺序决定的
```



再来看一些包展开的例子

```c++
#include <iostream>

template<class ...T>
T baz(T ...t){ 
    return 0;
}

template<class ...Args>
void foo(Args ...args) {}

template<class ...Args>
class bar 
{
public:  
    bar(Args ...args)  
    {       
        foo(baz(&args...) + args...);  
    }
};

int main()
{  
    bar<int, double, unsigned int> b(1, 5.0, 8);
}
```

这里的解包过程：

1. 对函数模板`baz(&args...)`的解包：`&args...`是包解开，`&args`是模式，这部分会被解包为：`baz(&a1, &a2, &a3)`

2.  第二部分是对`foo(baz(&args...) + args...)`的解包，由于`baz(&args...)`已经被解包，因此相当于对`foo(baz(&a1, &a2, &a3) +args...)`解包，模式为：`baz(&a1, &a2, &a3) + args`最终得到的结果是：

   `foo(baz(&a1, &a2, &a3) + a1, baz(&a1, &a2, &a3) + a2, baz(&a1, &a2, &a3) + a3)`



### sizeof...运算符

`sizeof...`用于统计形参包中形参的个数。返回类型是`std::size_t`例如：

```c++
template<class ...Args> 
void foo(Args ...args){  
    std::cout << "foo sizeof...(args) = " << sizeof...(args) <<std::endl;
}
```



### 常用可变参数模板技巧

要实现可变参数模板，需要按照已下几个步骤

1. 重载一个固定参数函数（可以是模板也可以非模板），用于结束编译时的递归调用。
2. 定义一个模板参数包，表示允许任意种类型的模板参数
3. 定义一个函数参数包，保存任意个函数参数
4. 展开参数包，递归调用模板。

```c++
template<typename T> //定义一个固定参数的函数重载
T add(T value)
{
    return value;
}

template<typename T, typename... Ts> //定义可变参数模板
T add(T head, Ts... rest)
{
    std::cout << sizeof...(reset) << std::endl; //打印参数包中变量的个数
    return head + add(rest...);
}
```

几种参数的解析

1. `Typename... Ts`：声明一个模板参数包，可以容纳可变数量的模板类型
2. `Ts... rest`：函数参数包，表示可以容纳可变数量的函数参数
3. `rest...`：函数参数包的展开

### 包展开的应用

```c++
#include <utility>
#include <iostream>
#include <tuple>
#include <array>

template<typename T, T... Ints> //这里的T... Ints 不是一个模板参数包，在本例中是：int... Ints
void output_sequece(std::integer_sequence<T, Ints...>&& seq) // Ints展开为：0,1,2,3,4,5...
{
    auto tup = std::make_tuple(Ints...);
    std::apply([](decltype(Ints)... args){ ((std::cout << args << " "),...); }, tup); //这里函数参数需要类型，使用decltyp获得参数类型
    constexpr std::array<int, seq.size()> arr = {Ints...};
    for(auto val : arr)
        std::cout << val << " ";
    //((std::cout << Ints << " "), ...);
    //std::tuple<decltype(Ints)...> tup2;
    //decltype(tup)
}

int main(void)
{
    output_sequece(std::make_integer_sequence<int, 10>());
}
```

**注意**：

- 注意区分：`typename... T` 和 `T... Ints` 这里我们可以将前者的T认为是一系列的类型，而后者则是一系类 T 类型的值。所以 对于前者声明Tuple：

  `std::tuple<T...> foo;`，对于后者则需要使用：`std::tuple<delctype(Ints)...> foo;`



**可变参数模板的实质**

​	模板种的递归调用实际上并不是真正的递归调用。

例如：调用`add(1,2,3)`生成代码如下

```c++
int add(int value)
{
    return value;
}

int add(int head, int arg1)
{
    return head + add(arg1);
}

int add(int head, int arg1, int arg2)
{
    return head + add(arg1, arg2);
}
```

### 可变参数模板非迭代版本

上述的可变参数模板是通过在编译时迭代来实现的。我们可以将可变参数打包为tuple然后再通过[迭代tuple](#tuple的遍历)的方式来进行迭代。

```c++
template<typename... Ts>
auto make_string(const Ts& ...values)
{
    auto tuple = std::tie(values...); //绑定为tuple
    tuple_for_each(tuple, [&sstr](const auto& v){sstr << v;}); //参见遍历元组
    return sstr.str();
}
```

也可以使用`if constexpr`表达式：

```c++
template<typename Head, typename... Args>
auto add(Head head, Args... args) -> decltype(head)
{
    if constexpr (sizeof...(args) >= 1)
    {
        return head + add(args...);
    }
    else
    {
        return head;
    }
}
```



### C++17 折叠表达式

使用折叠表达式来进行多个数求和：

```c++
#include <iostream>
template<class... Args>

auto sum(Args ...args)
{  
    return (args + ...);
}

int main(){  
    std::cout << sum(1, 5.0, 11.7) << std::endl;
}
```

在C++17的标准中有4种折叠规则，分别是一元向左折叠、一元向右折叠、二元向左折叠和二元向右折叠。

- 一元向右折叠

  ( args op ... ) 折叠为 (  arg0 op (arg1 op ... (argN-1 op argN)) )

- 一元向左折叠

  (  ... op args ) 折叠为 ( ( ( arg0 op arg1 ) op ... ) op argN )

- 二元向右折叠

  (  args op ... op init ) 折叠为 (  arg0 op (arg1 op ... ( argN-1 op ( argN op init ) ) ) )

  相较于一元多了一个初始值

- 二元向左折叠

  (init op ... op args) 折叠为 (((((init op arg0) op arg1) op arg2) op...) op argN)

注意二元折叠的两个op必须相同。

因此上述例子中`(args + ...)`是一个一元向右折叠，其结果为：`1 + (5.0 + 11.7)`



**一个二元折叠的例子**：

```c++
#include <iostream>
#include <string>

template<class ...Args>
void print(Args ...args)
{  
    (std::cout << ... << args) << std::endl; //二元向左折叠，std::cout是初始值
}
int main()
{  
    print(std::string("hello "), "c++ ", "world");
}
```

编译器的代码翻译：`(((std::cout << std::string("hello ")) << "c++ ")<< "world") << std::endl;`



#### 折叠表达式在模板包展开中的应用

现在我想实现一个函数能够打印`tuple`中的所有元素：

```c++
template<typename... Ts>
std::ostream& operator<<(std::ostream& os, std::tuple<Ts...> const& theTuple)
{
    std::apply
    (
        [&os](Ts const&... tupleArgs)
        {
            os << '[';
            std::size_t n{0};
            ((os << tupleArgs << (++n != sizeof...(Ts) ? ", " : "")), ...); // 一元向右折叠( args op ... ) ，其中 ，是op
            os << ']';
        }, theTuple
    );
    return os;
}
```

**注意**：逗号表达式的求值顺序从左至右（无论是否有括号）：

```c++
void output(int i)
{
   std::cout << i << std::endl;
}
 
int main()
{
   int i = (output(0), (output(1), (output(2), 4))); 
}
//输出
0
1
2
// i == 4
```



## template中声明模板参数为模板类

假设我们现在实现一个模板类如下：

```c++
template <typename T,typename CONT = std::vector<T> >
class Stack{

};

int main() {
    Stack<int> stack1;
    Stack<int,std::vector<int>> stack2;  //1
    return 0;
}
```

如注释1所示，如果不使用默认参数，需要写明第一和第二个模板参数的具体类型，但是如果使用类模板作为参数则不需要指定第二个容器所包含元素的类型，只需要指定容器类型

```c++
template <typename T, template<typename ELEM> class CONT = std::deque>
class Stack{
private:
    CONT<T> elems;
public:
    void push(T const&);
    void pop();
    T top() const;
    bool empty() const{
        return elems.empty();
    }
};
```

这里的`CONT`声明为一个类模板，其中由于`ELEM`不被使用故可以省略为

```c++
template <typename T, template<typename> class CONT = std::deque>
```



## 算法库常用函数

算法库包含头文件：`<algorithm>`

1. 排序

   1. `std::sort()`：默认升序排序
   2. `std::stable_sort()`：稳定排序（保持排序前后相对顺序不变）
   3. `std::is_sorted()`：判断是否以及有序

2. 初始化范围

   1. `std::generate()`：将传入谓词的返回值赋值给迭代器指示的范围

      声明如下：

      ```c++
      template< class ForwardIt, class Generator >
      void generate( ForwardIt first, ForwardIt last, Generator g );
      ```

      可能的实现：

      ```c++
      template<class ForwardIt, class Generator>
      constexpr // Since C++20
      void generate(ForwardIt first, ForwardIt last, Generator g)
      {
          while (first != last) {
              *first++ = g();
          }
      }
      ```

      使用示例：

      ```c++
      #include <algorithm>
      #include <iostream>
      #include <vector>
       
      int f()
      { 
          static int i;
          return ++i;
      }
       
      int main()
      {
          std::vector<int> v(5);
          auto print = [&] {
              for (std::cout << "v: "; auto iv: v)
                  std::cout << iv << " ";
              std::cout << "\n";
          };
       
          std::generate(v.begin(), v.end(), f);
          print();
      }
      ```

   2. `std::itoa()`：为一个范围赋值自增序列，通过调用自增元素的`++`方法。

## 实现一个完整的迭代器

[参考文章](https://www.cplusplus.com/reference/iterator/)

一个迭代器根据它所具有的功能可以分为四大类：

1. Input/Output：输入和输出迭代器是最有限的迭代器类型：它们可以执行顺序的单通道输入或输出操作。
2. Forward：具有1中的所有功能，支持在一个方向上迭代，所有**标准容器都至少支持**这种迭代器。
3. Bidirectional：具有ForWard的所有功能，并且是双向迭代器
4. Random Access：实现`Bidirectional`的所有功能，并且具有随机访问的能力

![](./img/iterator-1.JPG)

完成一个`Random Access`迭代器

我们还是以[使自定义类型支持range-based for loop](#使自定义类型支持range-based for loop)中的`FooArray`为例，实现它的`Random Access`迭代器

实现一个迭代器的步骤：

1. 类型萃取定义：
   - iterator_category：迭代器所属种类：`input_iterator_tag`, `output_iterator_tag`, `forward_iterator_tag`, `bidirectional_iterator_tag`, `random_access_iterator_tag`。
   - value_type：容器元素的类型
   - difference_type：迭代器作差的计算类型
   - pointer：指向容器元素的，指针类型
   - reference：容器元素的引用类型
2. 实现特定迭代器种类的方法

```c++
template <typename T, typename Container, const size_t size>
class FooArrayIterator
{
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    FooArrayIterator(Container& container, size_t index)
        : container(container), index(index)
    {}

    reference operator*() const
    {
        return container.GetAt(index);
    }

    pointer operator->() const
    {
        return &container.GetAt(index);
    }

    FooArrayIterator& operator++()
    {
        ++index;
        return *this;
    }

    FooArrayIterator operator++(int)
    {
        FooArrayIterator temp = *this;
        ++(*this);
        return temp;
    }

    FooArrayIterator& operator--()
    {
        --index;
        return *this;
    }

    FooArrayIterator operator--(int)
    {
        FooArrayIterator temp = *this;
        --(*this);
        return temp;
    }

    FooArrayIterator& operator+=(difference_type n)
    {
        index += n;
        return *this;
    }

    FooArrayIterator& operator-=(difference_type n)
    {
        index -= n;
        return *this;
    }

    FooArrayIterator operator+(difference_type n) const
    {
        FooArrayIterator temp = *this;
        temp += n;
        return temp;
    }

    FooArrayIterator operator-(difference_type n) const
    {
        FooArrayIterator temp = *this;
        temp -= n;
        return temp;
    }

    difference_type operator-(const FooArrayIterator& other) const
    {
        return index - other.index;
    }

    reference operator[](difference_type n) const
    {
        return container.GetAt(index + n);
    }

    bool operator==(const FooArrayIterator& other) const
    {
        return index == other.index;
    }

    bool operator!=(const FooArrayIterator& other) const
    {
        return !(*this == other);
    }

    bool operator<(const FooArrayIterator& other) const
    {
        return index < other.index;
    }

    bool operator<=(const FooArrayIterator& other) const
    {
        return index <= other.index;
    }

    bool operator>(const FooArrayIterator& other) const
    {
        return index > other.index;
    }

    bool operator>=(const FooArrayIterator& other) const
    {
        return index >= other.index;
    }

private:
    Container& container;
    size_t index;
};
```



## 通过非成员函数进行容器访问

对于标准容器通常提供了`begin()`和`end()`成员函数，返回一个迭代器对象用于访问容器内容。同时在c++11/c++14开始提供了非成员函数进行等价的操作，这些非成员函数提供了对：**标准库容器**、**c风格数组**、和特殊处理后的**用户自定义类型**进行访问。

这些非成员函数分别是：`std::begin()/std::end()`、`std::cbegin()/std::cend()`、`std::rbegin()/std::rend()`、`std::crbegin()/std::crend()`。他们定义于头文件`<iterator>`中但一般标准容器头均隐式包含了该头文件。

**对标准容器库使用**：

```c++
#include <vector>
#include <iostream>

int main(void)
{
    std::vector<int> v1{1,2,3,4,5};
    auto sv1 = std::size(v1); //sv1 = 5
    auto ev1 = std::empty(v1); //ev1 = false
    auto dv1 = std::data(v1); //dv1 = v1.data()
    for(auto i = std::begin(v1); i != std::end(v1); ++i)
            std::cout << *i << std::endl;
}
```

**对C风格数组使用**：

```c++
#include <iostream>
#include <iterator>

int main(void)
{
    int v1[5]{1,2,3,4,5};
    auto sv1 = std::size(v1); //sv1 = 5
    auto ev1 = std::empty(v1); //ev1 = false
    auto dv1 = std::data(v1); //dv1 = v1.data()
    for(auto i = std::begin(v1); i != std::end(v1); ++i)
            std::cout << *i << std::endl;
}
```



## 使用chrono::duration表达时间间隔

c++11提供灵活的日期和时间工具，定义于`<chrono>`头文件的`std::chrono`名称空间下，其中常用的表达时间间隔的类`std::chrono::duration`。

常用的`std::chrono::duration`预定义类型有：

```c++
std::chrono::hours            half_day(12);
std::chrono::minutes          half_hour(30);
std::chrono::seconds          half_minutes(30);
std::chrono::milliseconds     half_second(500);
std::chrono::microseconds     half_millisecond(500);
std::chrono::nanoseconds      half_microsecond(500);
```

也可以使用**字面量初始化**`std::chrono::duration`对象

```c++
using namespace std::chrono_literals; //注意必须包含这个名称空间

auto half_day = 12h;
auto half_hour = 30min;
auto half_minute = 30s;
auto half_second = 500ms;
...............  = 500us;
...............  = 500ns;
```

**精度转换**

1. 低精度向高精度转换

   低向高转换可以直接隐式转换

   ```c++
   std::chrono::hours     half_day_in_h(12);
   std::chrono::minutes   half_day_in_min(half_day_in_h);
   std::cout << half_day_in_h.count() << "h" << std::endl; //12h
   std::cout << half_day_in_min.count() << "min" << std::endl; //720min
   ```

   

2. 高精度向低精度转换

   由高向低需要使用`std::chrono::duration_cast`

   ```c++
   using namespace std::chrono_literals;
   
   auto total_seconds = 12345s;
   auto hourse = std::chrono::duration_cast<std::chrono::hours>(total_seconds);
   ```

   还另外提供了三种转换`std::chrono::floor<>`、`std::chrono::round<>`和`std::chrono::ceil`分别是向下取整，四舍五入，向上取整

**算术运算**：

​	支持+、-、*、/、%运算，但是**注意**：当两个精度不同的`std::chrono::duration`对象进行运算时，结果自动转为最高精度，例如：分钟和秒运算结果为秒。

**实质**

1. 关于`std::chrono::hours`等预定义类型的实际定义如下：

   ```c++
   namespace std{
       namespace chrono{
           typedef duration<long long, ratio<1, 1000000000> > nanoseconds;
           typedef duration<long long, ratio<1, 1000000> >    microseconds;
           typedef duration<long long, ratio<1, 1000> >       milliseconds;
           typedef duration<long long>                        seconds;
           typedef duration<int, ratio<60> >                  minutes;
           typedef duration<int, ratio<3600> >                hours;
       }
   }
   ```

   其中`ratio`表示倍数，其基本单位是秒。

## 测量函数运行时间

1. 使用标准时钟保存当前时间

   ```c++
   auto start = std::chrono::high_resolution_clock::now();
   ```

2. 执行函数

3. 获得函数执行结束的时间并与开始时间作差值后转换为需要的分辨率

   ```c++
   auto diff = std::chrono::high_resolution_clock::now() - start;
   std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() << std::endl;
   ```

**运行解释：**

1. 标准定义了三种时钟

   - system_clock：这个使用当前系统的实时时钟，表示时间点
   - high_resolution_clock：使用当前系统的最高精度，表示时间点
   - steady_clock：表示这个时钟是稳定的（不可调），也就是后一个时间点减去前一个时间点一定为正数。

   为了测量程序运行时间我们应该选用稳定时钟：`high_resolution_clock`或`steady_clock`

2. 使用`high_resolution_clock`的静态方法`now()`得到`time_point`对象

   两个`time_point`对象相减得到`duration`对象。



## 使用std::optional存储可选值

​	在一些情况下我们可能需要一种变量它可以有值也可以为空，例如一个查找字典的函数在没有找到时返回为`nullptr`在找到时返回字典元素，那么这种情况下可以使用`std::optional`来表示

- 通过构造、赋值语句存储值

  ```c++
  std::optional<int> v1; //empty
  std::optional<int> v2(42);
  v1 = 42; //v1 contains 42
  std::optional<int> v3 = 42;
  ```

- 读取 `optional` 中的值

  ```c++
  std::optional<int> v2(42);
  std::cout << *v2 << std::endl;
  
  std::optional<foo> v2{ foo{42, 10.5} };
  std::cout << v2->a << " " << v2->b << std::endl;
  ```

- 检查是否包含值

  ```c++
  std::optional<int> v2(42);
  if(v2) std::cout << *v2 << std::endl; //通过隐式转换
  if(v2.has_value()) ... //通过成员方法
  ```

在下列情况中使用`optional` 

- 从函数中返回，存在可能失败的情况

  ```c++
  template <typename K, typename V>
  std::optional<V> find(int const key, std::map<K, V> const & m)
  {
  	auto pos = m.find(key);
  	if (pos != m.end())
  		return pos->second;
  	return {};
  }       
  ```

- 函数参数可选

  ```c++
  std::string extract(std::string const & text, std::optional<int> start, std::optional<int> end)
  {
  	auto s = start.value_or(0);
  	auto e = end.value_or(text.length());
  	return text.substr(s, e - s);
  }
  auto v1 = extract("sample"s,         
  ```

- 成员函数可选

  ```c++
  struct book
  {
  	std::string                title;
  	std::optional<std::string> subtitle;
  	std::vector<std::string>   authors;
  	std::string                publisher;
  	std::string                isbn;
  	std::optional<int>         pages;
  	std::optional<int>         year;
  };   
  ```

  

## 使用std::any存储任意变量

c++没有类似于java一样的完整的继承体系，因此没有类似于java的`Object`类一样能够存储任意类型的变量，但是在`C++17`后，标准使用了`std::any`对象用于存储任意类型。其定义于`<any>`中

1. 如何使用

   1. 存储值，使用构造函数或赋值运算符

      ```c++
      std::any value(40); //integer 40
      value = 42.0;       //double 42.0
      ```

      

   2. 读取值，使用非成员函数`std::any_cast()`

      ```c++
      std::any value = 42.0;
      try{
          auto d = std::any_cast<double>(value); //必须于存储的类型完全一致，否则抛出异常
          std::cout << d << std::endl;
      }catch(const std::bad_any_cast & e)
      {
          std::cout << e.what() << std::endl;
      }
      ```

      

   3. 查看是否符合某种类型

      ```c++
      inline bool is_integer(const std::any& a)
      {
      	return a.type() == typeid(int);
      }
      ```

      关于`typeid()`的说明：

      - 当使用模板类型作为`typeid()` 的参数时，应该连同模板参数一起作为类型名

        ```c++
        template<typename T>
        class Foo
        {};
        
        std::any value = Foo<int>();
        value.type() == typeid(Foo<int>);    //true
        value.type() == typeid(Foo<double>); //False
        ```

        

   4. 检查是否包含值
   
      ```c++
      std::any a;
      if(a.has_value())
          std::cout << "has value" << std::endl;
      else
          std::cout << "no value" << std::endl;
      ```
   
      
   
   5. 修改内容
   
      ```c++
      //emplace() 原地构造新对象
      //reset() 销毁保存的对象
      //swap() 与另一个std::any作交换
      std::any value = 42;
      value.has_value(); //true
      value.reset();
      value.has_value(); //false
      ```



## 用户自定义Hash

用户自定义`Hash`函数的要求：

1. 接收 `Key` 类型的单个参数。（Key类型就是用户自定义类型）
2. 返回表示参数散列值的`std::size_t`类型。
3. 调用时不抛出异常。
4. 对于二个相等的参数 `k1` 与 `k2` ， `std::hash<Key>()(k1) == std::hash<Key>()(k2)` 。
5. 对于二个相异而不相等的参数 `k1` 与 `k2` ，`std::hash<Key>()(k1) == std::hash\<Key>()(k2)` 的概率应非常小，接近 `1.0/std::numeric_limits::max() `。

STL容器已经对基本类型提供了非常好的`Hash`函数，大多数情况下我们可以在自定义类型中重用这些函数。

自定义的过程：

1. 定义判等函数，判断两个自定义类型是否相等

2. 定义`Hash`函数，函数应该符合`用户自定义`Hash`函数的要求`。在这里建议的hash函数是`combine`了参与1中比较的所有参数的hash函数。在boost库中提供了`boost::hash_combine`其实现为：

   ```c++
   template<typename T> 
   void hash_combine(size_t& seed, T const& v)
   {
       //seed是一个size_t的参数表示 hash值
       //v 自定义类型中参与比较的成员变量
       seed ^= hash_value(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
   }
   ```

   那么我们可以在C++中自己实现如下：

   ```c++
   #include <iostream>
   #include <string>
   #include <functional>
   #include <unordered_set>
   #include <stdio.h>
   
   //User define type
   class Person
   {
   public:
           Person(const std::string& name, int32_t age) : _name(name), _age(age){}
           std::string name() const { return _name; }
           int32_t age() const { return _age; }
       	//need for hash
           bool operator==(const Person& p) const { return  _name == p.name() && _age == p.age(); }
   private:
           std::string _name;
           int32_t _age;
   };
   
   //boost hash_combine
   template<typename T>
   void hash_combine(size_t& seed, T const& v)
   {
           seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
   }
   
   //Hash Function for Person
   auto PersonHash = [](const Person& p) {
           size_t seed{0};
           hash_combine(seed, p.name());
           hash_combine(seed, p.age());
           return seed;
   };
   
   int main(void)
   {
       //create unordered_set for Person
       std::unordered_set<Person, decltype(PersonHash)> set(100, PersonHash);
       return 0;
   }
   ```
   
   为了简便起见，我们可以特化`std::hash模板类`
   
   ```c++
   template<>
   struct std::hash<Person>
   {
           std::size_t operator()(Person const& p) const
           {
                   size_t seed{0};
                   hash_combine(seed, p.name());
                   hash_combine(seed, p.age());
                   return seed;
           }
   };
   
   int main(void)
   {
           std::unordered_set<Person> set(100);
           return 0;
   }
   ```
   

## 智能指针

C++提供了三种类型的智能指针：`std::unique_ptr`，`std::shared_ptr`和`std::weak_ptr`。它们都是为了解决资源管理问题，特别是防止内存泄漏，而设计的。

1. **`std::unique_ptr`**：这是最轻量级的智能指针，它不需要额外的内存来管理资源，因此其内存大小通常与裸指针相同。`std::unique_ptr`拥有其指向的对象，当`std::unique_ptr`被销毁时（例如离开其作用域），它所拥有的对象也会被自动销毁。`std::unique_ptr`不能被复制，只能被移动，这保证了同一时间只有一个`std::unique_ptr`可以指向一个对象，防止了资源的多重释放。在效率上，`std::unique_ptr`几乎与裸指针一样，因为它不需要额外的引用计数或同步操作。

2. **`std::shared_ptr`**：这是一个引用计数的智能指针，它需要额外的内存来存储引用计数。每当一个新的`std::shared_ptr`指向一个对象，引用计数就会增加，当`std::shared_ptr`被销毁时，引用计数就会减少，当引用计数变为0时，对象就会被自动销毁。`std::shared_ptr`可以被复制和赋值，可以有多个`std::shared_ptr`指向同一个对象。在效率上，`std::shared_ptr`比`std::unique_ptr`低一些，因为它需要维护引用计数，而且这个操作需要是线程安全的。

`shared_ptr` 的引用计数信息并不存储在 `shared_ptr` 本身，而是存储在一个**独立的控制块（control block）**中。每个 `shared_ptr` 拷贝会共享这个控制块，控制块中包含两个计数器：一个是强引用计数（shared count），一个是弱引用计数（weak count）。控制块和被管理对象可能在同一块内存中（`make_shared` 场景），也可能是分开分配的（直接用 `new` + `shared_ptr`）。

这样可以看出来`make_shared` 优化了控制块和对象之间的内存布局，通过一个分配操作同时创建对象和控制块，可以减少堆分配次数，提高缓存局部性。

```c++
struct ControlBlock {
    std::atomic<long> shared_count;  // 被 shared_ptr 持有的次数
    std::atomic<long> weak_count;    // 被 weak_ptr 持有的次数（+1 来防止提前释放 control block 本身）
    void* ptr_to_object;
    deleter function;                // 自定义析构器（可选）
    allocator function;              // 自定义分配器（可选）
};
```

> **为什么controlBlock必须要在堆上？**
>
> `std::shared_ptr` 支持多个智能指针实例共享同一个底层对象的所有权。为了使得所有这些智能指针实例都能看到并更新同一个引用计数，这个计数必须存在于一个所有实例都可访问的单一、共享的位置。如果引用计数是一个栈上的成员变量，每个 `shared_ptr` 的副本将会拥有自己的计数副本，这样就无法达到多个智能指针之间的正确同步和共享。
>
> 智能指针的核心功能是自动管理其指向的对象的生命周期。使用堆分配的引用计数允许智能指针独立于任何函数或作用域，随时正确地管理对象的生命周期。如果使用栈分配的引用计数，那么计数的有效性将受限于它所在作用域的生命周期，这将导致无法跨作用域或线程安全地共享和管理智能指针。
>
> `std::shared_ptr` 还与 `std::weak_ptr` 配合使用，后者允许对对象的非拥有引用（即不增加引用计数）。为了使 `weak_ptr` 能够检查所关联的 `shared_ptr` 是否仍存在，引用计数（通常包括强引用计数和弱引用计数）需要独立于任何单个 `shared_ptr` 或 `weak_ptr` 实例。这样，即使所有 `shared_ptr` 实例都已销毁，引用计数结构（和可能还未被销毁的底层对象）依然可由 `weak_ptr` 访问。

3. **`std::weak_ptr`**：这是一种特殊的智能指针，它指向一个由`std::shared_ptr`管理的对象，但是它不会增加引用计数。`std::weak_ptr`主要用于防止智能指针的循环引用问题。它的内存大小和效率与`std::shared_ptr`相似，因为它也需要存储引用计数，但不会改变引用计数。

总的来说，`std::unique_ptr`在内存和效率上都是最优的，但它不能共享所有权。`std::shared_ptr`可以共享所有权，但需要额外的内存和计算来维护引用计数。`std::weak_ptr`用于解决循环引用问题，它的内存和效率与`std::shared_ptr`相似。在选择使用哪种智能指针时，需要根据具体的需求和约束进行权衡。



### shared_ptr

shared_ptr本身的引用计数是线程安全的（它使用某种原子操作来保证多线程下的安全性）；但是shared_ptr不会保证它托管的资源是线程安全的，多个线程如果同时访问资源，需要自己实现同步。

### 智能指针补充

为了适应复杂环境下的使用需求，`std::shared_ptr`提供了下面几个辅助类

- weak_ptr
- bad_weak_ptr
- enable_shared_from_this

#### 除了循环引用，什么时候还应该用weak_ptr

+ 从业务逻辑上考虑，shared_ptr指向了一个对象，那么这个对象的生命周期就会延长，因为增加了引用计数。但是有很多时候，我们的业务逻辑本身只是对这个资源以观察者的角度来查看的，那么这个时候也应该用 **weak_ptr**。

### weak_ptr

在某些使用环境下，`std::shared_ptr`的引用计数会出现问题导致资源无法释放，这里需要使用`weak_ptr`。

- 存在循环引用时
- 希望共享但是又不拥有对象的所有权

上述情况下都可以使用`weak_ptr`，当最后一个`shared_ptr`引用失效时，对应的`weak_ptr`自动为空。你不能对`weak_ptr`使用`*`和`->`运算符。因此访问管理的 对象必须转换为`std::shared_ptr`因为如下原因：

1. 转换成`std::shared_ptr`以检查对象是否被释放
2. 当正在处理对象时，`std::shared_ptr`不会被释放，而`std::weak_ptr`可能被释放

介绍一个循环引用导致内存泄漏的例子：

![](./img/1.JPG)

上图中，`std::shared_ptr<kid>`的引用计数为：3，当user释放掉kid后，kid由于循环引用导致计数为：2，因此不会释放，而导致内存泄漏。

因此可以考虑，将mom和dad指向kid的指针设计为`weak_ptr`

![](./img/2.JPG)

`weak_ptr`不会增加引用计数，此时kid的计数为：1，user释放将同时释放所有资源。

#### 使用lock()获得weak_ptr关联的shared_ptr

如果没有关联，则返回空指针。

如果你不确定当前的`weak_ptr`所关联的对象是否存在，你可以使用`expired()`方法，该方法当**不关联**时返回`true`



### enable_shared_from_this

​	`enable_shared_from_this`：是一个基类，作用是为派生类提供一个`shared_from_this()`方法，该方法返回一个指向派生对象`this`指针有效的`std::shared_ptr`对象。

`enable_shared_from_this`的使用场景：当需要一个指向this指针的shared_ptr时。如果直接将this指针赋值给shared_ptr将导致计数错误，和悬空指针问题。例如：

```c++
#include <iostream>
#include <thread>
#include <memory>

class Y: public std::enable_shared_from_this<Y>
{
public:
    void say() const { std::cout << "Hello, world" << std::endl; }
    std::shared_ptr<Y> share()
    {
        return shared_from_this();
    }
    void foo()
    {
        std::thread([this] {
            global_y = share();  // Correct
            // global_y = this;  // Incorrect
        }).detach();
    }
};

std::shared_ptr<Y> global_y = nullptr;

int main(void)
{
    {
        std::shared_ptr<Y> y = std::make_shared<Y>();
        y->foo();
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));  // Wait for the new thread to finish
    if (global_y) {
        global_y->say();
    }
    return 0;
}
```

> ​	在这个例子中，如果你试图直接赋值`global_y = this`，那么当`y`在`main`函数的作用域结束时被销毁，`global_y`将会成为一个悬挂指针，这将导致未定义行为。但是，如果你使用`Y::share()`，那么`global_y`将持有一个有效的`shared_ptr`，即使`y`被销毁，`global_y`仍然可以安全地使用。



### 自定义Deleter

我们可以定义自己的`Deleter`来释放智能指针管理的对象例如：

```c++
shared_ptr<string> pNico(new string("nico"), [](string* p){ //注意这里lambda的入参
    cout << "delete " << *p << endl;
    delete p;
})
```

利用上述方法我们可以用 `std::shared_ptr` 来管理数组。（注意：该方式在C++17已经不推荐，C++17中支持`shared_ptr<T[]>`声明，并且支持数组访问）

```c++
shared_ptr<int> pNico(new int[10], [](int* p){
    delete[] p; //用数组的删除方法
});
```

对于上述的lambda实现可以简化为

```c++
shared_ptr<int> pNico(new int[10], std::default_delete<int[]>());
```

**注意**：上述方案不支持`[]`也不支持`+`



### 线程安全的 shared pointer 接口

​	首先：可以明确一点，C++11标准要求 `use_count` 是原子的。





## 关于const的位置

​	声明一个整型常量时，建议使用`int const`而不是`const int`

1. 针对问题什么是不变的有着更好的一致性，`int const`更好的回答恒定不变的部分始终指的是`const`限定符前的内容。
2. 在使用typedef替换后，`const int`的含义有时候会发生变化，而`int const`不会。

## assert

### static_assert

`static_assert`是一种在编译期检查条件是否满足的assert，常用于模板编程中判断条件是否满足

```c++
struct alignas(8) item
{
  int id;
  bool activate;
  double value;
};
static_assert(sizeof(item) == 16, "size of item must be 16 byte")
```

### assert

`assert` 在程序运行期间检查参数是否符合 要求定义于头文件 `<assert>` 中

```c++
#include <iostream>
#include <cassert>

int main()
{
    assert(2+2==4);
    std::cout << "Checkpoint #1\n";
 
    assert((void("void helps to avoid 'unused value' warning"), 2*2==4));
    std::cout << "Checkpoint #2\n";
 
    assert((010+010==16) && "Yet another way to add an assert message");
    std::cout << "Checkpoint #3\n";
}
```



## 使用type traits查询属性类型

所有type traits支持在头文件`<type_traits>`中声明

1. 与`enable_if`配合，决定模板函数能够实例化的类型

   ```c++
   template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
   T multiply(T const t1, T const t2)
   {
       return t1 * t2;
   }
   
   auto v1 = multiply(42.0,1.5); //OK
   auto v2 = multiply("42"s, "1.5"s) //error
   ```

   如上限制了模板参数必须为算术类型：整型或浮点类型等，如果定义为其它类型，那么`enable_if`将不会定义type从而导致编译错误

   

2. 与`static_assert`配合，保证类型符合要求

   ```c++
   template<typename T>
   struct pod_wrapper
   {
       static_assert(std::is_pod<T>::value, "Type is not a POD!");
       T value;
   };
   
   pod_wrapper<int> i{ 42 }; //OK
   pod_wrapper<std::string> s{ "42"s }; //error
   ```

**type traits的工作原理**：

​	type traits分为两个种类：

- 提供类型的信息、属性或关系例如：

  `is_integer`、`is_arithmetic`、`is_array`、`is_enum`、`is_class`、`is_const`、`is_pod`、`is_same`等，这些Traits提供bool类型的成员变量：value

- 修改属性的类型例如：

  `add_const`、`remove_const`、`add_pointer`、`remove_pointer`、`make_signed`、`make_unsigned`等，这些Traits提供了typedef的类型：type，用于保存转换后的类型。

## 设计自己的Type Traits

假设现在有两个类`foo`和`bar`均提供 序列化 相关接口如下：

```c++
struct foo
{
  std::string serialize()
  {
      return "plain"s;
  }
};

struct bar
{
  std::string serialize_with_encoding()
  {
      return "encoded"s;
  }
};
```

现需要一个统一的`serial`接口来调用不同类的序列化方法。

1. 首先，我们需要识别这个类的序列化是否是`encoding`的也就是需要构建自己的type trits.

   ```c++
   template<typename T>
   struct is_serializable_with_encoding
   {
     static const bool value = false;  
   };
   
   //特化
   template<>
   struct is_serializable_with_encoding<bar>
   {
     static const bool value = true;  
   };
   ```

   关键在于特化

2. 提供一个根据`is_serializable_with_encoding`的不同结果执行不同函数的模板对象

   ```c++
   template<bool b>
   struct serializer
   {
     template<typename T>
     static auto serialize(T& v)
     {
         return v.serialize();
     }
   };
   
   template<>
   struct serializer<true>
   {
     template<typename T>
     static auto serialize(T& v)
     {
         return v.serialize_with_encoding();
     }
   };
   ```

   同样，关键在于特化一个`bool == True`的情况表明该类含有`encoding`方法

3. 自此可以提供一个统一的serialize接口如下：

   ```c++
   template<typename T>
   auto serialize(T& v)
   {
       return serializer<is_serializable_with_encoding<T>::value>::serialize(v);
   }
   ```



### 使用特化技术，获取函数签名参数

现在我们来实现一个`std::function`模板类，该模板类，接受一个函数签名作为模板参数。

```c++
// 声明模板类，但不定义它。
template <typename T>
class Function;

// 特化模板类，用于函数类型。
template <typename ReturnType, typename... Args>
class Function<ReturnType(Args...)> {
public:
    // 使用using声明来定义ReturnType别名。
    using RetureType = ReturnType;
    Function(std::function<ReturnType(Args...)> f) : f_(f) {}
    ReturnType operator()(Args... args) {
        return f_(std::forward<Args>(args)...);
    }
private:
    std::function<ReturnType(Args...)> f_;
};
```



## 使用std::conditional

`std::conditional` 根据条件变量，在编译时从两种类型选取一种类型 typedef 为 type。使用实例：

```c++
using long_type = std::conditional<sizeof(void*) <= 4, long, long long>::type;
```

也可以级联使用

```c++
template<int size>
using number_type = typename std::conditional<size <=1,
	std::int8_t,
	typename std::conditional<size<=2,
	std::int16_t,
	std::int32_t>::type
    >::type;
```

可能的实现：

```c++
template<bool Test, class T1, class T2>
struct conditional
{
    typedef T2 type;
};

template<T1, T2>
struct conditional<true, T1, T2>  //条件为真使用第一个
{
	typedef T1 type;  
};
```



## 程序鲁棒性与运行效率

### 异常

​	使用异常能够提高程序的运行效率（避免了大量无意义的错误检查）

**在类的构造和析构函数中**使用`throw`的规则：

1. 可以在构造中调用`throw`表示异常
2. 不要在析构中调用`throw`，因为它总是会调用`terminate()`而不会被捕获

**c++提供的异常种类**：

- **std::logic_error**：用于指示程序中存在逻辑错误，比如非法操作数、下标越界等等
  - std::invalid_argument
  - std::out_of_range
  - std::length_error
- **std::runtime_error**：指示程序出现了不可预测的错误或非程序错误例如：溢出错误、下溢错误或操作系统错误等。
  - std::overflow_error
  - std::underflow_error
  - std::system_error
- **以bad_开头的异常**：std::bad_alloc, std::bad_cast等

**从 std::exception 中派生自己的异常**

```c++
class simple_error : public std::exception
{
public:
    virtual const char* what() const noexcept override //注意这里异常的完整声明
    {
        return "simple exception";
    }
};
```

**从 std::logic_error 或 std::runtime_error 中派生**

```c++
class another_logic_error : public std::logic_error
{
public:
    another_logic_error() : std::loginc_error("simple login exception"){}
};
```

### 使用 noexcept 指示程序无异常抛出

### 存储异常到 `exception_ptr` 

c++11允许将一个异常存入`std::exception_ptr` 中，一遍延迟处理

```c++
std::exception_ptr eptr;
void foo()
{
    try{
        throw ...;
    }catch(...){
        eptr = std::current_exception(); //存储异常
    }
}

void bar()
{
    if(eptr != nullptr){
        std::rethrow_exception(eptr); //处理异常
    }
}
```



### 正确使用 const 声明

​	使用const的几个地方：

1. 对那些不允许在函数内部更改的参数

   ```c++
   void connect(std::string const & url, int const timeout = 2000)
   {}
   ```

   注意：为什么将`const`放到类型之后，在前面章节中有提到。[关于const的位置](#关于const的位置)

2. 指示类的成员变量不会更改

   ```c++
   class user_settings
   {
   public:
       int const min_update_interval = 25;
   };
   ```

3. 类的成员函数不会更改类的状态

   ```c++
   class user_settings
   {
   public:
       bool can_show_online() const { return show_online }
   };
   ```

### 使用 constexpr 提高程序运行效率

​	如果对于一个函数的所有参数都可以在编译时确定，那么编译器可以执行该函数并获得运行结果，从而在运行时直接使用结果而提高程序运行效率。

在如下情形下使用 constexpr：

- 定义可以在编译时确定运行结果的非成员函数

  ```c++
  constexpr unsigned int factorial(unsigned int const n)
  {
      return n > 1? n * factorial(n - 1) : 1;
  }
  
  //factorial(6)
  ```

- 定义可在编译时确定的类构造方法（例如对const成员赋值的构造方法）和成员函数

  ```c++
  class point3d
  {
      double const x_;
      double const y_;
      double const z_;
  public:
      constexpr point3d(double const x = 0, double const y = 0, double const z = 0): x_(x), y_(y), z_(z){}
      constexpr double get_x() const { return x_; }
      constexpr double get_x() const { return y_; }
      constexpr double get_x() const { return z_; }
  };
  ```

- 定义可以在编译期间确定的变量

  ```c++
  constexpr unsigned int size = factorial(6);
  constexpr point3d p{0,1,2};
  constexpr auto x = p.get_x();
  ```

**constexpr** 用于说明声明的变量可以在编译时确定值，或函数可以在编译时运行。对于constexpr函数或对象可以使用宏进行替换。

**注意**：当编译器无法在编译时确定所有参数时，constexpr失效，这次调用自动转为运行时执行，例如：

```c++
constexpr unsigned int size = factorial(6); //compile time evaluation

int n;
std::cin >> n;
auto result = factorial(n); //runtime evaluation
```

**使用 constexpr 的限制**

- 声明变量时
  - 变量类型必须是literal类型
  - 必须在声明时初始化变量
  - 初始化变量的表达式必须为constexpr
- 声明函数时
  - 非虚函数
  - 返回类型或参数必须全为literal类型
  - ......
- 声明构造函数时
  - 所有参数为literal类型
  - 没有虚基类
  - 没有 try 
  - ......

### 使用 if constexpr 语句实现编译时多态

参考如下语句：

```c++
#include <iostream>

struct Bear { auto roar() const { std::cout << "roar" << std::endl; } };
struct Duck { auto quack() const { std::cout << "quack" << std::endl; } };

template<typename Animal>
auto speak(const Animal& a)
{
	if (std::is_same<Animal, Bear>::value) { a.roar(); }
	else if (std::is_same<Animal, Duck>::value) { a.quack(); }
}
```

我们期望使用一种：编译时多态的机制并且不使用特化来实现。如何我们使用`Bear`的实例调用`speak`时编译会出错，这是因为`else if`语句虽然判断为`false`了，但是`a.quack()`仍然被调用了。我们期望一种`if`语句，当它为false时跳过它的编译，这就是`if constexpr`的作用：现改进如下：

```c++
template<typename Animal>
auto speak(const Animal& a)
{
	if constexpr (std::is_same<Animal, Bear>::value) { a.roar(); }
	else if constexpr (std::is_same<Animal, Duck>::value) { a.quack(); }
    //如果需要添加else语句，则不需要添加constexpr例如：
    //else
    //	.......
}
```

现在讨论执行：`Bear bear; speak(bear);`语句时编译器生成的代码可能如下：

```c++
auto speak(const Bear& a) { a.roar(); }
```

注意：如果所有的`if constexpr`均不匹配，那么编译器会生成一个空函数（假设用 int 实例化上述模板）：

```c++
auto speak(const int& a) {}
```



### 正确使用类型转换

​	使用C风格的类型转换会破坏C++的类型安全：`(type) expression` or `type(expression)`

​	为了类型安全考虑，c++提供如下类型转换：`static_cast, dynamic_cast, const_cast, reinterpret_cast`

- static_cast

  对非多态类型进行类型转换，例如：整型转枚举，一种指针类型转另一种指针类型（基类指针转派生类，或相反），**不会执行运行时检查，但是在编译时若转换的指针类型之间无关则编译报错例如：int* 转 double***。

- dynamic_cast

  **只能用于指针和引用**，主要用于**多态类型之间的类型转换**，执行运行时检查 。

  如果无法转换，那么`dynamic_cast`就会**返回一个空指针**（如果是用于指针的转换）或者抛出一个`std::bad_cast`**异常**（如果是用于引用的转换）。

- const_cast

  用于将const类型转非const

- reinterpret_cast

  类似于传统的c类型转换，不能保证类型安全

## 设计模式优化

### 优化工厂模式下if...else语句

​	假设有一个图像基类`Image`，其具有处理不同图像的子类：`BitmapImage`、`PngImage`、`JpgImage`等，那么在工厂中创建实例可能实现如下：

```c++
std::shared_ptr<Image> Create(std::string_view type)
{
    if(type == "bmp")
        return std::make_shared<BitmapImage>();
    else if(type == "png")
        return std::make_shared<PngImage>();
    else if(type == "jpg")
        return std::make_shared<jpgImage>();
    return nullptr
}
```

如上所示随着 `if...else` 语句的增加将会阻碍代码的阅读，因此我们的目标是减少 `if...else`语句，可以如下实现：

```c++
std::shared_ptr<Image> Create(std::string_view type)
{
    static std::map<std::string, std::function<std::shared_ptr<Image>()>> mapping{
        {"bmp", [](){return std::make_shared<BitmapImage>();}},
        {"png", [](){return std::make_shared<PngImage>();}},
        {"jpg", [](){return std::make_shared<JpgImage>();}}
    };//如果要在函数外部初始化静态成员mapping则只能在源文件中初始化。
    auto it = mapping.find(type.data());
    if(it != mapping.end())
        return it->second();
    return nullptr;
}
```

### 静态多态机制

​	在实现多态时，往往会使用`virtual`，但是`virtual`是在运行时动态绑定，会降低程序的运行效率，现在讨论一种依靠模板的静态继承机制，例如继承如下类：

```c++
class control
{
public:
    void draw();
private:
    virtual void erase_background();
    virtual void paint();
};
```

可能的改写如下：

```c++
  1 template<typename T>
  2 class control
  3 {
  4 public:
  5         void draw()
  6         {
  7                 static_cast<T*>(this)->erase_background();
  8                 static_cast<T*>(this)->paint();
  9         }
 10 };
 11
 12 class button : public control<button>
 13 {
 14 public:
 15         void erase_background()
 16         {
 17                 std::cout << "erasing button background..." << std::endl;
 18         }
 19         void paint()
 20         {
 21                 std::cout << "painting button..." << std::endl;
 22         }
 23 };
 24
 25 class checkbox : public control<checkbox>
 26 {
 27 public:
 28         void erase_background()
 29         {
 30                 std::cout << "erasing checkbox background..." << std::endl;
 31         }
 32         void paint()
 33         {
 34                 std::cout << "painting checkbox..." << std::endl;
 35         }
 36 };
 38 template<typename T>
 39 void draw_control(control<T>& c)
 40 {
 41         c.draw();
 42 }
```

如代码7、8行所示，在调用虚方法时，使用模板参数 T ，在子类中继承模板基类并且将子类作为模板参数传入。

**注意事项**：

- 基类中调用的子类方法必须为public，否则应该在子类中声明基类为友元类

  ```c++
  class button : public control<button>
  {
  private:
          friend class control<button>;
          void erase_background()
          {
                  std::cout << "erasing button background..." << std::endl;
          }
          void paint()
          {
                  std::cout << "painting button..." << std::endl;
          }
  };
  ```

- 无法存储到 同类容器 中，例如: `vector`、`list`等

- 使用该技术可能导致程序变大

### 实现线程安全的singleton

**实现步骤：**

1. 定义一个 singleton class

   ```c++
   class Singleton
   {
   };
   ```

2. 将默认构造函数设置为 `private`

   ```c++
   private:
       Singleton(){}
   ```

3. 删除 拷贝/赋值 构造

   ```c++
   public:
       Singleton(Singleton const &) = delete;
       Singleton& operator=(Singleton const&) = delete;
   ```

4. 定义 `Instance()` 方法

   ```c++
   public:
       static Singleton& Instance()
       {
           static Singleton single;
           return single;
       }
   ```

**实现原理：**

​	c++11后，对静态对象的实例化只会进行一次，无论是否是多线程环境。



# Parallel STL

自`C++17`起，一部分算法库进行了并行计算扩展。将算法转换成并行计算，只需要添加对应的`parallel execution policy`。

以：`std::find`为例：

```c++
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <execution>

auto roller_coasters = std::vector<std::string>{
	"woody", "steely", "loopy", "upside_down"
};
//非并行版本
auto val = *std::find(roller_coasters.begin(), roller_coasters.end(), "loopy");

//并行版本
auto val_par = *std::find(std::execution::par, roller_coasters.begin(), roller_coasters.end(), "loopy");
```



## Execution policies

`C++17`提供三种执行策略定义于头文件`<execution>`：

### Sequenced policy

​	`std::execution::seq`：即顺序执行，非并行

### Parallel policy

​	`std::execution::par`：常规的并行策略，当执行过程中遇到异常时，抛出到主线程。可对互斥资源加锁

### Parallel unsequenced police

​	`std::execution::par_unseq`：拥有比`std::execution::par`更加严格的约束：

- 谓词不应该抛出异常：否则会出现未定义情况，或crash
- 谓词不能使用mutex或同步操作：否则可能出现死锁，但可以使用`atomic`



# 实验特性

## std::experimental::is_detected

该方法定义在头文件`<experimental/type_traits>`

用于判断类中的特定成员是否存在：

```c++
#include <experimental/type_traits>
struct Octopus
{
  auto mess_with_arms() {}
};

struct Whale
{
  auto blow_a_fountain() {}
};

template<typename T>
using can_mess_with_arms = decltype(&T::mess_with_arms);

namespace exp = std::experimental;
static_assert(exp::is_detected<can_mess_with_arms, Octopus>::value, "Octopus don\'t have mess_with_arms"); //判断是否存在
static_assert(exp::is_detected<can_mess_with_arms, Whale>::value, "Whale don\'t have mess_with_arms"); //判断是否存在
```

****
