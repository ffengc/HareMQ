# 详细实现

- [详细实现](#详细实现)
  - [项目目录结构创建](#项目目录结构创建)
  - [工具类准备](#工具类准备)
    - [日志打印工具](#日志打印工具)
    - [sqlite基础操作](#sqlite基础操作)
    - [字符串操作](#字符串操作)
    - [UUID生成类](#uuid生成类)
    - [文件基础操作](#文件基础操作)
    - [测试文件相关操作](#测试文件相关操作)
  - [消息类型定义](#消息类型定义)
  - [交换机数据管理](#交换机数据管理)
  - [交换机部分单元测试](#交换机部分单元测试)


## 项目目录结构创建

![](./work-assets/1.png)

## 工具类准备

我们需要以下这些工具代码:

- 日志打印工具
- 文件基础操作
- sqlite基础操作
- 字符串操作
- UUID生成类


### 日志打印工具

这个日志组件我经常使用，直接见代码即可。`HareMQ/mqcommon/logger.hpp`。

### sqlite基础操作

在 `mqcommon` 里面创建一个 `helper.hpp` 即可，把 demo 里面实现的 `db.hpp` 里面的内容复制过去即可。

**具体解释可以见 demo 的文档: [docs/sqlite.md](../docs/sqlite.md)**

### 字符串操作

其实就是字符串切割，之前也写过很多次了，用`boost`里面的就行了。

```cpp
class string_helper {
    static size_t split(const std::string& str, const std::string& sep, std::vector<std::string>* out, bool if_compress = true) {
        // boost split
        if (if_compress) {
            boost::split(*out, str, boost::is_any_of(sep), boost::token_compress_on);
            return out->size();
        } else {
            boost::split(*out, str, boost::is_any_of(sep), boost::token_compress_off);
            return out->size();
        }
    }
};
```

### UUID生成类

> [!TIP]
> UUID(Universally Unique ldentifier), 也叫通用唯一识别码，通常由32位16进制数字字符组成。UUID的标准型式包含32个16进制数字字符，以连字号分为五段，形式为8-4-4-4-12的32个字符，如: `550e8400-e29b-41d4-a716-446655440000`。在这里，uuid生成，我们采用生成8个随机数字，加上8字节序号，共16字节数组生成32位16进制字符的组合形式来确保全局唯一的同时能够根据序号来分辨数据。

```cpp
class uuid_helper {
public:
    std::string uuid() {
        std::random_device rd;
        std::mt19937_64 generator(rd());
        std::uniform_int_distribution<int> distribution(0, 255);
        std::stringstream ss;
        for (int i = 0; i < 8; ++i) {
            ss << std::setw(2) << std::setfill('0') << std::hex << distribution(generator);
            if (i == 3 || i == 5 || i == 7)
                ss << "-";
            static std::atomic<size_t> seq(1); // 这里一定要静态，保证多次调用都是自增的
            size_t num = seq.fetch_add(1);
            for (int i = 7; i >= 0; i--) {
                ss << std::setw(2) << std::setfill('0') << std::hex << ((num >> (i * 8)) & 0xff);
                if (i == 6)
                    ss << "-";
            }
        }
        return ss.str();
    }
};
```

### 文件基础操作


基本框架:

```cpp
class file_helper {
private:
    std::string __file_name;

public:
    file_helper(const std::string& file_name)
        : __file_name(file_name) { }
    ~file_helper() = default;

public:
    bool exists();
    size_t size();
    bool read(std::string& body);
    bool read(std::string& body, size_t offset, size_t len);
    bool write(const std::string& body);
    bool write(const std::string& body, size_t offset);
    bool create();
    bool remove();
    bool create_dir();
    bool remove_dir();
    static std::string parent_dir(const std::string& file_name);
    bool rename(const std::string& name);
};
```

### 测试文件相关操作

`mptest/file_test.cc`

先测试两个简单的功能:

```cpp
void test1() {
    hare_mq::file_helper helper("../mqcommon/logger.hpp");
    hare_mq::LOG(DEBUG) << "file if exists: " << helper.exists() << std::endl;
    hare_mq::LOG(DEBUG) << "file size: " << helper.size() << std::endl;
}
```

![](./work-assets/2.png)


测试目录创建和文件创建：

```cpp
void test2() {
    hare_mq::file_helper helper("./aaa/bbb/ccc/tmp.hpp");
    if (helper.exists() == false) {
        std::string p = hare_mq::file_helper::parent_dir(helper.path()); // 先获取父级目录
        hare_mq::LOG(DEBUG) << p << std::endl;
        if (hare_mq::file_helper(p).exists() == false) {
            // 创建目录
            hare_mq::file_helper::create_dir(p);
        }
        hare_mq::file_helper::create(helper.path());
    }
}
```

![](./work-assets/3.png)

测试全局的读写:


```cpp
void test3() {
    hare_mq::file_helper helper1("../mqcommon/logger.hpp");
    hare_mq::file_helper helper2("./aaa/bbb/ccc/tmp.hpp");
    std::string body;
    helper1.read(body);
    // write to tmp.hpp
    helper2.write(body);
}
```

把`logger.hpp`里面的数据读出来，然后写到`tmp.hpp`里面去。


测试特定位置的读写:

```cpp
void test4() {
    hare_mq::file_helper helper("./aaa/bbb/ccc/tmp.hpp");
    // 把6-19个字节读取出来
    char str[16] = {0};
    helper.read(str, 6, 13); // 这里要读 (6, 19] 这些字符，应该传入 6, 13
    hare_mq::LOG(DEBUG) << std::string(str) << std::endl;
    helper.write("123456\n", 19, 7);
}
```

![](./work-assets/4.png)

通过测试。


测试一下`rename`:

```cpp
void test5() {
    hare_mq::file_helper helper("./aaa/bbb/ccc/tmp.hpp");
    helper.rename(hare_mq::file_helper::parent_dir(helper.path()) + "/test.hpp");
}
```

![](./work-assets/5.png)

符合预期。


测试删除:

```cpp
void test6() {
    hare_mq::LOG(DEBUG) << "before run" << std::endl;
    system("tree .");
    hare_mq::file_helper::create("./aaa/bbb/ccc/tmp.hpp");
    hare_mq::LOG(DEBUG) << "run: create(\"./aaa/bbb/ccc/tmp.hpp\");" << std::endl;
    system("tree .");
    hare_mq::file_helper::remove("./aaa/bbb/ccc/tmp.hpp");
    hare_mq::LOG(DEBUG) << "run: remove(\"./aaa/bbb/ccc/tmp.hpp\");" << std::endl;
    system("tree .");
    hare_mq::file_helper::remove_dir("./aaa/bbb/ccc/");
    hare_mq::LOG(DEBUG) << "run: remove_dir(\"./aaa/bbb/ccc/\");" << std::endl;
    system("tree .");
    hare_mq::file_helper::remove_dir("./aaa");
    hare_mq::LOG(DEBUG) << "run: remove_dir(\"./aaa\");" << std::endl;
    system("tree .");
}
```

![](./work-assets/6.png)

符合预期。


## 消息类型定义

因此定义消息类型，其实就是定义一个消息类型的proto文件，并生成相关代码。

消息的结构:
1. 消息本身要素：
   1. 消息属性: 消息属性包含这些内容。消息ID、消息投递模式: 非持久化/持久化模式、消息的`routing_key`
   2. 消息有效载荷内容
2. 消息额外存储所需要素
3. 消息额外存储所需要素
   1. 消息的存储长度
   2. 消息的长度
   3. 消息是否有效：注意这里并不使用bool类型，而是使用字符0/1，因为bool类型在持久化的时候所占长度不同，会导致，修改文件中消息有效位后消息长度发生变化，因此不用bool类型。

定义proto文件。
```proto
syntax = "proto3";
package hare_mq;
enum ExchangeType {
    UNKNOWTYPE = 0;
    DIRECT = 1;
    FANOUT = 2;
    TOPIC = 3;
};
enum DeliveryMode {
    UNKNOWTYPE = 0;
    UNDURABLE = 1;
    DURABLE = 2;
};
message BasicProperties {
    string id = 1;
    DeliveryMode delivery_mode = 2;
    string routing_key = 3;
};
message Message {
    message Payload {
        BasicProperties properties = 1;
        string body = 2;
    };
    Payload payload = 1;
    uint32 offset = 2;
    uint32 length = 3;
    string valid = 4;
};
```

![](./work-assets/7.png)

## 交换机数据管理

现在要开始编写`mqserver`里面的`exchagne,.hpp`了。

代码基本结构如下所示:

```cpp
namespace hare_mq {
/**
 * 1. 交换机类
 * 2. 交换机数据持久化管理类
 * 3. 交换机数据内存管理类
 */
struct exchange {
    /* 交换机类 */
public:
    using ptr = std::shared_ptr<exchange>;
    std::string name; // 交换机名称
    ExchangeType type; // 交换机类型
    bool durable; // 持久化标志
    bool auto_delete; // 自动删除标志
    std::unordered_map<std::string, std::string> args; // 其他参数
public:
    exchange(const std::string ename,
        ExchangeType etype,
        bool edurable,
        bool eauto_delete,
        const std::unordered_map<std::string, std::string>& eargs)
        : name(ename)
        , type(etype)
        , auto_delete(eauto_delete)
        , args(eargs) { }
    // args存储的格式是键值对，在存储数据库的时候，会组织一个字符串进行存储 key=value&key=value
    void set_args(const std::string& str_args) {
        /**
         * 解析 str_args 字符串: key=value&key=value... 存到 args 成员变量中去
         */
    }
    std::string get_args() {
        /**
         * set_args()的反操作，把args里面的数据序列化成 key=value&key=value... 的格式
         */
    }
};

class exchange_mapper {
    /* 交换机数据持久化管理类 */
private:
    sqlite_helper __sql_helper; // sqlite操作句柄
public:
    exchange_mapper(const std::string& dbfile); // 构造，需要传递数据库文件名称
public:
    void create_table(); // 创建表
    void remove_table(); // 删除表
    void insert(exchange::ptr& e); // 插入交换机
    void remove(const std::string& name); // 移除交换机
    exchange::ptr one(const std::string& name); // 获取单个交换机
    std::unordered_map<std::string, exchange::ptr> all(); // 获取全部交换机
};

class exchange_manager {
    /* 交换机数据内存管理类 */
private:
    exchange_mapper __mapper; // 持久化管理
    std::unordered_map<std::string, exchange::ptr> __exchanges; // 管理所有的交换机
    std::mutex __mtx; // exchange_manager 会被多线程调用，管理一个互斥锁
public:
    exchange_manager(const std::string& dbfile);
    void declare_exchange(const std::string& name,
        ExchangeType type,
        bool durable,
        bool auto_delete,
        std::unordered_map<std::string, std::string>& args); // 声明交换机
    void delete_exchange(const std::string& name); // 删除交换机
    exchange::ptr select_exchange(const std::string& name); // 选择一台交换机
    bool exists(const std::string& name); // 判断交换机是否存在
    void clear_exchange(); // 清理所有交换机
};

} // namespace hare_mq
```

具体代码可以见代码所示。

## 交换机部分单元测试

