/*
 * io.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: IO

**************************************************/

#ifndef ATOM_IO_IO_HPP
#define ATOM_IO_IO_HPP

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>
#include "macro.hpp"
namespace fs = std::filesystem;

namespace atom::io {
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
[[nodiscard]] auto createDirectory(const std::string &path) -> bool;

/**
 * @brief Creates a directory with the specified path.
 */
struct CreateDirectoriesOptions {
    bool verbose = true;
    bool dryRun = false;
    int delay = 0;
    std::function<bool(const std::string &)> filter = [](const std::string &) {
        return true;
    };
    std::function<void(const std::string &)> onCreate =
        [](const std::string &) {};
    std::function<void(const std::string &)> onDelete =
        [](const std::string &) {};
} ATOM_ALIGNAS(128);

/**
 * @brief Creates a directory with the specified path.
 *
 * @param basePath The base path of the directory to create.
 * @param subdirs The subdirectories to create.
 * @param options The options for creating the directory.
 * @return True if the operation was successful, false otherwise.
 *
 * 使用指定路径创建一个目录。
 *
 * @param basePath 要创建的目录的基本路径。
 * @param subdirs 要创建的子目录。
 * @param options 创建目录的选项。
 * @return 如果操作成功，则返回true，否则返回false。
 */
auto createDirectoriesRecursive(
    const fs::path &basePath, const std::vector<std::string> &subdirs,
    const CreateDirectoriesOptions &options) -> bool;

/**
 * @brief Creates a directory with the specified path.
 *
 * @param date The directory to create.
 * @param rootDir The root directory of the directory to create.
 * @return True if the operation was successful, false otherwise.
 *
 * 使用指定路径创建一个目录。
 *
 * @param date 要创建的目录。
 * @param rootDir 要创建的目录的根目录。
 * @return 如果操作成功，则返回true，否则返回false。
 */
void createDirectory(const std::string &date, const std::string &rootDir);

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
[[nodiscard]] auto removeDirectory(const std::string &path) -> bool;

/**
 * @brief Removes a directory with the specified path.
 *
 * @param basePath The base path of the directory to remove.
 * @param subdirs The subdirectories to remove.
 * @param options The options for removing the directory.
 * @return True if the operation was successful, false otherwise.
 *
 * 删除具有指定路径的目录。
 *
 * @param basePath 要删除的目录的基本路径。
 * @param subdirs 要删除的子目录。
 * @param options 删除目录的选项。
 * @return 如果操作成功，则返回true，否则返回false。
 */
[[nodiscard]] auto removeDirectoriesRecursive(
    const fs::path &basePath, const std::vector<std::string> &subdirs,
    const CreateDirectoriesOptions &options = {}) -> bool;

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
[[nodiscard]] auto renameDirectory(const std::string &old_path,
                                   const std::string &new_path) -> bool;

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
[[nodiscard]] auto moveDirectory(const std::string &old_path,
                                 const std::string &new_path) -> bool;

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
[[nodiscard]] auto copyFile(const std::string &src_path,
                            const std::string &dst_path) -> bool;

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
[[nodiscard]] auto moveFile(const std::string &src_path,
                            const std::string &dst_path) -> bool;

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
[[nodiscard]] auto renameFile(const std::string &old_path,
                              const std::string &new_path) -> bool;

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
[[nodiscard]] auto removeFile(const std::string &path) -> bool;

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
[[nodiscard]] auto createSymlink(const std::string &target_path,
                                 const std::string &symlink_path) -> bool;

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
[[nodiscard]] auto removeSymlink(const std::string &path) -> bool;

/**
 * @brief Returns the size of a file in bytes.
 *
 * @param path The path of the file to get the size of.
 * @return The size of the file in bytes, or 0 if the file does not exist or
 * cannot be read.
 *
 * 返回文件的大小（以字节为单位）。
 *
 * @param path 要获取大小的文件的路径。
 * @return 文件的大小（以字节为单位），如果文件不存在或无法读取，则返回0。
 */
[[nodiscard]] auto fileSize(const std::string &path) -> std::uintmax_t;

/**
 * @brief Truncates a file to a specified size.
 *
 * @param path The path of the file to truncate.
 * @param size The size to truncate the file to.
 * @return True if the operation was successful, false otherwise.
 *
 * 截断文件到指定的大小。
 *
 * @param path 要截断文件的路径。
 * @param size 要截断文件到的大小。
 * @return 如果操作成功，则返回true，否则返回false。
 */
auto truncateFile(const std::string &path, std::streamsize size) -> bool;

/**
 * @brief Recursively walks through a directory and its subdirectories, applying
 * a callback function to each file.
 *
 * This function traverses a directory and its subdirectories, calling the
 * specified callback function for each file encountered.
 *
 * @param root The root path of the directory to walk.
 * @return a json string containing the file information.
 */
auto jwalk(const std::string &root) -> std::string;

/**
 * @brief Recursively walks through a directory and its subdirectories, applying
 * a callback function to each file.
 *
 * This function traverses a directory and its subdirectories, calling the
 * specified callback function for each file encountered.
 *
 * @param root     The root path of the directory to walk.
 * @param callback The callback function to execute for each file.
 */
void fwalk(const fs::path &root,
           const std::function<void(const fs::path &)> &callback);

/**
 * @brief Convert Windows path to Linux path.
 *
 * This function converts a Windows path to a Linux path by replacing
 * backslashes with forward slashes.
 *
 * @param windows_path The Windows path to convert.
 * @return The converted Linux path.
 */
[[nodiscard]] auto convertToLinuxPath(std::string_view windows_path)
    -> std::string;

/**
 * @brief Convert Linux path to Windows path.
 *
 * This function converts a Linux path to a Windows path by replacing forward
 * slashes with backslashes.
 *
 * @param linux_path The Linux path to convert.
 * @return The converted Windows path.
 */
[[nodiscard]] auto convertToWindowsPath(std::string_view linux_path)
    -> std::string;

[[nodiscard]] auto normPath(std::string_view raw_path) -> std::string;

/**
 * @brief Check if the folder name is valid.
 *
 * @param folderName The folder name to check.
 * @return True if the folder name is valid, false otherwise.
 *
 * 检查文件夹名称是否有效。
 *
 * @param folderName 要检查的文件夹名称。
 * @return 如果文件夹名称有效，则返回true，否则返回false。
 */
[[nodiscard]] auto isFolderNameValid(const std::string &folderName) -> bool;

/**
 * @brief Check if the file name is valid.
 *
 * @param fileName The file name to check.
 * @return True if the file name is valid, false otherwise.
 *
 * 检查文件名称是否有效。
 *
 * @param fileName 要检查的文件名称。
 * @return 如果文件名称有效，则返回true，否则返回false。
 */
[[nodiscard]] auto isFileNameValid(const std::string &fileName) -> bool;

/**
 * @brief Check if the folder exists.
 *
 * @param folderPath The folder path to check.
 * @return True if the folder exists, false otherwise.
 *
 * 检查文件夹是否存在。
 *
 * @param folderPath 要检查的文件夹路径。
 * @return 如果文件夹存在，则返回true，否则返回false。
 */
[[nodiscard]] auto isFolderExists(const std::string &folderPath) -> bool;

/**
 * @brief Check if the file exists.
 *
 * @param filePath The file path to check.
 * @return True if the file exists, false otherwise.
 *
 * 检查文件是否存在。
 *
 * @param filePath 要检查的文件路径。
 * @return 如果文件存在，则返回true，否则返回false。
 */
[[nodiscard]] auto isFileExists(const std::string &filePath) -> bool;

/**
 * @brief Check if the folder is empty.
 *
 * @param folderPath The folder path to check.
 * @return True if the folder is empty, false otherwise.
 *
 * 检查文件夹是否为空。
 *
 * @param folderPath 要检查的文件夹路径。
 * @return 如果文件夹为空，则返回true，否则返回false。
 */
[[nodiscard]] auto isFolderEmpty(const std::string &folderPath) -> bool;

/**
 * @brief Check if the path is an absolute path.
 *
 * @param path The path to check.
 * @return True if the path is an absolute path, false otherwise.
 *
 * 检查路径是否为绝对路径。
 *
 * @param path 要检查的路径。
 * @return 如果路径为绝对路径，则返回true，否则返回false。
 */
[[nodiscard]] auto isAbsolutePath(const std::string &path) -> bool;

/**
 * @brief Change the working directory.
 *
 * @param directoryPath The directory path to change to.
 * @return True if the working directory was changed successfully, false
 * otherwise.
 *
 * 更改工作目录。
 *
 * @param directoryPath 要更改到的目录路径。
 * @return 如果工作目录更改成功，则返回true，否则返回false。
 */
[[nodiscard]] auto changeWorkingDirectory(const std::string &directoryPath)
    -> bool;

[[nodiscard]] std::pair<std::string, std::string> getFileTimes(
    const std::string &filePath);

/**
 * @brief The option to check the file type.
 *
 * @remark The file type is checked by the file extension.
 */
enum class FileOption { PATH, NAME };

/**
 * @brief Check the file type in the folder.
 *
 * @param folderPath The folder path to check.
 * @param fileType The file type to check.
 * @param fileOption The option to check the file type.
 * @return A vector of file paths.
 *
 * 检查文件夹中文件的类型。
 *
 * @param folderPath 要检查的文件夹路径。
 * @param fileType 要检查的文件类型。
 * @param fileOption 要检查文件类型的选项。
 * @return 一个文件路径的向量。
 * @remark The file type is checked by the file extension.
 */
[[nodiscard]] auto checkFileTypeInFolder(
    const std::string &folderPath, const std::string &fileType,
    FileOption fileOption) -> std::vector<std::string>;

/**
 * @brief Check whether the specified file exists.
 * 检查指定文件是否存在
 *
 * @param fileName The name of the file. 文件名称
 * @param fileExt The extension of the file. 文件扩展名
 * @return true if the file exists.
 *         如果文件存在，则返回 true
 * @return false if the file does not exist or an error occurred.
 *         如果文件不存在或发生错误，则返回 false
 */
auto isExecutableFile(const std::string &fileName,
                      const std::string &fileExt) -> bool;
}  // namespace atom::io

#endif
