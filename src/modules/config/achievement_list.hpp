/*
 * achievement_list.hpp
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

Description: Achiement List

**************************************************/

#ifndef ACHIEVEMENT_LIST_H
#define ACHIEVEMENT_LIST_H

#include <vector>
#include <memory>
#include "nlohmann/json_fwd.hpp"

#include "achievement.hpp"

using json = nlohmann::json;

namespace Lithium::AAchievement
{

    /**
     * @brief 成就列表类，允许添加、移除、修改和查询成就，以及将其写入或从文件中读取。
     */
    class AchievementList
    {
    public:
        /**
         * @brief 默认构造函数，会调用带文件名参数的构造函数。
         */
        AchievementList();

        /**
         * @brief 带文件名参数的构造函数，从文件中读取成就列表。
         *
         * @param filename 持久化文件的文件名。
         */
        explicit AchievementList(const std::string &filename);

        /**
         * @brief 添加一个成就到成就列表中。
         *
         * @param achievement 要添加的成就指针。
         */
        void addAchievement(const std::shared_ptr<Achievement> &achievement);

        /**
         * @brief 根据成就名称从成就列表中移除一个成就。
         *
         * @param name 要移除的成就名称。
         */
        void removeAchievementByName(const std::string &name);

        /**
         * @brief 根据成就名称修改一个成就。
         *
         * @param name 要修改的成就名称。
         * @param achievement 修改后的成就指针。
         */
        void modifyAchievementByName(const std::string &name, const std::shared_ptr<Achievement> &achievement);

        /**
         * @brief 判断是否存在某个名称的成就。
         *
         * @param name 要查询的成就名称。
         * @return 存在则返回 true，否则返回 false。
         */
        bool hasAchievement(const std::string &name) const;

        /**
         * @brief 根据成就名称将一个成就标记为已完成。
         *
         * @param name 要标记为已完成的成就名称。
         */
        void completeAchievementByName(const std::string &name);

        /**
         * @brief 打印所有成就的名称、描述和状态（已完成或未完成）。
         */
        void printAchievements() const;

    private:
        std::string m_filename;                                   // 持久化文件的文件名
        std::vector<std::shared_ptr<Achievement>> m_achievements; // 成就列表

        /**
         * @brief 添加一些天文摄影成就到列表中。
         */
        void addAstronomyPhotographyAchievements();

        /**
         * @brief 将成就列表写入持久化文件中。
         */
        void writeToFile() const;

        /**
         * @brief 从持久化文件中读取成就列表。
         */
        void readFromFile();
    };

}
#endif // ACHIEVEMENT_LIST_H