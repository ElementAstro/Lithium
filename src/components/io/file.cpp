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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <filesystem>

#include <openssl/md5.h>

namespace Lithium::File
{


bool fileExists(const std::string& filename)
{
    return std::filesystem::exists(filename);
}

FileManager::FileManager() : m_file()
{
}

bool FileManager::createFile(const std::string& filename)
{
    if (fileExists(filename))
    {
        std::cerr << "File \"" << filename << "\" already exists!" << std::endl;
        return false;
    }
    std::ofstream outfile(filename);
    if (!outfile)
    {
        std::cerr << "Error creating file \"" << filename << "\"!" << std::endl;
        return false;
    }
    outfile.close();
    std::fclose(std::fopen(filename.c_str(), "a")); // create a link to the file
    std::cout << "Created file \"" << filename << "\"" << std::endl;
    return true;
}

bool FileManager::openFile(const std::string& filename)
{
    if (!fileExists(filename))
    {
        std::cerr << "File \"" << filename << "\" does not exist!" << std::endl;
        return false;
    }
    m_filename = filename;
    m_file.open(filename, std::ios::in | std::ios::out);
    if (!m_file)
    {
        std::cerr << "Could not open file \"" << filename << "\"!" << std::endl;
        return false;
    }
    std::cout << "Opened file \"" << filename << "\"" << std::endl;
    return true;
}

bool FileManager::readFile(std::string& contents)
{
    if (!m_file.is_open())
    {
        std::cerr << "No file is currently open!" << std::endl;
        return false;
    }
    std::stringstream buffer;
    buffer << m_file.rdbuf();
    contents = buffer.str();
    std::cout << "Read contents of file \"" << m_filename << "\"" << std::endl;
    return true;
}

bool FileManager::writeFile(const std::string& contents)
{
    if (!m_file.is_open())
    {
        std::cerr << "No file is currently open!" << std::endl;
        return false;
    }
    m_file << contents;
    std::cout << "Wrote contents to file \"" << m_filename << "\"" << std::endl;
    return true;
}

bool FileManager::moveFile(const std::string& oldFilename, const std::string& newFilename)
{
    if (!fileExists(oldFilename))
    {
        std::cerr << "File \"" << oldFilename << "\" does not exist!" << std::endl;
        return false;
    }
    if (fileExists(newFilename))
    {
        std::cerr << "File \"" << newFilename << "\" already exists!" << std::endl;
        return false;
    }
    int result = std::rename(oldFilename.c_str(), newFilename.c_str());
    if (result != 0)
    {
        std::cerr << "Could not move file from \"" << oldFilename << "\" to \"" << newFilename << "\"!" << std::endl;
        return false;
    }
    std::cout << "Moved file from \"" << oldFilename << "\" to \"" << newFilename << "\"" << std::endl;
    return true;
}

bool FileManager::deleteFile(const std::string& filename)
{
    if (!fileExists(filename))
    {
        std::cerr << "File \"" << filename << "\" does not exist!" << std::endl;
        return false;
    }
    if (std::remove(filename.c_str()) != 0)
    {
        std::cerr << "Could not delete file \"" << filename << "\"!" << std::endl;
        return false;
    }
    std::cout << "Deleted file \"" << filename << "\"" << std::endl;
    return true;
}

long FileManager::getFileSize()
{
    if (!m_file.is_open())
    {
        std::cerr << "No file is currently open!" << std::endl;
        return -1;
    }
    m_file.seekg(0, m_file.end);
    long fileSize = m_file.tellg();
    m_file.seekg(0, m_file.beg);
    if (fileSize == -1)
    {
        std::cerr << "Could not get file size of \"" << m_filename << "\"!" << std::endl;
    }
    else
    {
        std::cout << "File size of \"" << m_filename << "\" is " << fileSize << " bytes" << std::endl;
    }
    return fileSize;
}

std::string FileManager::calculateMD5()
{
    if (!m_file.is_open())
    {
        std::cerr << "No file is currently open!" << std::endl;
        return "";
    }
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    char buffer[1024];
    while (m_file.read(buffer, sizeof(buffer)))
    {
        MD5_Update(&md5Context, buffer, sizeof(buffer));
    }
    MD5_Final(reinterpret_cast<unsigned char*>(buffer), &md5Context);
    std::stringstream md5Stream;
    md5Stream << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        md5Stream << std::setw(2) << static_cast<int>(buffer[i]);
    }
    std::cout << "MD5 value for file \"" << m_filename << "\" is " << md5Stream.str() << std::endl;
    return md5Stream.str();
}

std::string FileManager::getFileDirectory(const std::string& filename)
{
    size_t pos = filename.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        std::cerr << "Could not get directory of file \"" << filename << "\"" << std::endl;
        return "";
    }
    else
    {
        std::string directory = filename.substr(0, pos);
        std::cout << "Directory of file \"" << filename << "\" is \"" << directory << "\"" << std::endl;
        return directory;
    }
}

}