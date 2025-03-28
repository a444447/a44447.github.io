# C11——C17编程技巧

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

  ```c++
  ///  普通懒汉式实现 -- 线程不安全 //
  #include <iostream> // std::cout
  #include <mutex>    // std::mutex
  #include <pthread.h> // pthread_create
  
  class SingleInstance
  {
  public:
      // 获取单例对象
      static SingleInstance *GetInstance();
      // 释放单例，进程退出时调用
      static void deleteInstance();
  	// 打印单例地址
      void Print();
  private:
  	// 将其构造和析构成为私有的, 禁止外部构造和析构
      SingleInstance();
      ~SingleInstance();
      // 将其拷贝构造和赋值构造成为私有函数, 禁止外部拷贝和赋值
      SingleInstance(const SingleInstance &signal);
      const SingleInstance &operator=(const SingleInstance &signal);
  private:
      // 唯一单例对象指针
      static SingleInstance *m_SingleInstance;
  };
  
  //初始化静态成员变量
  SingleInstance *SingleInstance::m_SingleInstance = NULL;
  
  SingleInstance* SingleInstance::GetInstance()
  {
  	if (m_SingleInstance == NULL)
  	{
  		m_SingleInstance = new (std::nothrow) SingleInstance;  // 没有加锁是线程不安全的，当线程并发时会创建多个实例
  	}
      return m_SingleInstance;
  }
  
  void SingleInstance::deleteInstance()
  {
      if (m_SingleInstance)
      {
          delete m_SingleInstance;
          m_SingleInstance = NULL;
      }
  }
  
  void SingleInstance::Print()
  {
  	std::cout << "我的实例内存地址是:" << this << std::endl;
  }
  
  SingleInstance::SingleInstance()
  {
      std::cout << "构造函数" << std::endl;
  }
  
  SingleInstance::~SingleInstance()
  {
      std::cout << "析构函数" << std::endl;
  }
  ///  普通懒汉式实现 -- 线程不安全  //
  ```

- 线程安全、内存安全的懒汉式

  上述代码出现的问题：

  1. GetInstance()可能会引发竞态条件，第一个线程在if中判断 `m_instance_ptr`是空的，于是开始实例化单例;同时第2个线程也尝试获取单例，这个时候判断`m_instance_ptr`还是空的，于是也开始实例化单例;这样就会实例化出两个对象,这就是线程安全问题的由来

     解决办法：①加锁。②局部变量实例

  2. 类中只负责new出对象，却没有负责delete对象，因此只有构造函数被调用，析构函数却没有被调用;因此会导致内存泄漏。

     解决办法：使用共享指针

  > c++11标准中有一个特性：如果当变量在初始化的时候，并发同时进入声明语句，并发线程将会阻塞等待初始化结束。这样保证了并发线程在获取静态局部变量的时候一定是初始化过的，所以具有线程安全性。因此这种懒汉式是最推荐的，因为：
  >
  > 1. 通过局部静态变量的特性保证了线程安全 (C++11, GCC > 4.3, VS2015支持该特性);
  > 2. 不需要使用共享指针和锁
  > 3. get_instance()函数要返回引用而尽量不要返回指针，

  ```c++
  ///  内部静态变量的懒汉实现  //
  class Singleton
  {
  public:
      ~Singleton(){
          std::cout<<"destructor called!"<<std::endl;
      }
      //或者放到private中
      Singleton(const Singleton&)=delete;
      Singleton& operator=(const Singleton&)=delete;
      static Singleton& get_instance(){
          //关键点！
          static Singleton instance;
          return instance;
      }
      //不推荐，返回指针的方式
      /*static Singleton* get_instance(){
          static Singleton instance;
          return &instance;
  	}*/
  private:
      Singleton(){
          std::cout<<"constructor called!"<<std::endl;
      }
  };
  
  ```

  > 使用锁、共享指针实现的懒汉式单例模式
  >
  > - 基于 shared_ptr, 用了C++比较倡导的 RAII思想，用对象管理资源,当 shared_ptr 析构的时候，new 出来的对象也会被 delete掉。以此避免内存泄漏。
  > - 加了锁，使用互斥量来达到线程安全。这里使用了两个 if判断语句的技术称为**双检锁**；好处是，只有判断指针为空的时候才加锁，避免每次调用 get_instance的方法都加锁，锁的开销毕竟还是有点大的。
  >
  > 不足之处在于： 使用智能指针会要求用户也得使用智能指针，非必要不应该提出这种约束; 使用锁也有开销; 同时代码量也增多了，实现上我们希望越简单越好。

  ```c++
  #include <iostream>
  #include <memory> // shared_ptr
  #include <mutex>  // mutex
  
  // version 2:
  // with problems below fixed:
  // 1. thread is safe now
  // 2. memory doesn't leak
  
  class Singleton {
  public:
      typedef std::shared_ptr<Singleton> Ptr;
      ~Singleton() {
          std::cout << "destructor called!" << std::endl;
      }
      Singleton(Singleton&) = delete;
      Singleton& operator=(const Singleton&) = delete;
      static Ptr get_instance() {
  
          // "double checked lock"
          if (m_instance_ptr == nullptr) {
              std::lock_guard<std::mutex> lk(m_mutex);
              if (m_instance_ptr == nullptr) {
                  m_instance_ptr = std::shared_ptr<Singleton>(new Singleton);
              }
          }
          return m_instance_ptr;
      }
  
  
  private:
      Singleton() {
          std::cout << "constructor called!" << std::endl;
      }
      static Ptr m_instance_ptr;
      static std::mutex m_mutex;
  };
  
  // initialization static variables out of class
  Singleton::Ptr Singleton::m_instance_ptr = nullptr;
  std::mutex Singleton::m_mutex;
  ```

  

