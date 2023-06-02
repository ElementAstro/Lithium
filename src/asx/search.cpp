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

Date: 2023-5-25

Description: Astro Serch

**************************************************/

#include "search.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <span>

#include <spdlog/spdlog.h>

namespace OpenAPT::ASX
{
    // 读取数据文件，返回包含 Data 的 vector
    std::vector<Data> ReadFromJson(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file)
        {
            spdlog::error("Error opening file for reading.");
            throw std::runtime_error("Error opening file for reading.");
        }

        json j;
        file >> j;

        std::vector<Data> data;
        for (const auto &element : j)
        {
            data.push_back(element.get<Data>());
        }

        return data;
    }

    // 将 Data 写入到文件中
    void WriteToJson(const std::vector<Data> &data, const std::string &filename)
    {
        json j = data;

        std::ofstream out_file(filename);
        if (!out_file)
        {
            spdlog::error("Error opening file for writing.");
            throw std::runtime_error("Error opening file for writing.");
        }

        out_file << std::setw(4) << j;
    }

    // 向 vector 中插入一个 Data 对象
    void InsertData(std::vector<Data> &data, const Data &d)
    {
        data.push_back(d);
    }

    // 根据名称从 vector 中删除 Data 对象
    void DeleteData(std::vector<Data> &data, const std::string &name)
    {
        data.erase(std::remove_if(data.begin(), data.end(), [&name](const Data &d)
                                  { return d.Name == name; }),
                   data.end());
    }

    // 根据名称排序 vector 中 Data 对象
    void SortByName(std::vector<Data> &data)
    {
        std::ranges::sort(data, {}, &Data::Name);
    }

    // 根据自定义过滤器对 vector 中 Data 对象进行过滤，并返回过滤后的结果
    std::vector<Data> FilterBy(const std::vector<Data> &data, const auto &filter)
    {
        std::vector<Data> result;

        std::ranges::copy_if(data, std::back_inserter(result), filter);

        return result;
    }

    // 根据名称对 vector 中 Data 对象进行模糊搜索，并返回搜索结果
    std::vector<Data> SearchByName(const std::vector<Data> &data, const std::string &name)
    {
        return FilterBy(data, [&name](const Data &d)
                        { return d.Name.find(name) != std::string::npos; });
    }

    // 将度分秒形式的坐标转换为十进制形式
    double ToDecimal(const std::string &str)
    {
        double deg, min, sec;
        sscanf(str.c_str(), "%lf:%lf:%lf", &deg, &min, &sec);
        deg += min / 60 + sec / 3600;
        return deg;
    }

    // 根据坐标范围对 vector 中 Data 对象进行搜索，并返回搜索结果
    std::vector<Data> SearchByRaDec(const std::vector<Data> &data, const std::string &ra, const std::string &dec, const double ra_range, const double dec_range)
    {
        double target_ra = ToDecimal(ra);
        double target_dec = ToDecimal(dec);

        auto within_range = [target_ra, target_dec, ra_range, dec_range](const Data &d) -> bool
        {
            double d_ra = ToDecimal(d.RA);
            double d_dec = ToDecimal(d.Dec);

            return std::abs(d_ra - target_ra) <= ra_range && std::abs(d_dec - target_dec) <= dec_range;
        };

        return FilterBy(data, within_range);
    }

    // 根据 Const 对象进行搜索，并返回搜索结果
    std::vector<Data> SearchByConst(const std::vector<Data> &data, const Constellation &constellation)
    {
        auto by_const = [&constellation](const Data &d) -> bool
        {
            return d.Const == constellation;
        };

        return FilterBy(data, by_const);
    }

    // 统计 vector 中 Data 对象的数量，并返回数量
    size_t CountData(const std::vector<Data> &data)
    {
        return data.size();
    }

    // 根据名称查找 vector 中 Data 对象，并返回对应对象的指针
    Data *FindByName(std::vector<Data> &data, const std::string &name)
    {
        auto it = std::ranges::find_if(data, [&name](const Data &d)
                                       { return d.Name == name; });

        if (it != data.end())
        {
            return &(*it);
        }

        return nullptr;
    }

    // 使用函数式编程，将 vector 中 Data 对象进行转换，并返回转换后的结果
    std::vector<std::pair<std::string, std::string>> Transform(const std::vector<Data> &data, const auto &transform)
    {
        std::vector<std::pair<std::string, std::string>> result;

        std::ranges::transform(data, std::back_inserter(result), transform);

        return result;
    }

    // 使用函数式编程，将 vector 中 Data 对象进行累加，并返回累加后的结果
    double Accumulate(const std::vector<Data> &data, const double initial_value, const auto &accumulate)
    {
        return std::ranges::accumulate(data, initial_value, accumulate);
    }
}

/*
int main() {
    std::vector<Data> data = ReadFromJson("data.json");

    // 插入一条数据
    Data d;
    d.Id = 10001;
    d.Name = "NGC 2264";
    d.Type = "Open Cluster";
    d.RA = "06:41:08.9";
    d.Dec = "+09:53:00";
    d.Const = "Monoceros";
    InsertData(data, d);

    // 根据名字删除一条数据
    DeleteData(data, "Pleiades");

    // 按名字排序
    SortByName(data);

    // 根据名字查找数据
    auto result = SearchByName(data, "Andromeda");

    // 根据赤经和赤纬以及范围查找数据
    auto result2 = SearchByRaDec(data, "05:22:00", "+45:00:00", 0.5, 0.5);

    // 将修改后的数据写回文件
    WriteToJson(data, "data.json");

    return 0;
}

[
    {
        "Id": 10001,
        "Name": "Orion Nebula",
        "Type": "Nebula",
        "RA": "05:35:17.3",
        "Dec": "-05:23:28",
        "Const": "Orion"
    },
    {
        "Id": 10002,
        "Name": "Pleiades",
        "Type": "Open Cluster",
        "RA": "03:47:24",
        "Dec": "+24:07:00",
        "Const": "Taurus"
    },
    {
        "Id": 10003,
        "Name": "Andromeda Galaxy",
        "Type": "Galaxy",
        "RA": "00:42:44.4",
        "Dec": "+41:16:09",
        "Const": "Andromeda"
    }
]

*/
