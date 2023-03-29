/*
 * file.cpp
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

#include "file.hpp"

#include <spdlog/spdlog.h>
#include <openssl/md5.h>

/**
 * @brief 检查文件是否存在。
 * @param filename 文件名
 * @return 文件是否存在
 */
bool fileExists(const std::string& filename) {
    std::ifstream infile(filename.c_str());
    return infile.good();
}

bool FileManager::createFile(const std::string& filename) {
    if (fileExists(filename)) {
        spdlog::error("File \"{}\" already exists!", filename);
        return false;
    }
    std::ofstream outfile(filename.c_str());
    if (!outfile) {
        spdlog::error("Error creating file \"{}\"!", filename);
        return false;
    }
    outfile.close();
    std::fclose(std::fopen(filename.c_str(), "a")); // create a link to the file
    spdlog::info("Created file \"{}\"", filename);
    return true;
}

bool FileManager::openFile(const std::string& filename) {
    if (!fileExists(filename)) {
        spdlog::error("File \"{}\" does not exist!", filename);
        return false;
    }
    m_filename = filename;
    m_file.open(filename.c_str(), std::ios::in | std::ios::out);
    if (!m_file) {
        spdlog::error("Could not open file \"{}\"!", filename);
        return false;
    }
    spdlog::info("Opened file \"{}\"", filename);
    return true;
}

bool FileManager::readFile(std::string& contents) {
    if (!m_file.is_open()) {
        spdlog::error("No file is currently open!");
        return false;
    }
    std::stringstream buffer;
    buffer << m_file.rdbuf();
    contents = buffer.str();
    spdlog::info("Read contents of file \"{}\"", m_filename);
    return true;
}

bool FileManager::writeFile(const std::string& contents) {
    if (!m_file.is_open()) {
        spdlog::error("No file is currently open!");
        return false;
    }
    m_file << contents;
    spdlog::info("Wrote contents to file \"{}\"", m_filename);
    return true;
}

bool FileManager::moveFile(const std::string& oldFilename, const std::string& newFilename) {
    if (!fileExists(oldFilename)) {
        spdlog::error("File \"{}\" does not exist!", oldFilename);
        return false;
    }
    if (fileExists(newFilename)) {
        spdlog::error("File \"{}\" already exists!", newFilename);
        return false;
    }
    int result = std::rename(oldFilename.c_str(), newFilename.c_str());
    if (result != 0) {
        spdlog::error("Could not move file \"{}\" to \"{}\"!", oldFilename, newFilename);
        return false;
    }
    spdlog::info("Moved file from \"{}\" to \"{}\"", oldFilename, newFilename);
    return true;
}

bool FileManager::deleteFile(const std::string& filename) {
    if (!fileExists(filename)) {
        spdlog::error("File \"{}\" does not exist!", filename);
        return false;
    }
    if (remove(filename.c_str()) != 0) {
        spdlog::error("Could not delete file \"{}\"!", filename);
        return false;
    }
    spdlog::info("Deleted file \"{}\"", filename);
    return true;
}

long FileManager::getFileSize() {
    if (!m_file.is_open()) {
        spdlog::error("No file is currently open!");
        return -1;
    }
    m_file.seekg(0, m_file.end);
    long fileSize = m_file.tellg();
    m_file.seekg(0, m_file.beg);
    if (fileSize == -1) {
        spdlog::error("Could not get file size of \"{}\"!", m_filename);
    } else {
        spdlog::info("File size of \"{}\" is {} bytes", m_filename, fileSize);
    }
    return fileSize;
}

std::string FileManager::calculateMD5() {
    if (!m_file.is_open()) {
        spdlog::error("No file is currently open!");
        return "";
    }
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    char buffer[1024];
    while (m_file.read(buffer, sizeof(buffer))) {
        MD5_Update(&md5Context, buffer, sizeof(buffer));
    }
    MD5_Final(reinterpret_cast<unsigned char*>(buffer), &md5Context);
    std::stringstream md5Stream;
    md5Stream << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        md5Stream << std::setw(2) << static_cast<int>(buffer[i]);
    }
    spdlog::info("MD5 value for file \"{}\" is {}", m_filename, md5Stream.str());
    return md5Stream.str();
}

std::string FileManager::getFileDirectory(const std::string& filename) {
    size_t pos = filename.find_last_of("/\\");
    if (pos == std::string::npos) {
        spdlog::error("Could not get directory of file \"{}\"", filename);
        return "";
    } else {
        std::string directory = filename.substr(0, pos);
        spdlog::info("Directory of file \"{}\" is \"{}\"", filename, directory);
        return directory;
    }
}
