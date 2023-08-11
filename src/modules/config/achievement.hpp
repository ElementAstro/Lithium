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

#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include <string>
#include "nlohmann/json_fwd.hpp"

using json = nlohmann::json;

namespace Lithium::AAchievement
{
    // 成就类
    /**
     * @brief 表示一个成就的类。
     *
     * 该类用于存储成就的名称、描述以及完成状态，并提供相关操作方法。
     */
    class Achievement
    {
    public:
        /**
         * @brief 默认构造函数。
         *
         * 默认构造函数创建一个空的成就对象。
         */
        Achievement();

        /**
         * @brief 构造函数。
         *
         * @param name        成就名称
         * @param description 成就描述
         */
        Achievement(const std::string &name, const std::string &description);

        /**
         * @brief 获取成就名称。
         *
         * @return 成就名称
         */
        std::string getName() const;

        /**
         * @brief 获取成就描述。
         *
         * @return 成就描述
         */
        std::string getDescription() const;

        /**
         * @brief 判断成就是否已完成。
         *
         * @return 如果成就已完成，则返回 true，否则返回 false
         */
        bool isCompleted() const;

        /**
         * @brief 标记成就为已完成状态。
         *
         * 将成就的完成状态标记为已完成。
         */
        void markAsCompleted();

        /**
         * @brief 将成就转换为 JSON 格式。
         *
         * @return 表示成就的 JSON 对象
         */
        json to_json() const;

        /**
         * @brief 从 JSON 数据中创建成就。
         *
         * 根据传入的 JSON 数据创建一个成就对象。
         *
         * @param j 包含成就信息的 JSON 对象
         * @return 指向新创建成就对象的共享指针
         */
        static std::shared_ptr<Achievement> from_json(const json &j);

    private:
        std::string m_name;        ///< 成就名称
        std::string m_description; ///< 成就描述
        bool m_isCompleted;        ///< 是否已完成
    };

}

#endif // ACHIEVEMENT_H
