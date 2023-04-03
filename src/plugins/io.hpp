/*
 * io.hpp
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

#include <string>

namespace OpenAPT::File
{
    bool create_directory(const std::string& path);
    bool remove_directory(const std::string& path);
    bool rename_directory(const std::string& old_path, const std::string& new_path);
    bool move_directory(const std::string& old_path, const std::string& new_path);
    bool copy_file(const std::string& src_path, const std::string& dst_path);
    bool move_file(const std::string& src_path, const std::string& dst_path);
    bool rename_file(const std::string& old_path, const std::string& new_path);
    bool remove_file(const std::string& path);
    bool create_symlink(const std::string& target_path, const std::string& symlink_path);
    bool remove_symlink(const std::string& path);
    std::uintmax_t file_size(const std::string& path);
    void traverse_directory(const std::string& path);
} // namespace OpenAPT::File

