/*
 * mysql.cpp
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

Description: Simple Mysql wrapper

**************************************************/

#include "mysql.hpp"
#include <cstring>

#include "atom/log/loguru.hpp"

MysqlDB::MysqlDB(const char *host, const char *user, const char *password, const char *database)
{
    db = mysql_init(nullptr);
    if (db == nullptr)
    {
        handleMySQLError();
        return;
    }

    if (!mysql_real_connect(db, host, user, password, database, 0, nullptr, 0))
    {
        handleMySQLError();
    }
}

MysqlDB::~MysqlDB()
{
    if (db != nullptr)
    {
        mysql_close(db);
    }
}

bool MysqlDB::executeQuery(const char *query)
{
    if (mysql_query(db, query) != 0)
    {
        handleMySQLError();
        return false;
    }

    return true;
}

void MysqlDB::selectData(const char *query)
{
    if (!executeQuery(query))
    {
        return;
    }

    MYSQL_RES *result = mysql_store_result(db);
    if (result == nullptr)
    {
        handleMySQLError();
        return;
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        for (int i = 0; i < num_fields; i++)
        {
            std::cout << row[i] << " ";
        }
        std::cout << std::endl;
    }

    mysql_free_result(result);
}

int MysqlDB::getIntValue(const char *query)
{
    if (!executeQuery(query))
    {
        return 0;
    }

    MYSQL_RES *result = mysql_store_result(db);
    if (result == nullptr)
    {
        handleMySQLError();
        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    int value = 0;
    if (row != nullptr)
    {
        value = atoi(row[0]);
    }

    mysql_free_result(result);

    return value;
}

double MysqlDB::getDoubleValue(const char *query)
{
    if (!executeQuery(query))
    {
        return 0.0;
    }

    MYSQL_RES *result = mysql_store_result(db);
    if (result == nullptr)
    {
        handleMySQLError();
        return 0.0;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    double value = 0.0;
    if (row != nullptr)
    {
        value = atof(row[0]);
    }

    mysql_free_result(result);

    return value;
}

const char *MysqlDB::getTextValue(const char *query)
{
    if (!executeQuery(query))
    {
        return "";
    }

    MYSQL_RES *result = mysql_store_result(db);
    if (result == nullptr)
    {
        handleMySQLError();
        return "";
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    const char *value = "";
    if (row != nullptr)
    {
        value = row[0];
    }

    mysql_free_result(result);

    return value;
}

bool MysqlDB::searchData(const char *query, const char *searchTerm)
{
    if (!executeQuery(query))
    {
        return false;
    }

    MYSQL_RES *result = mysql_store_result(db);
    if (result == nullptr)
    {
        handleMySQLError();
        return false;
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        for (int i = 0; i < num_fields; i++)
        {
            if (strcmp(row[i], searchTerm) == 0)
            {
                mysql_free_result(result);
                return true;
            }
        }
    }

    mysql_free_result(result);

    return false;
}

bool MysqlDB::updateData(const char *query)
{
    return executeQuery(query);
}

bool MysqlDB::deleteData(const char *query)
{
    return executeQuery(query);
}

bool MysqlDB::beginTransaction()
{
    return executeQuery("START TRANSACTION");
}

bool MysqlDB::commitTransaction()
{
    return executeQuery("COMMIT");
}

bool MysqlDB::rollbackTransaction()
{
    return executeQuery("ROLLBACK");
}

void MysqlDB::handleMySQLError()
{
    if (db == nullptr)
    {
        return;
    }

    const char *error_msg = mysql_error(db);
    if (error_msg != nullptr && strlen(error_msg) > 0)
    {
        LOG_F(ERROR, "MySQL error: {}", error_msg);
        if (errorCallback != nullptr)
        {
            errorCallback(error_msg);
        }
    }
}

bool MysqlDB::validateData(const char *query, const char *validationQuery)
{
    if (!executeQuery(query))
    {
        return false;
    }

    MYSQL_RES *result = mysql_store_result(db);
    if (result == nullptr)
    {
        handleMySQLError();
        return false;
    }

    int num_fields = mysql_num_fields(result);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        if (!executeQuery(validationQuery))
        {
            mysql_free_result(result);
            return false;
        }

        MYSQL_RES *validationResult = mysql_store_result(db);
        if (validationResult == nullptr)
        {
            handleMySQLError();
            mysql_free_result(result);
            return false;
        }

        MYSQL_ROW validationRow = mysql_fetch_row(validationResult);
        if (validationRow == nullptr)
        {
            mysql_free_result(validationResult);
            mysql_free_result(result);
            return false;
        }

        bool isValid = true;
        for (int i = 0; i < num_fields; i++)
        {
            if (strcmp(row[i], validationRow[i]) != 0)
            {
                isValid = false;
                break;
            }
        }

        mysql_free_result(validationResult);

        if (isValid)
        {
            mysql_free_result(result);
            return true;
        }
    }

    mysql_free_result(result);

    return false;
}

void MysqlDB::selectDataWithPagination(const char *query, int limit, int offset)
{
    char sql[1024];
    snprintf(sql, sizeof(sql), "%s LIMIT %d OFFSET %d", query, limit, offset);

    selectData(sql);
}

void MysqlDB::setErrorMessageCallback(const std::function<void(const char *)> &errorCallback)
{
    this->errorCallback = errorCallback;
}
