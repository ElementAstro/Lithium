#pragma once
#include <vector>
#include <string>
#include <sqlite3.h>

class SQLiteManager
{
public:
    SQLiteManager(const std::string &db_path);
    ~SQLiteManager();

    void update_profile(const std::string &name, int port, int autostart, int autoconnect);
    int add_custom_driver(const std::string &label);
    void delete_custom_driver(int id);
    void add_remote_driver(const std::string &name, const std::vector<std::string> &drivers);
    void delete_remote_driver(const std::string &name, const std::vector<std::string> &drivers);

private:
    sqlite3 *__conn;

    int __get_last_insert_rowid();

    static int __exec_callback(void *data, int argc, char **argv, char **col_names);
};
