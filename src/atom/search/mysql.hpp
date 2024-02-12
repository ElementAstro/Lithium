/*
 * mysql.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-6

Description: Simple Mysql wrapper

**************************************************/

#pragma once

#include <mariadb/mysql.h>
#include <functional>

class MysqlDB
{
public:
    MysqlDB(const char *host, const char *user, const char *password, const char *database);
    ~MysqlDB();

    bool executeQuery(const char *query);
    void selectData(const char *query);
    int getIntValue(const char *query);
    double getDoubleValue(const char *query);
    const char *getTextValue(const char *query);
    bool searchData(const char *query, const char *searchTerm);
    bool updateData(const char *query);
    bool deleteData(const char *query);
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    void handleMySQLError();
    bool validateData(const char *query, const char *validationQuery);
    void selectDataWithPagination(const char *query, int limit, int offset);
    void setErrorMessageCallback(const std::function<void(const char *)> &errorCallback);

private:
    MYSQL *db;

private:
    std::function<void(const char *)> errorCallback = [](const char *errorMessage)
    {
    };
};

/*
int main()
{
    const char *host = "localhost";
    const char *user = "root";
    const char *password = "password";
    const char *database = "mydatabase";

    MysqlDB db(host, user, password, database);

    // 执行查询语句
    db.selectData("SELECT * FROM customers");

    // 执行更新语句
    bool updateResult = db.updateData("UPDATE customers SET address='New Address' WHERE id=1");
    if (updateResult)
    {
        std::cout << "Update successful" << std::endl;
    }
    else
    {
        std::cout << "Update failed" << std::endl;
    }

    // 获取整数值
    int intValue = db.getIntValue("SELECT COUNT(*) FROM customers");
    std::cout << "Total customers: " << intValue << std::endl;

    // 获取浮点数值
    double doubleValue = db.getDoubleValue("SELECT AVG(price) FROM products");
    std::cout << "Average price: " << doubleValue << std::endl;

    // 获取文本值
    const char *textValue = db.getTextValue("SELECT name FROM customers WHERE id=1");
    std::cout << "Customer name: " << textValue << std::endl;

    return 0;
}
*/