/*
 * file.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: File Manager

**************************************************/

#include "file.hpp"

#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/utils/aes.hpp"

#include <openssl/evp.h>

namespace fs = std::filesystem;

namespace atom::io {

bool FileManager::createFile(const std::string &filename) {
    if (fs::exists(filename)) {
        LOG_F(ERROR, "File \"{}\" already exists!", filename);
        return false;
    }
    std::ofstream outfile(filename);
    if (!outfile) {
        LOG_F(ERROR, "Error creating file \"{}\"!", filename);
        return false;
    }
    DLOG_F(INFO, "Created file \"{}\"", filename);
    return true;
}

bool FileManager::openFile(const std::string &filename) {
    if (!fs::exists(filename)) {
        LOG_F(ERROR, "File \"{}\" does not exist!", filename);
        return false;
    }
    m_filename = filename;
    m_file.open(filename, std::ios::in | std::ios::out);
    if (!m_file) {
        LOG_F(ERROR, "Could not open file \"{}\"!", filename);
        return false;
    }
    DLOG_F(INFO, "Opened file \"{}\"", filename);
    return true;
}

bool FileManager::readFile(std::string &contents) {
    if (!m_file.is_open()) {
        LOG_F(ERROR, "No file is currently open!");
        return false;
    }
    contents = std::string(std::istreambuf_iterator<char>(m_file),
                           std::istreambuf_iterator<char>());
    DLOG_F(INFO, "Read contents of file \"{}\"", m_filename);
    return true;
}

bool FileManager::writeFile(const std::string &contents) {
    if (!m_file.is_open()) {
        LOG_F(ERROR, "No file is currently open!");
        return false;
    }
    m_file << contents;
    DLOG_F(INFO, "Wrote contents to file \"{}\"", m_filename);
    return true;
}

bool FileManager::moveFile(const std::string &oldFilename,
                           const std::string &newFilename) {
    if (!fs::exists(oldFilename)) {
        LOG_F(ERROR, "File \"{}\" does not exist!", oldFilename);
        return false;
    }
    if (fs::exists(newFilename)) {
        LOG_F(ERROR, "File \"{}\" already exists!", newFilename);
        return false;
    }
    std::error_code ec;
    fs::rename(oldFilename, newFilename, ec);
    if (ec) {
        LOG_F(ERROR, "Could not move file from \"{}\" to \"{}\"!", oldFilename,
              newFilename);
        return false;
    }
    DLOG_F(INFO, "Moved file from \"{}\" to \"{}\"", oldFilename, newFilename);
    return true;
}

bool FileManager::deleteFile(const std::string &filename) {
    if (!fs::exists(filename)) {
        LOG_F(ERROR, "File \"{}\" does not exist!", filename);
        return false;
    }
    std::error_code ec;
    fs::remove(filename, ec);
    if (ec) {
        LOG_F(ERROR, "Could not delete file \"{}\"!", filename);
        return false;
    }
    DLOG_F(INFO, "Deleted file \"{}\"", filename);
    return true;
}

long FileManager::getFileSize() {
    if (!m_file.is_open()) {
        LOG_F(ERROR, "No file is currently open!");
        return -1;
    }
    auto fileSize = fs::file_size(m_filename);
    if (fileSize == static_cast<uintmax_t>(-1)) {
        LOG_F(ERROR, "Could not get file size of \"{}\"!", m_filename);
    } else {
        DLOG_F(INFO, "File size of \"{}\" is {} bytes", m_filename, fileSize);
    }
    return static_cast<long>(fileSize);
}

std::string FileManager::getFileDirectory(const std::string &filename) {
    auto parentPath = fs::path(filename).parent_path();
    if (parentPath.empty()) {
        LOG_F(ERROR, "Could not get directory of file \"{}\"", filename);
        return "";
    } else {
        DLOG_F(INFO, "Directory of file \"{}\" is \"{}\"", filename,
               parentPath.string());
        return parentPath.string();
    }
}

}  // namespace atom::io
