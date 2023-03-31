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

#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
using json = nlohmann::json;

class Database {
    private:
    struct Data {
        std::string Name = "";
        std::string Type = "";
        std::string RA = "";
        std::string Dec = "";
        std::string Const = "";
    };
public:
    Database(std::string db_name) : db(nullptr) {
        int rc = sqlite3_open(db_name.c_str(), &db);
        if (rc != SQLITE_OK) {
            spdlog::error("Cannot open database: {}", sqlite3_errmsg(db));
            sqlite3_close(db);
            throw std::runtime_error("Cannot open database");
        }
    }

    ~Database() {
        sqlite3_close(db);
    }

    void read_from_database() {
        char* errmsg = 0;

        int rc = sqlite3_exec(db, "SELECT * FROM objects;", callback, &data, &errmsg);
        if (rc != SQLITE_OK) {
            spdlog::error("Error reading from database: {}", errmsg);
            sqlite3_free(errmsg);
            throw std::runtime_error("Error reading from database");
        }
    }

    void insert_data(Data d) {
        data.push_back(d);
        std::string sql = "INSERT INTO objects (Name, Type, RA, Dec, Const) VALUES (\"" +
            d.Name + "\", \"" + d.Type + "\", \"" + d.RA + "\", \"" + d.Dec + "\", \"" + d.Const + "\");";

        char* errmsg;
        int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errmsg);
        if (rc != SQLITE_OK) {
            spdlog::error("Error inserting data into database: {}", errmsg);
            sqlite3_free(errmsg);
            throw std::runtime_error("Error inserting data into database");
        }
    }

    void delete_data(std::string name) {
        auto it = std::remove_if(data.begin(), data.end(), [&](const Data& d) {
            return d.Name == name;
        });
        data.erase(it, data.end());

        std::string sql = "DELETE FROM objects WHERE Name = \"" + name + "\";";

        char* errmsg;
        int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errmsg);
        if (rc != SQLITE_OK) {
            spdlog::error("Error deleting data from database: {}", errmsg);
            sqlite3_free(errmsg);
            throw std::runtime_error("Error deleting data from database");
        }
    }

    void sort_by_name() {
        std::sort(data.begin(), data.end(), [](const Data& d1, const Data& d2) {
            return d1.Name < d2.Name;
        });
    }

    std::vector<Data> filter_by(std::function<bool(const Data&)> filter) const {
        std::vector<Data> result;
        std::copy_if(data.begin(), data.end(), std::back_inserter(result), filter);
        return result;
    }

    void optimize_database() {
        sort_by_name();
        // TODO: 实现其他优化
    }

    void save_to_database() {
        std::string sql = "BEGIN TRANSACTION; ";
        for (auto& d : data) {
            sql += "UPDATE objects SET Type = \"" + d.Type +
                "\", RA = \"" + d.RA +
                "\", Dec = \"" + d.Dec +
                "\", Const = \"" + d.Const +
                "\" WHERE Name = \"" + d.Name + "\"; ";
        }
        sql += "COMMIT;";

        char* errmsg;
        int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errmsg);
        if (rc != SQLITE_OK) {
            spdlog::error("Error updating database: {}", errmsg);
            sqlite3_free(errmsg);
            throw std::runtime_error("Error updating database");
        }
    }

    std::vector<Data> search_by_name(std::string name) {
        std::vector<Data> result;
        for (auto& d : data) {
            if (d.Name.find(name) != std::string::npos) {
                result.push_back(d);
            }
        }
        return result;
    }

    std::vector<Data> search_by_ra_dec(std::string ra, std::string dec, double ra_range, double dec_range) {
        std::vector<Data> result;
        for (auto& d : data) {
            double d_ra = to_decimal(d.RA);
            double d_dec = to_decimal(d.Dec);
            double input_ra = to_decimal(ra);
            double input_dec = to_decimal(dec);
            if (std::abs(input_ra - d_ra) <= ra_range && std::abs(input_dec - d_dec) <= dec_range) {
                result.push_back(d);
            }
        }
        return result;
    }

    void save_to_json(std::string filename) {
        json j;
        for (auto d : data) {
            j.push_back({
                {"Name", d.Name},
                {"Type", d.Type},
                {"RA", d.RA},
                {"Dec", d.Dec},
                {"Const", d.Const}
            });
        }
        std::ofstream o(filename);
        o << j.dump(2);
    }

private:
    static int callback(void *pdata, [[maybe_unused]] int argc, char **argv, [[maybe_unused]] char **col_name) {
        std::vector<Data>* d = static_cast<std::vector<Data>*>(data);
        Data tmp;
        tmp.Name = argv[0];
        tmp.Type = argv[1];
        tmp.RA = argv[2];
        tmp.Dec = argv[3];
        tmp.Const = argv[4];
        data->push_back(tmp);

        d->push_back(tmp);
        return 0;
    }

    double to_decimal(std::string str) {
        double degree = 0.0;
        bool has_hour = false, has_minute = false, has_second = false;
        std::string delimiter = ":";

        size_t pos = 0;
        std::string token;
        while ((pos = str.find(delimiter)) != std::string::npos) {
            token = str.substr(0, pos);
            if (!has_hour) {
                degree += std::stod(token) * 15; // 如果是小时，则转换为度数
                has_hour = true;
            } else if (!has_minute) {
                degree += std::stod(token) / 4;
                has_minute = true;
            } else if (!has_second) {
                degree += std::stod(token) / 240;
                has_second = true;
            }
            str.erase(0, pos + delimiter.length());
        }
        if (!has_second) {
            degree += std::stod(str) / 240;
        } else {
            degree += std::stod(str) / 240 / 60;
        }
        return degree;
    }


private:
    sqlite3* db;
    std::vector<Data> data;


};

int main() {
    try {
        Database db("data.db");

        spdlog::info(".");

        // 从数据库中读取数据
        db.read_from_database();

        spdlog::info(".");

        // 根据名字搜索数据
        std::string a = "IC0001";
        auto results = db.search_by_name(a);
        for (auto& d : results) {
            std::cout << d.Name << ", " << d.Type << ", " << d.RA << ", " << d.Dec << ", " << d.Const << std::endl;
        }

        spdlog::info(".");

        // 根据赤经和赤纬搜索数据
        results = db.search_by_ra_dec("06:45:08.9", "-16:42:58", 0.1, 0.1);
        for (auto& d : results) {
            std::cout << d.Name << ", " << d.Type << ", " << d.RA << ", " << d.Dec << ", " << d.Const << std::endl;
        }

        spdlog::info(".");

        // 排序
        db.sort_by_name();

        spdlog::info(".");

        // 保存到json文件
        db.save_to_json("stars.json");

        spdlog::info(".");

        // 更新数据库
        db.save_to_database();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}