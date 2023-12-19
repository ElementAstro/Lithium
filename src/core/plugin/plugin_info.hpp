/*
 * plugin_info.hpp
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

Date: 2023-8-6

Description: Basic Plugin Infomation

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "atom/type/json.hpp"

using json = nlohmann::json;

struct _Scripts
{
    std::string dev;   /**< Development script */
    std::string build; /**< Build script */
    std::string start; /**< Start script */
    std::string lint;  /**< Lint script */
};

struct _Dependencies
{
    std::unordered_map<std::string, std::string> regular; /**< Regular dependencies */
    std::unordered_map<std::string, std::string> dev;     /**< Development dependencies */
};

struct _PackageJson
{
    std::string name;              /**< Package name */
    std::string version;           /**< Package version */
    bool isPrivate;                /**< Is the package private? */
    _Scripts scripts;              /**< Scripts section */
    _Dependencies dependencies;    /**< Regular dependencies */
    _Dependencies devDependencies; /**< Development dependencies */
};

/**
 * @brief The PackageInfo class provides functionality to load and save package.json files,
 * as well as access and modify package information.
 */
class PackageInfo
{
public:
    /**
     * @brief Constructs a PackageInfo object with the specified filename.
     * @param filename The name of the package.json file.
     */
    PackageInfo(const std::string &filename);

    /**
     * @brief Loads the package.json file.
     */
    void loadPackageJson();

    /**
     * @brief Saves the package.json file.
     */
    void savePackageJson();

    /**
     * @brief Gets the name of the package.
     * @return The name of the package.
     */
    std::string getName() const;

    /**
     * @brief Gets the version of the package.
     * @return The version of the package.
     */
    std::string getVersion() const;

    /**
     * @brief Checks if the package is private.
     * @return True if the package is private, false otherwise.
     */
    bool isPrivate() const;

    /**
     * @brief Sets the name of the package.
     * @param name The name of the package.
     */
    void setName(const std::string &name);

    /**
     * @brief Sets the version of the package.
     * @param version The version of the package.
     */
    void setVersion(const std::string &version);

    /**
     * @brief Sets whether the package is private.
     * @param isPrivate True if the package is private, false otherwise.
     */
    void setIsPrivate(bool isPrivate);

    /**
     * @brief Converts the package.json data to a _PackageJson struct.
     * @return The package.json data as a _PackageJson struct.
     */
    _PackageJson toStruct() const;

private:
    std::string filename_; /**< The filename of the package.json file. */
    json package_;         /**< The parsed package.json data. */
};

/*
int main()
{
    PackageInfo manager("package.json");
    manager.loadPackageJson();

    std::cout << "Name: " << manager.getName() << std::endl;
    std::cout << "Version: " << manager.getVersion() << std::endl;
    std::cout << "Private: " << std::boolalpha << manager.isPrivate() << std::endl;

    manager.setName("cobalt-web");
    manager.setVersion("0.1.0");
    manager.setIsPrivate(true);

    manager.savePackageJson();

    return 0;
}

*/
