/*
 * compress.cpp
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

Date: 2023-3-31

Description: Compressor using ZLib

**************************************************/

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <stdexcept>
#include <string>
#include <zlib.h>

#ifdef _WIN32
#include <windows.h>
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

constexpr int CHUNK = 16384;

namespace OpenAPT::File
{

    bool compress_file(const std::string &file_name, const std::string &output_folder)
    {
        std::filesystem::path input_path(file_name);
        if (!std::filesystem::exists(input_path))
        {
            // spdlog::error("Input file {} does not exist.", file_name);
            return false;
        }

        std::filesystem::path output_path = std::filesystem::path(output_folder) / input_path.filename().concat(".gz");
        gzFile out = gzopen(output_path.string().c_str(), "wb");
        if (!out)
        {
            // spdlog::error("Failed to create compressed file {}", output_path.string());
            return false;
        }

        std::ifstream in(file_name, std::ios::binary);
        if (!in)
        {
            // spdlog::error("Failed to open input file {}", file_name);
            gzclose(out);
            return false;
        }

        char buf[CHUNK];
        while (!in.eof())
        {
            in.read(buf, CHUNK);
            int bytesRead = static_cast<int>(in.gcount());

            if (gzwrite(out, buf, bytesRead) != bytesRead)
            {
                // spdlog::error("Failed to compress file {}", file_name);
                in.close();
                gzclose(out);
                return false;
            }
        }

        in.close();
        gzclose(out);
        // spdlog::info("Compressed file {} -> {}", file_name, output_path.string());
        return true;
    }

    bool decompress_file(const std::string &file_name, const std::string &output_folder)
    {
        std::filesystem::path input_path(file_name);
        if (!std::filesystem::exists(input_path))
        {
            // spdlog::error("Input file {} does not exist.", file_name);
            return false;
        }

        std::filesystem::path output_path = std::filesystem::path(output_folder) / input_path.filename().stem().concat(".out");
        FILE *out = fopen(output_path.string().c_str(), "wb");
        if (!out)
        {
            // spdlog::error("Failed to create decompressed file {}", output_path.string());
            return false;
        }

        gzFile in = gzopen(file_name.c_str(), "rb");
        if (!in)
        {
            // spdlog::error("Failed to open compressed file {}", file_name);
            fclose(out);
            return false;
        }

        char buf[CHUNK];
        int bytesRead;
        while ((bytesRead = gzread(in, buf, CHUNK)) > 0)
        {
            if (fwrite(buf, 1, bytesRead, out) != static_cast<size_t>(bytesRead))
            {
                // spdlog::error("Failed to decompress file {}", file_name);
                fclose(out);
                gzclose(in);
                return false;
            }
        }

        fclose(out);
        gzclose(in);
        // spdlog::info("Decompressed file {} -> {}", file_name, output_path.string());
        return true;
    }

    bool compress_folder(const char *folder_name)
    {
        // Size of the read/write buffer
        char outfile_name[256];
        sprintf(outfile_name, "%s.gz", folder_name);
        gzFile out = gzopen(outfile_name, "wb");
        if (!out)
        {
            // spdlog::error("Failed to create compressed file {}", outfile_name);
            return false;
        }
#ifdef _WIN32
        HANDLE hFind;
        // File handle
        WIN32_FIND_DATAA findData;
        char searchPath[256];
        sprintf(searchPath, "%s\\*", folder_name);
        hFind = FindFirstFileA(searchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            // spdlog::error("Failed to open folder {}", folder_name);
            gzclose(out);
            return false;
        }
        do
        {
            // Ignore "." and ".." directories
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
                continue;
            // Construct the file path
            char file_name[256];
            sprintf(file_name, "%s%c%s", folder_name, DIR_SEPARATOR, findData.cFileName);
            // If it's a directory, recursively call this function
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                bool res = compress_folder(file_name);
                if (!res)
                {
                    FindClose(hFind);
                    gzclose(out);
                    return false;
                }
                continue;
            }
            // Otherwise, it's a file, compress it
            if (access(file_name, F_OK) == -1)
            {
                // If the file doesn't exist
                continue;
            }
            FILE *in = fopen(file_name, "rb");
            if (!in)
            {
                // spdlog::warn("Failed to open file {}", file_name);
                continue;
            }
            char buf[CHUNK];
            int len;
            while ((len = fread(buf, 1, CHUNK, in)) > 0)
            {
                if (gzwrite(out, buf, len) != len)
                {
                    fclose(in);
                    gzclose(out);
                    // spdlog::error("Failed to compress file {}", file_name);
                    return false;
                }
            }
            fclose(in);
            // spdlog::info("Compressed file {}", file_name);
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
#else
        DIR *dir;
        struct dirent *entry;
        dir = opendir(folder_name);
        if (!dir)
        {
            // spdlog::error("Failed to open folder {}", folder_name);
            gzclose(out);
            return false;
        }
        while ((entry = readdir(dir)) != NULL)
        {
            // Ignore "." and ".." directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            // Construct the file path
            char file_name[512];
            int ret = snprintf(file_name, sizeof(file_name), "%s/%s", folder_name, entry->d_name);
            if (ret < 0 || ret >= sizeof(file_name))
            {
                // Check for truncation or other errors in snprintf()
                // spdlog::error("Failed to compress file {} because the output was truncated or an error occurred in snprintf()", entry->d_name);
                closedir(dir);
                gzclose(out);
                return false;
            }
            // If it's a directory, recursively call this function
            struct stat st;
            if (stat(file_name, &st) == 0 && S_ISDIR(st.st_mode))
            {
                bool res = compress_folder(file_name);
                if (!res)
                {
                    closedir(dir);
                    gzclose(out);
                    return false;
                }
                continue;
            }
            // Otherwise, it's a file, compress it
            if (access(file_name, F_OK) == -1)
            {
                // If the file doesn't exist
                continue;
            }
            FILE *in = fopen(file_name, "rb");
            if (!in)
            {
                // spdlog::warn("Failed to open file {}", file_name);
                continue;
            }
            char buf[CHUNK];
            int len;
            while ((len = fread(buf, 1, CHUNK, in)) > 0)
            {
                if (gzwrite(out, buf, len) != len)
                {
                    fclose(in);
                    gzclose(out);
                    // spdlog::error("Failed to compress file {}", file_name);
                    return false;
                }
            }
            fclose(in);
            // spdlog::info("Compressed file {}", file_name);
        }
        closedir(dir);
#endif
        gzclose(out);
        // spdlog::info("Compressed folder {} -> {}", folder_name, outfile_name);
        return true;
    }

}

/*
int main()
{
    std::string file_path = "path_to_your_file";
    std::string output_folder = "path_to_your_output_folder";

    try
    {
        bool compressed = compress_file(file_path, output_folder);
        if (compressed)
        {
            std::cout << "File compressed successfully." << std::endl;
        }
        else
        {
            std::cout << "Failed to compress file." << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Exception occurred: " << ex.what() << std::endl;
    }

    try
    {
        bool decompressed = decompress_file(file_path, output_folder);
        if (decompressed)
        {
            std::cout << "File decompressed successfully." << std::endl;
        }
        else
        {
            std::cout << "Failed to decompress file." << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Exception occurred: " << ex.what() << std::endl;
    }

    return 0;
}
*/