#### 饿汉式

```c++

// 饿汉实现 /
class Singleton
{
    
public:
    // 获取单实例
    static Singleton* GetInstance();
    // 释放单实例，进程退出时调用
    static void deleteInstance();
    // 打印实例地址
    void Print();

private:
    // 将其构造和析构成为私有的, 禁止外部构造和析构
    Singleton();
    ~Singleton();

    // 将其拷贝构造和赋值构造成为私有函数, 禁止外部拷贝和赋值
    Singleton(const Singleton &signal);
    const Singleton &operator=(const Singleton &signal);

private:
    // 唯一单实例对象指针
    static Singleton *g_pSingleton;
};

// 代码一运行就初始化创建实例 ，本身就线程安全
Singleton* Singleton::g_pSingleton = new (std::nothrow) Singleton;

Singleton* Singleton::GetInstance()
{
    return g_pSingleton;
}

void Singleton::deleteInstance()
{
    if (g_pSingleton)
    {
    
        delete g_pSingleton;
        g_pSingleton = NULL;
    }
}

void Singleton::Print()
{
    std::cout << "我的实例内存地址是:" << this << std::endl;
}

Singleton::Singleton()
{
    std::cout << "构造函数" << std::endl;
}

Singleton::~Singleton()
{
    std::cout << "析构函数" << std::endl;
}
// 饿汉实现 /
```

#### 面试题

- 懒汉模式和恶汉模式的实现（判空！！！加锁！！！），并且要能说明原因（为什么判空两次？）
- 构造函数的设计（为什么私有？除了私有还可以怎么实现（进阶）？）
- 对外接口的设计（为什么这么设计？）
- 单例对象的设计（为什么是static？如何初始化？如何销毁？（进阶））
- 对于C++编码者，需尤其注意C++11以后的单例模式的实现（为什么这么简化？怎么保证的（进阶））

单例模式（Singleton Pattern）是一种设计模式，确保一个类只有一个实例，并提供全局访问点来访问这个实例。虽然它在许多场景中非常有用，但也有一些缺点和潜在的问题：

 **全局状态问题**

