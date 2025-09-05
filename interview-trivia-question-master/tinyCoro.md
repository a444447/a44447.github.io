# 协程入门

## 有栈协程和无栈协程

需要注意的是，协程可以按调度的分配来分为：**对称协程和非对称协程**

如果按栈的分配方式又可以分为 **有栈协程和无栈协程**

### 有栈协程

理解有栈协程的时候最好结合 **普通函数调用的栈帧**。当普通函数调用的时候都会生成一个栈帧。

> 一般来说生成的栈帧是，先压入rbp(上一个栈帧的基地址，这样才能返回)，然后更新当前的rbp，再一次压入入参以及函数内部的局部变量。

所以对于普通函数来说，无论是函数的切换还是递归，本质就是栈帧的切换。有栈协程利用的也是这个思想，它分配一个内存空间存储**当前的上下文**，这样就可以手动切换函数，想要恢复函数只要去持有函数上下文的那个内存区域拿出上下文即可。这样的能够自由切换的**有栈协程也是对称协程。**

可以结合linux的`ucontext`库来看这点。u

### 无栈协程

无栈协程可以把协程当作一个类，里面有执行业务逻辑的成员函数，以及记录状态的成员变量。"类"中的成员函数就像状态机，整个函数被分为了几个部分，当遇到某个节点需要切换协程的时候，会改变记录状态的成员变量。当下一次恢复的时候，再次执行"类"的成员函数，就会根据**不同的状态继续执行下一个部分**。我们要做的就是自行实现切换的调度逻辑。

**所有的协程共用的都是一个栈，即系统栈，也就不必自行去给协程分配栈，因为是函数调用，我们当然也不必去显式的保存寄存器的值。**

虽然我们说无栈协程不需要和有栈协程一样为栈信息开辟空间，*无栈协程仍然需要内存空间存储当前状态，只是存储的内容与有栈协程不同*。而且无栈协程属于非对称协程，因此它无法再任意函数调用层级挂起。

c++20是 **无栈协程**。想一下c中通过label-goto这样的方式进行跳转， C++ 协程后大家可以观察到协程的汇编代码存在大量的 label 和 jmp 指令，这正好对应了上述讲到的状态机。



### c++ 协程设计思路

我们之前说了，对于无栈协程，*它是把协程当作一个类，并且在需要切换的地方加入调度点，函数自然就被划分为几个部分*。所以c++协程内置了几个关键字把函数进行切分（其实也就是这个关键字把函数设置了几个调度点）

C++ 的函数包含指定关键字（**co_await，co_yield，co_return**）时，编译器会将其看作协程，而在关键字出现的地方编译器会安插调度点，在调度点用户可以使用协程的方法来指定协程是继续运行还是选择切换执行流程。



> **❓ C++ 协程的创建需要额外的内存，为何说是无栈协程？** C++ 协程不保存栈状态，而是通过对协程状态机的设计，使用了堆内存保存自身运行的某些状态，因此可以理解为仍然需要额外分配内存，但保存的并不是栈结构因此从定义上讲属于无栈协程

> **❓ 怎么理解 C++ 协程是非对称协程？** 在非对称协程中，协程的控制流是单向的，协程让出控制权时只能返回给它的直接调用者。C++20 协程通过 co_await 挂起时，会返回到调用者或恢复者，**而不是直接切换到另一个协程**，而对称协程让出控制权时可以随意指定协程。 在后续的讲解中我们可以看到 C++ 协程可以通过对称转换优化来实现对称协程的行为。

### promise

要写一个协程函数，c++规定了需要返回一个用户自定义的类`UserFacing`， 这个类在方法上没有限制，需要提供该类的promise_type，只要满足了这个就可以作为协程函数的返回值。

```
class UserFacing
{
public:
  class promise_type; // UsingFacing 需要满足的限制
};

UserFacing coro() // 协程返回值较为特殊，必须是 UserFacing 类型
{
  // coding .....
  co_return;
}
```

反正最简单的理解就是，**协程一定是需要返回一个自定义函数的，并且这个函数必须要给它指定promise_type**。

