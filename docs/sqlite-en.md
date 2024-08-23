# SQLite3

- [简体中文](./sqlite.md)
- [English](./sqlite-en.md)

***
- [SQLite3](#sqlite3)
  - [What is SQLite](#what-is-sqlite)
  - [Why do you need SQLite](#why-do-you-need-sqlite)
  - [Official Documentation](#official-documentation)
  - [Encapsulating Helper](#encapsulating-helper)
  - [Conduct some experiments](#conduct-some-experiments)

## What is SQLite

SQLite is an in-process lightweight database that implements a self-sufficient, serverless, zero-configuration, transactional SQL database engine. It is a zero-configuration database, which means that unlike other databases, we do not need to configure it in the system. Like other databases, the SQLite engine is not an independent process and can be statically or dynamically connected as required by the application. SQLite directly accesses its storage files.

## Why do you need SQLite

> [!NOTE]
> - No need for a separate server process or operating system (serverless).
> - SQLite does not require configuration.
> - A complete SQLite database is stored in a single cross-platform disk file.
> - SQLite is very small and lightweight, less than 400KiB when fully configured, less than 250KiB when optional features are omitted, SQLite is self-sufficient, which means that no external dependencies are required.
> - SQLite transactions are fully ACID-compliant, allowing safe access from multiple processes or threads.
> - SQLite supports most query language features of the SQL92 (SQL2) standard.
> - SQLite is written in ANSI-C and provides a simple and easy-to-use API.
> - SQLite runs on UNlX (Linux, MacOs-X, Android, iOS) and Windows (Win32, WinCE, WinRT).

## Official Documentation

- **[https://www.sqlite.org/c3ref/funclist.html](https://www.sqlite.org/c3ref/funclist.html)**

## Encapsulating Helper

Because we will not use all functions, we first encapsulate some commonly used methods into a `.hpp` file for easy subsequent use.

```cpp
/**
 * Encapsulate common methods of sqlite
 */

#ifndef __YUFC_SQLITE_HELPER__
#define __YUFC_SQLITE_HELPER__

#include "../log.hpp"
#include <iostream>
#include <sqlite3.h>
#include <string>

class sqlite_helper {
public:
    typedef int (*sqlite_callback)(void*, int, char**, char**);

private:
    sqlite3* __handler;
    std::string __db_file;

public:
    sqlite_helper(const std::string& db_file)
        : __db_file(db_file)
        , __handler(nullptr) { }
    bool open(int safe_lavel = SQLITE_OPEN_FULLMUTEX) {
        // int sqlite3_open_v2(const char* filename, sqlite3 **ppDb, int flags, const char* zVfs);
        int ret = sqlite3_open_v2(__db_file.c_str(), &__handler, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | safe_lavel, nullptr);
        if (ret != SQLITE_OK) {
            LOG(ERROR) << "create database failed: " << sqlite3_errmsg(__handler) << std::endl;
            return false;
        }
        return true;
    }
    bool exec(const std::string& sql, sqlite_callback& cb, void* arg) {
        // int sqlite3_exec(sqlite3*, char* sql, int (*callback)(void*, int, char**, char**), void* arg, char**err);
        int ret = sqlite3_exec(__handler, sql.c_str(), cb, arg, nullptr);
        if (ret != SQLITE_OK) {
            LOG(ERROR) << "run exec: [" << sql << "] failed: " << sqlite3_errmsg(__handler) << std::endl;
            return false;
        }
        return true;
    }
    bool close() {
        if (__handler) {
            if (sqlite3_close_v2(__handler))
                return true;
            LOG(ERROR) << "close error" << std::endl;
            return false;
        }
        LOG(ERROR) << "null sql handler" << std::endl;
        return false;
    }
};

#endif
```

## Conduct some experiments

Let’s try inserting some data::

```cpp
int main() {
    // 1. Create/Open db File
    sqlite_helper helper("./test.db");
    assert(helper.open());
    // 2. Create a table (if it does not exist)
    const char* create_sql = "create table if not exists student(sn int primary key, name varchar(32), age int);";
    assert(helper.exec(create_sql, nullptr, nullptr));
    // 3. Add new data (add, delete, check and modify)
    const char* insert_sql = "insert into student values(1, 'Sam', 18), (2, 'Jack', 19), (3, 'Lucy', 18);";
    assert(helper.exec(insert_sql, nullptr, nullptr));
    // 4. Close the database
    helper.close();
    return 0;
}
```

After running, there will be a database file called `test.db`

```sh
sqlite3 test.db
```

You can see our data:

![](./assets/15.png)

Test query:

```cpp
int select_cb(void* arg, int col_count, char** result, char** fields_name) {
    std::vector<std::string>* arr = (std::vector<std::string>*)arg; // Get the passed in array
    arr->push_back(result[0]); // Because the query result has only one field, just push one.
    return 0; // Here, 0 must be returned to indicate normal operation, otherwise abort may be triggered.
}

int main() {
    // 1. Create/Open db File
    sqlite_helper helper("./test.db");
    assert(helper.open());
    // 2. Create a table (if it does not exist)
    const char* create_sql = "create table if not exists student(sn int primary key, name varchar(32), age int);";
    assert(helper.exec(create_sql, nullptr, nullptr));
    // 3. Add new data (add, delete, check and modify)
    const char* insert_sql = "insert into student values(1, 'Sam', 18), (2, 'Jack', 19), (3, 'Lucy', 18);";
    assert(helper.exec(insert_sql, nullptr, nullptr));
    const char* select_sql = "select name from student;";
    std::vector<std::string> arr;
    assert(helper.exec(select_sql, select_cb, &arr));
    for (const auto& name : arr)
        std::cout << name << " ";
    std::cout << std::endl;
    // 4. Close the database
    helper.close();
    return 0;
}
```

![](./assets/16.png)