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
#include <vector>
#include <cstring>
#include <zlib.h>

#include "loguru/loguru.hpp"
#include "libzippp/libzippp.h"
#include "nlohmann/json.hpp"

#include "io.hpp"

using namespace libzippp;

class ProgressListener : public ZipProgressListener
{
public:
    ProgressListener(void) {}
    virtual ~ProgressListener(void) {}

    void progression(double p)
    {
        DLOG_F(INFO, "-- Progression: %lf", p);
    }

    int cancel(void)
    {
        DLOG_F(INFO, "Canceled");
        return 0;
    }
};

#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH_SEPARATOR '/'
#endif

constexpr int CHUNK = 16384;

namespace Lithium::File
{

    bool compress_file(const std::string &file_name, const std::string &output_folder)
    {
        std::filesystem::path input_path(file_name);
        if (!std::filesystem::exists(input_path))
        {
            DLOG_F(ERROR, "Input file %s does not exist.", file_name.c_str());
            return false;
        }

        std::filesystem::path output_path = std::filesystem::path(output_folder) / input_path.filename().concat(".gz");
        gzFile out = gzopen(output_path.string().c_str(), "wb");
        if (!out)
        {
            DLOG_F(ERROR, "Failed to create compressed file %s", output_path.string().c_str());
            return false;
        }

        std::ifstream in(file_name, std::ios::binary);
        if (!in)
        {
            DLOG_F(ERROR, "Failed to open input file %s", file_name.c_str());
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
                DLOG_F(ERROR, "Failed to compress file %s", file_name.c_str());
                in.close();
                gzclose(out);
                return false;
            }
        }

        in.close();
        gzclose(out);
        DLOG_F(INFO, "Compressed file %s -> %s", file_name.c_str(), output_path.string().c_str());
        return true;
    }

    bool decompress_file(const std::string &file_name, const std::string &output_folder)
    {
        std::filesystem::path input_path(file_name);
        if (!std::filesystem::exists(input_path))
        {
            DLOG_F(ERROR, "Input file %s does not exist.", file_name.c_str());
            return false;
        }

        std::filesystem::path output_path = std::filesystem::path(output_folder) / input_path.filename().stem().concat(".out");
        FILE *out = fopen(output_path.string().c_str(), "wb");
        if (!out)
        {
            DLOG_F(ERROR, "Failed to create decompressed file %s", output_path.string().c_str());
            return false;
        }

        gzFile in = gzopen(file_name.c_str(), "rb");
        if (!in)
        {
            DLOG_F(ERROR, "Failed to open compressed file %s", file_name.c_str());
            fclose(out);
            return false;
        }

        char buf[CHUNK];
        int bytesRead;
        while ((bytesRead = gzread(in, buf, CHUNK)) > 0)
        {
            if (fwrite(buf, 1, bytesRead, out) != static_cast<size_t>(bytesRead))
            {
                DLOG_F(ERROR, "Failed to decompress file %s", file_name.c_str());
                fclose(out);
                gzclose(in);
                return false;
            }
        }

        fclose(out);
        gzclose(in);
        DLOG_F(INFO, "Decompressed file %s -> %s", file_name.c_str(), output_path.string().c_str());
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
            DLOG_F(ERROR, "Failed to create compressed file %s", outfile_name);
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
            DLOG_F(ERROR, "Failed to open folder %s", folder_name);
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
            sprintf(file_name, "%s%c%s", folder_name, PATH_SEPARATOR, findData.cFileName);
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
                DLOG_F(WARNING, "Failed to open file %s", file_name);
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
                    DLOG_F(ERROR, "Failed to create compressed file {}", outfile_name);
                    return false;
                }
            }
            fclose(in);
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
#else
        DIR *dir;
        struct dirent *entry;
        dir = opendir(folder_name);
        if (!dir)
        {
            DLOG_F(ERROR, "Failed to open folder %s", folder_name);
            gzclose(out);
            return false;
        }
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            char file_name[512];
            int ret = snprintf(file_name, sizeof(file_name), "%s/%s", folder_name, entry->d_name);
            if (ret < 0 || ret >= sizeof(file_name))
            {
                DLOG_F(ERROR, "Failed to compress file %s because the output was truncated or an error occurred in snprintf()", entry->d_name);
                closedir(dir);
                gzclose(out);
                return false;
            }
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
                DLOG_F(WARNING, "Failed to open file %s", file_name);
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
                    DLOG_F(ERROR, "Failed to compress file %s", file_name);
                    return false;
                }
            }
            fclose(in);
        }
        closedir(dir);
#endif
        gzclose(out);
        DLOG_F(INFO, "Compressed folder %s -> %s", folder_name, outfile_name);
        return true;
    }

    bool extract_zip(const std::string &zip_file, const std::string &destination_folder)
    {
        std::ifstream file(zip_file, std::ios::binary);
        if (!file)
        {
            DLOG_F(ERROR, "Failed to open ZIP file: %s", zip_file.c_str());
            return false;
        }
        file.close();

        ZipArchive zip("archive.zip");
        zip.setErrorHandlerCallback([](const std::string &message,
                                       const std::string &strerror,
                                       int zip_error_code,
                                       int system_error_code)
                                    { DLOG_F(ERROR, "Exract zip file failed : %s %s", message.c_str(), strerror.c_str()); });
        zip.open(ZipArchive::ReadOnly);
        ProgressListener pl;
        zip.addProgressListener(&pl);
        zip.setProgressPrecision(0.1);

        try
        {
            std::vector<ZipEntry> entries = zip.getEntries();
            for (const auto &entry : entries)
            {
                std::string name = entry.getName();
                int size = entry.getSize();
                DLOG_F(ERROR, "Extracting file: %s, size: %d", name.c_str(), size);
                std::string textData = entry.readAsText();
                std::filesystem::path file_path = std::filesystem::path(destination_folder) / name;
                std::ofstream file(file_path);
                if (file.is_open())
                {
                    file << textData;
                    file.close();
                    DLOG_F(INFO, "File extracted: %s", file_path.c_str());
                }
                else
                {
                    DLOG_F(ERROR, "Failed to create file: %s", file_path.c_str());
                }
            }
            zip.close();
            DLOG_F(INFO, "ZIP file extracted successfully.");
            return true;
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "Failed to extract ZIP file: %s", e.what());
            return false;
        }
    }

    bool create_zip(const std::string &source_folder, const std::string &zip_file)
    {
        try
        {
            ZipArchive zip(zip_file);
            zip.setErrorHandlerCallback([](const std::string &message,
                                           const std::string &strerror,
                                           int zip_error_code,
                                           int system_error_code)
                                        { DLOG_F(ERROR, "Create zip file failed : %s %s", message.c_str(), strerror.c_str()); });

            zip.open(ZipArchive::Write);
            ProgressListener pl;
            zip.addProgressListener(&pl);
            zip.setProgressPrecision(0.1);

            for (const auto &entry : std::filesystem::recursive_directory_iterator(source_folder))
            {
                if (std::filesystem::is_regular_file(entry))
                {
                    std::string file_path = entry.path().string();
                    std::string relative_path = std::filesystem::relative(file_path, source_folder).string();

                    zip.addFile(file_path, relative_path);
                }
            }

            // 关闭 ZIP 文件
            zip.close();

            std::cout << "ZIP file created successfully: " << zip_file << std::endl;
            return true;
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "Failed to create ZIP file: %s", e.what());
            return false;
        }
    }
}
