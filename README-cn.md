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