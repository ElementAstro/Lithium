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

#ifdef _WIN32 // Windows下的实现
#include <windows.h>
#define DIR_SEPARATOR '\\'
#else // Linux下的实现
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define DIR_SEPARATOR '/'
#endif
#include <zlib.h>
#include <cstdio>
#include <cstring>

#include <spdlog/spdlog.h>

namespace OpenAPT::Compressor
{

#define CHUNK 16384

    // Function: compress_file()
    // Description: Compress a file using gzip algorithm and save it as a .gz file.
    // Parameters:
    // - file_name (const char*) : The path of the file to be compressed.
    // Return value: bool - true if the file is compressed successfully, false otherwise.
    /**
     * @brief 使用 gzip 算法压缩一个文件，并将其保存为 .gz 文件。
     *
     * 该函数接收一个文件路径，打开文件流，使用 gzip 算法压缩文件内容，并将压缩后的结果保存为 .gz 文件。如果操作成功，返回 true，否则返回 false。
     * 如果出现任何异常，则返回 false。
     *
     * @param file_name (const char*) : 要压缩的文件路径。
     * @return bool - 如果成功压缩文件，则返回 true；否则返回 false。
     */
    bool compress_file(const char *file_name)
    {
        // Create the name for the compressed file
        char outfile_name[256];
#ifdef _WIN32 // Windows OS
        sprintf(outfile_name, "%s.gz", file_name);
#else // Linux OS
        snprintf(outfile_name, sizeof(outfile_name), "%s.gz", file_name);
#endif
        // Open the input file stream and the output compressed file stream
        FILE *in = fopen(file_name, "rb");
        gzFile out = gzopen(outfile_name, "wb");
        if (!in || !out)
        {
            spdlog::error("Failed to open file {} or create compressed file {}", file_name, outfile_name);
            return false;
        }
        // Read the input file content in CHUNK-size chunks and compress them into the output file stream
        char buf[CHUNK];
        int len;
        while ((len = fread(buf, 1, CHUNK, in)) > 0)
        {
            if (gzwrite(out, buf, len) != len)
            {
                spdlog::error("Failed to compress file {}", file_name);
                fclose(in);
                gzclose(out);
                return false;
            }
        }
        // Close the input and output file streams
        fclose(in);
        gzclose(out);
        spdlog::info("Compressed file {} -> {}", file_name, outfile_name);
        return true;
    }

    // Function: decompress_file()
    // Description: Decompress a file that was compressed using gzip algorithm and save it as a regular file.
    // Parameters:
    // - file_name (const char*) : The path of the compressed file to be decompressed.
    // Return value: bool - true if the file is decompressed successfully, false otherwise.

    /**
     * @brief 对使用 gzip 算法压缩的文件进行解压缩，并将其保存为普通文件。
     *
     * 该函数接收一个压缩文件的路径，打开压缩文件流，将压缩文件内容解压缩，并将解压后的结果保存为普通文件。如果操作成功，返回 true，否则返回 false。
     * 如果出现任何异常，则返回 false。
     *
     * @param file_name (const char*) : 待解压的压缩文件路径。
     * @return bool - 如果成功解压文件，则返回 true；否则返回 false。
     */
    bool decompress_file(const char *file_name)
    {
        // Create the name for the decompressed file
        char outfile_name[256];
#ifdef _WIN32 // Windows OS
        sprintf(outfile_name, "%s.out", file_name);
#else // Linux OS
        snprintf(outfile_name, sizeof(outfile_name), "%s.out", file_name);
#endif

        // Open the input compressed file stream and the output file stream
        gzFile in = gzopen(file_name, "rb");
        FILE *out = fopen(outfile_name, "wb");
        if (!in || !out)
        {
            spdlog::error("Failed to open compressed file {} or create decompressed file {}", file_name, outfile_name);
            return false;
        }

        // Read the compressed file content in CHUNK-size chunks and decompress them into the output file stream
        char buf[CHUNK];
        int len;
        while ((len = gzread(in, buf, CHUNK)) > 0)
        {
            if (fwrite(buf, 1, len, out) != len)
            {
                spdlog::error("Failed to decompress file {}", file_name);
                gzclose(in);
                fclose(out);
                return false;
            }
        }

        // Close the input and output file streams
        gzclose(in);
        fclose(out);
        spdlog::info("Decompressed file {} -> {}", file_name, outfile_name);
        return true;
    }

    /**
     * @brief Compress files in a specified folder.
     *
     * This function compresses all files (including files in subdirectories) in a specified folder
     * using gzip compression and saves the compressed file in the same folder with extension .gz.
     * Both Windows and Linux are supported.
     *
     * @param folder_name The name of the folder to compress.
     * @return true if the compression is successful, false otherwise.
     */
    bool compress_folder(const char *folder_name)
    {
        // Size of the read/write buffer
        char outfile_name[256];
        sprintf(outfile_name, "%s.gz", folder_name);
        gzFile out = gzopen(outfile_name, "wb");
        if (!out)
        {
            spdlog::error("Failed to create compressed file {}", outfile_name);
            return false;
        }
#ifdef _WIN32 // Windows-specific implementation
        HANDLE hFind;
        // File handle
        WIN32_FIND_DATAA findData;
        char searchPath[256];
        sprintf(searchPath, "%s\\*", folder_name);
        hFind = FindFirstFileA(searchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            spdlog::error("Failed to open folder {}", folder_name);
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
            char absolute_path[256];
            realpath(file_name, absolute_path);
            FILE *in = fopen(absolute_path, "rb");
            if (!in)
            {
                spdlog::warn("Failed to open file {}", file_name);
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
                    spdlog::error("Failed to compress file {}", file_name);
                    return false;
                }
            }
            fclose(in);
            spdlog::info("Compressed file {}", file_name);
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
#else // Linux-specific implementation
        DIR *dir;
        struct dirent *entry;
        dir = opendir(folder_name);
        if (!dir)
        {
            spdlog::error("Failed to open folder {}", folder_name);
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
                spdlog::error("Failed to compress file {} because the output was truncated or an error occurred in snprintf()", entry->d_name);
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
            char absolute_path[256];
            char *abs_path = realpath(file_name, absolute_path);
            if (abs_path == NULL)
            {
                spdlog::warn("Path not found");
                continue;
            }
            FILE *in = fopen(absolute_path, "rb");
            if (!in)
            {
                spdlog::warn("Failed to open file {}", file_name);
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
                    spdlog::error("Failed to compress file {}", file_name);
                    return false;
                }
            }
            fclose(in);
            spdlog::info("Compressed file {}", file_name);
        }
        closedir(dir);
#endif
        gzclose(out);
        spdlog::info("Compressed folder {} -> {}", folder_name, outfile_name);
        return true;
    }

}
