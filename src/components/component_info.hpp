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

#include "atom/components/types.hpp"

#include "atom/type/json.hpp"

using json = nlohmann::json;

struct _ComponentMain
{
    std::string m_component_name;
    std::string m_func_name;
    std::string m_component_type;
};

struct _ComponentInfo
{
    // Basic Info
    std::string m_name;        /**< Package name */
    std::string m_version;     /**< Package version */
    std::string m_description; /**< Package description */
    std::string m_author;      /**< Package author */
    std::string m_license;     /**< Package license */
    std::string m_type;        /**< Package type */

    // Other Info
    std::string m_repository_url;                              /**< Package repository */
    std::string m_repository_type;                             /**< Package repository type, such as git */
    std::string m_homepage;                                    /**< Package homepage */
    std::string m_bugs_url;                                    /**< Package bugs */
    std::vector<std::string> m_keywords;                       /**< Package keywords */

    // Scripts Info
    std::unordered_map<std::string, std::string> scripts;      /**< Scripts section */
    std::unordered_map<std::string, std::string> dependencies; /**< Regular dependencies */

    // Main
    std::unordered_map<std::string, _ComponentMain> main;
};

/**
 * @brief The ComponentInfo class provides functionality to load and save package.json files,
 * as well as access and modify package information.
 */
class ComponentInfo
{
public:
    /**
     * @brief Constructs a ComponentInfo object with the specified filename.
     * @param filename The name of the package.json file.
     * @note When you create this object, the package.json file will not be loaded.
     * @note You need to call loadPackageJson() to load the package.json file. This is for performance reasons.
     */
    ComponentInfo(const std::string &filename);

    /**
     * @brief Destroys the ComponentInfo object.
     */
    ~ComponentInfo();

    /**
     * @brief Loads the package.json file.
     */
    void loadPackageJson();

    /**
     * @brief Saves the package.json file.
     */
    void savePackageJson();

    /**
     * @brief Checks if the package.json file is loaded.
     * @return True if the package.json file is loaded, false otherwise.
     */
    bool isLoaded() const;

    /**
     * @brief Gets the package.json file.
     * @return The package.json file.
     */
    json getPackageJson() const;

    /**
     * @brief Converts the package.json data to a _Comp struct.
     * @return The package.json data as a _Comp struct.
     */
    _ComponentInfo toStruct() const;

private:
    std::string filename_; /**< The filename of the package.json file. */
    json package_;         /**< The parsed package.json data. */

    bool need_save_{true};  /**< Whether the package.json file needs to be saved. */
    bool is_loaded_{false}; /**< Whether the package.json file has been loaded. */
};

/*
int main()
{
    ComponentInfo manager("package.json");
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
