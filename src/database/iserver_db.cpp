#include "iserver_db.hpp"

#include <cstdlib>
#include <iostream>

SQLiteManager::SQLiteManager(const std::string &db_path)
{
    int rc = sqlite3_open(db_path.c_str(), &__conn);
    if (rc)
    {
        std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(__conn) << std::endl;
        sqlite3_close(__conn);
        exit(1);
    }
}

SQLiteManager::~SQLiteManager()
{
    sqlite3_close(__conn);
}

void SQLiteManager::update_profile(const std::string &name, int port, int autostart, int autoconnect)
{
    std::string sql = "UPDATE profile SET port=?, autostart=?, autoconnect=? WHERE name=?";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(__conn, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error preparing SQLite statement: " << sqlite3_errmsg(__conn) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_bind_int(stmt, 1, port);
    sqlite3_bind_int(stmt, 2, autostart);
    sqlite3_bind_int(stmt, 3, autoconnect);
    sqlite3_bind_text(stmt, 4, name.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Error updating SQLite record: " << sqlite3_errmsg(__conn) << std::endl;
    }

    sqlite3_finalize(stmt);
}

int SQLiteManager::add_custom_driver(const std::string &label)
{
    std::string sql = "INSERT OR IGNORE INTO custom (label) VALUES (?)";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(__conn, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error preparing SQLite statement: " << sqlite3_errmsg(__conn) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, label.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Error inserting into SQLite table: " << sqlite3_errmsg(__conn) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    int id = __get_last_insert_rowid();

    sqlite3_finalize(stmt);
    return id;
}

void SQLiteManager::delete_custom_driver(int id)
{
    std::string sql = "DELETE FROM custom WHERE id=?";
    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(__conn, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error preparing SQLite statement: " << sqlite3_errmsg(__conn) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Error deleting from SQLite table: " << sqlite3_errmsg(__conn) << std::endl;
    }

    sqlite3_finalize(stmt);
}

void SQLiteManager::add_remote_driver(const std::string &name, const std::vector<std::string> &drivers)
{
    std::string sql = "INSERT INTO remote (drivers, profile) VALUES (?, (SELECT id FROM profile WHERE name=?))";
    for (int i = 1; i < drivers.size(); ++i)
    {
        sql += ", (?, (SELECT id FROM profile WHERE name=?))";
    }

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(__conn, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error preparing SQLite statement: " << sqlite3_errmsg(__conn) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    int index = 1;
    for (const auto &driver : drivers)
    {
        sqlite3_bind_text(stmt, index++, driver.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, index++, name.c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Error inserting into SQLite table: " << sqlite3_errmsg(__conn) << std::endl;
    }

    sqlite3_finalize(stmt);
}

void SQLiteManager::delete_remote_driver(const std::string &name, const std::vector<std::string> &drivers)
{
    std::string sql = "DELETE FROM remote WHERE profile=(SELECT id FROM profile WHERE name=?) AND drivers=?";
    for (int i = 1; i < drivers.size(); ++i)
    {
        sql += " OR drivers=?";
    }

    sqlite3_stmt *stmt = nullptr;
    int rc = sqlite3_prepare_v2(__conn, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error preparing SQLite statement: " << sqlite3_errmsg(__conn) << std::endl;
        sqlite3_finalize(stmt);
        return;
    }

    int index = 1;
    sqlite3_bind_text(stmt, index++, name.c_str(), -1, SQLITE_TRANSIENT);
    for (const auto &driver : drivers)
    {
        sqlite3_bind_text(stmt, index++, driver.c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Error deleting from SQLite table: " << sqlite3_errmsg(__conn) << std::endl;
    }

    sqlite3_finalize(stmt);
}

int SQLiteManager::__get_last_insert_rowid()
{
    std::string sql = "SELECT last_insert_rowid()";
    char *error_msg = nullptr;
    int row_id = 0;
    sqlite3_exec(
        __conn, sql.c_str(), [](void *data, int argc, char **argv, char **col_names)
        { *((int*)data) = atoi(argv[0]); return 0; },
        &row_id, &error_msg);
    return row_id;
}

int SQLiteManager::__exec_callback(void *data, int argc, char **argv, char **col_names)
{
    return 0;
}

/*
#include <iostream>
#include "SQLiteManager.h"

int main() {
    std::string db_path = "test.db";
    SQLiteManager manager(db_path);

    std::cout << "Adding custom driver..." << std::endl;
    int id = manager.add_custom_driver("My Custom Driver");
    if (id != -1) {
        std::cout << "Custom driver added with ID: " << id << std::endl;
    }

    std::cout << "Updating profile..." << std::endl;
    manager.update_profile("Default", 8080, 1, 0);

    std::cout << "Deleting custom driver..." << std::endl;
    manager.delete_custom_driver(id);

    std::cout << "Adding remote driver..." << std::endl;
    std::vector<std::string> drivers {"Driver A", "Driver B"};
    manager.add_remote_driver("Remote 1", drivers);

    std::cout << "Deleting remote driver..." << std::endl;
    manager.delete_remote_driver("Remote 1", drivers);

    return 0;
}

*/