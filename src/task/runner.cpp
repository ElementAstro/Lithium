/*
 * runner.hpp
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
 
Date: 2023-3-29
 
Description: Task Runner
 
**************************************************/

#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
#include <spdlog/spdlog.h>

/**
 * @brief 检查 JSON 文件是否格式正确
 * 
 * @param filename JSON 文件名
 * @return true JSON 格式正确
 * @return false JSON 格式错误或文件无法打开
 */
bool check_json(const std::string& filename) {
    // 打开 JSON 文件
    std::ifstream fin(filename);
    if (!fin) {
        spdlog::error("Failed to open {}", filename);
        return false;
    }
    // 读取 JSON 数据
    nlohmann::json j;
    try {
        fin >> j;
    } catch (nlohmann::json::parse_error& e) {
        spdlog::error("JSON Format error : {}", e.what());
        return false;
    }
    fin.close();
    spdlog::info("{} passed check", filename);
    return true;
}
