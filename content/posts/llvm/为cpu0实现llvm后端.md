---
title: "为cpu0实现llvm后端"
date: 2024-11-14T19:50:48+08:00
Categories: [编译器,llvm]
---
这是记录如何实现一个cpu0架构的llvm后端。

## 基础知识

由于对于编译原理、计算机组成都有些手生，这个章节专门用于记录一些基础知识。



## 第一部分

### Cpu0基本架构

首先接受一下Cpu0的**基本架构**

> - 32 位 RISC 架构；
> - 16 个[通用寄存器](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=通用寄存器&zhida_source=entity)，R0 到 R15；
>   - R0 是常数 0 寄存器（CR, Constant Register）
>   - R1-R10 是通用寄存器（GPR, General Purpose Register）
>   - R11 是全局指针寄存器（GP, Global Pointer register）
>   - R12 是帧指针寄存器（FP, Frame Pointer register）
>   - R13 是栈指针寄存器（SP, Stack Pointer register）
>   - R14 是链接寄存器（LR, Link Register）
>   - R15 是[状态字寄存器](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=状态字寄存器&zhida_source=entity)（SW, Status Word register）
> - 协处理寄存器，PC 和 EPC；
>   - PC 是程序计数器（Program Counter）
>   - EPC 是错误计数器（Error Program Counter）
> - 其他寄存器；
>   - IR 是[指令寄存器](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=指令寄存器&zhida_source=entity)（Instruction Register）
>   - MAR 是[内存地址寄存器](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=内存地址寄存器&zhida_source=entity)（Memory Address Register）
>   - MDR 是内存数据寄存器（Memory Data Register）
>   - HI 是 MULT 指令的结果的高位存储（high part）
>   - Lo 是 MULT 指令的结果的低位存储（low  part）

**Cpu0指令集**

指令分三种

+ A 类（Arithmetic 类）：用来做算术运算

| 操作码编码 OP | 返回寄存器编码 Ra | 输入寄存器编码 Rb | 输入寄存器编码 Rc | 辅助操作编码 Cx |
| ------------- | ----------------- | ----------------- | ----------------- | --------------- |
| 31-24         | 23-20             | 19-16             | 15-12             | 11-0            |

+ L 类（Load/Store 类）：用来访问内存

| 操作码编码 OP | 返回寄存器编码 Ra | 输入寄存器编码 Rb | 辅助操作编码 Cx |
| ------------- | ----------------- | ----------------- | --------------- |
| 31-24         | 23-20             | 19-16             | 15-0            |

+ J 类（Jump 类）：用来改变控制流。

| 操作码编码 OP | 辅助操作编码 Cx |
| ------------- | --------------- |
| 31-24         | 23-0            |

> 注意，根据我看的教程，作者说了教程支持的Cpu0架构有两款，对应了两款不同的ISA(Instruction Set)。第一套叫cpu032I。第二套是在第一套的基础上新增了几条指令，叫 cpu032II。
>
> cpu032I 中的[比较指令](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=比较指令&zhida_source=entity)继承自 ARM 的 CMP，而 cpu032II 中的比较指令新增了继承自 Mips 的 SLT，BEQ 等指令。

**状态字寄存器**

SW 寄存器用来标记一些状态，它的位模式为

| 保留           | 中断标记 I | 模式标记 M | 调试标记 D | 溢出标记 V | 进位标记 C | 零标记 Z | 负数标记 N |
| -------------- | ---------- | ---------- | ---------- | ---------- | ---------- | -------- | ---------- |
| 31-14, 12-9, 4 | 13         | 8-6        | 5          | 3          | 2          | 1        | 0          |

CMP 指令主动设置这个寄存器，条件分支指令会参考这里的值作为条件来跳转。

**指令流水线**

Cpu0 的指令采用 5 级流水线：取指（IF, Instruction Fetch）、解码（ID, Instruction Decode）、执行（EX, EXecute）、内存访问（MEM, MEMory access）、写回（WB, Write Back）。

**取指、解码、执行**时任何指令都会做的步骤，**内存访问**是针对 load/store 指令，**写回**是针对 load。

### LLVM代码结构

- docs/
  放着一些文档，很多文档在官方上能找到。
- examples/
  存放着一些官方认可的示例，比如有很简单的 Fibonacci 计算器实现，简单的前端案例 Kaleidoscope（这个是有个教程的），介绍 JIT 的 HowToUseJIT。不过这里没有后端的东西。
