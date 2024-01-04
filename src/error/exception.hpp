/*
 * exception.hpp
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

Date: 2024-1-4

Description: All of the exceptions in the project

**************************************************/

#pragma once

#include <stdexcept>

namespace Lithium
{
    class PackageDependencyException : public std::runtime_error
    {
    public:
        PackageDependencyException(const std::string& package_name, const std::string& dependency_name)
            : std::runtime_error("Package " + package_name + " depends on " + dependency_name)
        {
        }
    };
}