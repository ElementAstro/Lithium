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

#include "loguru/loguru.hpp"

using namespace std;

namespace Lithium::AAchievement
{

    AchievementList::AchievementList() : AchievementList("achievements.json")
    {
        addAstronomyPhotographyAchievements();
    }

    AchievementList::AchievementList(const std::string &filename)
        : m_filename{filename}, m_achievements{}
    {
        readFromFile();
    }

    void AchievementList::addAchievement(const std::shared_ptr<Achievement> &achievement)
    {
        m_achievements.emplace_back(achievement);
        LOG_F(INFO, "Achievement %s added to", achievement->getName().c_str());
        writeToFile();
    }

    void AchievementList::removeAchievementByName(const std::string &name)
    {
        const auto it = std::find_if(m_achievements.begin(), m_achievements.end(), [&](const auto &achievement)
                                     { return achievement->getName() == name; });
        if (it != m_achievements.end())
        {
            m_achievements.erase(it);
            LOG_F(INFO, "Achievement %s removed from", name.c_str());
            writeToFile();
        }
    }

    void AchievementList::modifyAchievementByName(const std::string &name, const std::shared_ptr<Achievement> &achievement)
    {
        const auto it = std::find_if(m_achievements.begin(), m_achievements.end(), [&](const auto &a)
                                     { return a->getName() == name; });
        if (it != m_achievements.end())
        {
            *it = achievement;
            LOG_F(INFO, "Achievement %s modified.", name.c_str());
            writeToFile();
        }
    }

    bool AchievementList::hasAchievement(const std::string &name) const
    {
        const auto it = std::find_if(m_achievements.begin(), m_achievements.end(), [&](const auto &achievement)
                                     { return achievement->getName() == name; });
        return it != m_achievements.end();
    }

    void AchievementList::completeAchievementByName(const std::string &name)
    {
        const auto it = std::find_if(m_achievements.begin(), m_achievements.end(), [&](const auto &achievement)
                                     { return achievement->getName() == name; });
        if (it != m_achievements.end())
        {
            (*it)->markAsCompleted();
            LOG_F(INFO, "Achievement %s marked as completed.", name.c_str());
            writeToFile();
        }
    }

    void AchievementList::printAchievements() const
    {
        for (const auto &achievement : m_achievements)
        {
            const auto status = achievement->isCompleted() ? "Completed" : "Incomplete";
            LOG_F(INFO, "  Name: %s, Description: %s, Status: %s", achievement->getName().c_str(), achievement->getDescription().c_str(), status);
        }
    }

    void AchievementList::writeToFile() const
    {
        json j;
        for (const auto &achievement : m_achievements)
        {
            j.emplace_back(achievement->to_json());
        }

        std::ofstream file{m_filename};
        if (!file)
        {
            return;
        }

        file << std::setw(4) << j << std::endl;
        LOG_F(INFO, "Achievements written to file %s.", m_filename.c_str());
    }

    void AchievementList::readFromFile()
    {
        std::ifstream file{m_filename};
        if (!file)
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

        m_achievements.reserve(j.size());
        std::transform(begin(j), end(j), std::back_inserter(m_achievements), Achievement::from_json);

        LOG_F(INFO, "Achievements read from file %s.", m_filename.c_str());
    }

    void AchievementList::addAstronomyPhotographyAchievements()
    {
        // 添加一些天文摄影成就
    }

}