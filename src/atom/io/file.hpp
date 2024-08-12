/*
 * file.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: File Manager

**************************************************/

#ifndef ATOM_IO_FILE_HPP
#define ATOM_IO_FILE_HPP

#include <fstream>
#include <string>

namespace atom::io {
/**
 * @class FileManager
 * @brief 文件管理器类，提供文件的创建、打开、读取、写入、移动、删除等功能。
 */
class FileManager {
public:
    /**
     * 创建文件
     * @param filename 文件名
     * @return 是否成功创建文件
     */
    auto createFile(const std::string &filename) -> bool;

    /**
     * 打开文件
     * @param filename 文件名
     * @return 是否成功打开文件
     */
    auto openFile(const std::string &filename) -> bool;

    /**
     * 读取文件内容
     * @param contents 存储文件内容的变量
     * @return 是否成功读取文件
     */
    auto readFile(std::string &contents) -> bool;

    /**
     * 写入文件内容
     * @param contents 文件内容
     * @return 是否成功写入文件
     */
    auto writeFile(const std::string &contents) -> bool;

    /**
     * 移动文件
     * @param oldFilename 原文件名
     * @param newFilename 新文件名
     * @return 是否成功移动文件
     */
    auto moveFile(const std::string &oldFilename,
                  const std::string &newFilename) -> bool;

    /**
     * 删除文件
     * @param filename 文件名
     * @return 是否成功删除文件
     */
    auto deleteFile(const std::string &filename) -> bool;

    /**
     * 获取文件大小
     * @return 文件大小（字节数），如果获取失败则返回-1
     */
    auto getFileSize() -> long;

    /**
     * 获取文件所在目录路径
     * @param filename 文件名
     * @return 文件所在目录的路径
     */
    static auto getFileDirectory(const std::string &filename) -> std::string;

private:
    std::fstream m_file_;     ///< 文件流对象
    std::string m_filename_;  ///< 当前打开的文件名
};

}  // namespace atom::io

#endif
