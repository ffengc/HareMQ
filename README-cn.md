![](./assets/1.png)

<div align="center">

# HareMQ

仿照RabbitMQ实现一个C++版本简化消息队列组件。为了学习RabbitMQ，这个项目囊括了高性能消息系统的精髓和核心功能，模仿 RabbitMQ 的基本架构，专注于发布-订阅机制。

<a href="https://github.com/ffengc">
    <img src="https://img.shields.io/static/v1?label=Github&message=ffengc&color=blue" alt="ffengc.github">
</a>
<a href="https://ffengc.github.io">
    <img src="https://img.shields.io/static/v1?label=Page&message=ffengc.github.io&color=red" alt="ffengc.github.io">
</a>
<a href="https://ffengc.github.io/gh-blog/">
    <img src="https://img.shields.io/static/v1?label=Blog&message=Blog Page&color=brightgreen" alt="Mutable.ai Auto Wiki">
</a>

</div>

- **[简体中文](./README-cn.md)**
- **[English](./README.md)**

> [!TIP]
> 如果你对本项目的运行和实现有任何疑问，或者对本项目有更好的优化建议，可以直接联系我，或者在仓库的issue中留言。

## 关于MQ中间件

消息队列（MQ）中间件是一种软件或服务，用于在不同的应用程序、系统或服务之间异步传递消息。它允许各种系统之间独立通信，不必直接连接到彼此，从而提高了系统的可扩展性、灵活性和可维护性。消息队列中间件的一些典型用途包括解耦服务组件、增强并发处理能力、以及平衡负载。

RabbitMQ 是一个流行的开源消息队列系统，支持多种消息协议，主要使用 AMQP（高级消息队列协议）。RabbitMQ 允许应用程序通过简单的协议来发送、接收和存储消息，直到被接收。以下是其一些关键特点：

1. **灵活的路由**：RabbitMQ 提供了多种消息路由方式，包括直接、主题、头和扇出交换，这使得它在消息传递时非常灵活。
2. **可靠性**：RabbitMQ 支持消息持久化，确保消息不会因为服务器故障而丢失。
3. **高可用性**：可以配置RabbitMQ集群来保证服务的高可用性和故障转移。
4. **多种客户端支持**：支持多种编程语言的客户端库，如Python、Java、.NET等。

消息队列中间件如 RabbitMQ 在大数据处理、微服务架构、分布式系统和实时数据处理等多种场景下都有广泛应用。

**本项目会针对于RabbitMQ，学习并提取里面的精华部分，学习RabbitMQ的基本原理，并对它进行一个简单的模拟实现。通过学习这个项目，可以加深对消息队列中间件的理解，对后续的开发有很大的帮助。**

## 技术栈

- **序列化框架：** Protobuf进行⼆进制序列化
- **⽹络通信：** ⾃定义应⽤层协议+muduo库：对tcp⻓连接的封装、并且使⽤epoll的事件驱动模式，实现⾼并发服务器与客⼾端•
- **源数据信息数据库：** SQLite3
-  **单元测试框架：** Gtest

## 环境配置

<details>
  <summary><strong>配置和部署</strong></summary>


### 基本工具

**首先需要以下基本工具：**

高于7的`gcc/g++`版本, git, cmake 等

### 安装`protobuf`

是一个序列化和反序列化工具。

安装依赖：
```sh
# centos
sudo yum install autoconf automake libtool curl make gcc-c++ unzip
# ubuntu
sudo apt update
sudo apt install autoconf automake libtool curl make g++ unzip
```
下载`protobuf`包：
```sh
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.20.2/protobuf-all-3.20.2.tar.gz
```
编译安装：
```sh
# 解压
tar -zxf protobuf-all-3.20.2.tar.gz
cd protobuf-3.20.2/
# 运行目录配置脚本
./autogen.sh
# 运行配置脚本
./configure
# 编译(时间较长)
make
# 安装
sudo make install
# 确认是否安装成功
protoc --version
```
![](./assets/2.png)
如图所示即安装成功。

### 安装muduo库

下载源代码：

```sh
git clone https://github.com/chenshuo/muduo.git
```

安装依赖：
```sh
# centos
sudo yum install gcc-c++ cmake make zlib zlib-devel boost-devel
# ubuntu
sudo apt update
sudo apt install g++ cmake make zlib1g zlib1g-dev libboost-all-dev
```
编译安装：
```
./build.sh
./build.sh install
```

> ‼️这里要说明一下，如果编译过程提示protoc相关库找不到，是因为protobuf当时安装路径和muduo要求的不同，需要行把相关库链接到指定位置（具体要看报错信息）。
> 此外还有可能出现的问题是，boost库相关的错误（机子上装有python的conda），可能会出现muduo找boost的时候找到conda的boost里面去了，解决方法就是暂时把annaconda3隐藏，就可以编译成功。

### 验证muduo是否安装成功

> **Tips:** 编译好之后的`muduo`可执行是在上级目录的`build`里的，而不是在`muduo`目录里，是在和`muduo`同级的`build`目录下。

![](./assets/3.png)

进入muduo一些测试可执行的目录：`build/release-cpp11/bin`

运行demo服务端：
```sh
./protobuf_server 9091
```
同样，如果出现链接错误，就把对应的库链接到对应的地方即可。

启动demo客户端：
```sh
./protobuf_client 0.0.0.0 9091
```

![](./assets/4.png)

如图所示即通过测试。

### 安装SQLite3

这是一个轻量级的数据库。

```sh
# centos
sudo yum install sqlite-devel
# ubuntu
sudo apt install sqlite3
# 验证安装
sqlite3 --version
```

### 安装gtest测试框架

```sh
# centos
sudo yum install epel-release
sudo yum install dnf
sudo dnf install dnf-plugins-core
sudo dnf install gtest gtest-devel
# ubuntu
sudo apt update
sudo apt install libgtest-dev
```

测试gtest是否安装成功：

运行`env/test.cc`代码，如果输出正常则安装成功。

![](./assets/5.png)

</details>

## 项目相关框架介绍和学习

这是在我做这个项目过程中，学习一些要用的第三方框架所整理的文档和代码。

如果对这一部分不感兴趣可以直接跳过。

- 文档: [protobuf.md](./docs/proto.md), 代码: `HareMQ/demo/protobuf`
- 文档: [muduo.md](./docs/muduo.md), 代码: `HareMQ/demo/muduo`
- 文档: [sqlite.md](./docs/sqlite.md), 代码: `HareMQ/demo/sqlite`
- 文档: [gtest.md](./docs/gtest.md), 代码: `HareMQ/demo/gtest`
- 文档: [C++异步操作](./docs/asynchronous.md), 代码: `HareMQ/demo/asynchronous`
- 文档: [基于C++异步操作实现的线程池组件](./docs/thread_pool.md), 代码: `HareMQ/demo/thread_pool`

## 项目框架

