/*
 * sqlite.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-12-6

Description: Simple Sqilte3 wrapper

**************************************************/

#include "sqlite.hpp"

#include "atom/log/loguru.hpp"

SqliteDB::SqliteDB(const char *dbPath)
{
    errorCallback = [](const char *errorMessage)
    {
        LOG_F(ERROR, "{}", errorMessage);
    };
    int rc = sqlite3_open(dbPath, &db);
    if (rc)
    {
        errorCallback(sqlite3_errmsg(db));
    }
    else
    {
        DLOG_F(INFO, "Open database: {}", dbPath);
    }
}

SqliteDB::~SqliteDB()
{
    sqlite3_close(db);
    DLOG_F(INFO, "Close database");
}

bool SqliteDB::executeQuery(const char *query)
{
    char *errorMessage = 0;
    int rc = sqlite3_exec(db, query, 0, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {
        errorCallback(sqlite3_errmsg(db));
        sqlite3_free(errorMessage);
        return false;
    }
    return true;
}

void SqliteDB::selectData(const char *query)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);

    if (rc == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            // 处理查询结果
        }
    }
    else
    {
        errorCallback(sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

int SqliteDB::getIntValue(const char *query)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    int result = 0;

    if (rc == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result = sqlite3_column_int(stmt, 0);
        }
    }
    else
    {
        errorCallback(sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    return result;
}

double SqliteDB::getDoubleValue(const char *query)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    double result = 0.0;

    if (rc == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result = sqlite3_column_double(stmt, 0);
        }
    }
    else
    {
        errorCallback(sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    return result;
}

const unsigned char *SqliteDB::getTextValue(const char *query)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    const unsigned char *result;

    if (rc == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result = sqlite3_column_text(stmt, 0);
        }
    }
    else
    {
        errorCallback(sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    return result;
}

bool SqliteDB::searchData(const char *query, const char *searchTerm)
{
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, 0);
    bool result = false;

    if (rc == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, searchTerm, -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            result = true;
        }
    }
    else
    {
        errorCallback(sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    return result;
}

bool SqliteDB::updateData(const char *query)
{
    return executeQuery(query);
}

bool SqliteDB::deleteData(const char *query)
{
    return executeQuery(query);
}

bool SqliteDB::beginTransaction()
{
    return executeQuery("BEGIN TRANSACTION");
}

bool SqliteDB::commitTransaction()
{
    return executeQuery("COMMIT TRANSACTION");
}

bool SqliteDB::rollbackTransaction()
{
    return executeQuery("ROLLBACK TRANSACTION");
}

void SqliteDB::handleSQLError()
{
    errorCallback(sqlite3_errmsg(db));
}

bool SqliteDB::validateData(const char *query, const char *validationQuery)
{
    if (!executeQuery(query))
    {
        return false;
    }

    int validationResult = getIntValue(validationQuery);

    if (validationResult == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void SqliteDB::selectDataWithPagination(const char *query, int limit, int offset)
{
    // 构建带有分页的查询语句
    std::string queryWithPagination = query;
    queryWithPagination += " LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);

    // 执行分页查询
    selectData(queryWithPagination.c_str());
}

void SqliteDB::setErrorMessageCallback(const std::function<void(const char *)> &errorCallback)
{
    this->errorCallback = errorCallback;
}