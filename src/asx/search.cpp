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
    struct ObjectData {
        std::string name = "";
        std::string type = "";
        std::string right_ascension = "";
        std::string declination = "";
        std::string constellation = "";
    };

public:
    explicit Database(const std::string& db_name) : db_(nullptr) {
        int rc = sqlite3_open(db_name.c_str(), &db_);
        if (rc != SQLITE_OK) {
            spdlog::error("Cannot open database: {}", sqlite3_errmsg(db_));
            sqlite3_close(db_);
            throw std::runtime_error("Cannot open database");
        }
    }

    ~Database() {
        sqlite3_close(db_);
    }

    void readObjectsFromDatabase() {
        char* errmsg = nullptr;
        int rc = sqlite3_exec(db_, "SELECT * FROM objects;", callback, &objects_data_, &errmsg);
        if (rc != SQLITE_OK) {
            spdlog::error("Error reading objects from database: {}", errmsg);
            sqlite3_free(errmsg);
            throw std::runtime_error("Error reading objects from database");
        } 
    }

    void insertObject(const ObjectData& data) {
        objects_data_.push_back(data);
        std::string sql = "INSERT INTO objects (Name, Type, RA, Dec, Constellation) VALUES (\"" +
            data.name + "\", \"" + data.type + "\", \"" + data.right_ascension + "\", \"" +
            data.declination + "\", \"" + data.constellation + "\");";

        char* errmsg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK) {
            spdlog::error("Error inserting object into database: {}", errmsg);
            sqlite3_free(errmsg);
            objects_data_.pop_back();
            throw std::runtime_error("Error inserting object into database");
        }
    }

    void deleteObject(const std::string& name) {
        auto it = std::remove_if(objects_data_.begin(), objects_data_.end(), [&](const ObjectData& data) {
            return data.name == name;
        });
        objects_data_.erase(it, objects_data_.end());

        std::string sql = "DELETE FROM objects WHERE Name = \"" + name + "\";";

        char* errmsg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK) {
            spdlog::error("Error deleting object from database: {}", errmsg);
            sqlite3_free(errmsg);
            throw std::runtime_error("Error deleting object from database");
        }
    }

    void sortByObjectName() {
        std::sort(objects_data_.begin(), objects_data_.end(), [](const ObjectData& d1, const ObjectData& d2) {
            return d1.name < d2.name;
        });
    }

    std::vector<ObjectData> filterObjectsBy(std::function<bool(const ObjectData&)> filter) const {
        std::vector<ObjectData> result;
        std::copy_if(objects_data_.begin(), objects_data_.end(), std::back_inserter(result), filter);
        return result;
    }

    void optimizeDatabase() {
        sortByObjectName();
        // TODO: Implement other optimization methods
    }

    void saveObjectsToDatabase() {
        std::string sql = "BEGIN TRANSACTION; ";
        for (const auto& data : objects_data_) {
            sql += "UPDATE objects SET Type = \"" + data.type +
                "\", RA = \"" + data.right_ascension +
                "\", Dec = \"" + data.declination +
                "\", Constellation = \"" + data.constellation +
                "\" WHERE Name = \"" + data.name + "\"; ";
        }
        sql += "COMMIT;";

        char* errmsg = nullptr;
        int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK) {
            spdlog::error("Error updating database: {}", errmsg);
            sqlite3_free(errmsg);
            throw std::runtime_error("Error updating database");
        }
    }

    std::vector<ObjectData> searchObjectsByName(const std::string& name) const {
        std::vector<ObjectData> result;
        for (const auto& data : objects_data_) {
            if (data.name.find(name) != std::string::npos) {
                result.push_back(data);
            }
        }
        return result;
    }

    std::vector<ObjectData> searchObjectsByRaDec(const std::string& ra, const std::string& dec, double ra_range, double dec_range) const {
        std::vector<ObjectData> result;
        for (const auto& data : objects_data_) {
            double object_ra = toDecimal(data.right_ascension);
            double object_dec = toDecimal(data.declination);
            double input_ra = toDecimal(ra);
            double input_dec = toDecimal(dec);
            if (std::abs(input_ra - object_ra) <= ra_range && std::abs(input_dec - object_dec) <= dec_range) {
                result.push_back(data);
            }
        }
        return result;
    }

    void saveObjectsToJsonFile(const std::string& filename) const {
        json j;
        for (const auto& data : objects_data_) {
            j.push_back({
                {"Name", data.name},
                {"Type", data.type},
                {"RA", data.right_ascension},
                {"Dec", data.declination},
                {"Constellation", data.constellation}
            });
        }
        std::ofstream o(filename);
        o << j.dump(2);
    }

private:
    static int callback(void *pdata, [[maybe_unused]] int argc, char **argv, [[maybe_unused]] char **col_name) {
        auto* objects_data = static_cast<std::vector<ObjectData>*>(pdata);
        ObjectData tmp;
        tmp.name = argv[0];
        tmp.type = argv[1];
        tmp.right_ascension = argv[2];
        tmp.declination = argv[3];
        tmp.constellation = argv[4];
        objects_data->push_back(tmp);
        return 0;
    }

    double toDecimal(const std::string& str) const {
        double degree = 0.0;
        bool has_hour = false, has_minute = false, has_second = false;
        std::string delimiter = ":";

        size_t pos = 0;
        std::string token;
        while ((pos = str.find(delimiter)) != std::string::npos) {
            token = str.substr(0, pos);
            if (!has_hour) {
                degree += std::stod(token) * 15; // If it's hours, convert to degrees
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
    sqlite3* db_;
    std::vector<ObjectData> objects_data_;
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