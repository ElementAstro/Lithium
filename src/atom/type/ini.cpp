/*
 * ini.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: INI File Read/Write Library

**************************************************/

#include "ini.hpp"

#include <fstream>
#include <sstream>

#include "atom/error/exception.hpp"
#include "atom/utils/string.hpp"

namespace atom::type {

auto INIFile::has(const std::string &section,
                  const std::string &key) const -> bool {
    std::shared_lock lock(m_sharedMutex_);
    if (auto it = data_.find(section); it != data_.end()) {
        return it->second.contains(key);
    }
    return false;
}

auto INIFile::hasSection(const std::string &section) const -> bool {
    std::shared_lock lock(m_sharedMutex_);
    return data_.contains(section);
}

auto INIFile::sections() const -> std::vector<std::string> {
    std::shared_lock lock(m_sharedMutex_);
    std::vector<std::string> result;
    result.reserve(data_.size());
    for (const auto &[section, _] : data_) {
        result.emplace_back(section);
    }
    return result;
}

auto INIFile::keys(const std::string &section) const
    -> std::vector<std::string> {
    std::shared_lock lock(m_sharedMutex_);
    if (auto it = data_.find(section); it != data_.end()) {
        std::vector<std::string> result;
        result.reserve(it->second.size());
        for (const auto &[key, _] : it->second) {
            result.emplace_back(key);
        }
        return result;
    }
    return {};
}

void INIFile::load(const std::string &filename) {
    std::unique_lock lock(m_sharedMutex_);
    std::ifstream file(filename);
    if (!file) {
        THROW_EXCEPTION("Failed to open file: " + filename);
    }

    std::string currentSection;
    for (std::string line; std::getline(file, line);) {
        parseLine(line, currentSection);
    }
}

void INIFile::save(const std::string &filename) const {
    std::shared_lock lock(m_sharedMutex_);
    std::ofstream file(filename);
    if (!file) {
        THROW_FILE_NOT_WRITABLE("Failed to create file: ", filename);
    }

    for (const auto &[section, entries] : data_) {
        file << "[" << section << "]\n";
        for (const auto &[key, value] : entries) {
            file << key << "=";
            if (value.type() == typeid(int)) {
                file << std::any_cast<int>(value);
            } else if (value.type() == typeid(float)) {
                file << std::any_cast<float>(value);
            } else if (value.type() == typeid(double)) {
                file << std::any_cast<double>(value);
            } else if (value.type() == typeid(std::string)) {
                file << std::any_cast<std::string>(value);
            } else if (value.type() == typeid(const char *)) {
                file << std::any_cast<const char *>(value);
            } else if (value.type() == typeid(bool)) {
                file << std::boolalpha << std::any_cast<bool>(value);
            } else {
                THROW_INVALID_ARGUMENT("Unsupported type");
            }
            file << "\n";
        }
        file << "\n";
    }
}

void INIFile::parseLine(std::string_view line, std::string &currentSection) {
    if (line.empty() || line.front() == ';') {
        return;
    }
    if (line.front() == '[') {
        auto pos = line.find(']');
        if (pos != std::string_view::npos) {
            currentSection = atom::utils::trim(line.substr(1, pos - 1));
        }
    } else {
        auto pos = line.find('=');
        if (pos != std::string_view::npos) {
            auto key = atom::utils::trim(line.substr(0, pos));
            auto value = atom::utils::trim(line.substr(pos + 1));
            data_[currentSection][key] = value;
        }
    }
}

auto INIFile::toJson() const -> std::string {
    std::shared_lock lock(m_sharedMutex_);
    std::ostringstream oss;
    oss << "{";
    for (const auto &[section, entries] : data_) {
        oss << "\"" << section << "\": {";
        for (const auto &[key, value] : entries) {
            oss << "\"" << key << "\": ";
            if (value.type() == typeid(int)) {
                oss << std::any_cast<int>(value);
            } else if (value.type() == typeid(float)) {
                oss << std::any_cast<float>(value);
            } else if (value.type() == typeid(double)) {
                oss << std::any_cast<double>(value);
            } else if (value.type() == typeid(std::string)) {
                oss << "\"" << std::any_cast<std::string>(value) << "\"";
            } else if (value.type() == typeid(const char *)) {
                oss << "\"" << std::any_cast<const char *>(value) << "\"";
            } else if (value.type() == typeid(bool)) {
                oss << std::boolalpha << std::any_cast<bool>(value);
            } else {
                THROW_INVALID_ARGUMENT("Unsupported type");
            }
            oss << ",";
        }
        oss.seekp(-1, std::ios_base::end);
        oss << "},";
    }
    oss.seekp(-1, std::ios_base::end);
    oss << "}";
    return oss.str();
}

auto INIFile::toXml() const -> std::string {
    std::shared_lock lock(m_sharedMutex_);
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    oss << "<config>\n";
    for (const auto &[section, entries] : data_) {
        oss << "  <section name=\"" << section << "\">\n";
        for (const auto &[key, value] : entries) {
            oss << "    <entry name=\"" << key << "\" type=\"";
            if (value.type() == typeid(int)) {
                oss << "int\">" << std::any_cast<int>(value);
            } else if (value.type() == typeid(float)) {
                oss << "float\">" << std::any_cast<float>(value);
            } else if (value.type() == typeid(double)) {
                oss << "double\">" << std::any_cast<double>(value);
            } else if (value.type() == typeid(std::string)) {
                oss << "string\">" << std::any_cast<std::string>(value);
            } else if (value.type() == typeid(const char *)) {
                oss << "string\">" << std::any_cast<const char *>(value);
            } else if (value.type() == typeid(bool)) {
                oss << "bool\">" << std::boolalpha
                    << std::any_cast<bool>(value);
            } else {
                THROW_INVALID_ARGUMENT("Unsupported type");
            }
            oss << "</entry>\n";
        }
        oss << "  </section>\n";
    }
    oss << "</config>\n";
    return oss.str();
}

}  // namespace atom::type