> 为什么一定要设计成单独自定义一个类？
> 因为编译器对 promise 做了诸多限制，且 promise 持有协程运行的数据，而面向用户的对象可以让用户自定义如何去操作 promise 的数据，数据与处理逻辑分离开来算是设计上的解耦。 从后续学习中我们也可以看到协程本身是需要存储状态以及数据的，UserFacing 像是为调用者提供了一个入口来操纵协程。

#### coroutine_handle

coroutine_handle是一个协程的句柄，是一个类模板`coroutine_handle<promise_type> handle=...`。通过这个handle可以：

- **handle.promise()**。通过该方法可以从协程句柄获取 promise。
- **handle.done()**。该方法用于判定协程是否执行结束。
- **handle.resume()**。该方法可以使暂停的协程继续运行，注意如果此时 handle 关联的协程执行结束，调用该方法会产生 core dump。
- **handle.destroy()**。该方法负责协程帧内存的回收，用户需要避免重复调用。

```
注意,可直接使用 coroutine_handle<>即模板参数默认为空，这类似于 void*指针,存储任意类型的 promise。但此时无法调用 handle.promise() 方法，用户若想获取存储的 promise 需要使用类型转换。
```

#### promise

promise就是协程核心的数据结构了，用户通过promise来访问编译器为协程分配的内存，也可以用promise存储协程运行的时候的临时值。

##### 协程的构造

调用协程的时候，编译器会去找该协程绑定的promise_type,然后在协程帧上将promise_type对应的promise对象构造出来，这个时候需要选取promise的构造函数

> 可以为 promise 类指定多个构造函数.
>
> **C++ 协程设计为何要将协程的参数列表与 promise 构造函数关联起来**？ 因为协程不仅可以是普通函数，还可以是类的成员函数。读者应该了解 C++ 中对象调用成员函数都会隐式的传递 this 指针，而在编译器视角看成员函数的第一个参数也是类指针，用户不需要显式的添加。
>
> 同理，协程作为成员函数时参数列表也不需要添加类指针，但此时编译器构造协程对象会传递 this 指针，所以 promise 的构造函数参数需要带有类指针，这样协程运行过程中才可以通过类指针访问类成员和方法。

##### get_return_object

> ```
> // 函数原型
> UserFacing promise::get_return_object();
> ```

用户调用协程时获取的 UserFacing 对象是编译器通过 promise 的 get_return_object 函数构造出来的，该函数参数为空，返回类型需要与协程的返回类型一致。

##### initial_suspend

> ```
> // 函数原型
> awaiter promise::initial_suspend();
> ```

前文我们提到 C++ 为协程设计了多个调度点，第一个调度点便是在协程创建时，由 initial_suspend 方法实现调度逻辑。比如调用协程并构造完协程帧后，编译器就会调用和协程相关的promise对象的initial_suspend方法通过返回awaiter来决定是直接运行协程还是暂停执行...

C++ 官方提供了默认的 awaiter 实现：

- **std::suspend_always**。暂停协程执行，执行权返回给调用者。
- **std::suspend_never**。协程继续执行。

##### final_suspend

与 initial_suspend 类似，final_suspend 函数负责协程执行结束后的调度点逻辑，返回值同样是 awaiter 类型，用户可以通过自定义 awaiter 来转移执行权，也可以直接返回 std::suspend_alaways 或者 std::suspend_never，调用 final_suspend 函数时会执行下列伪代码：

```
co_await p.final_suspend();
destruct promise p
destruct parameters in coroutine frame
destroy coroutine state
```

换句话说，如果 final_suspend 返回了 suspend_never，那么编译器会接着执行后续的资源清理操作，如果 UserFacing 在析构函数中再次执行 handle.destroy，那么会出现 core dump，所以一般建议不要返回 suspend_never，因为资源的释放最好在用户侧来做。

##### co_return & return_value

协程的 co_return 就像普通函数的 return 一样，用于终止协程并选择性的返回值。根据 co_return 是否返回值，编译器会做出不同的处理：

