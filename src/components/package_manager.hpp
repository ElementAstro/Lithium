/*
 * package_manager.hpp
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

Description: Package manager to solve the dependency problem.

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

namespace Lithium
{

    /**
     * @brief This is the class which contains the dependencies relationships of the components.
     * @note This class will not contains any component's shared_ptr or unique_ptr.
     */
    class Package
    {
    public:
        /**
         * @brief Construct a new Package object.
         * @param modName The name of the component.
         */
        explicit Package(const std::string &modName) : name(modName) {}

        /**
         * @brief Destroy the Package object.
         */
        ~Package() = default;

        /**
         * @brief Add a dependency to the component.
         * @param dep The name of the dependency.
         */
        void addDependency(const std::string &dep)
        {
            m_dependencies.push_back(dep);
        }

        /**
         * @brief Add a vector of dependencies to the component.
         * @param deps The vector of dependencies.
         */
        void addDependencies(const std::vector<std::string> &deps)
        {
            m_dependencies.insert(m_dependencies.end(), deps.begin(), deps.end());
        }

        /**
         * @brief Get the dependencies of the component.
         * @return The vector of dependencies.
         */
        std::vector<std::string> getDependencies() const
        {
            return m_dependencies;
        }

        /**
         * @brief Add a dependency to the component.
         * @param dep The name of the dependency.
         */
        void addDepPackage(const std::shared_ptr<Package> &dep)
        {
            m_depPackages.push_back(dep);
        }

        /**
         * @brief Add a vector of dependencies to the component.
         * @param deps The vector of dependencies.
         */
        void addDepPackages(const std::vector<std::shared_ptr<Package>> &deps)
        {
            m_depPackages.insert(m_depPackages.end(), deps.begin(), deps.end());
        }

        /**
         * @brief Get the dependencies of the component.
         * @return The vector of dependencies.
         */
        std::vector<std::shared_ptr<Package>> getDepPackages() const
        {
            return m_depPackages;
        }

        /**
         * @brief Get the name of the component.
         * @return The name of the component.
         */
        const std::string getName() const
        {
            return name;
        }

    private:
        std::string m_name;                                  // The name of the component.
        std::vector<std::string> m_dependencies;             // The dependencies of the component.
        std::vector<std::shared_ptr<Package>> m_depPackages; // The dependencies of the component.
    };

    /**
     * @brief This is the class which contains the dependencies relationships of the components.
     * @note This class will not contains any component's shared_ptr or unique_ptr.
     * @note So, the PackageManager is not same as the ComponentManager.
     */
    class PackageManager
    {
    public:
        /**
         * @brief Construct a new Package Manager object.
         */
        explicit PackageManager() = default;

        /**
         * @brief Destroy the Package Manager object.
         */
        ~PackageManager() = default;

        /**
         * @brief Add a component to the package manager.
         * @param name The name of the component.
         * @note The name of the component must be unique.
         * @note The name of the component must be the same as the name of the component in the package.json.
         */
        void addPackage(const std::string &name);

        /**
         * @brief Remove a component from the package manager.
         * @param name The name of the component.
         * @note If there is no component with the name, this function will do nothing.
         */
        void removePackage(const std::string &name);

        /**
         * @brief Get a component from the package manager.
         * @param name The name of the component.
         * @return The component.
         * @note If there is no component with the name, this function will return nullptr.
         */
        std::shared_ptr<Package> getPackage(const std::string &name);

        /**
         * @brief Modify a component in the package manager.
         * @param package The component.
         * @note If there is no component with the name, this function will do nothing.
         */
        void modifyPackage(const std::shared_ptr<Package> &package);

        /**
         * @brief Add a dependency to the component.
         * @param modName The name of the component.
         * @param depName The name of the dependency.
         * @note If there is no component with the name, this function will do nothing.
         */
        void addDependency(const std::string &modName, const std::string &depName);

        /**
         * @brief Resolve the dependencies of the component.
         * @param modName The name of the component.
         * @param resolvedDeps The vector of resolved dependencies.
         * @param missingDeps The vector of missing dependencies.
         * @return True if the dependencies are resolved successfully, otherwise false.
         * @note If there is no component with the name, this function will return false.
         */
        bool resolveDependencies(const std::string &modName, std::vector<std::string> &resolvedDeps, std::vector<std::string> &missingDeps);

    private:
#if ENABLE_FASTHASH
        emhash8::HashMap<std::string, std::shared_ptr<Package>> m_packages;
#else
        std::unordered_map<std::string, std::shared_ptr<Package>> m_packages;
#endif

        /**
         * @brief Check the circular dependencies of the component.
         * @param modName The name of the component.
         * @param visited The map of visited components.
         * @param recursionStack The map of recursion stack.
         * @return True if the dependencies are resolved successfully, otherwise false.
         * @note If there is no component with the name, this function will return false.
         */
        bool checkCircularDependencies(const std::string &modName, std::unordered_map<std::string, bool> &visited, std::unordered_map<std::string, bool> &recursionStack);

        /**
         * @brief Check the missing dependencies of the component.
         * @param modName The name of the component.
         * @param missingDeps The vector of missing dependencies.
         * @return True if the dependencies are resolved successfully, otherwise false.
         * @note If there is no component with the name, this function will return false.
         */
        bool checkMissingDependencies(const std::string &modName, std::vector<std::string> &missingDeps);
    };
}
