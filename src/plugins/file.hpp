/*
 * file.hpp
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

Date: 2023-3-29

Description: File Manager

**************************************************/

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <cstring>

class FileManager
{
public:
    FileManager() = default;
    ~FileManager() = default;

    /**
     * @brief 创建指定的文件。
     * 如果文件已经存在，将返回false，并打印错误信息。
     * 在创建文件后，我们还将使用标准C库中的std::fopen()函数来创建一个链接到该文件，
     * 这样可以通过多个名称访问该文件。
     * @param filename 文件名
     * @return 是否成功创建文件
     */
    bool createFile(const std::string &filename);

    /**
     * @brief 打开指定的文件
     * 如果文件不存在，将返回false，并打印错误信息。
     * @param filename 文件名
     * @return 是否成功打开文件
     */
    bool openFile(const std::string &filename);

    /**
     * @brief 读取当前打开的文件的内容
     * 如果当前没有打开的文件，将返回false，并打印错误信息。
     * @param contents 用于存储读取的内容的字符串
     * @return 是否成功读取文件
     */
    bool readFile(std::string &contents);

    /**
     * @brief 写当前打开的文件
     * 如果当前没有打开的文件，将返回false，并打印错误信息。
     * @param contents 要写入文件的内容字符串
     * @return 是否成功写文件
     */
    bool writeFile(const std::string &contents);

    /**
     * @brief 将旧文件重命名为新文件
     * 如果旧文件不存在或新文件已经存在，将返回false，并打印错误信息。
     * @param oldFilename 旧文件名
     * @param newFilename 新文件名
     * @return 是否成功移动文件
     */
    bool moveFile(const std::string &oldFilename, const std::string &newFilename);

    /**
     * @brief 删除指定的文件
     * 如果文件不存在，将返回false，并打印错误信息。
     * @param filename 文件名
     * @return 是否成功删除文件
     */
    bool deleteFile(const std::string &filename);

    /**
     * @brief 获取当前打开的文件的大小（单位：字节）
     * 如果文件大小获取失败，将返回-1。
     * 如果当前没有打开的文件，将返回false，并打印错误信息。
     * @return 文件大小（单位：字节）
     */
    long getFileSize();

    /**
     * @brief 计算当前打开的文件的MD5值
     * 如果当前没有打开的文件，将返回空字符串，并打印错误信息。
     * @return 文件的MD5值
     */
    std::string calculateMD5();

    /**
     * @brief 获取指定文件所在目录的路径
     * 如果文件不存在或者发生其他错误，将返回空字符串，并打印错误信息。
     * @param filename 文件名
     * @return 文件所在目录的路径
     */
    std::string getFileDirectory(const std::string &filename);

private:
    std::string m_filename; ///< 当前打开的文件名
    std::fstream m_file;    ///< 当前打开的文件流
};