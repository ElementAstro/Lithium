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

#include "loguru/loguru.hpp"

#include <openssl/md5.h>

namespace Lithium::File
{

    bool fileExists(const std::string &filename)
    {
        return std::filesystem::exists(filename);
    }

    FileManager::FileManager() : m_file()
    {
    }

    bool FileManager::createFile(const std::string &filename)
    {
        if (fileExists(filename))
        {
            LOG_F(ERROR, "File \"%s\" already exists!", filename.c_str());
            return false;
        }
        std::ofstream outfile(filename);
        if (!outfile)
        {
            LOG_F(ERROR, "Error creating file \"%s\"!", filename.c_str());
            return false;
        }
        outfile.close();
        std::fclose(std::fopen(filename.c_str(), "a")); // create a link to the file
        LOG_F(INFO, "Created file \"%s\"", filename.c_str());
        return true;
    }

    bool FileManager::openFile(const std::string &filename)
    {
        if (!fileExists(filename))
        {
            LOG_F(ERROR, "File \"%s\" does not exist!", filename.c_str());
            return false;
        }
        m_filename = filename;
        m_file.open(filename, std::ios::in | std::ios::out);
        if (!m_file)
        {
            LOG_F(ERROR, "Could not open file \"%s\"!", filename.c_str());
            return false;
        }
        LOG_F(INFO, "Opened file \"%s\"", filename.c_str());
        return true;
    }

    bool FileManager::readFile(std::string &contents)
    {
        if (!m_file.is_open())
        {
            LOG_F(ERROR, "No file is currently open!");
            return false;
        }
        std::stringstream buffer;
        buffer << m_file.rdbuf();
        contents = buffer.str();
        LOG_F(INFO, "Read contents of file \"%s\"", m_filename.c_str());
        return true;
    }

    bool FileManager::writeFile(const std::string &contents)
    {
        if (!m_file.is_open())
        {
            LOG_F(ERROR, "No file is currently open!");
            return false;
        }
        m_file << contents;
        LOG_F(INFO, "Wrote contents to file \"%s\"", m_filename.c_str());
        return true;
    }

    bool FileManager::moveFile(const std::string &oldFilename, const std::string &newFilename)
    {
        if (!fileExists(oldFilename))
        {
            LOG_F(ERROR, "File \"%s\" does not exist!", oldFilename.c_str());
            return false;
        }
        if (fileExists(newFilename))
        {
            LOG_F(ERROR, "File \"%s\" already exists!", newFilename.c_str());
            return false;
        }
        int result = std::rename(oldFilename.c_str(), newFilename.c_str());
        if (result != 0)
        {
            LOG_F(ERROR, "Could not move file from \"%s\" to \"%s\"!", oldFilename.c_str(), newFilename.c_str());
            return false;
        }
        LOG_F(INFO, "Moved file from \"%s\" to \"%s\"", oldFilename.c_str(), newFilename.c_str());
        return true;
    }

    bool FileManager::deleteFile(const std::string &filename)
    {
        if (!fileExists(filename))
        {
            LOG_F(ERROR, "File \"%s\" does not exist!", filename.c_str());
            return false;
        }
        if (std::remove(filename.c_str()) != 0)
        {
            LOG_F(ERROR, "Could not delete file \"%s\"!", filename.c_str());
            return false;
        }
        LOG_F(INFO, "Deleted file \"%s\"", filename.c_str());
        return true;
    }

    long FileManager::getFileSize()
    {
        if (!m_file.is_open())
        {
            LOG_F(ERROR, "No file is currently open!");
            return -1;
        }
        m_file.seekg(0, m_file.end);
        long fileSize = m_file.tellg();
        m_file.seekg(0, m_file.beg);
        if (fileSize == -1)
        {
            LOG_F(ERROR, "Could not get file size of \"%s\"!", m_filename.c_str());
        }
        else
        {
            LOG_F(INFO, "File size of \"%s\" is %ld bytes", m_filename.c_str(), fileSize);
        }
        return fileSize;
    }

    std::string FileManager::calculateMD5()
    {
        if (!m_file.is_open())
        {
            LOG_F(ERROR, "No file is currently open!");
            return "";
        }
        MD5_CTX md5Context;
        MD5_Init(&md5Context);
        char buffer[1024];
        while (m_file.read(buffer, sizeof(buffer)))
        {
            MD5_Update(&md5Context, buffer, sizeof(buffer));
        }
        MD5_Final(reinterpret_cast<unsigned char *>(buffer), &md5Context);
        std::stringstream md5Stream;
        md5Stream << std::hex << std::setfill('0');
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        {
            md5Stream << std::setw(2) << static_cast<int>(buffer[i]);
        }
        LOG_F(INFO, "MD5 value for file \"%s\" is %s", m_filename.c_str(), md5Stream.str().c_str());
        return md5Stream.str();
    }

    std::string FileManager::getFileDirectory(const std::string &filename)
    {
        size_t pos = filename.find_last_of("/\\");
        if (pos == std::string::npos)
        {
            LOG_F(ERROR, "Could not get directory of file \"%s\"", filename.c_str());
            return "";
        }
        else
        {
            std::string directory = filename.substr(0, pos);
            LOG_F(INFO, "Directory of file \"%s\" is \"%s\"", filename.c_str(), directory.c_str());
            return directory;
        }
    }

}