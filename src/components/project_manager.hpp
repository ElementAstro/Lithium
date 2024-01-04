/*
 * project_manager.hpp
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

Description: Project Manager not only the cmake project, but also the git project.

**************************************************/

#pragma once

#include "project_info.hpp"
#include <vector>
#include <string>
#include <memory>

namespace Lithium
{
    /**
     * @brief The ProjectManager class manages a collection of projects.
     */
    class ProjectManager
    {
    public:
        /**
         * @brief addProject adds a project to the project manager.
         *
         * @param project a shared pointer to the project to add.
         * @return true if the project was added successfully, false otherwise.
         */
        bool addProject(const std::shared_ptr<Project> &project);

        /**
         * @brief removeProject removes a project from the project manager.
         *
         * @param name the name of the project to remove.
         * @return true if the project was removed successfully, false otherwise.
         */
        bool removeProject(const std::string &name);

        /**
         * @brief listProjects displays a list of all projects in the project manager.
         */
        void listProjects() const;

        /**
         * @brief buildProject builds a project with the specified name.
         *
         * @param name the name of the project to build.
         * @return a pair where the first element is true if the project was built successfully, false otherwise,
         *         and the second element is an error message if there was an error during the build.
         */
        std::pair<bool, std::string> buildProject(const std::string &name);

        /**
         * @brief showProjectDetails displays the details of a project with the specified name.
         *
         * @param name the name of the project to show details for.
         */
        void showProjectDetails(const std::string &name) const;

    private:
        std::vector<std::shared_ptr<Project>> m_projects; /**< The collection of projects managed by the project manager. */
    };

} // namespace Lithium
