/*
 * Module_manager.hpp
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

Description: Module manager to solve the dependency problem.

**************************************************/

#pragma once

#include <string>
#include <vector>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "error/exception.hpp"

namespace Lithium::module
{
    /**
     * @brief This is the class which contains the dependencies relationships of the modules.
     * @note This class will not contains any module's shared_ptr or unique_ptr.
     * @note So, the ModuleManager is not same as the moduleManager.
     */
    class ModuleManager
    {
    public:
        /**
         * @brief Construct a new Module Manager object.
         */
        explicit ModuleManager() = default;

        /**
         * @brief Destroy the Module Manager object.
         */
        ~ModuleManager() = default;

        // -------------------------------------------------------------------
        // Common methods
        // -------------------------------------------------------------------

        static std::shared_ptr<ModuleManager> createShared()
        {
            return std::make_shared<ModuleManager>();
        }

        static std::unique_ptr<ModuleManager> createUnique()
        {
            return std::make_unique<ModuleManager>();
        }

        // -------------------------------------------------------------------
        // Module methods
        // -------------------------------------------------------------------

        /**
         * @brief Add a module to the Module manager.
         * @param name The name of the module.
         * @note The name of the module must be unique.
         * @note The name of the module must be the same as the name of the module in the Module.json.
         */
        void addModule(const std::string &name);

        /**
         * @brief Remove a module from the Module manager.
         * @param name The name of the module.
         * @note If there is no module with the name, this function will do nothing.
         */
        void removeModule(const std::string &name);

        /**
         * @brief Get a module from the Module manager.
         * @param name The name of the module.
         * @return The module.
         * @note If there is no module with the name, this function will return nullptr.
         */
        std::shared_ptr<Module> getModule(const std::string &name);

        /**
         * @brief Modify a module in the Module manager.
         * @param Module The module.
         * @note If there is no module with the name, this function will do nothing.
         */
        void modifyModule(const std::shared_ptr<Module> &Module);

        /**
         * @brief Add a dependency to the module.
         * @param modName The name of the module.
         * @param depName The name of the dependency.
         * @note If there is no module with the name, this function will do nothing.
         */
        void addDependency(const std::string &modName, const std::string &depName);

        /**
         * @brief Resolve the dependencies of the module.
         * @param modName The name of the module.
         * @param resolvedDeps The vector of resolved dependencies.
         * @param missingDeps The vector of missing dependencies.
         * @return True if the dependencies are resolved successfully, otherwise false.
         * @note If there is no module with the name, this function will return false.
         */
        bool resolveDependencies(const std::string &modName, std::vector<std::string> &resolvedDeps, std::vector<std::string> &missingDeps);

    private:
#if ENABLE_FASTHASH
        emhash8::HashMap<std::string, std::shared_ptr<ModuleInfo>> m_modules;
#else
        std::unordered_map<std::string, std::shared_ptr<Module>> m_modules;
#endif

        /**
         * @brief Check the circular dependencies of the module.
         * @param modName The name of the module.
         * @param visited The map of visited modules.
         * @param recursionStack The map of recursion stack.
         * @return True if the dependencies are resolved successfully, otherwise false.
         * @note If there is no module with the name, this function will return false.
         */
        bool checkCircularDependencies(const std::string &modName, std::unordered_map<std::string, bool> &visited, std::unordered_map<std::string, bool> &recursionStack);

        /**
         * @brief Check the missing dependencies of the module.
         * @param modName The name of the module.
         * @param missingDeps The vector of missing dependencies.
         * @return True if the dependencies are resolved successfully, otherwise false.
         * @note If there is no module with the name, this function will return false.
         */
        bool checkMissingDependencies(const std::string &modName, std::vector<std::string> &missingDeps);
    };
}
