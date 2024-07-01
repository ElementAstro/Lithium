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
#include <iostream>

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

namespace atom::io {

auto FileManager::createFile(const std::string &filename) -> bool {
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

auto FileManager::openFile(const std::string &filename) -> bool {
    if (!fs::exists(filename)) {
        LOG_F(ERROR, "File \"{}\" does not exist!", filename);
        return false;
    }
    m_filename_ = filename;
    m_file_.open(filename, std::ios::in | std::ios::out);
    if (!m_file_) {
        LOG_F(ERROR, "Could not open file \"{}\"!", filename);
        return false;
    }
    DLOG_F(INFO, "Opened file \"{}\"", filename);
    return true;
}

auto FileManager::readFile(std::string &contents) -> bool {
    if (!m_file_.is_open()) {
        LOG_F(ERROR, "No file is currently open!");
        return false;
    }
    contents = std::string(std::istreambuf_iterator<char>(m_file_),
                           std::istreambuf_iterator<char>());
    DLOG_F(INFO, "Read contents of file \"{}\"", m_filename_);
    return true;
}

auto FileManager::writeFile(const std::string &contents) -> bool {
    if (!m_file_.is_open()) {
        LOG_F(ERROR, "No file is currently open!");
        return false;
    }
    m_file_ << contents;
    DLOG_F(INFO, "Wrote contents to file \"{}\"", m_filename_);
    return true;
}

auto FileManager::moveFile(const std::string &oldFilename,
                           const std::string &newFilename) -> bool {
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

auto FileManager::deleteFile(const std::string &filename) -> bool {
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

auto FileManager::getFileSize() -> long {
    if (!m_file_.is_open()) {
        LOG_F(ERROR, "No file is currently open!");
        return -1;
    }
    auto fileSize = fs::file_size(m_filename_);
    if (fileSize == static_cast<uintmax_t>(-1)) {
        LOG_F(ERROR, "Could not get file size of \"{}\"!", m_filename_);
    } else {
        DLOG_F(INFO, "File size of \"{}\" is {} bytes", m_filename_, fileSize);
    }
    return static_cast<long>(fileSize);
}

auto FileManager::getFileDirectory(const std::string &filename) -> std::string {
    auto parentPath = fs::path(filename).parent_path();
    if (parentPath.empty()) {
        LOG_F(ERROR, "Could not get directory of file \"{}\"", filename);
        return "";
    }
    DLOG_F(INFO, "Directory of file \"{}\" is \"{}\"", filename,
           parentPath.string());
    return parentPath.string();
}

}  // namespace atom::io
