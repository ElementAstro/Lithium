/*
 * search.cpp
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

Date: 2023-3-31

Description: AstroSearchX (ASX)

**************************************************/

#include "search.hpp"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace OpenAPT::ASX
{

    int Callback(void *data, int argc, char **argv, char **col_name)
    {
        if (argc != 6) {
            return SQLITE_OK;
        }
        auto &result = *static_cast<std::vector<Data> *>(data);

        result.emplace_back(Data{atoi(argv[0]), argv[1], argv[2], argv[3], argv[4],argv[5]});
        return SQLITE_OK;
    }

    sqlite3 *OpenDatabase(std::string db_name)
    {
        sqlite3 *db;
        int rc = sqlite3_open(db_name.c_str(), &db);
        if (rc != SQLITE_OK)
        {
            spdlog::error("Failed to open database: {}", sqlite3_errmsg(db));
            return nullptr;
        }
        spdlog::debug("Opened database successfully");
        return db;
    }

    std::vector<Data> ReadFromDatabase(sqlite3 *db)
    {
        std::vector<Data> data;
        std::string sql = "SELECT * FROM objects";
        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
            return data;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            std::string name(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));
            std::string type(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));
            std::string ra(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3)));
            std::string dec(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4)));
            std::string constellation(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5)));
            data.emplace_back(Data{id, name, type, ra, dec, constellation});
        }

        sqlite3_finalize(stmt);

        spdlog::debug("Read from database successfully");
        return data;
    }

    void InsertData(sqlite3 *db, const Data &d)
    {
        sqlite3_stmt *stmt;
        std::string sql = "INSERT INTO objects (name, type, ra, dec, constellation) VALUES (?, ?, ?, ?, ?)";
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
            return;
        }

        sqlite3_bind_text(stmt, 1, d.Name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, d.Type.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, d.RA.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, d.Dec.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, d.Const.c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            spdlog::error("Failed to insert data into database: {}", sqlite3_errmsg(db));
        } else {
            spdlog::debug("Inserted data into database successfully");
        }

        sqlite3_finalize(stmt);
    }

    void DeleteData(sqlite3 *db, const std::string &name)
    {
        sqlite3_stmt *stmt;
        std::string sql = "DELETE FROM objects WHERE name = ?";
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
            return;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            spdlog::error("Failed to delete data from database: {}", sqlite3_errmsg(db));
        } else {
            spdlog::debug("Deleted data from database successfully");
        }

        sqlite3_finalize(stmt);
    }

    void SortByName(std::vector<Data> &data)
    {
        std::sort(data.begin(), data.end(), [](const Data &a, const Data &b)
                { return a.Name < b.Name; });
    }

    std::vector<Data> FilterBy(const std::vector<Data> &data, std::function<bool(const Data &)> filter)
    {
        std::vector<Data> result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result), filter);
        return result;
    }

    void OptimizeDatabase(sqlite3 *db)
    {
        std::string sql = "VACUUM";
        char *err_msg = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK)
        {
            spdlog::error("Failed to optimize database: {}", err_msg);
        }
        else
        {
            spdlog::debug("Optimized database successfully");
        }
    }

    bool SaveToDatabase(sqlite3 *db, const std::vector<Data> &data)
    {
        sqlite3_stmt *stmt;
        std::string sql = "DELETE FROM objects";
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
            return false;
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            spdlog::error("Failed to update database: {}", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return false;
        } else {
            spdlog::debug("Updated database successfully");
        }

        sqlite3_finalize(stmt);

        sql = "INSERT INTO objects (name, type, ra, dec, constellation) VALUES (?, ?, ?, ?, ?)";
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            spdlog::error("Failed to prepare statement: {}", sqlite3_errmsg(db));
            return false;
        }

        for (const auto &d : data)
        {
            sqlite3_bind_text(stmt, 1, d.Name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, d.Type.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, d.RA.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, d.Dec.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, d.Const.c_str(), -1, SQLITE_TRANSIENT);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                spdlog::error("Failed to insert data into database: {}", sqlite3_errmsg(db));
                sqlite3_finalize(stmt);
                return false;
            } else {
                spdlog::debug("Inserted data into database successfully");
            }

            sqlite3_clear_bindings(stmt);
            sqlite3_reset(stmt);
        }

        sqlite3_finalize(stmt);

        return true;
    }

    std::vector<Data> SearchByName(const std::vector<Data> &data, const std::string &name)
    {
        std::vector<Data> result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result), [&name](const Data &d)
                    { return d.Name.find(name) != std::string::npos; });
        return result;
    }

    std::vector<Data> SearchByRaDec(const std::vector<Data> &data, std::string ra, std::string dec, double ra_range, double dec_range)
    {
        double d_ra, d_dec, q_ra, q_dec;
        try {
            d_ra = ToDecimal(ra);
            d_dec = ToDecimal(dec);
            q_ra = std::stod(ra);
            q_dec = std::stod(dec);
        } catch (std::invalid_argument& e) {
            spdlog::error("Invalid ra or dec format: {}", e.what());
            return {};
        }

        std::vector<Data> result;
        for (const auto &d : data)
        {
            double obj_ra = ToDecimal(d.RA);
            double obj_dec = ToDecimal(d.Dec);

            if (std::abs(obj_ra - d_ra) <= ra_range && std::abs(obj_dec - d_dec) <= dec_range)
            {
                result.push_back(d);
            }
        }
        return result;
    }

    double ToDecimal(const std::string &str)
    {
        size_t delim = str.find(":");
        double hours = std::stod(str.substr(0, delim));
        double mins = std::stod(str.substr(delim + 1, 2));
        double secs = std::stod(str.substr(delim + 4));
        double decimal = hours * 15.0 + mins / 4.0 + secs / 240.0;
        if (str.find("-") != std::string::npos)
        {
            decimal *= -1.0;
        }
        return decimal;
    }

    void SaveToJson(const std::vector<Data> &data, const std::string &filename)
    {
        json j;
        for (const auto &d : data)
        {
            j.push_back({{"name", d.Name},
                        {"type", d.Type},
                        {"ra", d.RA},
                        {"dec", d.Dec},
                        {"constellation", d.Const}});
        }
        std::ofstream file(filename);
        if (!file.is_open())
        {
            spdlog::error("Failed to open file for writing: {}", filename);
        }
        else
        {
            file << std::setw(4) << j << "\n";
            spdlog::info("Saved data to JSON file: {}", filename);
        }
        file.close();
    }

} // namespace OpenAPT::ASX
