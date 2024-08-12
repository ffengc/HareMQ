/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/route.hpp"
#include <gtest/gtest.h>

using namespace hare_mq;

class route_test : public testing::Environment {
public:
    virtual void SetUp() override { }
    virtual void TearDown() override { }
};

TEST(route_test, legal_routing_key) {
    std::string r1 = "news.music.pop";
    std::string r2 = "news..music.pop";
    std::string r3 = "news.,music.pop";
    std::string r4 = "news.music_123.pop";
    ASSERT_EQ(router::is_legal_routing_key(r1), true);
    ASSERT_EQ(router::is_legal_routing_key(r2), true);
    ASSERT_EQ(router::is_legal_routing_key(r3), false);
    ASSERT_EQ(router::is_legal_routing_key(r4), true);
}

TEST(route_test, legal_binding_key) {
    std::string b1 = "news.music.pop";
    std::string b2 = "news.#.music.pop";
    std::string b3 = "news.#.*.music.pop"; //
    std::string b4 = "news.*.#.music.pop"; //
    std::string b5 = "news.#.#.music.pop"; //
    std::string b6 = "news.*.*.music.pop";
    std::string b7 = "news.music_123.pop";
    std::string b8 = "news.,music_123.pop"; //
    std::string b9 = "#";
    ASSERT_EQ(router::is_legal_binding_key(b1), true);
    ASSERT_EQ(router::is_legal_binding_key(b2), true);
    ASSERT_EQ(router::is_legal_binding_key(b3), false);
    ASSERT_EQ(router::is_legal_binding_key(b4), false);
    ASSERT_EQ(router::is_legal_binding_key(b5), false);
    ASSERT_EQ(router::is_legal_binding_key(b6), true);
    ASSERT_EQ(router::is_legal_binding_key(b7), true);
    ASSERT_EQ(router::is_legal_binding_key(b8), false);
    ASSERT_EQ(router::is_legal_binding_key(b9), true);
}

TEST(route_test, route) {
    std::vector<std::string> binding_keys = {
        "aaa",
        "aaa.bbb",
        "aaa.bbb",
        "aaa.bbb",
        "aaa.#.bbb",
        "aaa.bbb.#",
        "#.bbb.ccc",
        "aaa.bbb.ccc",
        "aaa.*",
        "aaa.*.bbb",
        "*.aaa.bbb",
        "#",
        "aaa.#",
        "aaa.#",
        "aaa.#.ccc",
        "aaa.#.ccc",
        "aaa.#.ccc",
        "#.ccc",
        "#.ccc",
        "aaa.#.ccc.ccc",
        "aaa.#.bbb.*.bbb"
    };
    std::vector<std::string> routing_keys = {
        "aaa",
        "aaa.bbb",
        "aaa.bbb.ccc",
        "aaa.ccc",
        "aaa.bbb.ccc",
        "aaa.ccc.bbb",
        "aaa.bbb.ccc.ddd",
        "aaa.bbb.ccc",
        "aaa.bbb",
        "aaa.bbb.ccc",
        "aaa.bbb",
        "aaa.bbb.ccc",
        "aaa.bbb",
        "aaa.bbb.ccc",
        "aaa.ccc",
        "aaa.bbb.ccc",
        "aaa.aaa.bbb.ccc",
        "ccc",
        "aaa.bbb.ccc",
        "aaa.bbb.ccc.ccc.ccc",
        "aaa.ddd.ccc.bbb.eee.bbb"
    };
    std::vector<bool> results = {
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        false,
        false,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true,
        true
    };
    ASSERT_EQ(binding_keys.size(), routing_keys.size());
    ASSERT_EQ(binding_keys.size(), results.size());
    for (size_t i = 0; i < binding_keys.size(); ++i) {
        LOG(DEBUG) << "i: " << i << "/21 " << routing_keys[i] << ":" << binding_keys[i] << std::endl; 
        ASSERT_EQ(router::route(ExchangeType::TOPIC, routing_keys[i], binding_keys[i]), results[i]);
    }
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new route_test);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}