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
    bool createFile(const std::string &filename);

    /**
     * 打开文件
     * @param filename 文件名
     * @return 是否成功打开文件
     */
    bool openFile(const std::string &filename);

    /**
     * 读取文件内容
     * @param contents 存储文件内容的变量
     * @return 是否成功读取文件
     */
    bool readFile(std::string &contents);

    /**
     * 写入文件内容
     * @param contents 文件内容
     * @return 是否成功写入文件
     */
    bool writeFile(const std::string &contents);

    /**
     * 移动文件
     * @param oldFilename 原文件名
     * @param newFilename 新文件名
     * @return 是否成功移动文件
     */
    bool moveFile(const std::string &oldFilename,
                  const std::string &newFilename);

    /**
     * 删除文件
     * @param filename 文件名
     * @return 是否成功删除文件
     */
    bool deleteFile(const std::string &filename);

    /**
     * 获取文件大小
     * @return 文件大小（字节数），如果获取失败则返回-1
     */
    long getFileSize();

    /**
     * 获取文件所在目录路径
     * @param filename 文件名
     * @return 文件所在目录的路径
     */
    static std::string getFileDirectory(const std::string &filename);

private:
    std::fstream m_file;     ///< 文件流对象
    std::string m_filename;  ///< 当前打开的文件名
};

}  // namespace atom::io

#endif
