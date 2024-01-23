/*
 * finder.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Addons finder

**************************************************/

#include "finder.hpp"

#include "atom/log/loguru.hpp"

namespace Lithium
{
    AddonFinder::AddonFinder(const std::filesystem::path &path, const FilterFunction &filterFunc)
        : m_path(path), m_dirContainer(m_path)
    {
    }

    void AddonFinder::print() const
    {
        printDir(m_dirContainer);
    }

    bool AddonFinder::traverseDir(const std::filesystem::path &path)
    {
        if (std::filesystem::exists(m_path) && std::filesystem::is_directory(m_path))
        {
            traverseDir(m_path, m_dirContainer);
        }
        else
        {
            LOG_F(ERROR, "Invalid path: {}", m_path.string());
            return false;
        }
        return
    }

    std::vector<std::string> AddonFinder::getAvailableDirs() const
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

    bool AddonFinder::hasFile(const std::filesystem::path &path, const std::string &filename)
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

    AddonFinder::DirContainer::DirContainer(const std::filesystem::path &path) : m_path(path) {}

    const std::filesystem::path &AddonFinder::DirContainer::getPath() const
    {
        return m_path;
    }

    const std::vector<DirContainer> &AddonFinder::DirContainer::getSubdirs() const
    {
        return m_subdirs;
    }

    const std::vector<std::filesystem::path> &AddonFinder::DirContainer::getFiles() const
    {
        return m_files;
    }

    void AddonFinder::DirContainer::addSubdir(const DirContainer &subdir)
    {
        m_subdirs.push_back(subdir);
    }

    void AddonFinder::DirContainer::addFile(const std::filesystem::path &file)
    {
        m_files.push_back(file);
    }

    void AddonFinder::traverseDir(const std::filesystem::path &path, DirContainer &container)
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

    void AddonFinder::printDir(const DirContainer &dir, int level)
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
