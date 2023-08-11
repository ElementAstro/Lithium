/*
 * achievement.cpp
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

Date: 2023-4-9

Description: Achiement

**************************************************/

#include "achievement.hpp"
#include <memory>
#include "nlohmann/json.hpp"

namespace Lithium::AAchievement
{
    // 默认构造函数
    Achievement::Achievement()
        : m_name(""), m_description(""), m_isCompleted(false) {}

    // 构造函数
    Achievement::Achievement(const std::string &name, const std::string &description)
        : m_name(name), m_description(description), m_isCompleted(false) {}

    // 获取成就名称
    std::string Achievement::getName() const
    {
        return m_name;
    }

    // 获取成就描述
    std::string Achievement::getDescription() const
    {
        return m_description;
    }

    // 判断成就是否已完成
    bool Achievement::isCompleted() const
    {
        return m_isCompleted;
    }

    // 标记成就为已完成
    void Achievement::markAsCompleted()
    {
        m_isCompleted = true;
    }

    // 将成就转换为 JSON 格式
    json Achievement::to_json() const
    {
        json j;
        j["name"] = m_name;
        j["description"] = m_description;
        j["isCompleted"] = m_isCompleted;
        return j;
    }

    // 从 JSON 数据中创建成就
    std::shared_ptr<Achievement> Achievement::from_json(const json &j)
    {
        std::string name = j["name"];
        std::string description = j["description"];
        bool isCompleted = j["isCompleted"];
        auto achievement = std::make_shared<Achievement>(name, description);
        if (isCompleted)
        {
            achievement->markAsCompleted();
        }
        return achievement;
    }
}