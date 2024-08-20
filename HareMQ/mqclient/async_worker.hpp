/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_CLIENT_LOOP_WORKER__
#define __YUFC_CLIENT_LOOP_WORKER__

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/thread_pool.hpp"
#include "muduo/net/EventLoopThread.h"

namespace hare_mq {
class async_worker {
public:
    using ptr = std::shared_ptr<async_worker>;
    muduo::net::EventLoopThread loop_thread;
    thread_pool pool;
};
} // namespace hare_mq

#endif