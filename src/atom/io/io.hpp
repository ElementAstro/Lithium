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

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace Atom::IO
{
    /**
     * @brief Creates a directory with the specified path.
     *
     * @param path The path of the directory to create.
     * @return True if the operation was successful, false otherwise.
     *
     * 使用指定路径创建一个目录。
     *
     * @param path 要创建的目录的路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool create_directory(const std::string &path);

    /**
     * @brief Removes an empty directory with the specified path.
     *
     * @param path The path of the directory to remove.
     * @return True if the operation was successful, false otherwise.
     *
     * 删除具有指定路径的空目录。
     *
     * @param path 要删除的目录的路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool remove_directory(const std::string &path);

    /**
     * @brief Renames a directory with the specified old and new paths.
     *
     * @param old_path The old path of the directory to be renamed.
     * @param new_path The new path of the directory after renaming.
     * @return True if the operation was successful, false otherwise.
     *
     * 重命名具有指定旧路径和新路径的目录。
     *
     * @param old_path 要重命名的目录的旧路径。
     * @param new_path 重命名后目录的新路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool rename_directory(const std::string &old_path, const std::string &new_path);

    /**
     * @brief Moves a directory from one path to another.
     *
     * @param old_path The old path of the directory to be moved.
     * @param new_path The new path of the directory after moving.
     * @return True if the operation was successful, false otherwise.
     *
     * 将一个目录从一个路径移动到另一个路径。
     *
     * @param old_path 要移动的目录的旧路径。
     * @param new_path 移动后目录的新路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool move_directory(const std::string &old_path, const std::string &new_path);

    /**
     * @brief Copies a file from source path to destination path.
     *
     * @param src_path The source path of the file to be copied.
     * @param dst_path The destination path of the copied file.
     * @return True if the operation was successful, false otherwise.
     *
     * 从源路径复制文件到目标路径。
     *
     * @param src_path 要复制的文件的源路径。
     * @param dst_path 复制后文件的目标路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool copy_file(const std::string &src_path, const std::string &dst_path);

    /**
     * @brief Moves a file from source path to destination path.
     *
     * @param src_path The source path of the file to be moved.
     * @param dst_path The destination path of the moved file.
     * @return True if the operation was successful, false otherwise.
     *
     * 从源路径移动文件到目标路径。
     *
     * @param src_path 要移动的文件的源路径。
     * @param dst_path 移动后文件的目标路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool move_file(const std::string &src_path, const std::string &dst_path);

    /**
     * @brief Renames a file with the specified old and new paths.
     *
     * @param old_path The old path of the file to be renamed.
     * @param new_path The new path of the file after renaming.
     * @return True if the operation was successful, false otherwise.
     *
     * 重命名具有指定旧路径和新路径的文件。
     *
     * @param old_path 要重命名的文件的旧路径。
     * @param new_path 重命名后文件的新路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool rename_file(const std::string &old_path, const std::string &new_path);

    /**
     * @brief Removes a file with the specified path.
     *
     * @param path The path of the file to remove.
     * @return True if the operation was successful, false otherwise.
     *
     * 删除具有指定路径的文件。
     *
     * @param path 要删除的文件的路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool remove_file(const std::string &path);

    /**
     * @brief Creates a symbolic link with the specified target and symlink paths.
     *
     * @param target_path The path of the target file or directory for the symlink.
     * @param symlink_path The path of the symlink to create.
     * @return True if the operation was successful, false otherwise.
     *
     * 使用指定的目标路径和符号链接路径创建符号链接。
     *
     * @param target_path 符号链接的目标文件或目录的路径。
     * @param symlink_path 要创建的符号链接的路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool create_symlink(const std::string &target_path, const std::string &symlink_path);

    /**
     * @brief Removes a symbolic link with the specified path.
     *
     * @param path The path of the symlink to remove.
     * @return True if the operation was successful, false otherwise.
     *
     * 删除具有指定路径的符号链接。
     *
     * @param path 要删除的符号链接的路径。
     * @return 如果操作成功，则返回true，否则返回false。
     */
    [[nodiscard]] bool remove_symlink(const std::string &path);

    /**
     * @brief Returns the size of a file in bytes.
     *
     * @param path The path of the file to get the size of.
     * @return The size of the file in bytes, or 0 if the file does not exist or cannot be read.
     *
     * 返回文件的大小（以字节为单位）。
     *
     * @param path 要获取大小的文件的路径。
     * @return 文件的大小（以字节为单位），如果文件不存在或无法读取，则返回0。
     */
    [[nodiscard]] std::uintmax_t file_size(const std::string &path);

    /**
     * @brief Traverses a directory and prints the names of all files and directories within it.
     *
     * @param path The path of the directory to traverse.
     *
     * 遍历目录并打印其中所有文件和目录的名称。
     *
     * @param path 要遍历的目录的路径。
     */
    void traverse_directory(const std::string &path);

    /**
     * @brief Traverse the directories recursively and collect all folder paths.
     *
     * This function traverses the directories recursively starting from the specified directory,
     * and collects all folder paths encountered during the traversal.
     *
     * @param directory The starting directory path.
     * @param[out] folders A vector to store the collected folder paths.
     */
    void traverse_directories(const std::filesystem::path &directory, std::vector<std::string> &folders);

    /**
     * @brief Convert Windows path to Linux path.
     *
     * This function converts a Windows path to a Linux path by replacing backslashes with forward slashes.
     *
     * @param windows_path The Windows path to convert.
     * @return The converted Linux path.
     */
    [[nodiscard]] std::string convert_windows_to_linux_path(const std::string &windows_path);

    /**
     * @brief Convert Linux path to Windows path.
     *
     * This function converts a Linux path to a Windows path by replacing forward slashes with backslashes.
     *
     * @param linux_path The Linux path to convert.
     * @return The converted Windows path.
     */
    [[nodiscard]] std::string convert_linux_to_windows_path(const std::string &linux_path);

    [[nodiscard]] bool is_full_path(const std::string& path);

    [[nodiscard]] bool isFolderNameValid(const std::string& folderName);

    [[nodiscard]] bool isFileNameValid(const std::string& fileName);

    [[nodiscard]] bool isFolderExists(const std::string& folderPath);

    [[nodiscard]] bool isFileExists(const std::string& filePath);
} // namespace Lithium::File
