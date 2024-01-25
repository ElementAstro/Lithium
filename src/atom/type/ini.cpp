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

#include "atom/utils/exception.hpp"

bool INIFile::has(const std::string &section, const std::string &key) const
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = data.find(section);
    if (it != data.end())
    {
        auto entryIt = it->second.find(key);
        if (entryIt != it->second.end())
        {
            if (entryIt->second.has_value())
            {
                return true;
            }
        }
    }
    return false;
}

bool INIFile::hasSection(const std::string &section) const
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    return data.find(section) != data.end();
}

void INIFile::load(const std::string &filename)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw Atom::Utils::Exception::FileNotReadable_Error("Failed to open file: " + filename);
    }

    std::string line;
    std::string currentSection;
    while (std::getline(file, line))
    {
        parseLine(line, currentSection);
    }

    file.close();
}

void INIFile::save(const std::string &filename)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    std::ofstream file(filename);
    if (!file.is_open())
    {
        throw Atom::Utils::Exception::FileNotWritable_Error("Failed to create file: " + filename);
    }

    for (const auto &section : data)
    {
        file << "[" << section.first << "]\n";
        for (const auto &entry : section.second)
        {
            if (entry.second.type() == typeid(int))
            {
                file << entry.first << "=" << std::any_cast<int>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(float))
            {
                file << entry.first << "=" << std::any_cast<float>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(double))
            {
                file << entry.first << "=" << std::any_cast<double>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(std::string))
            {
                file << entry.first << "=" << std::any_cast<std::string>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(const char *))
            {
                file << entry.first << "=" << std::any_cast<const char *>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(bool))
            {
                file << entry.first << "=" << std::any_cast<bool>(entry.second) << "\n";
            }
            else
            {
                throw Atom::Utils::Exception::InvalidArgument_Error("Unsupported type");
            }
        }
        file << "\n";
    }

    file.close();
}

void INIFile::parseLine(const std::string &line, std::string &currentSection)
{
    std::stringstream ss(line);
    std::string token;
    if (std::getline(ss, token, '='))
    {
        // 去除字符串前后的空格和制表符
        token = trim(token);
        if (token[0] == '[' && token[token.length() - 1] == ']')
        {
            currentSection = token.substr(1, token.length() - 2);
        }
        else
        {
            std::string key = trim(token);
            std::string value;
            if (std::getline(ss, value))
            {
                value = trim(value);
                data[currentSection][key] = value;
            }
        }
    }
}

std::string INIFile::trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos)
    {
        return "";
    }
    else
    {
        return str.substr(start, end - start + 1);
    }
}

std::string INIFile::toJson() const
{
    std::ostringstream oss;
    oss << "{";
    for (const auto &section : data)
    {
        oss << "\"" << section.first << "\": {";
        for (const auto &entry : section.second)
        {
            try
            {
                if (entry.second.type() == typeid(int))
                {
                    oss << "\"" << entry.first << "\": " << std::any_cast<int>(entry.second) << ", ";
                }
                else if (entry.second.type() == typeid(float))
                {
                    oss << "\"" << entry.first << "\": " << std::any_cast<float>(entry.second) << ", ";
                }
                else if (entry.second.type() == typeid(double))
                {
                    oss << "\"" << entry.first << "\": " << std::any_cast<double>(entry.second) << ", ";
                }
                else if (entry.second.type() == typeid(std::string))
                {
                    oss << "\"" << entry.first << "\": \"" << std::any_cast<std::string>(entry.second) << "\", ";
                }
                else if (entry.second.type() == typeid(const char *))
                {
                    oss << "\"" << entry.first << "\": \"" << std::any_cast<const char *>(entry.second) << "\", ";
                }
                else if (entry.second.type() == typeid(bool))
                {
                    oss << "\"" << entry.first << "\": " << std::any_cast<bool>(entry.second) << ", ";
                }
            }
            catch (const std::bad_any_cast &e)
            {
                throw Atom::Utils::Exception::InvalidArgument_Error("Unsupported type");
            }
        }
    }
    oss << "}";
    return oss.str();
}

std::string INIFile::toXml() const
{
    std::ostringstream oss;
    oss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    oss << "<config>\n";
    for (const auto &section : data)
    {
        oss << "  <section name=\"" << section.first << "\">\n";
        for (const auto &entry : section.second)
        {
            try
            {
                if (entry.second.type() == typeid(int))
                {
                    oss << "    <entry name=\"" << entry.first << "\" type=\"int\">" << std::any_cast<int>(entry.second) << "</entry>\n";
                }
                else if (entry.second.type() == typeid(float))
                {
                    oss << "    <entry name=\"" << entry.first << "\" type=\"float\">" << std::any_cast<float>(entry.second) << "</entry>\n";
                }
                else if (entry.second.type() == typeid(double))
                {
                    oss << "    <entry name=\"" << entry.first << "\" type=\"double\">" << std::any_cast<double>(entry.second) << "</entry>\n";
                }
                else if (entry.second.type() == typeid(std::string))
                {
                    oss << "    <entry name=\"" << entry.first << "\" type=\"string\">" << std::any_cast<std::string>(entry.second) << "</entry>\n";
                }
                else if (entry.second.type() == typeid(const char *))
                {
                    oss << "    <entry name=\"" << entry.first << "\" type=\"string\">" << std::any_cast<const char *>(entry.second) << "</entry>\n";
                }
                else if (entry.second.type() == typeid(bool))
                {
                    oss << "    <entry name=\"" << entry.first << "\" type=\"bool\">" << std::any_cast<bool>(entry.second) << "</entry>\n";
                }
                        }
            catch (const std::bad_any_cast &e)
            {
                throw Atom::Utils::Exception::InvalidArgument_Error("Unsupported type");
            }
        }
        oss << "  </section>\n";
    }
    oss << "</config>\n";
    return oss.str();
}

/*
int main()
{
    INIFile ini;
    try
    {
        ini.load("config.ini");

        // 获取配置项
        std::optional<std::string> usernameOpt = ini.get<std::string>("User", "Username");
        std::optional<std::string> passwordOpt = ini.get<std::string>("User", "Password");

        if (usernameOpt.has_value() && passwordOpt.has_value())
        {
            std::cout << "Username: " << usernameOpt.value() << std::endl;
            std::cout << "Password: " << passwordOpt.value() << std::endl;
        }
        else
        {
            std::cout << "Username or Password not found" << std::endl;
        }

        // 修改配置项
        ini.set("User", "Password", std::string("new_pas"));

        // 存储到文件
        ini.save("config_modified.ini");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

*/