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