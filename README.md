<div align="center">

# HareMQ

A C++ version of the simplified message queue component is implemented based on RabbitMQ. In order to learn RabbitMQ, this project encompasses the essence and core functions of a high-performance messaging system, imitates the basic architecture of RabbitMQ, and focuses on the publish-subscribe mechanism.

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
> If you have any questions about the operation and implementation of this project, or if you have better optimization suggestions for this project, you can contact me directly or leave an issue in the repository.

## About MQ Middleware

Message queuing (MQ) middleware is a software or service used to asynchronously pass messages between different applications, systems, or services. It allows various systems to communicate independently without having to connect directly to each other, thereby improving the scalability, flexibility, and maintainability of the system. Some typical uses of message queue middleware include decoupling service components, enhancing concurrent processing capabilities, and balancing loads.

RabbitMQ is a popular open source message queue system that supports multiple message protocols, mainly using AMQP (Advanced Message Queuing Protocol). RabbitMQ allows applications to send, receive, and store messages through a simple protocol until they are received. Here are some of its key features:

1. **Flexible routing**: RabbitMQ provides a variety of message routing methods, including direct, topic, header, and fan-out exchanges, which makes it very flexible in messaging.
2. **Reliability**: RabbitMQ supports message persistence to ensure that messages are not lost due to server failures.
3. **High availability**: RabbitMQ clusters can be configured to ensure high availability and failover of services.
4. **Multiple client support**: Supports client libraries in multiple programming languages, such as Python, Java, .NET, etc.


Message queue middleware such as RabbitMQ is widely used in various scenarios such as big data processing, microservice architecture, distributed systems, and real-time data processing.

**This project will focus on RabbitMQ, learn and extract the essence of it, learn the basic principles of RabbitMQ, and perform a simple simulation implementation of it. By studying this project, you can deepen your understanding of message queue middleware, which will be of great help to subsequent development.**

## Technology stack

- **Serialization framework:** Protobuf for binary serialization
- **Network communication:** Custom application layer protocol + muduo library: Encapsulation of TCP long connection, and use of epoll event-driven mode to achieve high-concurrency server and client•
- **Source data information database:** SQLite3
- **Unit test framework:** Gtest


## Environment Configuration

<details>
  <summary><strong>Configuration and deployment</strong></summary>


### Basic tools

**First, you need the following basic tools:**

`gcc/g++` versions higher than 7, git, cmake, etc.

### Install `protobuf`

It is a serialization and deserialization tool.

Installation dependencies:
```sh
# centos
sudo yum install autoconf automake libtool curl make gcc-c++ unzip
# ubuntu
sudo apt update
sudo apt install autoconf automake libtool curl make g++ unzip
```
Download the protobuf package:
```sh
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.20.2/protobuf-all-3.20.2.tar.gz
```
Compile and install:

```sh
# unzip
tar -zxf protobuf-all-3.20.2.tar.gz
cd protobuf-3.20.2/
# Run the directory configuration script
./autogen.sh
# Run the configuration script
./configure
# Compile (takes longer)
make
# Install
sudo make install
# Confirm whether the installation is successful
protoc --version
```

![](./assets/2.png)

The installation is successful as shown in the figure.

### Install the muduo lib

Download the code.
```sh
git clone https://github.com/chenshuo/muduo.git
```

Installation dependencies:
```sh
# centos
sudo yum install gcc-c++ cmake make zlib zlib-devel boost-devel
# ubuntu
sudo apt update
sudo apt install g++ cmake make zlib1g zlib1g-dev libboost-all-dev
```

> ‼️ Here I want to explain that if the compilation process prompts that the protoc related library cannot be found, it is because the installation path of protobuf at that time is different from that required by muduo. You need to link the related library to the specified location (depending on the error message).
> Another possible problem is the error related to the boost library (python conda is installed on the machine). It may appear that when muduo looks for boost, it finds the boost in conda. The solution is to temporarily hide annaconda3, and then the compilation will be successful.

### Verify that muduo is installed successfully

> **Tips:** The compiled `muduo` executable is in the `build` directory of the parent directory, not in the `muduo` directory. It is in the `build` directory at the same level as `muduo`.

![](./assets/3.png)

Enter the directory where muduo tests can be executed: `build/release-cpp11/bin`


run the demo server:
```sh
./protobuf_server 9091
```
Similarly, if a link error occurs, just link the corresponding library to the corresponding place.

run the demo client:

```sh
./protobuf_client 0.0.0.0 9091
```

![](./assets/4.png)

The test is passed as shown in the figure.

### Install SQLite3

This is a lightweight database.

```sh
# centos
sudo yum install sqlite-devel
# ubuntu
sudo apt install sqlite3
# Verify installation
sqlite3 --version
```

### Install the gtest testing framework

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

Test whether gtest is installed successfully:

Run the `env/test.cc` code. If the output is normal, the installation is successful.

![](./assets/5.png)

</details>