单例模式本质上是一种全局状态的实现，这意味着单例对象在整个应用中都是共享的。这可能会导致以下问题：

- **状态管理困难**：由于单例对象是全局共享的，多个部分的代码可能会在不知不觉中改变它的状态，导致意外的副作用。
- **难以追踪依赖**：由于单例实例通常在多个地方被访问和修改，追踪某个类如何依赖单例实例变得非常困难，这可能会导致代码的可维护性降低。

**不易测试**

由于单例模式会在应用中创建全局唯一的实例，它通常会造成以下问题：

- **单元测试困难**：在测试中很难隔离单例的行为，因为每个测试可能会共享相同的单例实例。这使得单元测试变得更加复杂和不独立。
- **依赖注入受限**：单例模式直接控制实例的创建过程，导致它不容易与其他依赖注入框架（如Spring）配合使用，从而使得模块化和可测试性降低。

### 工厂模式

https://blog.csdn.net/qq_55882840/article/details/139043332

1.工厂模式简介      

工厂模式的三种类型：
- 简单工厂模式
- 工厂方法模式
- 抽象工厂模式
工厂模式的作用是生产对象，可以简化代码、提高可维护性，并旦可以通过工厂类生产多种对象。简单工厂模式适用于创建简单的对象，工厂模式适用于创建复杂的对象，而抽象工厂模式适用于创建更复杂的对象。

#### 2. 简单工厂模式

简单工厂模式是一种创建型设计模式，旨在通过一个工厂方法来创建对象，而无需直接暴露对象的实例化逻辑。简单工厂模式通常包括一个工厂类和多个产品类。工厂类负责根据客户端的请求，返回对应的产品类实例。就是用户申请一个产品，由工厂负责创建对象。而不是用户自己创建对象。在简单工厂模式中，客户端只需要通过调用工厂类的方法，并传入相应的参数，而无需直接实例化产品类。工厂类根据客户端传入的参数决定创建哪个产品类的实例，并将实例返回给客户端。

优点：

封装了实例化的细节，使得客户端与具体产品类解耦，增强了灵活性和可维护性。客户端只需要知道需要什么类型的产品，而无需关心具体的实现细节。同时，如果需要新增产品类时，只需修改工厂类即可，不需要修改客户端代码。      
缺点：

简单工厂模式也有一些限制。例如，当需要创建多种类型的产品实例时，工厂类的代码可能会变得复杂，并且随着产品类型的增加，工厂类的责任也会越来越大。因此，在一些复杂的场景下，可能需要使用其他更灵活的创建型设计模式，如工厂方法模式或抽象工厂模式。总的来说，简单工厂模式提供了一种简单而灵活的方式来创建对象，对于一些简单的对象创建场景是很有用的。

简单工厂模式
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
```
简单工厂类
```c++
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
客户端调用方式
```c++
int main() {
    Product* productA = SimpleFactory::createProduct("A");
    productA->operation(); // 调用具体产品A的操作
    Product* productB = SimpleFactory::createProduct("B");
    productB->operation(); // 调用具体产品B的操作
    delete productA;
    delete productB;
    return 0;
}
```





随着技术的不断进步，当前需要使用越来越多的方式来访问应用程序了。MVC 模式允许使用各种不同样式的视图来访问同一个服务端的代码，这得益于多个视图（如 WEB（HTTP）浏览器或者无线浏览器（WAP））能共享一个模型。

## STL

C++ STL从广义来讲包括了三类:算法，容器和迭代器。

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



## malloc机制

注意分配的时候会多要16字节，记录了这个malloc要的内存块大小信息。

## 内存泄漏

### 原因

1. 忘记释放内存
2. 智能指针可能会有循环引用

### 如何避免

1. 多使用智能指针，以及RAII机制

### 如何定位

1. 工具: valgrind