- **不返回值**。此时 co_return 仅用于终止协程执行，编译器随后调用 promise.return_void 方法，此函数可实现为空，在某些情况下也可以执行协程结束后的清理工作，但用户必须为 promise 定义 return_void 方法。
- **返回值**。假设 co_return 返回值的类型为 T，此时编译器调用 promise.return_value 方法，并将 co_return 的返回值作为参数传入，**用户可以自定义 return_value 函数的参数类型**，就像调用正常函数一样，只要 T 可以转换为该参数类型即可。样例程序中因为 co_return 返回了值，所以 promise_type 也增添了一个成员函数用于存储该值，在 return_value 函数体内完成赋值。

需要注意的是 C++ 标准规定 return_value 和 return_void 函数不能同时存在，并且当协程不存在 co_return 关键字时用户也需要定义 return_void 方法，因为协程执行结束后编译器会隐式调用该函数。

> 需要关注的一点是，协程函数和普通函数的返回值处理不一样，普通函数就是返回值所以可以直接渠道，协程函数对于调用方来说返回的是UserFacing对象，那么怎么拿到co_return的值？一般的逻辑是在 promise 中增加一个成员变量并在 return_value 函数中为其赋值，co_return 后协程执行确实结束了，但协程帧并不会自动回收，promise 对象依然存在，用户可以在 UserFacing 中添加获取该值的方法，UserFacing 一般存储了 promise 的 coroutine_handle，通过该 handle 访问 promise 的成员变量。

##### co_yield & yield_value

> ```
> // 函数原型
> co_yield T;
> awaiter promise::yield_value(T);
> ```

co_yield 用于协程在运行过程中向外输出值。与 co_return 类似，我们也需要在 promise 中为其新增成员变量，当执行到 co_yield 语句时，编译器调用 yield_value 方法，co_yield 的值作为参数，函数体内将该值赋予给 promise 成员变量。外部访问该 co_yield 的值的流程与 co_return 类似。

与 co_return 不同的是，co_yield 之后协程的运行并不一定结束，所以 yield_value 通过返回 awaiter 类型来决定协程的执行权如何处理，一般返回 std::suspend_alaways 转移控制权到调用者，用户也可返回自定义的 awaiter，但通常不要返回 std::suspend_never 等让协程继续运行的 awaiter，因为此时协程继续运行的话如果再次碰到 co_yield 那么上次 yield 的值就会被覆盖。

##### unhandled_exception

> ```
> // 函数原型
> void promise::unhandled_exception();
> ```

如果协程在运行过程中抛出了异常且没有捕获，那么协程的运行会提前终止，且无法通过 coroutine_handle 恢复协程。此时编译器调用 promise 的 unhandled_exception 方法，该方法没有参数，我们通常实现该函数为利用标准库提供的 std::current_exception 方法获取当前发生的异常，并将异常作为变量存储，注意异常不会再向上传播。此时控制权转移到协程调用者，用户可以在 UserFacing 的方法中获取存储的异常，并再次抛出异常，如样例程序中 Task 的 next 方法所示。

>  **💡为何普通函数在抛出异常未捕获后异常会一直向上传递直到被捕获，而协程抛出异常未捕获却并不会向上传递？** C++ 协程关于异常处理的流程如下所示，编译器为我们隐式的添加了 try/catch 语句，因此异常并不会传播到调用者。综合来看 C++ 协程的设计者通过 unhandled_exception 使得协程的异常处理更加灵活。

```
try{
 // coroutine body
} catch {
 promise.unhandled_exception();
}
```

#### awaiter

刚才说的协程设计了多种类型调度点，这是调度点的逻辑都需要在awaiter里面实现。C++ 协程标准要求 awaiter 必须实现下列三个方法：

- **await_ready**
- **await_suspend**
- **await_resume**

# 项目

## 异步IO执行模块封装

### 1. **基于 `base_io_awaiter` 生成 `io_awaiter`**

你从 `base_io_awaiter` 继承并构造自己的 `awaiter` 类（例如 `tcp_accept_awaiter`），这一步的目的是让你为每种 I/O 操作定义一个特定的 **awaiter 类型**。

