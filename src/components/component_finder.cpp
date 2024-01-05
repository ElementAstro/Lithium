/*
 * component_finder.cpp
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

Description: Component finder (the core of the plugin system)

**************************************************/

#include "component_finder.hpp"

namespace Lithium
{
    ComponentFinder::ComponentFinder(const std::filesystem::path &path, const FilterFunction &filterFunc)
        : m_path(path), m_dirContainer(m_path)
    {
        if (std::filesystem::exists(m_path) && std::filesystem::is_directory(m_path))
        {
            traverseDir(m_path, m_dirContainer);
        }
    }

    void ComponentFinder::print() const
    {
        printDir(m_dirContainer);
    }

    std::vector<std::string> ComponentFinder::getAvailableDirs() const
    {
        std::vector<std::string> matchingSubdirs;

        // Recursive function to find matching subdirectories
        std::function<void(const DirContainer &)> findMatchingSubdirs = [&](const DirContainer &dir)
        {
            for (const auto &subdir : dir.getSubdirs())
            {
                if (m_filterFunc(subdir.getPath()))
                {
                    matchingSubdirs.push_back(subdir.getPath().filename().string());
                }
                findMatchingSubdirs(subdir);
            }
        };

        findMatchingSubdirs(m_dirContainer);

        return matchingSubdirs;
    }

    bool ComponentFinder::hasFile(const std::filesystem::path &path, const std::string &filename)
    {
        for (const auto &entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                if (hasFile(entry.path(), filename))
                {
                    return true;
                }
            }
            else
            {
                if (entry.path().filename() == filename)
                {
                    return true;
                }
            }
        }
        return false;
    }

    ComponentFinder::DirContainer::DirContainer(const std::filesystem::path &path) : m_path(path) {}

    const std::filesystem::path &ComponentFinder::DirContainer::getPath() const
    {
        return m_path;
    }

    const std::vector<DirContainer> &ComponentFinder::DirContainer::getSubdirs() const
    {
        return m_subdirs;
    }

    const std::vector<std::filesystem::path> &ComponentFinder::DirContainer::getFiles() const
    {
        return m_files;
    }

    void ComponentFinder::DirContainer::addSubdir(const DirContainer &subdir)
    {
        m_subdirs.push_back(subdir);
    }

    void ComponentFinder::DirContainer::addFile(const std::filesystem::path &file)
    {
        m_files.push_back(file);
    }

    void ComponentFinder::traverseDir(const std::filesystem::path &path, DirContainer &container)
    {
        for (const auto &entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                DirContainer subdir(entry.path());
                traverseDir(entry.path(), subdir);
                if (!subdir.getFiles().empty())
                {
                    container.addSubdir(subdir);
                }
            }
            else
            {
                if (!m_filterFunc || m_filterFunc(entry.path()))
                {
                    container.addFile(entry.path());
                }
            }
        }
    }

    void ComponentFinder::printDir(const DirContainer &dir, int level)
    {
        for (int i = 0; i < level; ++i)
        {
            std::cout << "  ";
        }
        std::cout << "+ " << dir.getPath().filename() << std::endl;
        for (const auto &subdir : dir.getSubdirs())
        {
            printDir(subdir, level + 1);
        }
        for (const auto &file : dir.getFiles())
        {
            for (int i = 0; i < level + 1; ++i)
            {
                std::cout << "  ";
            }
            std::cout << "- " << file.filename() << std::endl;
        }
    }

}