- include/
  存放 llvm 中作为库的那部分接口代码的 API 头文件。注意不是所有头文件，内部使用的头文件不放在这里。其中我们关心的都在 `include/llvm` 中
  `include/llvm` 中，按库的名称来划分子目录，比如 Analysis，CodeGen，Target 等。
- lib/
  存放绝大多数的源码。
  - lib/Analysis
    两个 LLVM IR 核心功能之一，各种[程序分析](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=程序分析&zhida_source=entity)，比如变量活跃性分析等。
  - lib/Transforms
    两个 LLVM IR 核心功能之二，做 IR 到 IR 的程序变换，比如死代码消除，[常量传播](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=常量传播&zhida_source=entity)等。
  - lib/IR
    LLVM IR 实现的核心，比如 LLVM IR 中的一些概念，比如 BasicBlock，会在这里定义。
  - lib/AsmParser
    LLVM 汇编的 parser 实现，注意 LLVM 汇编不是机器汇编。
  - lib/Bitcode
    LLVM 位码 (bitcode) 的操作。
  - lib/Target
    目标架构下的所有描述，包括指令集、寄存器、机器调度等等和机器相关的信息。我们的教程主要新增代码都在这个路径下边。这个路径下又会细分不同的后端平台，比如 X86，ARM，我们新增的后端，会在这里新开一个目录 Cpu0。
  - lib/CodeGen
    代码生成库的实现核心。LLVM 官方会把后端分为目标相关的（target dependent）代码和目标无关的（target independent）代码。这里就存放这目标无关的代码，比如指令选择，[指令调度](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=指令调度&zhida_source=entity)，寄存器分配等。这里的代码一般情况下不用动，除非你的后端非常奇葩。
  - lib/MC
    存放与 Machine Code 有关的代码，MC 是后端到挺后边的时候，代码发射时的一种中间表示，也是整个 LLVM 常规[编译流程](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=编译流程&zhida_source=entity)中最后一个中间表示。这里提供的一些类是作为我们 lib/Target/Cpu0 下的类的基类。
  - lib/ExecutionEngine
    解释执行位码文件和 JIT 的一些实现代码。

- projects/
  刚开始接触 LLVM 时，以为这里是开发的重点，实际并不是。这个路径下会放一些不是 LLVM 架构，但会基于它的库来开发的一些第三方的程序工程。如果你不是在 LLVM 上搭建一个前端或后端或优化，而是基于他们的一部分功能来实现自己的需求，可以把代码放在这里边。
- test/
  LLVM 支持一整套完整的测试，测试工具叫 [lit](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=lit&zhida_source=entity)，这个路径下放着各种测试用例。LLVM 的测试用例有一套自己的规范。
- unittests/
  顾名思义，这里放着[单元测试](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=单元测试&zhida_source=entity)的测试用例。
- tools/
  这个目录里边放着各种 LLVM 的工具的源码（也就是驱动那些库的驱动程序），比如做 LLVM IR 汇编的 [llvm-as](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=llvm-as&zhida_source=entity)，后端编译器 llc，优化驱动器 opt 等。注意，驱动程序的源码和库的源码是分开的，这是 LLVM 架构的优势，你完全可以说不喜欢 llc，然后自己在这里实现一个驱动来调你的后端。
- utils/
  一些基于 LLVM 源码的工具，这些工具可能比较重要，但不是 LLVM 架构的核心。 里边有个目录用 vim 或 emacs 的朋友一定要看一下，就是 `utils/vim` 和 `utils/emacs`，里边有些[配置文件](https://zhida.zhihu.com/search?content_id=166511868&content_type=Article&match_order=1&q=配置文件&zhida_source=entity)，比如自动化 LLVM 格式规范的配置，高亮 TableGen 语法的配置，调一调，开心好几天有没有。

### Cpu0后端初始化

#### TableGen 描述

实现后端的时候我们首先需要编写和目标相关的描述文件，也就是.td，.td 文件会有多个，分门别类的来描述目标平台的各种信息，比如寄存器信息、指令信息、调度信息等。

这些.td文件会在build编译器的时候被转换为c++源码，然后这些源码就可以在代码中使用。.td 文件在被处理之后，会在 build 路径下的 `lib/Target/Cpu0/` 下，生成一些 .inc 文件，而这些文件就可以被我们的代码所 include。生成的规范是我们在 CMakeLists.txt 中明确的。

**注意，实际逻辑是，我们在写.td文件时就应该知道这些文件转换后的c++源码是什么样子，然后我们在代码中使用还没有生成的代码。所以我们会遇到静态检查无法通过的情况，但是只要.td文件是对的，编译是能通过的，因为TableGen是首先被调用**

