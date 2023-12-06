/*
 * sqlite.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-6

Description: Simple Sqilte3 wrapper

**************************************************/

#pragma once

#include <sqlite3.h>
#include <string>
#include <functional>

/**
 * @class SqliteDB
 * @brief 表示一个用于操作SQLite数据库的类
 */
class SqliteDB
{
private:
    sqlite3 *db; /**< SQLite数据库连接对象 */

public:
    /**
     * @brief 构造函数
     * @param dbPath 数据库文件的路径
     */
    SqliteDB(const char *dbPath);

    /**
     * @brief 析构函数
     */
    ~SqliteDB();

    /**
     * @brief 执行查询语句
     * @param query 查询语句
     * @return 执行是否成功
     */
    bool executeQuery(const char *query);

    /**
     * @brief 查询并获取数据
     * @param query 查询语句
     */
    void selectData(const char *query);

    /**
     * @brief 获取整型值
     * @param query 查询语句
     * @return 整型值
     */
    int getIntValue(const char *query);

    /**
     * @brief 获取浮点型值
     * @param query 查询语句
     * @return 浮点型值
     */
    double getDoubleValue(const char *query);

    /**
     * @brief 获取文本值
     * @param query 查询语句
     * @return 文本值
     */
    const unsigned char *getTextValue(const char *query);

    /**
     * @brief 在查询结果中搜索指定的项
     * @param query 查询语句
     * @param searchTerm 要搜索的项
     * @return 是否找到匹配项
     */
    bool searchData(const char *query, const char *searchTerm);

    /**
     * @brief 更新数据
     * @param query 更新语句
     * @return 更新是否成功
     */
    bool updateData(const char *query);

    /**
     * @brief 删除数据
     * @param query 删除语句
     * @return 删除是否成功
     */
    bool deleteData(const char *query);

    /**
     * @brief 开始数据库事务
     * @return 是否成功开始事务
     */
    bool beginTransaction();

    /**
     * @brief 提交数据库事务
     * @return 是否成功提交事务
     */
    bool commitTransaction();

    /**
     * @brief 回滚数据库事务
     * @return 是否成功回滚事务
     */
    bool rollbackTransaction();

    /**
     * @brief 处理SQLite错误
     */
    void handleSQLError();

    /**
     * @brief 验证数据是否符合指定的查询条件
     * @param query 查询语句
     * @param validationQuery 验证条件语句
     * @return 验证结果
     */
    bool validateData(const char *query, const char *validationQuery);

    /**
     * @brief 分页查询并获取数据
     * @param query 查询语句
     * @param limit 每页记录数
     * @param offset 偏移量
     */
    void selectDataWithPagination(const char *query, int limit, int offset);

    /**
     * @brief 设置错误消息回调函数
     * @param errorCallback 错误消息回调函数
     */
    void setErrorMessageCallback(const std::function<void(const char *)> &errorCallback);

private:
    std::function<void(const char *)> errorCallback = [](const char *errorMessage)
    {
    };
};

/*
int main()
{
    // 创建数据库对象，并指定数据库文件路径
    SqliteDB db("example.db");

    db.setErrorMessageCallback([](const char *errorMessage)
                               {
                                   std::cerr << "Error: " << errorMessage << std::endl;
                                   // 进行其他处理，如记录日志等
                               });

    // 执行创建表的 SQL 语句
    const char *createTableQuery = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)";
    db.executeQuery(createTableQuery);

    // 执行插入数据的 SQL 语句
    const char *insertQuery = "INSERT INTO users (name, age) VALUES ('Alice', 25)";
    db.executeQuery(insertQuery);

    // 执行查询数据的 SQL 语句
    const char *selectQuery = "SELECT * FROM users";
    db.selectData(selectQuery);

    bool exists = db.searchData("SELECT * FROM users WHERE name = 'Alice'", "Alice");
    if (exists)
    {
        std::cout << "User Alice exists" << std::endl;
    }
    else
    {
        std::cout << "User Alice does not exist" << std::endl;
    }

    // 获取整数、浮点数和文本值
    int userId = db.getIntValue("SELECT id FROM users WHERE name = 'Alice'");
    double userAge = db.getDoubleValue("SELECT age FROM users WHERE name = 'Alice'");
    const unsigned char *userName = db.getTextValue("SELECT name FROM users WHERE id = 1");
    std::cout << "User Age: " << userAge << std::endl;
    std::cout << "User ID: " << userId << std::endl;
    std::cout << "User Name: " << userName << std::endl;

    // 更新数据
    const char *updateQuery = "UPDATE users SET age = 26 WHERE name = 'Alice'";
    db.updateData(updateQuery);

    // 删除数据
    const char *deleteQuery = "DELETE FROM users WHERE name = 'Alice'";
    db.deleteData(deleteQuery);

    return 0;
}
*/