## 讲讲大端小端，如何检测

大端模式：是指**数据的高字节保存在内存的低地址**中，而数据的低字节保存在内存的高地址端

小端模式，是指**数据的高字节保存在内存的高地址**中，低位字节保存在在内存的低地址端

可以使用union-->所有成员都共享相同的内存地址,`union` 的大小由其最大成员的大小决定

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



## 为什么引用对象不可以修改

​	从编译的⻆度来讲，程序在**编译时分别将指针和引用添加到符号表**上，符号表中记录的是变量名及变量所对应地址。**指针变量在符号表上对应的地址值为指针变量的地址值**，而**引用在符号表上对应的地址值为引用对象的地址值（与实参名字不同，地址相同）。符号表生成之后就不会再改**，因此指针可以改变其指向的对象（指针变量中的值可以改），而引用对象则不能修改。



## 指针和引用的区别

指针和引用都是一种内存地址的概念，区别呢，**指针是一个实体，引用只是一个别名**。在程序编译的时候，**将指针和引用添加到符号表中。指针它指向一块内存，指针的内容是所指向的内存的地址，在编译的时候，则是将“指针变量名-指针变量的地址”添加到符号表中**，所以说，指针包含的内容是可以改变的，允许拷⻉和赋值，有 const 和非 const 区别，甚至可以为空，**sizeof 指针得到的是指针类型的大小**。

**而对于引用来说，它只是一块内存的别名，在添加到符号表的时候，是将"引用变量名-引用对象的地址"添加到符号表中**，符号表一经完成不能改变，所以引用必须而且只能在定义时被绑定到一块内存上，后续不能更改，也不能为空，也没有 const 和非 const 区别。**sizeof 引用得到代表对象的大小**。而 sizeof 指针得到的是指针本身的大小。另外在参数传递中，指针需要被解引用后才可以对对象进行操作，而直接对引用进行的修改会直接作用到引用对象上。



## 哪些函数不能是虚函数

- **构造函数**，构造函数初始化对象，派生类必须知道基类函数干了什么，才能进行构造；当有虚函数时，每一个类有一个虚表，每一个对象有一个虚表指针，虚表指针在构造函数中初始化；
- **内联函数**，内联函数在动多态的情况下不会内联，但是在没有涉及到多态时可以内联。
- **静态函数**，静态函数不属于对象属于类，静态成员函数没有this指针，因此静态函数设置为虚函数没有任何意义。
- **友元函数**，友元函数不属于类的成员函数，不能被继承。对于没有继承特性的函数没有虚函数的说法。
- **普通函数**，普通函数不属于类的成员函数，不具有继承特性，因此普通函数没有虚函数。



## 手动实现一个 智能指针

```c++
template<typename T>
class SharedPtr
{
public:
	SharedPtr() : _m_ptr(nullptr), _m_ref_count(nullptr) {}
	SharedPtr(T* ptr) : _m_ptr(ptr), _m_ref_count(new size_t(1)) {}

	SharedPtr(const SharedPtr& rhs) : _m_ptr(rhs._m_ptr), _m_ref_count(rhs._m_ref_count)
	{
		++(*_m_ref_count);
	}

	SharedPtr(SharedPtr&& rhs) noexcept : _m_ptr(rhs._m_ptr), _m_ref_count(rhs._m_ref_count)
	{
		rhs._m_ptr = nullptr;
		rhs._m_ref_count = nullptr;
	}

	SharedPtr& operator=(const SharedPtr& rhs)
	{
		if (this != &rhs)
		{
			_m_ptr = rhs._m_ptr;
			_m_ref_count = rhs._m_ref_count;
			++(*_m_ref_count);
		}
		return *this;
	}

	SharedPtr& operator=(SharedPtr&& rhs) noexcept
	{
		if (this != &rhs)
		{
			_m_ptr = rhs._m_ptr;
			_m_ref_count = rhs._m_ref_count;
			rhs._m_ptr = nullptr;
			rhs._m_ref_count = nullptr;
		}
		return *this;
	}

	~SharedPtr()
	{
		if (_m_ref_count && --(*_m_ref_count) == 0)
		{
			delete _m_ptr;
			delete _m_ref_count;
		}
	}
private:
	T* _m_ptr;
	size_t* _m_ref_count;
};
```



