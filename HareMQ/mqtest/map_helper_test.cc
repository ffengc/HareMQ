/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqcommon/helper.hpp"
#include <gtest/gtest.h>
using namespace hare_mq;

class map_helper_test : public testing::Environment {
public:
    virtual void SetUp() override {
    }
    virtual void TearDown() override {
    }
};

TEST(map_helper_test, test) {
    google::protobuf::Map<std::string, std::string> proto_map;
    proto_map["one"] = "1";
    proto_map["two"] = "2";
    std::unordered_map<std::string, std::string> std_map = map_helper::ConvertProtoMapToStdMap(proto_map);
    ASSERT_EQ(std_map.size(), 2);
    ASSERT_EQ(std_map["one"], "1");
    ASSERT_EQ(std_map["two"], "2");
    google::protobuf::Map<std::string, std::string> converted_proto_map = map_helper::ConvertStdMapToProtoMap(std_map);
    ASSERT_EQ(converted_proto_map.size(), 2);
    ASSERT_EQ(converted_proto_map["one"], "1");
    ASSERT_EQ(converted_proto_map["two"], "2");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new map_helper_test);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}