/*
 * addon.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Addon manager to solve the dependency problem.

**************************************************/

#include "addons.hpp"

#include <queue>
#include <fstream>

#include "atom/log/loguru.hpp"

namespace Lithium
{
    bool AddonManager::addModule(const std::filesystem::path &path, const std::string &name)
    {
        if (m_modules.find(name) != m_modules.end())
        {
            LOG_F(ERROR, "Addon {} has already been added.", name);
            return false;
        }
        if (std::filesystem::exists(path))
        {
            std::filesystem::path new_path = path / "package.json";
            try
            {
                m_modules[name] = json::parse(std::ifstream(new_path.string()));
            }
            catch (const std::exception &e)
            {
                LOG_F(ERROR, "Addon {} package.json file does not exist.", name);
                return false;
            }
        }
        else
        {
            LOG_F(ERROR, "Addon {} does not exist.", name);
            return false;
        }
        return true;
    }

    bool AddonManager::removeModule(const std::string &name)
    {
        if (m_modules.find(name) != m_modules.end())
        {
            m_modules.erase(name);
            DLOG_F(INFO, "Addon {} has been removed.", name);
            return true;
        }
        LOG_F(WARNING, "Addon {} does not exist.", name);
        return false;
    }

    json AddonManager::getModule(const std::string &name)
    {
        if (m_modules.find(name) != m_modules.end())
        {
            return m_modules[name];
        }
        LOG_F(ERROR, "Addon {} does not exist.", name);
        return nullptr;
    }

    bool AddonManager::resolveDependencies(const std::string &modName, std::vector<std::string> &resolvedDeps, std::vector<std::string> &missingDeps)
    {
        // 检查模组是否存在
        if (m_modules.find(modName) == m_modules.end())
        {
            LOG_F(ERROR, "Addon {} does not exist.", modName);
            return false;
        }
        std::unordered_map<std::string, int> inDegree;
        std::queue<json> q;

        for (const auto &pair : m_modules)
        {
            inDegree[pair.second["name"]] = 0;
            for (auto &dep : pair.second["dependencies"])
            {
                inDegree[dep["name"]]++;
            }
        }

        // 将入度为0的模组加入队列
        const json &mod = m_modules[modName];
        q.push(mod);
        while (!q.empty())
        {
            json currentMod = q.front();
            q.pop();

            resolvedDeps.push_back(currentMod["name"].get<std::string>());

            for (const json &dep : currentMod["dependencies"].get<std::vector<json>>())
            {
                inDegree[dep.get<std::string>()]--;
                if (inDegree[dep.get<std::string>()] == 0)
                {
                    q.push(dep);
                }
            }
        }
        // 检查是否有循环依赖关系
        if (resolvedDeps.size() < m_modules.size() || !checkMissingDependencies(modName, missingDeps))
        {
            return false;
        }
        return true;
    }

    bool AddonManager::checkMissingDependencies(const std::string &modName, std::vector<std::string> &missingDeps)
    {
        std::unordered_map<std::string, bool> expectedDeps;
        for (const std::string &depName : m_modules[modName]["dependencies"].get<std::vector<std::string>>())
        {
            expectedDeps[depName] = true;
        }
        for (const std::string &dep : m_modules[modName]["dependencies"].get<std::vector<std::string>>())
        {
            if (expectedDeps.find(dep) != expectedDeps.end())
            {
                expectedDeps.erase(dep);
            }
        }
        for (const auto &pair : expectedDeps)
        {
            missingDeps.push_back(pair.first);
        }
        return missingDeps.empty();
    }

    bool AddonManager::checkCircularDependencies(const std::string &modName, std::unordered_map<std::string, bool> &visited, std::unordered_map<std::string, bool> &recursionStack)
    {
        if (!visited[modName])
        {
            visited[modName] = true;
            recursionStack[modName] = true;

            const json &mod = m_modules[modName];
            for (const json &dep : mod["dependencies"])
            {
                if (!visited[dep.get<std::string>()] && checkCircularDependencies(dep.get<std::string>(), visited, recursionStack))
                {
                    return true;
                }
                else if (recursionStack[dep["name"]])
                {
                    return true;
                }
            }
        }
        recursionStack[modName] = false;
        return false;
    }
} // namespace Lithium
