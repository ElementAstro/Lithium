/*
 * package_manager.cpp
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

#include "package_manager.hpp"

#include "error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace Lithium::Component
{
    void ModuleManager::addModule(const std::string &name)
    {
        if (m_packages.find(name) == m_packages.end())
        {
            m_packages[name] = std::make_shared<Module>(name);
        }
    }

    void ModuleManager::removeModule(const std::string &name)
    {
        if (m_packages.find(name) != m_packages.end())
        {
            m_packages.erase(name);
        }
    }

    std::shared_ptr<Module> ModuleManager::getModule(const std::string &name)
    {
        if (m_packages.find(name) != m_packages.end())
        {
            return m_packages[name];
        }
        return nullptr;
    }

    void ModuleManager::modifyModule(const std::shared_ptr<Module> &package)
    {
        if (m_packages.find(package->getName()) != m_packages.end())
        {
            m_packages[package->getName()] = package;
        }
    }

    void ModuleManager::addDependency(const std::string &modName, const std::string &depName)
    {
        if (m_packages.find(modName) != m_packages.end() && m_packages.find(depName) != m_packages.end())
        {
            m_packages[modName]->addDepModule(m_packages[depName]);
        }
    }

    bool ModuleManager::resolveDependencies(const std::string &modName, std::vector<std::string> &resolvedDeps, std::vector<std::string> &missingDeps)
    {
        std::unordered_map<std::string, int> inDegree;
        std::queue<std::shared_ptr<Module>> q;
        // 检查模组是否存在
        if (m_packages.find(modName) == m_packages.end())
        {
            return false;
        }

        // 统计每个模组的入度
        for (const auto &pair : m_packages)
        {
            inDegree[pair.second->getName()] = 0;
            for (auto &dep : pair.second->getDependencies())
            {
                inDegree[dep->getName()]++;
            }
        }

        // 将入度为0的模组加入队列
        const std::shared_ptr<Module> &mod = m_packages[modName];
        q.push(mod);
        while (!q.empty())
        {
            std::shared_ptr<Module> currentMod = q.front();
            q.pop();

            resolvedDeps.push_back(currentMod->getName());

            for (const std::shared_ptr<Module> &dep : currentMod->getDepModules())
            {
                inDegree[dep->getName()]--;
                if (inDegree[dep->getName()] == 0)
                {
                    q.push(dep);
                }
            }
        }
        // 检查是否有循环依赖关系
        if (resolvedDeps.size() < m_packages.size())
        {
            throw ModuleDependencyException(modName, "Circular dependencies detected.");
        }
        if (!checkMissingDependencies(modName, missingDeps))
        {
            return false;
        }
        return true;
    }

    bool ModuleManager::checkMissingDependencies(const std::string &modName, std::vector<std::string> &missingDeps)
    {
        std::unordered_map<std::string, bool> expectedDeps;
        for (const std::string &depName : m_packages[modName]->getDependencies())
        {
            expectedDeps[depName] = true;
        }
        for (const std::shared_ptr<Module> &dep : m_packages[modName]->getDepModules())
        {
            if (expectedDeps.find(dep->getName()) != expectedDeps.end())
            {
                expectedDeps.erase(dep->getName());
            }
        }
        for (const auto &pair : expectedDeps)
        {
            missingDeps.push_back(pair.first);
        }
        return missingDeps.empty();
    }

    bool ModuleManager::checkCircularDependencies(const std::string &modName, std::unordered_map<std::string, bool> &visited, std::unordered_map<std::string, bool> &recursionStack)
    {
        if (!visited[modName])
        {
            visited[modName] = true;
            recursionStack[modName] = true;

            const std::shared_ptr<Module> &mod = m_packages[modName];
            for (const std::shared_ptr<Mod> &dep : mod->getDepModules())
            {
                if (!visited[dep->getName()] && checkCircularDependencies(dep->getName(), visited, recursionStack))
                {
                    return true;
                }
                else if (recursionStack[dep->getName()])
                {
                    return true;
                }
            }
        }

        recursionStack[modName] = false;
        return false;
    }
} // namespace Lithium::Component