## 内存池

在 C++ 中，**动态内存分配（new/malloc）** 可能存在性能开销和内存碎片化问题，尤其是频繁分配和释放小对象时，性能损耗尤为明显。**内存池（Memory Pool）** 主要用于优化以下方面：

1. **减少分配开销**：`new`/`malloc` 可能会触发系统调用（如 `sbrk()` 或 `mmap()`），开销较大，而内存池可以提前预分配一大块内存，减少系统调用的次数。
2. **减少内存碎片**：系统分配的内存可能会导致碎片化，影响程序长期运行的稳定性。内存池通过预先规划的管理方式，减少碎片。

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

## extern"C"的用法

为了能够**正确的在C++**代码中调用**C**语言的代码：在程序中加上extern "C"后，相当于告诉编译器这部分

代码是C语言写的，因此要按照C语言进行编译，而不是C++；

## 野指针、悬空指针

野指针，指的是没有被初始化过的指针

```c++
int* p; // 未初始化
 std::cout<< *p << std::endl; // 未初始化就被使用
```

悬空指针，指针最初指向的内存已经被释放了的一种指针。

```c++
int * p = nullptr;
 int* p2 = new int;
 
 p = p2;
 delete p2;
```





## define宏定义和const的区别

1. **编译阶段**

   define是在编译的**预处理**阶段起作用，而const是在编译、运行的时候起作用

2. **安全性**

   define只做替换，不做类型检查和计算，也不求解，容易产生错误，一般最好加上一个大括号包含住

   全部的内容，要不然很容易出错

   const常量有数据类型，编译器可以对其进行类型安全检查

3. define只是将宏名称进行替换，在内存中会产生多分相同的备份。const在程序运行中只有一份备份，

**安全性**

define只做替换，不做类型检查和计算，也不求解，容易产生错误，一般最好加上一个大括号包含住

全部的内容，要不然很容易出错

const常量有数据类型，编译器可以对其进行类型安全检查

**内存占用**

define只是将宏名称进行替换，在内存中会产生多分相同的备份。const在程序运行中只有一份备份，

且可以执行常量折叠，能将复杂的的表达式计算出结果放入常量表

宏替换发生在编译阶段之前，属于文本插入替换；const作用发生于编译过程中。

宏不检查类型；const会检查数据类型。

宏定义的数据没有分配内存空间，只是插入替换掉；const定义的变量只是值不能改变，但要分配内

存空间。

## static关键字的作用与常用场景

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

   ```cpp
   static int staticVar = 0;  // 只能在当前文件中访问
   
   static void staticFunction() {  // 只能在当前文件中访问
       std::cout << staticVar << std::endl;
   }
   ```



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

### list

list 是⼀个双向链表

### 迭代器与指针的区别

迭代器实际上是对“遍历容器”这一操作进行了封装。迭代器不是指针，是类模板。重载了指针的一些操作符如：，->, * , ++, --等。

在编程中我们往往会用到各种各样的容器，但由于这些容器的底层实现各不相同，所以对他们进行遍历的方法也是不同的。例如，数组使用指针算数就可以遍历，但链表就要在不同节点直接进行跳转。

#### STL容器是线程安全的吗

STL容器不是线程安全的。对于vector，即使写方（生产者）是单线程写入，但是并发读的时候，由于潜在的内存重新申请和对象复制问题，会导致读方（消费者）的**迭代器失效**。实际表现也就是招致了core dump。另外一种情况，如果是多个写方，并发的push_back()，也会导致core dump。

