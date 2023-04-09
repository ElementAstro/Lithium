/*
 * achievement_list.cpp
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

#include "achievement_list.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <spdlog/spdlog.h>

using namespace std;

namespace OpenAPT::AAchievement
{

    AchievementList::AchievementList() 
    {
        addAstronomyPhotographyAchievements();
    }

    // 默认构造函数
    AchievementList::AchievementList(const std::string &filename)
        : m_filename(filename)
    {
        readFromFile();
    }

    void AchievementList::addAchievement(const std::shared_ptr<Achievement> &achievement)
    {
        m_achievements.emplace_back(achievement);
        spdlog::info("Achievement {} added to ", achievement->getName());
        writeToFile();
    }

    void AchievementList::removeAchievementByName(const std::string &name)
    {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
                               [&](const auto &achievement)
                               {
                                   return achievement->getName() == name;
                               });
        if (it != m_achievements.end())
        {
            m_achievements.erase(it);
            spdlog::info("Achievement {} removed from ", name);
            writeToFile();
        }
    }

    void AchievementList::modifyAchievementByName(const std::string &name, const std::shared_ptr<Achievement> &achievement)
    {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
                               [&](const auto &a)
                               {
                                   return a->getName() == name;
                               });
        if (it != m_achievements.end())
        {
            (*it) = achievement;
            spdlog::info("Achievement {} modified.", name);
            writeToFile();
        }
    }

    bool AchievementList::hasAchievement(const std::string &name) const
    {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
                               [&](const auto &achievement)
                               {
                                   return achievement->getName() == name;
                               });
        return it != m_achievements.end();
    }

    void AchievementList::completeAchievementByName(const std::string &name)
    {
        auto it = std::find_if(m_achievements.begin(), m_achievements.end(),
                               [&](const auto &achievement)
                               {
                                   return achievement->getName() == name;
                               });
        if (it != m_achievements.end())
        {
            (*it)->markAsCompleted();
            spdlog::info("Achievement {} marked as completed.", name);
            writeToFile();
        }
    }

    void AchievementList::printAchievements() const
    {
        spdlog::info("Achievements:");
        for (const auto &achievement : m_achievements)
        {
            std::string status = achievement->isCompleted() ? "Completed" : "Incomplete";
            spdlog::info("\tName: {}, Description: {}, Status: {}", achievement->getName(), achievement->getDescription(), status);
        }
    }

    void AchievementList::writeToFile() const
    {
        json j;
        for (const auto &achievement : m_achievements)
        {
            j.emplace_back(achievement->to_json());
        }

        std::ofstream file(m_filename);
        if (!file.is_open())
        {
            return;
        }

        file << std::setw(4) << j << std::endl;
        spdlog::info("Achievements written to file {}.", m_filename);
    }

    void AchievementList::readFromFile()
    {
        std::ifstream file(m_filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file.");
        }

        json j;
        try
        {
            file >> j;
        }
        catch (...)
        {
            throw std::runtime_error("Failed to parse JSON file.");
        }

        for (const auto &item : j)
        {
            m_achievements.emplace_back(Achievement::from_json(item));
        }

        spdlog::info("Achievements read from file {}.", m_filename);
    }

}