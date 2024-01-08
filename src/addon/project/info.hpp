/*
 * project_info.hpp
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

#pragma once

#include <string>
#include <ctime>

namespace Lithium
{
    /**
     * @brief This is the class which contains the information of the project.
     */
    class Project
    {
    public:
        /**
         * @brief Construct a new Project object.
         * @param name The name of the project.
         * @param buildCommand The build command of the project.
         */
        Project(const std::string &name, const std::string &buildCommand);

        /**
         * @brief Destroy the Project object.
         */
        ~Project() = default;

        /**
         * @brief Get the name of the project.
         * @return The name of the project.
         */
        std::string getName() const;

        /**
         * @brief Get the build command of the project.
         * @return The build command of the project.
         */
        std::string getBuildCommand() const;

        /**
         * @brief Get the last build status of the project.
         * @return The last build status of the project.
         */
        std::string getLastBuildStatus() const;

        /**
         * @brief Get the last build time of the project.
         * @return The last build time of the project.
         */
        std::time_t getLastBuildTime() const;

        /**
         * @brief Set the last build status of the project.
         * @param status The last build status of the project.
         */
        void setLastBuildStatus(const std::string &status);
        /**
         * @brief Set the last build time of the project.
         * @param time The last build time of the project.
         */
        void setLastBuildTime(std::time_t time);

    private:
        std::string m_name;            // The name of the project.
        std::string m_buildCommand;    // The build command of the project.
        std::string m_lastBuildStatus; // The last build status of the project.
        std::time_t m_lastBuildTime;   // The last build time of the project.
    };

    /**
     * @brief The GitProject class represents a simple version control system for a project.
     */
    class GitProject
    {
    public:
        /**
         * @brief GitProject constructor initializes the Git repository if it doesn't already exist.
         */
        GitProject();

        /**
         * @brief add adds the specified files to the Git repository.
         *
         * @param files a vector of file paths to add.
         * @return true if all files were added successfully, false otherwise.
         */
        bool add(const std::vector<std::string> &files);

        /**
         * @brief remove removes the specified files from the Git repository.
         *
         * @param files a vector of file paths to remove.
         * @return true if all files were removed successfully, false otherwise.
         */
        bool remove(const std::vector<std::string> &files);

        /**
         * @brief commit creates a new commit with the specified commit message.
         *
         * @param message the commit message.
         * @return true if the commit was created successfully, false otherwise.
         */
        bool commit(const std::string &message);

        /**
         * @brief status returns a vector of file paths that have been added to the Git repository.
         *
         * @return a vector of file paths.
         */
        std::vector<std::string> status();

        /**
         * @brief diff returns a vector of file paths that have been changed since the last commit.
         *
         * @return a vector of file paths.
         */
        std::vector<std::string> diff();

        /**
         * @brief checkout replaces the contents of a file with the version in the Git repository.
         *
         * @param file the file path to checkout.
         * @return true if the file was checked out successfully, false otherwise.
         */
        bool checkout(const std::string &file);

        /**
         * @brief show returns the contents of a file in the Git repository.
         *
         * @param file the file path to show.
         * @return the contents of the file as a string.
         */
        std::string show(const std::string &file);

    private:
        const std::string GIT_DIR = ".mygit"; /**< The name of the Git repository directory. */
    };

} // namespace Lithium
