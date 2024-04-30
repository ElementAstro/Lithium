/*
 * ifile.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-23

Description: File Wrapper

**************************************************/

#include "ifile.hpp"

#include <fstream>
#include <iostream>

namespace atom::io {
FileWrapper::FileWrapper(const fs::path& file_path) : file_path_(file_path) {}

void FileWrapper::write(const std::string& content) { write_file(content); }

void FileWrapper::write(const std::vector<uint8_t>& content) {
    write_file(content);
}

std::variant<std::string, std::vector<uint8_t>> FileWrapper::read() {
    if (is_binary_file()) {
        return read_file<std::vector<uint8_t>>();
    } else {
        return read_file<std::string>();
    }
}

bool FileWrapper::exists() const { return fs::exists(file_path_); }

void FileWrapper::remove() {
    if (exists()) {
        fs::remove(file_path_);
    }
}

fs::path FileWrapper::get_path() const { return file_path_; }

bool FileWrapper::is_binary_file() const {
    std::ifstream file(file_path_, std::ios::binary);
    if (file.is_open()) {
        char byte;
        while (file.get(byte)) {
            if (byte == '\0') {
                return true;
            }
        }
    }
    return false;
}

uintmax_t FileWrapper::get_size() const { return fs::file_size(file_path_); }

std::string FileWrapper::get_size_string() const {
    auto size = get_size();
    std::stringstream ss;
    if (size < 1024) {
        ss << size << " B";
    } else if (size < 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << size / 1024.0 << " KB";
    } else if (size < 1024 * 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << size / (1024.0 * 1024.0)
           << " MB";
    } else {
        ss << std::fixed << std::setprecision(2)
           << size / (1024.0 * 1024.0 * 1024.0) << " GB";
    }
    return ss.str();
}

std::string FileWrapper::get_last_write_time() const {
    auto last_write_time = fs::last_write_time(file_path_);
    auto time_point =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            last_write_time - fs::file_time_type::clock::now() +
            std::chrono::system_clock::now());
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void FileWrapper::rename(const fs::path& new_path) {
    fs::rename(file_path_, new_path);
    file_path_ = new_path;
}

void FileWrapper::copy_to(const fs::path& destination) const {
    fs::copy_file(file_path_, destination,
                  fs::copy_options::overwrite_existing);
}

void FileWrapper::move_to(const fs::path& destination) {
    fs::rename(file_path_, destination);
    file_path_ = destination;
}

bool FileWrapper::is_empty() const { return get_size() == 0; }

void FileWrapper::append(const std::string& content) {
    std::ofstream file(file_path_, std::ios::app);
    if (!file) {
        throw std::runtime_error("无法打开文件进行追加: " +
                                 file_path_.string());
    }
    file << content;
}

void FileWrapper::append(const std::vector<uint8_t>& content) {
    std::ofstream file(file_path_, std::ios::app | std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件进行追加: " +
                                 file_path_.string());
    }
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
}

void FileWrapper::write_at(const std::string& content,
                           std::streampos position) {
    std::fstream file(file_path_, std::ios::in | std::ios::out);
    if (!file) {
        throw std::runtime_error("无法打开文件进行写入: " +
                                 file_path_.string());
    }
    file.seekp(position);
    file << content;
}

void FileWrapper::write_at(const std::vector<uint8_t>& content,
                           std::streampos position) {
    std::fstream file(file_path_,
                      std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件进行写入: " +
                                 file_path_.string());
    }
    file.seekp(position);
    file.write(reinterpret_cast<const char*>(content.data()), content.size());
}

std::variant<std::string, std::vector<uint8_t>> FileWrapper::read_from(
    std::streampos start, std::streamsize count) {
    std::ifstream file(file_path_, std::ios::binary);
    if (!file) {
        throw std::runtime_error("无法打开文件进行读取: " +
                                 file_path_.string());
    }
    file.seekg(start);
    if (is_binary_file()) {
        std::vector<uint8_t> content(count);
        file.read(reinterpret_cast<char*>(content.data()), count);
        return content;
    } else {
        std::string content(count, '\0');
        file.read(&content[0], count);
        return content;
    }
}

std::string FileWrapper::get_extension() const {
    return file_path_.extension().string();
}

std::string FileWrapper::get_stem() const { return file_path_.stem().string(); }

std::string FileWrapper::get_parent_path() const {
    return file_path_.parent_path().string();
}

bool FileWrapper::is_directory() const { return fs::is_directory(file_path_); }

bool FileWrapper::is_regular_file() const {
    return fs::is_regular_file(file_path_);
}

bool FileWrapper::is_symlink() const { return fs::is_symlink(file_path_); }

std::uintmax_t FileWrapper::get_hard_link_count() const {
    return fs::hard_link_count(file_path_);
}

void FileWrapper::create_symlink(const fs::path& target) {
    fs::create_symlink(target, file_path_);
}

void FileWrapper::create_hardlink(const fs::path& target) {
    fs::create_hard_link(target, file_path_);
}

void FileWrapper::permissions(fs::perms prms) {
    fs::permissions(file_path_, prms);
}

fs::perms FileWrapper::permissions() const {
    return fs::status(file_path_).permissions();
}

}  // namespace atom::io
