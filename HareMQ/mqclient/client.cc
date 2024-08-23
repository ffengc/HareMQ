/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_CLIENT__
#define __YUFC_CLIENT__
#include "connection.hpp"
#include <pwd.h>
#include <sstream>
#include <sys/types.h>

const std::string red_bold = "\033[1;31m";
std::string yellow_bold = "\033[1;33m";
const std::string reset = "\033[0m";
std::atomic<bool> done(false); // 原子变量，用于同步线程
class client {
private:
    std::string __tag;
    hare_mq::channel::ptr __ch = nullptr; //
    hare_mq::connection::ptr __conn = nullptr;
    hare_mq::async_worker::ptr __async_worker = nullptr;
    std::unordered_map<std::string, std::string> __valid_cmds = {
        { "subscribe", "Subcribe a queue and continuous consuming messages from a queue (print to stdout)" },
        { "publish", "Push a message to the specified exchange" },
        { "publish_mode", "Continuous push message to specified exchange" },
        { "svrstat", "View the exchange information, queue information and binding information of the current server, note: This is a blocking interface" },
        { "declare_exchange", "Declare an exchange on the server" },
        { "declare_queue", "Declare a message queue on the server" },
        { "delete_exchange", "Delete an exchange on the server" },
        { "delete_queue", "Delete and queue on the server" },
        { "bind", "Bind an exchange and a queue" },
        { "unbind", "Unbind a binding between an exchagne and a queue" },
        { "cid", "Show current client's channel id" },
        { "cmds", "Show all commands" },
        { "exit", "Quit the client" },
        { "clear", "Clean the terminal" },
        { "logo", "Print the project logo" }
    };
    std::vector<std::string> __logo = {
        "+--------------------------------------------+",
        "|       _______________________________      |",
        "|      |                               |     |",
        "|      | HareMQ                        |     |",
        "|      | developed by ffengc           |     |",
        "|      |_______________________________|     |",
        "|          o                                 |",
        "|           o                                |",
        "|               .--.                         |",
        "|              |o_o |                        |",
        "|              |:_/ |                        |",
        "|             //   \\ \\                       |",
        "|            (|     | )                      |",
        "|           /'\\_   _/`\\                      |",
        "|           \\___)=(___/                      |",
        "+--------------------------------------------+"
    }; //
public:
    client(const std::string& tag)
        : __tag(tag) { printClientInfo(); }
    ~client() {
        __ch.reset();
        __conn.reset();
        __async_worker.reset();
    }
    void init() {
        // 1. 实例化异步工作线程
        __async_worker = std::make_shared<hare_mq::async_worker>();
        // 2. 实例化连接对象
        __conn = std::make_shared<hare_mq::connection>("127.0.0.1", 8085, __async_worker);
        // 3. 通过连接创建信道
        __ch = __conn->openChannel();
        hare_mq::LOG(INFO) << "channel connected" << std::endl;
    }
    void run(const std::string& config = "None") {
        print_logo();
        if (config != "None")
            if (!run_config(config))
                return;
        while (true) {
            std::string full_cmd;
            std::cout << "> ";
            fflush(stdout);
            std::getline(std::cin, full_cmd);
            std::vector<std::string> full_cmd_splited;
            hare_mq::string_helper::split(full_cmd, " ", &full_cmd_splited);
            if (full_cmd_splited.size() == 0)
                continue;
            std::string cmd = full_cmd_splited[0];
            auto it = __valid_cmds.find(cmd);
            if (it == __valid_cmds.end()) {
                std::cerr << red_bold << cmd << ": " << reset << "command not found, use 'cmds' to show all valid commands" << std::endl;
                continue;
            }
            if (full_cmd_splited[0] == "exit")
                return;
            run_cmd(full_cmd_splited);
        }
        __conn->closeChannel(__ch);
    } //
private:
    bool run_config(const std::string& filePath) {
        std::map<std::size_t, std::string> lines;
        std::ifstream file(filePath); // 打开文件
        if (!file.is_open()) {
            hare_mq::LOG(FATAL) << "cannot open: " << filePath << std::endl;
            return false;
        }
        std::string line;
        size_t num = 0;
        while (std::getline(file, line)) {
            num += 1;
            lines.insert({ num, line });
        }
        file.close();
        // 解析每一行
        hare_mq::LOG(INFO) << "running config file: '" << filePath << "' ..." << std::endl;
        for (const auto& e : lines) {
            std::vector<std::string> full_cmd_splited;
            hare_mq::string_helper::split(e.second, " ", &full_cmd_splited);
            if (full_cmd_splited.size() == 0)
                continue;
            std::string cmd = full_cmd_splited[0];
            auto it = __valid_cmds.find(cmd);
            if (it == __valid_cmds.end()) {
                std::cerr << red_bold << cmd << ": " << reset << "command not found, use 'cmds' to show all valid commands" << std::endl;
                return false;
            }
            bool res = run_cmd(full_cmd_splited);
            if (res) {
                std::cout << "[" << e.first << "] run " << e.second << " success" << std::endl;
            } else {
                std::cout << "[" << e.first << "] run " << e.second << " failed" << std::endl;
                return false;
            }
        }
        hare_mq::LOG(INFO) << "run config file: " << filePath << " success, please check the server stats" << std::endl;
        return true;
    }
    static void cb(hare_mq::channel::ptr& ch, const std::string& tag, const hare_mq::BasicProperties* bp, const std::string& body) {
        std::cout << tag << " recv a mesg# " << body << std::endl;
        ch->basic_ack(bp->id());
    }
    void print_logo() {
        for (const auto& e : __logo)
            std::cout << e << std::endl;
    }
    void usage(const std::string& cmd) {
        assert(__valid_cmds.find(cmd) != __valid_cmds.end());
        std::stringstream ss;
        ss << yellow_bold << cmd << ": " << reset << __valid_cmds[cmd] << std::endl;
        ss << "usage: " << std::endl;
        if (cmd == "bind") {
            ss << "    bind exchange_name queue_name binding_key";
        } else if (cmd == "unbind") {
            ss << "    unbind exchange_name queue_name";
        } else if (cmd == "declare_exchange") {
            ss << "    declare_exchange exchange_name ExchangeType durable auto_delete other_args" << std::endl;
            ss << "    ExchangeType: TOPIC/FANOUT/DIRECT" << std::endl;
            ss << "    durable, auto_delete: true/false" << std::endl;
            ss << "    other_args format: k1=v1&k2=v2&k3=v3...";
        } else if (cmd == "delete_exchange") {
            ss << "delete_exchange exchange_name";
        } else if (cmd == "declare_queue") {
            ss << "    declare_queue queue_name durable exclusive auto_delete other_args" << std::endl;
            ss << "    durable, exclusive, auto_delete: true/false" << std::endl;
            ss << "    other_args format: k1=v1&k2=v2&k3=v3...";
        } else if (cmd == "delete_queue") {
            ss << "delete_queue queue_name";
        } else if (cmd == "publish") {
            ss << "    publish exchange_name DURABLE binding_key" << std::endl;
            ss << "    DURABLE: hare_mq::DeliveryMode::DURABLE/UNDURABLE";
        } else if (cmd == "publish_mode") {
            ss << "    publish_mode exchange_name DURABLE binding_key" << std::endl;
            ss << "    DURABLE: hare_mq::DeliveryMode::DURABLE/UNDURABLE";
        } else if (cmd == "subscribe") {
            ss << "    subscribe consumer_tag queue_name auto_ack";
        }
        std::cerr << ss.str() << std::endl;
    }
    bool run_cmd(const std::vector<std::string>& full_cmd_splited) {
        if (full_cmd_splited[0] == "cmds") {
            for (const auto& e : __valid_cmds) {
                std::string line = "    " + e.first + ": " + e.second;
                std::cout << line << std::endl;
            }
            return true;
        } else if (full_cmd_splited[0] == "cid") {
            std::cout << "current channel id: " << __ch->cid() << std::endl;
            return true;
        } else if (full_cmd_splited[0] == "bind") {
            // ch->bind("exchange1", "queue1", "queue1");
            if (full_cmd_splited.size() != 4) {
                usage("bind");
                return false;
            }
            auto ok = __ch->bind(full_cmd_splited[1], full_cmd_splited[2], full_cmd_splited[3]);
            if (!ok) {
                std::cerr << red_bold << "bind failed" << reset << ", please check the server stats" << std::endl;
                return false;
            }
            return true;
        } else if (full_cmd_splited[0] == "unbind") {
            // __ch->unbind()
            if (full_cmd_splited.size() != 3) {
                usage("unbind");
                return false;
            }
            __ch->unbind(full_cmd_splited[1], full_cmd_splited[2]);
            return true;
        } else if (full_cmd_splited[0] == "declare_exchange") {
            // auto empty_map = std::unordered_map<std::string, std::string>();
            // ch->declare_exchange("exchange1", hare_mq::ExchangeType::TOPIC, true, false, empty_map);
            if (full_cmd_splited.size() != 6 && full_cmd_splited.size() != 5) {
                usage("declare_exchange");
                return false;
            }
            std::string name = full_cmd_splited[1];
            hare_mq::ExchangeType type = [&, this]() -> hare_mq::ExchangeType {
                if (full_cmd_splited[2] == "FANOUT")
                    return hare_mq::ExchangeType::FANOUT;
                else if (full_cmd_splited[2] == "DIRECT")
                    return hare_mq::ExchangeType::DIRECT;
                else if (full_cmd_splited[2] == "TOPIC")
                    return hare_mq::ExchangeType::TOPIC;
                else {
                    usage("declare_exchange");
                    return hare_mq::ExchangeType::UNKNOWTYPE;
                }
            }();
            if (type == hare_mq::ExchangeType::UNKNOWTYPE)
                return false;
            if ((full_cmd_splited[3] != "true" && full_cmd_splited[3] != "false")
                || (full_cmd_splited[4] != "true" && full_cmd_splited[4] != "false")) {
                usage("declare_exchange");
                return false;
            }
            bool durable = full_cmd_splited[3] == "true" ? true : false;
            bool auto_delete = full_cmd_splited[4] == "true" ? true : false;
            auto m = std::unordered_map<std::string, std::string>();
            if (full_cmd_splited.size() == 6 && full_cmd_splited[5] != "") {
                // 有args参数
                // k=v&k=v...
                std::vector<std::string> args;
                hare_mq::string_helper::split(full_cmd_splited[5], "&", &args);
                // 现在args里面是一个kv结构
                for (const auto& e : args) {
                    // e: k=v
                    std::size_t pos = e.find('=');
                    if (pos == std::string::npos) {
                        usage("declare_exchange");
                        return false;
                    }
                    std::string left = e.substr(0, pos);
                    std::string right = e.substr(pos + 1);
                    m.insert({ left, right });
                }
            }
            // hare_mq::LOG(DEBUG) << name << " " << type << " " << durable << " " << auto_delete << " " << m.size() << std::endl;
            if (!__ch->declare_exchange(name, type, durable, auto_delete, m)) {
                std::cerr << red_bold << "declare_exchange failed" << reset << ", please check the server stats" << std::endl;
                return false;
            }
            return true;
        } else if (full_cmd_splited[0] == "delete_exchange") {
            // __ch->delete_exchange();
            if (full_cmd_splited.size() != 2) {
                usage("delete_exchange");
                return false;
            }
            __ch->delete_exchange(full_cmd_splited[1]);
            return true;
        } else if (full_cmd_splited[0] == "declare_queue") {
            // const std::string &qname, bool qdurable, bool qexclusive, bool qauto_delete, const std::unordered_map<std::string, std::string> &qargs
            if (full_cmd_splited.size() != 6 && full_cmd_splited.size() != 5) {
                usage("declare_queue");
                return false;
            }
            std::string name = full_cmd_splited[1];
            if ((full_cmd_splited[2] != "true" && full_cmd_splited[2] != "false")
                || (full_cmd_splited[3] != "true" && full_cmd_splited[3] != "false")
                || (full_cmd_splited[4] != "true" && full_cmd_splited[4] != "false")) {
                usage("declare_queue");
                return false;
            }
            bool qdurable = full_cmd_splited[2] == "true" ? true : false;
            bool qexclusive = full_cmd_splited[3] == "true" ? true : false;
            bool qauto_delete = full_cmd_splited[4] == "true" ? true : false;
            auto m = std::unordered_map<std::string, std::string>();
            if (full_cmd_splited.size() == 6 && full_cmd_splited[5] != "") {
                // 有args参数
                // k=v&k=v...
                std::vector<std::string> args;
                hare_mq::string_helper::split(full_cmd_splited[5], "&", &args);
                // 现在args里面是一个kv结构
                for (const auto& e : args) {
                    // e: k=v
                    std::size_t pos = e.find('=');
                    if (pos == std::string::npos) {
                        usage("declare_queue");
                        return false;
                    }
                    std::string left = e.substr(0, pos);
                    std::string right = e.substr(pos + 1);
                    m.insert({ left, right });
                }
            }
            if (!__ch->declare_queue(name, qdurable, qexclusive, qauto_delete, m)) {
                std::cerr << red_bold << "declare_queue failed" << reset << ", please check the server stats" << std::endl;
                return false;
            }
            return true;
        } else if (full_cmd_splited[0] == "delete_queue") {
            // __ch->delete_queue();
            if (full_cmd_splited.size() != 2) {
                usage("delete_queue");
                return false;
            }
            __ch->delete_queue(full_cmd_splited[1]);
            return true;
        } else if (full_cmd_splited[0] == "svrstat") {
            __ch->basic_query();
            return true;
        } else if (full_cmd_splited[0] == "publish" || full_cmd_splited[0] == "publish_mode") {
            /*
                hare_mq::BasicProperties bp;
                bp.set_id(hare_mq::uuid_helper::uuid());
                bp.set_delivery_mode(hare_mq::DeliveryMode::DURABLE);
                bp.set_routing_key("news.music.pop");
                ch->basic_publish("exchange1", &bp, "Hello world-" + std::to_string(i)); // queue2 可以收到消息
                std::this_thread::sleep_for(std::chrono::seconds(3));
            */
            // publish ex1 DURABLE news.music.pop
            if (full_cmd_splited.size() != 4) {
                usage("publish");
                return false;
            }
            auto exchange_name = full_cmd_splited[1];
            hare_mq::BasicProperties bp;
            bp.set_id(hare_mq::uuid_helper::uuid());
            bp.set_delivery_mode([&, this]() -> hare_mq::DeliveryMode {
                if (full_cmd_splited[2] == "DURABLE")
                    return hare_mq::DeliveryMode::DURABLE;
                else if (full_cmd_splited[2] == "DIRECT")
                    return hare_mq::DeliveryMode::UNDURABLE;
                else {
                    return hare_mq::DeliveryMode::UNKNOWMODE;
                }
            }());
            auto routing_key = full_cmd_splited[3];
            do {
                // 先执行一次
                std::string mesg;
                std::cout << "[please input your message]# ";
                std::getline(std::cin, mesg);
                if (mesg == "quit")
                    break;
                __ch->basic_publish(exchange_name, &bp, mesg);
            } while (full_cmd_splited[0] == "publish_mode");
            return true;
        } else if (full_cmd_splited[0] == "subscribe") {
            // ch->basic_consume("consumer1", qname, false, fn);
            auto fn = std::bind(&cb, __ch, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            // subscribe consumer1 qname false
            // subscribe yufc queue2 false
            if (full_cmd_splited.size() != 4) {
                usage("subscribe");
                return false;
            }
            std::string consumer_name = full_cmd_splited[1];
            std::string queue_name = full_cmd_splited[2];
            if (full_cmd_splited[3] != "true" && full_cmd_splited[3] != "false") {
                usage("subscribe");
                return false;
            }
            bool auto_ack = full_cmd_splited[3] == "true" ? true : false;
            hare_mq::LOG(INFO) << "subscribed to queue: " << queue_name << std::endl;
            std::thread inputThread(check_quit_while_consuming); // 监听 stdin 是否有退出信号
            if (!__ch->basic_consume(consumer_name, queue_name, auto_ack, fn)) {
                std::cerr << red_bold << "subscribe a queue failed" << reset << ", please check the server stats" << std::endl;
            }
            while (!done) {
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
            inputThread.join();
            __ch->basic_cancel();
            return true;
        } else if (full_cmd_splited[0] == "clear") {
            system("clear");
            return true;
        } else if (full_cmd_splited[0] == "logo") {
            print_logo();
            return true;
        } else {
            assert(false);
        }
    } //
    void printClientInfo() {
        auto startTime = std::time(nullptr);
        std::string startTimeStr = std::ctime(&startTime);
        auto uid = getuid();
        struct passwd* pw = getpwuid(uid);
        std::string userName = (pw ? pw->pw_name : "Unknown User");
        pid_t pid = getpid();
        hare_mq::LOG(INFO) << std::endl
                           << "------------------- Client Start --------------------" << std::endl
                           << "Start Time: " << startTimeStr
                           << "User: " << userName << std::endl
                           << "Process ID: " << std::to_string(pid) << std::endl
                           << "-------------------------------------------------------" << std::endl;
    } //
private:
    static void check_quit_while_consuming() {
        std::string input;
        while (true) {
            std::getline(std::cin, input);
            if (input == "quit") {
                done = true;
                break;
            }
        }
    }
};
#endif
int main(int argc, char** argv) {
    client cli("yufc");
    cli.init();
    if (argc == 1)
        cli.run();
    else
        cli.run(argv[1]);
    return 0;
}