1、加锁

2、通过固定vector的大小，避免动态扩容（无push_back）来做到lock-free！即在开始并发读写之前（比如初始化）的时候，给vector设置好大小。代码如下：

```
vector<int> v;
v.resize(1000);
```



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

2. auto不能用于多单词关键字

   ```c++
   auto l1 = long long{42}; //error
   auto l2 = llong{42};     //ok
   ```

3. 对于问题1的解决方案使用**decltype**运算符

   ```c++
   struct Foo
   {
       int x_;
       int& getX() { return x_; }
   };
   
   decltype(auto) x = getX(); //x将会是引用类型
   ```

4. 当用一个auto关键字声明多个变量的时候，编译器遵从由左 往右的推导规则，以最左边的表达式推断auto的具体类型

   ```c++
   int n = 5;
   auto *pn = &n, m = 10;
   //auto *pn = &n, m = 10.0; //编译失败
   ```

   这里先根据`&n`确定auto的类型为int。进而m的类型为int

5. 当使用条件表达式初始化auto声明的变量时，编译器总是使 用表达能力更强的类型：

   ```c++
   auto i = true ? 5 : 8.0; // i的数据类型为double
   ```

6. 按照C++20之前的标准，无法在函数形参列表中使用auto声 明形参（注意，在C++14中，auto可以为lambda表达式声明形参）

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
>  + 对齐值一般指的是该类型的大小。
>
>  + 结构体中的成员变量的初始地址需要是对齐值的整数倍
>  + 整个结构体的大小必须是 **最大对齐值**的大小。
>  + 所以，为了符合以上要求，编译器可能会在成员间/结构体末尾插入额外填充字节。





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

![](/Users/afourseven/blog/interview-trivia-question-master/img/3.JPG)

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

## 构造与析构函数顺序

当对象被创建时，**基类的构造函数总是先于派生类的构造函数调用**。调用顺序如下：

1. **基类的构造函数**（按继承层次从最顶层到最底层）
2. **派生类的构造函数**
3. **成员变量的构造（按照声明顺序）**

当对象销毁时，**析构顺序与构造顺序相反**，即先销毁派生类（`Derived`），再销毁基类（`Base`）。

### 为什么构造函数不能是虚函数

一个成员函数被声明为虚函数后，需要依赖vptr去vtable中查找函数，如果构造函数是虚函数，就会产生以下问题：**虚表还未构造完成**：在构造函数执行时，对象的 vtable 还没有初始化，无法正确解析虚函数调用。



### 为什么析构函数是虚函数

如果基类的析构函数不是虚函数，且通过基类指针删除派生类对象，**派生类的析构函数不会被调用**，可能导致资源泄露。