- `base_io_awaiter` 作为所有 I/O 操作的基类，负责处理挂起（`await_suspend`）和恢复（`await_resume`）协程的逻辑。
- `io_info` 结构体包含了协程句柄、回调函数等上下文信息，它会在 `awaiter` 内部传递和使用。

### 2. **调用 `co_await custom_io_awaiter` 的流程**

当你在协程中调用 `co_await` 来等待 I/O 操作时，以下过程会发生：

1. **构造 `awaiter`**：
   - `co_await` 会构造一个 `io_awaiter` 类的实例（比如 `tcp_accept_awaiter`）。
   - 在构造函数中，你会准备 I/O 操作的参数，并将其填充到 `sqe` 中，同时将 `io_info` 绑定到 `sqe`。
2. **提交 I/O 操作**：
   - 在 `awaiter` 的构造函数里，调用 `local_engine().add_io_submit()`，将 I/O 操作提交到 `engine`，这时 I/O 请求被放入内核的提交队列（SQE）。
3. **挂起协程**：
   - `await_suspend()` 方法会在这里执行，协程会进入 **挂起状态**。这时，`io_info` 的 `handle` 会保存当前协程的句柄，并且协程将等待 I/O 完成。
   - `await_suspend` 不会立即恢复协程，而是将其挂起，等待外部的 I/O 完成回调来恢复。

### 3. **唤醒工作线程并提交 I/O 操作**

1. **`poll_submit` 被调用**：
   - `poll_submit()` 会检查是否有新的 I/O 操作已经完成。如果没有完成的任务，它会通过 `m_upxy.write_eventfd()` 唤醒阻塞的工作线程。
   - 这个唤醒操作使得 `context::run` 中的工作线程能够继续执行，并开始处理任务队列中的协程。
2. **工作线程执行 `exec_one_task`**：
   - `exec_one_task()` 会从任务队列 `m_task_queue` 中取出一个协程句柄，并恢复该协程的执行（调用 `handle.resume()`）。
   - `exec_one_task()` 的关键点是从任务队列中拿出一个任务，并恢复其执行。

### 4. **I/O 完成后的 `callback` 执行**

- 当 I/O 操作完成时（例如 TCP 连接成功），**回调函数**会被调用。
- 回调函数在 `callback` 中执行时，会把 I/O 操作的结果（如接收的数据、错误码等）放回 `io_info.result`。
- 然后，回调函数调用 `submit_to_context(data->handle)`，将 **协程句柄** 提交到当前 `context` 中的任务队列，准备恢复协程。

### 5. **恢复协程：**

1. **`submit_to_context(data->handle)`**：
   - 这将协程句柄（`data->handle`）放入 `context` 的任务队列，确保工作线程能够从队列中获取任务。
   - 工作线程在 `exec_one_task()` 中通过 `m_task_queue.pop()` 获取任务并恢复执行。
2. **协程恢复执行**：
   - 当工作线程从队列中取出协程句柄时，调用 `handle.resume()` 恢复协程执行。协程将继续从挂起的位置开始执行。

### 6. **总结**

- **`awaiter` 通过 `base_io_awaiter` 继承实现具体的 I/O 操作**，并在 `await_suspend()` 中挂起协程，等待 I/O 完成。
- **提交 I/O 操作时**，I/O 会被提交到 `engine`，同时通过 `add_io_submit()` 唤醒工作线程，等待执行。
- **当 I/O 完成时**，回调函数被执行，并通过 `submit_to_context(data->handle)` 恢复挂起的协程。
- **工作线程** 在 `poll_submit()` 被唤醒后，调用 `exec_one_task()` 从任务队列中取出协程，恢复执行。

### 7. **为什么 `submit_task` 恢复协程**

- `submit_task` 将协程句柄放回任务队列（`m_task_queue`）。
- `exec_one_task` 通过 `m_task_queue.pop()` 取出协程句柄并恢复执行。
- 恢复执行的协程从 I/O 操作完成后的挂起点继续运行，完成原本的任务。

这种设计的关键在于通过**任务队列和 `submit_task`**，将每个 I/O 操作的协程句柄保存起来，确保在 I/O 完成后可以通过工作线程恢复协程的执行。
