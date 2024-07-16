
#include "contacts.pb.h"

int main() {
    contacts::contact conn;
    conn.set_sn(10001);
    conn.set_name("Sam");
    conn.set_score(60.5);

    std::string str = conn.SerializeAsString();
    // conn.SerializeToString(&str); // 一样的
    contacts::contact stu;
    bool ret = stu.ParseFromString(str);
    if (ret == false) // 反序列化失败
        assert(false);
    std::cout << stu.sn() << " " << stu.name() << " " << stu.score() << std::endl;
    return 0;
}