```c++
Base* obj = new Derived();
delete obj; //如果析构函数不是虚函数，这里调用的是~Base()，那么虚类的析构函数不会被调用
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

   



## 移动语义补充

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

2. 第二部分是对`foo(baz(&args...) + args...)`的解包，由于`baz(&args...)`已经被解包，因此相当于对`foo(baz(&a1, &a2, &a3) +args...)`解包，模式为：`baz(&a1, &a2, &a3) + args`最终得到的结果是：

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

![](/Users/afourseven/blog/interview-trivia-question-master/img/iterator-1.JPG)

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

   



## 智能指针补充

为了适应复杂环境下的使用需求，`std::shared_ptr`提供了下面几个辅助类

- weak_ptr
- bad_weak_ptr
- enable_shared_from_this



### weak_ptr

在某些使用环境下，`std::shared_ptr`的引用计数会出现问题导致资源无法释放，这里需要使用`weak_ptr`。

- 存在循环引用时
- 希望共享但是又不拥有对象的所有权

上述情况下都可以使用`weak_ptr`，当最后一个`shared_ptr`引用失效时，对应的`weak_ptr`自动为空。你不能对`weak_ptr`使用`*`和`->`运算符。因此访问管理的 对象必须转换为`std::shared_ptr`因为如下原因：

1. 转换成`std::shared_ptr`以检查对象是否被释放
2. 当正在处理对象时，`std::shared_ptr`不会被释放，而`std::weak_ptr`可能被释放

介绍一个循环引用导致内存泄漏的例子：

![](/Users/afourseven/blog/interview-trivia-question-master/img/1.JPG)

上图中，`std::shared_ptr<kid>`的引用计数为：3，当user释放掉kid后，kid由于循环引用导致计数为：2，因此不会释放，而导致内存泄漏。

因此可以考虑，将mom和dad指向kid的指针设计为`weak_ptr`

![](/Users/afourseven/blog/interview-trivia-question-master/img/2.JPG)

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



## 右值

一个区分左值和右值的简单方法，就是看能不能对表达式取地址。

> 纯右值：像是一个函数返回非引用变量、运算表达式产生的临时变量、lambda表达式、原始字面量都是纯右值
>
> 将亡值: 就是要被移动的对象、T&&函数返回值、move返回值、static_cast<T&&>(val)类型转换

常量引用也可以赋予右指，比如`const int& = 1/const string& = "hello"`





右值引用T&& 有一个特点就是它是一个未定引用类型(universal references)，如果被一个左值初始化，它就是一个左值，被右值初始化就是右值——都取决于初始化。（**仅仅是发生自动类型推导，比如函数模板推到，auto得到时候才表现在出universal references**

```
template<typename T>
void f(T&& param); 

int a;
f(a);   // 传入左值,那么上述的T&& 就是lvalue reference,也就是左值引用绑定到了左值
f(1);   // 传入右值,那么上述的T&& 就是rvalue reference,也就是右值引用绑定到了左值xw
```



> 有关引用折叠：
>
> 





## 智能指针

智能指针是一个RAII类型，它把指针封装成类，然后当离开作用域的时候调用析构函数，自动释放资源。

>  `auto_ptr`在c++17中被删除了，因为它有一些设计缺陷：
>
> + 一个重要的原因是`auto_ptr`被复制的时候，它将原始指针直接转移给新对象而不是直接复制。*很容易造成意外的资源管理错误*
> + 在与容器配合使用的时候，比如vector<auto_ptr<int>> v， 当v发生复制的时候，原本v中的auto_ptr就不再拥有了资源，于是析构的时候会尝试释放这些已经被转移的资源

### unique_ptr

`std::unique_ptr`是独占的智能指针，它对持有的堆内存有唯一的拥有权，引用计数是1.`unique_ptr`禁止了复制语义，也就是说类似:

```c++
std::unique<ptr> sp1 = std::make_unique<ptr>(1);
std::unique<ptr> sp2(sp1); //无法通过编译
sp2 = sp1; //无法通过编译
```

但是，如果是函数返回一个`std::unique_ptr`还是允许的。

由于不能复制，于是`std::unique_ptr`使用移动构造的方式将持有的堆内存转移给另一个。

> 对于初始化智能指针，推荐的是make_unique这样的初始化方式

在默认情况下，智能指针在析构的时候只有释放持有的堆内存，但是如果有一些除了堆内存外还有要收回的资源（比如操作系统的套接字句柄、文件句柄等。可以自定义资源释放函数。

`std::unique_ptr<T, DeletorFuncPtr>`



### shared_ptr

`shared_ptr`持有的资源可以在多个`shared_ptr`之间共享，每多一个 **std::shared_ptr** 对资源的引用，资源引用计数将增加 1，每一个指向该资源的 **std::shared_ptr** 对象析构时，资源引用计数减 1，最后一个 **std::shared_ptr** 对象析构时，发现资源计数为 0，将释放其持有的资源。多个线程之间，递增和减少资源的引用计数是安全的。`use_count()`可以打印出引用计数。

当有这样的需求的时候： 返回包裹给当前对象(this)的一个std::shared_ptr对象给外部使用。

```
class A : public std::enable_shared_from_this<A>
{
public:
    A()
    {
        std::cout << "A constructor" << std::endl;
    }

