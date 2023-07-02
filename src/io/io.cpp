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

namespace fs = std::filesystem;

namespace OpenAPT::File
{

    bool create_directory(const std::string &path)
    {
        try
        {
            fs::create_directory(path);
            std::cout << "Directory created: " << path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to create directory " << path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool remove_directory(const std::string &path)
    {
        try
        {
            fs::remove_all(path);
            std::cout << "Directory removed: " << path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to remove directory " << path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool rename_directory(const std::string &old_path, const std::string &new_path)
    {
        try
        {
            fs::rename(old_path, new_path);
            std::cout << "Directory renamed from " << old_path << " to " << new_path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to rename directory from " << old_path << " to " << new_path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool move_directory(const std::string &old_path, const std::string &new_path)
    {
        try
        {
            fs::rename(old_path, new_path);
            std::cout << "Directory moved from " << old_path << " to " << new_path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to move directory from " << old_path << " to " << new_path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool copy_file(const std::string &src_path, const std::string &dst_path)
    {
        try
        {
            fs::copy_file(src_path, dst_path);
            std::cout << "File copied from " << src_path << " to " << dst_path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to copy file from " << src_path << " to " << dst_path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool move_file(const std::string &src_path, const std::string &dst_path)
    {
        try
        {
            fs::rename(src_path, dst_path);
            std::cout << "File moved from " << src_path << " to " << dst_path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to move file from " << src_path << " to " << dst_path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool rename_file(const std::string &old_path, const std::string &new_path)
    {
        try
        {
            fs::rename(old_path, new_path);
            std::cout << "File renamed from " << old_path << " to " << new_path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to rename file from " << old_path << " to " << new_path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool remove_file(const std::string &path)
    {
        try
        {
            fs::remove(path);
            std::cout << "File removed: " << path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to remove file " << path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool create_symlink(const std::string &target_path, const std::string &symlink_path)
    {
        try
        {
            fs::create_symlink(target_path, symlink_path);
            std::cout << "Symlink created from " << target_path << " to " << symlink_path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to create symlink from " << target_path << " to " << symlink_path << ": " << ex.what() << std::endl;
        }
        return false;
    }

    bool remove_symlink(const std::string &path)
    {
        try
        {
            fs::remove(path);
            std::cout << "Symlink removed: " << path << std::endl;
            return true;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to remove symlink " << path << ": " << ex.what() << std::endl;
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
            std::cerr << "Failed to get file size of " << path << ": " << ex.what() << std::endl;
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
                    std::cout << "Directory: " << entry.path().string() << std::endl;
                }
                else
                {
                    std::cout << "File: " << entry.path().string() << std::endl;
                }
            }
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to traverse directory " << path << ": " << ex.what() << std::endl;
        }
    }
}
