---
title: "Cmake入门"
date: 2024-05-29T19:04:12+08:00
categories: ["技术","未分类"]
---



{{< imgcap src="https://obsdian-1304266993.cos.ap-chongqing.myqcloud.com/typora/DALL%C2%B7E%202024-05-29%2023.41.31%20-%20A%20girl%20running%20energetically%20with%20a%20joyful%20expression%2C%20in%20a%20style%20reminiscent%20of%20the%20artist%20Mai%20Yoneyama.%20She%20has%20flowing%20hair%20and%20is%20dressed%20in%20a%20con.webp" title="DALL·E生成的图片，手部出现了bug">}}

{{< notice info >}}

旨在简单入门cmake的常用语法

{{< /notice >}}

## 最基本的结构

一个`CMakeLists.txt`的基本结构是这样的

```cmake
cmake_minimum_required(VERSION 3.18)

project(cmake_test)

set(CMAKE_CXX_STANDARD 17)

add_executable(cmake_test main.cc)
```

为了避免编译产生的中间文件污染我们的源代码，我们一般会新建一个`build`文件夹，然后在`build/`文件夹里面执行`cmake`构建。

## 详解

### CMakeLists中的变量

我们可以使用`SET()`来定义变量，比如`SET(LSTC main.cc other.cc)`,那么`LSTC`就会代替后面的字符串

我们可以使用`${NAME}`来获得变量的value

---

下面是cmake中常用的变量

|      变量名      |                             描述                             |
| :--------------: | :----------------------------------------------------------: |
| CMAKE_BINARY_DIR | 这个目录是执行cmake命令的时候，所在的目录。比如通过`cd build`,然后`cmake ..`，此时`build/`路径就是CMAKE_BINARY_DIR |
| CMAKE_SOURCE_DIR | 指的是包含CMakeLists.txt的路径，如果`CMAKE_BINARY_DIR`指向`path/to/project/build`,`CMAKE_SOURCE_DIR`指向`path/to/project` |
|                  |                                                              |
|                  |                                                              |
|                  |                                                              |
