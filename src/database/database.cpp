#include "DatabaseManager.h"
#include <stdexcept>

DatabaseManager::DatabaseManager(const std::string &db_path)
    : __conn(nullptr)
{
    // 打开数据库连接
    int rc = sqlite3_open(db_path.c_str(), &__conn);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(__conn)));
    }

    // 初始化日志记录器
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("database.log", true);
    __logger = std::make_shared<spdlog::logger>("database_manager", spdlog::sinks_init_list({console_sink, file_sink}));

    __logger->set_level(spdlog::level::debug);
    __logger->debug("Database connection opened.");
}

DatabaseManager::~DatabaseManager()
{
    // 关闭数据库连接
    if (__conn != nullptr)
    {
        sqlite3_close(__conn);
        __logger->debug("Database connection closed.");
    }
}

std::vector<std::vector<std::string>> DatabaseManager::execute_query(const std::string &sql)
{
    char *err_msg = nullptr;
    std::vector<std::vector<std::string>> rows;

    __logger->debug("Execute SQL query: {}", sql);

    int rc = sqlite3_exec(__conn, sql.c_str(), __query_callback, &rows, &err_msg);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to execute SQL query: " + std::string(err_msg));
    }

    __logger->debug("SQL query executed successfully, rows: {}", rows.size());

    return rows;
}

bool DatabaseManager::execute_update(const std::string &sql)
{
    char *err_msg = nullptr;

    __logger->debug("Execute SQL update: {}", sql);

    int rc = sqlite3_exec(__conn, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to execute SQL update: " + std::string(err_msg));
    }

    __logger->debug("SQL update executed successfully.");

    return true;
}

int DatabaseManager::get_last_insert_id()
{
    return static_cast<int>(sqlite3_last_insert_rowid(__conn));
}

std::string DatabaseManager::get_version()
{
    return sqlite3_libversion();
}

int DatabaseManager::__query_callback(void *data, int argc, char **argv, char **col_names)
{
    auto rows = static_cast<std::vector<std::vector<std::string>> *>(data);
    std::vector<std::string> row;

    for (int i = 0; i < argc; ++i)
    {
        row.emplace_back(argv[i] ? argv[i] : "");
    }

    rows->emplace_back(row);
    return 0;
}
