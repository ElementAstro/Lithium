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

#include <openssl/evp.h>

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
            LOG_F(ERROR, "File \"{}\" already exists!", filename);
            return false;
        }
        std::ofstream outfile(filename);
        if (!outfile)
        {
            LOG_F(ERROR, "Error creating file \"{}\"!", filename);
            return false;
        }
        outfile.close();
        std::fclose(std::fopen(filename.c_str(), "a")); // create a link to the file
        DLOG_F(INFO, "Created file \"{}\"", filename);
        return true;
    }

    bool FileManager::openFile(const std::string &filename)
    {
        if (!fileExists(filename))
        {
            LOG_F(ERROR, "File \"{}\" does not exist!", filename);
            return false;
        }
        m_filename = filename;
        m_file.open(filename, std::ios::in | std::ios::out);
        if (!m_file)
        {
            LOG_F(ERROR, "Could not open file \"{}\"!", filename);
            return false;
        }
        DLOG_F(INFO, "Opened file \"{}\"", filename);
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
        DLOG_F(INFO, "Read contents of file \"{}\"", m_filename);
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
        DLOG_F(INFO, "Wrote contents to file \"{}\"", m_filename);
        return true;
    }

    bool FileManager::moveFile(const std::string &oldFilename, const std::string &newFilename)
    {
        if (!fileExists(oldFilename))
        {
            LOG_F(ERROR, "File \"{}\" does not exist!", oldFilename);
            return false;
        }
        if (fileExists(newFilename))
        {
            LOG_F(ERROR, "File \"{}\" already exists!", newFilename);
            return false;
        }
        int result = std::rename(oldFilename.c_str(), newFilename.c_str());
        if (result != 0)
        {
            LOG_F(ERROR, "Could not move file from \"{}\" to \"{}\"!", oldFilename, newFilename);
            return false;
        }
        DLOG_F(INFO, "Moved file from \"{}\" to \"{}\"", oldFilename, newFilename);
        return true;
    }

    bool FileManager::deleteFile(const std::string &filename)
    {
        if (!fileExists(filename))
        {
            LOG_F(ERROR, "File \"{}\" does not exist!", filename);
            return false;
        }
        if (std::remove(filename.c_str()) != 0)
        {
            LOG_F(ERROR, "Could not delete file \"{}\"!", filename);
            return false;
        }
        DLOG_F(INFO, "Deleted file \"{}\"", filename);
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
            LOG_F(ERROR, "Could not get file size of \"{}\"!", m_filename);
        }
        else
        {
            DLOG_F(INFO, "File size of \"{}\" is %ld bytes", m_filename, fileSize);
        }
        return fileSize;
    }

    std::string FileManager::calculateSHA256()
    {
        if (!m_file.is_open())
        {
            LOG_F(ERROR, "No file is currently open!");
            return "";
        }

        EVP_MD_CTX *mdContext = EVP_MD_CTX_new();
        if (mdContext == nullptr)
        {
            LOG_F(ERROR, "Failed to create EVP_MD_CTX");
            return "";
        }

        if (EVP_DigestInit_ex(mdContext, EVP_sha256(), nullptr) != 1)
        {
            LOG_F(ERROR, "Failed to initialize EVP_MD_CTX");
            EVP_MD_CTX_free(mdContext);
            return "";
        }

        char buffer[1024];
        while (m_file.read(buffer, sizeof(buffer)))
        {
            if (EVP_DigestUpdate(mdContext, buffer, sizeof(buffer)) != 1)
            {
                LOG_F(ERROR, "Failed to update EVP_MD_CTX");
                EVP_MD_CTX_free(mdContext);
                return "";
            }
        }

        unsigned char hash[EVP_MAX_MD_SIZE];
        unsigned int hashLength = 0;
        if (EVP_DigestFinal_ex(mdContext, hash, &hashLength) != 1)
        {
            LOG_F(ERROR, "Failed to finalize EVP_MD_CTX");
            EVP_MD_CTX_free(mdContext);
            return "";
        }

        EVP_MD_CTX_free(mdContext);

        std::stringstream sha256Stream;
        sha256Stream << std::hex << std::setfill('0');
        for (unsigned int i = 0; i < hashLength; ++i)
        {
            sha256Stream << std::setw(2) << static_cast<int>(hash[i]);
        }

        DLOG_F(INFO, "SHA-256 value for file \"{}\" is {}", m_filename, sha256Stream.str());
        return sha256Stream.str();
    }

    std::string FileManager::getFileDirectory(const std::string &filename)
    {
        size_t pos = filename.find_last_of("/\\");
        if (pos == std::string::npos)
        {
            LOG_F(ERROR, "Could not get directory of file \"{}\"", filename);
            return "";
        }
        else
        {
            std::string directory = filename.substr(0, pos);
            DLOG_F(INFO, "Directory of file \"{}\" is \"{}\"", filename, directory);
            return directory;
        }
    }

}