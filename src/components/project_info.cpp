/*
 * project_info.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-8-6

Description: Project Infomation, such as the name, the build command, the dependencies, etc.

**************************************************/

#include "project_info.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace Lithium
{
    Project::Project(const std::string &name, const std::string &buildCommand)
        : m_name(name), m_buildCommand(buildCommand), m_lastBuildStatus("NotBuild") {}

    std::string Project::getName() const
    {
        return m_name;
    }

    std::string Project::getBuildCommand() const
    {
        return m_buildCommand;
    }

    std::string Project::getLastBuildStatus() const
    {
        return m_lastBuildStatus;
    }

    std::time_t Project::getLastBuildTime() const
    {
        return m_lastBuildTime;
    }

    void Project::setLastBuildStatus(const std::string &status)
    {
        m_lastBuildStatus = status;
    }

    void Project::setLastBuildTime(std::time_t time)
    {
        m_lastBuildTime = time;
    }

    GitProject::GitProject()
    {
        if (!fs::exists(GIT_DIR))
        {
            fs::create_directory(GIT_DIR);
            DLOG_F(INFO, "Initialized empty Git repository in {}", GIT_DIR);
        }
        else
        {
            LOG_F(WARNING, "Git repository already exists in {}. Please remove it manually.", GIT_DIR);
            if (!fs::is_empty(GIT_DIR))
            {
                LOG_F(WARNING, "Git repository is not empty. Please remove it manually.")
                return;
            }
        }
    }

    bool GitProject::add(const std::vector<std::string> &files)
    {
        bool success = true;
        for (const auto &file : files)
        {
            try
            {
                fs::copy(file, GIT_DIR + "/" + file);
            }
            catch (...)
            {
                success = false;
            }
        }
        return success;
    }

    bool GitProject::remove(const std::vector<std::string> &files)
    {
        bool success = true;
        for (const auto &file : files)
        {
            try
            {
                fs::remove(GIT_DIR + "/" + file);
            }
            catch (const std::filesystem::e)
            {
                success = false;
            }
        }
        return success;
    }

    bool GitProject::commit(const std::string &message)
    {
        std::ofstream commitFile(GIT_DIR + "/commit.txt");
        commitFile << message;
        commitFile.close();
        return true;
    }

    std::vector<std::string> GitProject::status()
    {
        std::vector<std::string> addedFiles;
        for (const auto &entry : fs::directory_iterator(GIT_DIR))
        {
            if (!fs::is_directory(entry))
            {
                addedFiles.push_back(entry.path().filename());
            }
        }
        std::sort(addedFiles.begin(), addedFiles.end());
        return addedFiles;
    }

    std::vector<std::string> GitProject::diff()
    {
        std::vector<std::string> changedFiles;
        for (const auto &entry : fs::directory_iterator(GIT_DIR))
        {
            if (!fs::is_directory(entry))
            {
                std::ifstream oldFile(entry.path());
                std::ifstream newFile(GIT_DIR + "/" + entry.path().filename().string());
                if (oldFile && newFile)
                {
                    std::stringstream oldBuffer;
                    std::stringstream newBuffer;
                    oldBuffer << oldFile.rdbuf();
                    newBuffer << newFile.rdbuf();
                    std::string oldContent = oldBuffer.str();
                    std::string newContent = newBuffer.str();
                    if (oldContent != newContent)
                    {
                        changedFiles.push_back(entry.path().filename());
                    }
                }
            }
        }
        std::sort(changedFiles.begin(), changedFiles.end());
        return changedFiles;
    }

    bool GitProject::checkout(const std::string &file)
    {
        if (fs::exists(GIT_DIR + "/" + file))
        {
            try
            {
                fs::copy(GIT_DIR + "/" + file, file, fs::copy_options::overwrite_existing);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
        return false;
    }

    std::string GitProject::show(const std::string &file)
    {
        std::ifstream fileStream(GIT_DIR + "/" + file);
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();
    }

} // namespace Lithium
