/*
 * sqlite.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-6

Description: Simple Sqilte3 wrapper

**************************************************/

#ifndef ATOM_SEARCH_SQLITE_HPP
#define ATOM_SEARCH_SQLITE_HPP

#include <sqlite3.h>
#include <functional>
#include <string>

/**
 * @class SqliteDB
 * @brief 表示一个用于操作SQLite数据库的类
 */
class SqliteDB {
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
    void setErrorMessageCallback(
        const std::function<void(const char *)> &errorCallback);

private:
    std::function<void(const char *)> errorCallback =
        [](const char *errorMessage) {};
};

#endif
