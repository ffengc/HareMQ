/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqcommon/logger.hpp"
using namespace hare_mq;
int main() {
    LOG(DEBUG) << "hello world" << std::endl;
    LOG(INFO) << "hello world" << std::endl;
    return 0;
}