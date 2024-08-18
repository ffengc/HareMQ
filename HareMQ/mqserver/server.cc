/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "broker_server.hpp"

int main() {
    hare_mq::BrokerServer svr(8085, "./data");
    svr.start();
    return 0;
}