    ~A()
    {
        std::cout << "A destructor" << std::endl;
    }

    std::shared_ptr<A> getSelf()
    {
        return shared_from_this();
    }
};

int main()
{
    std::shared_ptr<A> sp1(new A());

    std::shared_ptr<A> sp2 = sp1->getSelf();

    std::cout << "use count: " << sp1.use_count() << std::endl;

    return 0;
}
```

于是只需要继承` std::enable_shared_from_this<A>`

需要注意的是：

1. 不能共享栈对象的this给智能指针对象

```
A a;
std::shared_ptr<A> sp2 = a.getSelf(); //崩溃
std::cout << "use count: " << sp2.use_count() << std::endl; 
```

智能指针设计来是为了管理堆对象的。栈对象会在函数调用结束后自己销毁。

`shared_ptr`会出现循环引用的情况，多个对象互相持有对方的`shared_ptr`，这时候它们的引用计数永远不为0，因此对象的析构函数不会被调用，内存也不会被释放。

### weak_ptr

**std::weak_ptr** 是一个不控制资源生命周期的智能指针，是对对象的一种弱引用，只是提供了对其管理的资源的一个访问手段，引入它的目的为协助 **std::shared_ptr** 工作。

**std::weak_ptr** 可以从一个 **std::shared_ptr** 或另一个 **std::weak_ptr** 对象构造，**std::shared_ptr** 可以直接赋值给 **std::weak_ptr** ，也可以通过 **std::weak_ptr** 的 **lock()** 函数来获得 **std::shared_ptr**。它的构造和析构不会引起引用计数的增加或减少。**std::weak_ptr** 可用来解决 **std::shared_ptr** 相互引用时的死锁问题（即两个**std::shared_ptr** 相互引用，那么这两个指针的引用计数永远不可能下降为 0， 资源永远不会释放）。

由于`weak_ptr`不管理对象的生命周期，那么其引用的对象可能在某个时刻已经被销毁了——需要通过`expired()`方法来做这个测试。当返回true，表示资源已经销毁了；返回false,表示资源依然存在，这个时候就使用用`weak_ptr`的`lock()`方法得到一个`shared_ptr`对象然后继续操作。

*那么既然我们已经通过`expired()`知道了没有被销毁，为什么不直接操作weak_ptr，而是需要使用lock()呢？*这是因为**std::weak_ptr** 类没有重写 **operator->** 和 **operator*** 方法，因此不能像 **std::shared_ptr** 或 **std::unique_ptr** 一样直接操作对象，同时 **std::weak_ptr** 类也没有重写 **operator!** 操作，因此也不能通过 **std::weak_ptr** 对象直接判断其引用的资源是否存在。

`std::weak_ptr`的使用场景应该是，*如果那些资源可能就使用，不可用就不用的场景*。想象一个订阅者场景，消息发布器只有在某个订阅者存在的情况下才会向其发布消息，而不能管理订阅者的生命周期。



### 智能指针的大小

在 32 位机器上，**std_unique_ptr** 占 4 字节，**std::shared_ptr** 和 **std::weak_ptr** 占 8 字节；在 64 位机器上，**std_unique_ptr** 占 8 字节，**std::shared_ptr** 和 **std::weak_ptr** 占 16 字节。也就是说，**std_unique_ptr** 的大小总是和原始指针大小一样，**std::shared_ptr** 和 **std::weak_ptr** 大小是原始指针的一倍。



### tips

1. 作为类成员变量时，应该优先使用前置声明（forward declarations）

```c++
class A;
class Test {
  private:
  	std::unique_ptr<A> m_spA;
};
```

前置声明的方法，可以避免直接引入`A.h`这样整个头文件。