/*
 * io.cpp
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

Date: 2023-4-3

Description: IO

**************************************************/

#include "io.hpp"

#include <filesystem>
#include <iostream>
#include <algorithm>

#include "loguru/loguru.hpp"

#ifdef _WIN32
#include <windows.h>
const std::string PATH_SEPARATOR = "\\";
#else
const std::string PATH_SEPARATOR = "/";
#endif

namespace fs = std::filesystem;

namespace Lithium::File
{

    bool create_directory(const std::string &path)
    {
        try
        {
            fs::create_directory(path);
            LOG_F(INFO, "Directory created: %s", path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to create directory %s: %s", path.c_str(), ex.what());
        }
        return false;
    }

    bool remove_directory(const std::string &path)
    {
        try
        {
            fs::remove_all(path);
            LOG_F(INFO, "Directory removed: %s", path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to remove directory %s: %s", path.c_str(), ex.what());
        }
        return false;
    }

    bool rename_directory(const std::string &old_path, const std::string &new_path)
    {
        try
        {
            fs::rename(old_path, new_path);
            LOG_F(INFO, "Directory renamed from %s to %s", old_path.c_str(), new_path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to rename directory from %s to %s: %s", old_path.c_str(), new_path.c_str(), ex.what());
        }
        return false;
    }

    bool move_directory(const std::string &old_path, const std::string &new_path)
    {
        try
        {
            fs::rename(old_path, new_path);
            LOG_F(INFO, "Directory moved from %s to %s", old_path.c_str(), new_path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to move directory from %s to %s: %s", old_path.c_str(), new_path.c_str(), ex.what());
        }
        return false;
    }

    bool copy_file(const std::string &src_path, const std::string &dst_path)
    {
        try
        {
            fs::copy_file(src_path, dst_path);
            LOG_F(INFO, "File copied from %s to %s", src_path.c_str(), dst_path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to copy file from %s to %s: %s", src_path.c_str(), dst_path.c_str(), ex.what());
        }
        return false;
    }

    bool move_file(const std::string &src_path, const std::string &dst_path)
    {
        try
        {
            fs::rename(src_path, dst_path);
            LOG_F(INFO, "File moved from %s to %s", src_path.c_str(), dst_path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to move file from %s to %s: %s", src_path.c_str(), dst_path.c_str(), ex.what());
        }
        return false;
    }

    bool rename_file(const std::string &old_path, const std::string &new_path)
    {
        try
        {
            fs::rename(old_path, new_path);
            LOG_F(INFO, "File renamed from %s to %s", old_path.c_str(), new_path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to rename file from %s to %s: %s", old_path.c_str(), new_path.c_str(), ex.what());
        }
        return false;
    }

    bool remove_file(const std::string &path)
    {
        try
        {
            fs::remove(path);
            LOG_F(INFO, "File removed: %s", path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to remove file %s: %s", path.c_str(), ex.what());
        }
        return false;
    }

    bool create_symlink(const std::string &target_path, const std::string &symlink_path)
    {
        try
        {
            fs::create_symlink(target_path, symlink_path);
            LOG_F(INFO, "Symlink created from %s to %s", target_path.c_str(), symlink_path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to create symlink from %s to %s: %s", target_path.c_str(), symlink_path.c_str(), ex.what());
        }
        return false;
    }

    bool remove_symlink(const std::string &path)
    {
        try
        {
            fs::remove(path);
            LOG_F(INFO, "Symlink removed: %s", path.c_str());
            return true;
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to remove symlink %s: %s", path.c_str(), ex.what());
        }
        return false;
    }

    std::uintmax_t file_size(const std::string &path)
    {
        try
        {
            return fs::file_size(path);
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to get file size of %s: %s", path.c_str(), ex.what());
            return 0;
        }
    }

    void traverse_directory(const std::string &path)
    {
        try
        {
            for (const auto &entry : fs::recursive_directory_iterator(path))
            {
                if (entry.is_directory())
                {
                    LOG_F(INFO, "Directory: %s", entry.path().string().c_str());
                }
                else
                {
                    LOG_F(INFO, "File: %s", entry.path().string().c_str());
                }
            }
        }
        catch (const std::exception &ex)
        {
            LOG_F(ERROR, "Failed to traverse directory %s: %s", path.c_str(), ex.what());
        }
    }

    std::string convert_windows_to_linux_path(const std::string &windows_path)
    {
        std::string linux_path = windows_path;
        for (char &c : linux_path)
        {
            if (c == '\\')
            {
                c = '/';
            }
        }
        if (linux_path.length() >= 2 && linux_path[1] == ':')
        {
            linux_path[0] = tolower(linux_path[0]);
        }

        return linux_path;
    }

    std::string convert_linux_to_windows_path(const std::string &linux_path)
    {
        std::string windows_path = linux_path;
        for (char &c : windows_path)
        {
            if (c == '/')
            {
                c = '\\';
            }
        }
        if (windows_path.length() >= 2 && islower(windows_path[0]) && windows_path[1] == ':')
        {
            windows_path[0] = toupper(windows_path[0]);
        }

        return windows_path;
    }

    std::string get_absolute_directory()
    {
        fs::path program_path;
#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        program_path = buffer;
#else
        char buffer[PATH_MAX];
        ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
        if (length != -1)
        {
            program_path = std::string(buffer, length);
        }
#endif

        return program_path.parent_path().string();
    }

    std::string normalize_path(const std::string &path)
    {
        std::string normalized_path = path;
        std::replace(normalized_path.begin(), normalized_path.end(), '/', PATH_SEPARATOR.front());
        std::replace(normalized_path.begin(), normalized_path.end(), '\\', PATH_SEPARATOR.front());
        return normalized_path;
    }

    void traverse_directories(const fs::path &directory, std::vector<std::string> &folders)
    {
        for (const auto &entry : fs::directory_iterator(directory))
        {
            if (entry.is_directory())
            {
                std::string folder_path = normalize_path(entry.path().string());
                folders.push_back(folder_path);
                traverse_directories(entry.path(), folders);
            }
        }
    }
}
