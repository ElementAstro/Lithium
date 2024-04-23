/*
 * idirectory.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-23

Description: Directory Wrapper

**************************************************/

#include "idirectory.hpp"

#include <iostream>
#include <stdexcept>

namespace Atom::IO {
DirectoryWrapper::DirectoryWrapper(const fs::path& dir_path)
    : dir_path_(dir_path) {}

DirectoryWrapper::DirectoryWrapper(const std::string& dir_path)
    : dir_path_(dir_path) {}

bool DirectoryWrapper::exists() const {
    return fs::exists(dir_path_) && fs::is_directory(dir_path_);
}

void DirectoryWrapper::remove() {
    if (exists()) {
        fs::remove_all(dir_path_);
    }
}

fs::path DirectoryWrapper::get_path() const { return dir_path_; }

uintmax_t DirectoryWrapper::get_size() const {
    uintmax_t size = 0;
    for (const auto& entry : fs::recursive_directory_iterator(dir_path_)) {
        if (fs::is_regular_file(entry)) {
            size += fs::file_size(entry);
        }
    }
    return size;
}

std::string DirectoryWrapper::get_size_string() const {
    uintmax_t size = get_size();
    if (size < 1024) {
        return std::to_string(size) + " B";
    } else if (size < 1024 * 1024) {
        return std::to_string(size / 1024) + " KB";
    } else if (size < 1024 * 1024 * 1024) {
        return std::to_string(size / (1024 * 1024)) + " MB";
    } else {
        return std::to_string(size / (1024 * 1024 * 1024)) + " GB";
    }
}

std::vector<fs::path> DirectoryWrapper::list_files() const {
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(dir_path_)) {
        if (fs::is_regular_file(entry)) {
            files.push_back(entry.path());
        }
    }
    return files;
}

std::vector<fs::path> DirectoryWrapper::list_directories() const {
    std::vector<fs::path> directories;
    for (const auto& entry : fs::directory_iterator(dir_path_)) {
        if (fs::is_directory(entry)) {
            directories.push_back(entry.path());
        }
    }
    return directories;
}

void DirectoryWrapper::create_directory(const std::string& name) const {
    fs::create_directory(dir_path_ / name);
}
}  // namespace Atom::IO
