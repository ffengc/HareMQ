
#include "db.hpp"
#include <assert.h>
#include <iostream>
#include <vector>

int select_cb(void* arg, int col_count, char** result, char** fields_name) {
    std::vector<std::string>* arr = (std::vector<std::string>*)arg; // 拿到传进来的数组
    arr->push_back(result[0]); // 因为查询结果只有一个字段，所以push一个就行了
    return 0; // 这里一定要返回0表示正常，否则可能会触发abort
}

int main() {
    // 1. 创建/打开库文件
    sqlite_helper helper("./test.db");
    assert(helper.open());
    // 2. 创建表（不存在则创建）
    const char* create_sql = "create table if not exists student(sn int primary key, name varchar(32), age int);";
    assert(helper.exec(create_sql, nullptr, nullptr));
    // 3. 新增数据（增删查改）
    const char* insert_sql = "insert into student values(1, 'Sam', 18), (2, 'Jack', 19), (3, 'Lucy', 18);";
    assert(helper.exec(insert_sql, nullptr, nullptr));
    const char* select_sql = "select name from student;";
    std::vector<std::string> arr;
    assert(helper.exec(select_sql, select_cb, &arr));
    for (const auto& name : arr)
        std::cout << name << " ";
    std::cout << std::endl;
    // 4. 关闭数据库
    helper.close();
    return 0;
}