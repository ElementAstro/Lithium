/*
 * sqlite.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "sqlite.hpp"

#include <sqlite3.h>
#include <mutex>

#include "atom/log/loguru.hpp"

class SqliteDB::Impl {
public:
    sqlite3 *db{};
    std::function<void(std::string_view)> errorCallback;

    Impl()
        : errorCallback([](std::string_view msg) { LOG_F(ERROR, "{}", msg); }) {
    }

    ~Impl() {
        if (db != nullptr) {
            sqlite3_close(db);
            DLOG_F(INFO, "Database closed");
        }
    }

    auto open(std::string_view dbPath) -> bool {
        int rc = sqlite3_open(dbPath.data(), &db);
        if (rc != 0) {
            errorCallback(sqlite3_errmsg(db));
            return false;
        }
        DLOG_F(INFO, "Opened database: {}", dbPath);
        return true;
    }
};

SqliteDB::SqliteDB(std::string_view dbPath) : pImpl(std::make_unique<Impl>()) {
    std::lock_guard<std::mutex> lock(mtx);
    pImpl->open(dbPath);
}

SqliteDB::~SqliteDB() = default;

auto SqliteDB::executeQuery(std::string_view query) -> bool {
    std::lock_guard<std::mutex> lock(mtx);
    char *errorMessage = nullptr;
    int rc =
        sqlite3_exec(pImpl->db, query.data(), nullptr, nullptr, &errorMessage);
    if (rc != SQLITE_OK) {
        pImpl->errorCallback(sqlite3_errmsg(pImpl->db));
        sqlite3_free(errorMessage);
        return false;
    }
    return true;
}

void SqliteDB::selectData(std::string_view query) {
    std::lock_guard<std::mutex> lock(mtx);
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, query.data(), -1, &stmt, nullptr);

    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            // Handle the query result
        }
    } else {
        pImpl->errorCallback(sqlite3_errmsg(pImpl->db));
    }

    sqlite3_finalize(stmt);
}

auto SqliteDB::getIntValue(std::string_view query) -> std::optional<int> {
    std::lock_guard<std::mutex> lock(mtx);
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, query.data(), -1, &stmt, nullptr);
    std::optional<int> result;

    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_int(stmt, 0);
        }
    } else {
        pImpl->errorCallback(sqlite3_errmsg(pImpl->db));
    }

    sqlite3_finalize(stmt);

    return result;
}

auto SqliteDB::getDoubleValue(std::string_view query) -> std::optional<double> {
    std::lock_guard<std::mutex> lock(mtx);
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, query.data(), -1, &stmt, nullptr);
    std::optional<double> result;

    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }
    } else {
        pImpl->errorCallback(sqlite3_errmsg(pImpl->db));
    }

    sqlite3_finalize(stmt);

    return result;
}

auto SqliteDB::getTextValue(std::string_view query)
    -> std::optional<std::string> {
    std::lock_guard<std::mutex> lock(mtx);
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(pImpl->db, query.data(), -1, &stmt, nullptr);
    std::optional<std::string> result;

    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result =
                reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        }
    } else {
        pImpl->errorCallback(sqlite3_errmsg(pImpl->db));
    }

    sqlite3_finalize(stmt);

    return result;
}

auto SqliteDB::searchData(std::string_view query,
                          std::string_view searchTerm) -> bool {
    std::lock_guard<std::mutex> lock(mtx);
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(pImpl->db, query.data(), -1, &stmt, nullptr) ==
        SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, searchTerm.data(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return true;
        }
    } else {
        pImpl->errorCallback(sqlite3_errmsg(pImpl->db));
    }
    sqlite3_finalize(stmt);
    return false;
}

auto SqliteDB::updateData(std::string_view query) -> bool {
    return executeQuery(query);
}

auto SqliteDB::deleteData(std::string_view query) -> bool {
    return executeQuery(query);
}

auto SqliteDB::beginTransaction() -> bool {
    return executeQuery("BEGIN TRANSACTION");
}

auto SqliteDB::commitTransaction() -> bool {
    return executeQuery("COMMIT TRANSACTION");
}

auto SqliteDB::rollbackTransaction() -> bool {
    return executeQuery("ROLLBACK TRANSACTION");
}

auto SqliteDB::validateData(std::string_view query,
                            std::string_view validationQuery) -> bool {
    std::lock_guard<std::mutex> lock(mtx);
    if (!executeQuery(query)) {
        return false;
    }

    auto validationResult = getIntValue(validationQuery);
    return validationResult.value_or(0) == 1;
}

void SqliteDB::selectDataWithPagination(std::string_view query, int limit,
                                        int offset) {
    std::lock_guard<std::mutex> lock(mtx);
    std::string queryWithPagination = std::string(query) + " LIMIT " +
                                      std::to_string(limit) + " OFFSET " +
                                      std::to_string(offset);

    selectData(queryWithPagination);
}

void SqliteDB::setErrorMessageCallback(
    const std::function<void(std::string_view)> &errorCallback) {
    std::lock_guard<std::mutex> lock(mtx);
    pImpl->errorCallback = errorCallback;
}
