/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_COMMON_LOGGER__
#define __YUFC_COMMON_LOGGER__

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace hare_mq {
enum STATUES // 日志等级
{
    INFO,
    DEBUG,
    WARNING,
    ERROR,
    REQUEST,
    FATAL
};

// 定义颜色代码
#define RESET "\033[0m"
#define RED "\033[1;31m" // 加粗红色
#define GREEN "\033[1;32m" // 加粗绿色
#define YELLOW "\033[1;33m" // 加粗黄色
#define BLUE "\033[1;34m" // 加粗蓝色
#define MAGENTA "\033[1;35m" // 加粗洋红
#define CYAN "\033[1;36m" // 加粗青色

// 根据日志等级获取相应颜色
inline const char* GetColor(const std::string& level) {
    std::map<std::string, int> m = { { "INFO", 0 }, { "DEBUG", 1 }, { "WARNING", 2 }, { "ERROR", 3 }, { "FATAL", 5 }, { "REQUEST", 4 } };
    switch (m[level]) {
    case INFO:
        return BLUE;
    case DEBUG:
        return GREEN;
    case WARNING:
        return YELLOW;
    case ERROR:
        return MAGENTA;
    case FATAL:
        return RED;
    case REQUEST:
        return CYAN;
    default:
        return RESET;
    }
}

#define LOG_DIR "./log/"

class DualStream {
public:
    DualStream() {
        auto path = std::string(LOG_DIR);
        std::string cmd;
        cmd += "mkdir ";
        cmd += LOG_DIR;
        struct stat info;
        stat(path.c_str(), &info);
        if (!(info.st_mode & S_IFDIR))
            system(cmd.c_str());
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M");
        int pid = getpid(); // 获取当前进程ID
        // 构造文件名
        std::string filename = path + ss.str() + "_" + std::to_string(pid) + ".log";
        file.open(filename, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    ~DualStream() {
        if (file.is_open())
            file.close();
    }
    // template <typename T>
    DualStream& operator<<(const std::string& msg) {
        std::cout << msg;
        file << stripColorCodes(msg);
        return *this;
    }
    // 特别处理控制符比如 std::endl
    DualStream& operator<<(std::ostream& (*pf)(std::ostream&)) {
        std::cout << pf;
        file << pf;
        return *this;
    } //
private:
    std::ofstream file;
    std::string stripColorCodes(const std::string& input) {
        std::string output;
        std::istringstream stream(input);
        char ch;
        bool escapeSeq = false;
        while (stream.get(ch)) {
            if (ch == '\033') { // ESC character
                escapeSeq = true;
                continue;
            }
            if (escapeSeq) {
                if (ch == 'm') { // End of ANSI escape sequence
                    escapeSeq = false;
                }
                continue;
            }
            output.push_back(ch);
        }
        return output;
    }
};

// 全局日志文件流

DualStream logStream;

// LOG() << "message"
inline DualStream& Log(const std::string& level, const std::string& file_name, int line) {
    // 添加日志等级
    std::string levelTag = std::string("[") + level + "]";
    std::string coloredLevelTag = std::string(GetColor(level)) + levelTag + RESET;
    std::string message = coloredLevelTag;
    // 添加报错文件名称
    message += "[";
    message += file_name;
    message += "]";
    // 添加当前文件的行数
    message += "[";
    message += std::to_string(line);
    message += "]";
    // cout 本质内部是包含缓冲区的
    logStream << message << " "; // 不要endl进行刷新
    return logStream;
}

// 这种就是开放式的日志
#define LOG(level) Log(#level, __FILE__, __LINE__)
} // namespace name

#endif