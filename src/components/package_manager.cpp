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

Description: Package manager to solve the dependency problem.

**************************************************/

#include "package_manager.hpp"

#include "error/exception.hpp"

void PackageManager::addPackage(const std::string &name)
{
    if (m_packages.find(name) == m_packages.end())
    {
        m_packages[name] = std::make_shared<Package>(name);
    }
}

void PackageManager::removePackage(const std::string &name)
{
    if (m_packages.find(name) != m_packages.end())
    {
        m_packages.erase(name);
    }
}

std::shared_ptr<Package> PackageManager::getPackage(const std::string &name)
{
    if (m_packages.find(name) != m_packages.end())
    {
        return m_packages[name];
    }
    return nullptr;
}

void PackageManager::modifyPackage(const std::shared_ptr<Package> &package)
{
    if (m_packages.find(package->getName()) != m_packages.end())
    {
        m_packages[package->getName()] = package;
    }
}

void PackageManager::addDependency(const std::string &modName, const std::string &depName)
{
    if (m_packages.find(modName) != m_packages.end() && m_packages.find(depName) != m_packages.end())
    {
        m_packages[modName]->addDepPackage(m_packages[depName]);
    }
}

bool PackageManager::resolveDependencies(const std::string &modName, std::vector<std::string> &resolvedDeps, std::vector<std::string> &missingDeps)
{
    std::unordered_map<std::string, int> inDegree;
    std::queue<std::shared_ptr<Package>> q;
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
    const std::shared_ptr<Package> &mod = m_packages[modName];
    q.push(mod);
    while (!q.empty())
    {
        std::shared_ptr<Package> currentMod = q.front();
        q.pop();

        resolvedDeps.push_back(currentMod->getName());

        for (const std::shared_ptr<Package> &dep : currentMod->getDepPackages())
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
        throw PackageDependencyException(modName, "Circular dependencies detected.");
    }
    if (!checkMissingDependencies(modName, missingDeps))
    {
        return false;
    }
    return true;
}

bool PackageManager::checkMissingDependencies(const std::string &modName, std::vector<std::string> &missingDeps)
{
    std::unordered_map<std::string, bool> expectedDeps;
    for (const std::string &depName : m_packages[modName]->getDependencies())
    {
        expectedDeps[depName] = true;
    }
    for (const std::shared_ptr<Package> &dep : m_packages[modName]->getDepPackages())
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

bool PackageManager::checkCircularDependencies(const std::string &modName, std::unordered_map<std::string, bool> &visited, std::unordered_map<std::string, bool> &recursionStack)
{
    if (!visited[modName])
    {
        visited[modName] = true;
        recursionStack[modName] = true;

        const std::shared_ptr<Package> &mod = m_packages[modName];
        for (const std::shared_ptr<Mod> &dep : mod->getDepPackages())
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