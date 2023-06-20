#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sqlite3.h>
#include "spdlog/spdlog.h"

/**
 * @brief 数据库管理器类，用于管理数据库操作
 */
class DatabaseManager
{
public:
    /**
     * @brief 构造函数，打开指定路径的数据库
     * @param db_path 数据库文件路径
     */
    explicit DatabaseManager(const std::string& db_path);

    /**
     * @brief 析构函数，释放资源并关闭数据库连接
     */
    virtual ~DatabaseManager();

    /**
     * @brief 执行 SQL 查询语句
     * @param sql SQL 查询语句
     * @return 查询结果，用 vector<vector<string>> 表示，每个 vector<string> 表示一行记录
     */
    std::vector<std::vector<std::string>> execute_query(const std::string& sql);

    /**
     * @brief 执行 SQL 更新语句（如 INSERT、UPDATE、DELETE 等）
     * @param sql SQL 更新语句
     * @return 是否执行成功
     */
    bool execute_update(const std::string& sql);

    /**
     * @brief 获取最后插入的自增主键值
     * @return 自增主键值
     */
    int get_last_insert_id();

    /**
     * @brief 获取 SQLite 版本号
     * @return SQLite 版本号
     */
    std::string get_version();

private:
    sqlite3* __conn; ///< 数据库连接
    std::shared_ptr<spdlog::logger> __logger; ///< 日志记录器

    /**
     * @brief SQL 回调函数，用于接收查询结果
     * @param data 用户自定义数据
     * @param argc 查询结果列数
     * @param argv 查询结果，字符串数组
     * @param col_names 查询结果列名，字符串数组
     * @return 成功返回 0，失败返回 1
     */
    static int __query_callback(void* data, int argc, char** argv, char** col_names);
};

