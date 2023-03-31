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

#ifdef _WIN32  // Windows下的实现
#include <windows.h>
#define DIR_SEPARATOR '\\'
#else  // Linux下的实现
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define DIR_SEPARATOR '/'
#endif
#include <zlib.h>
#include <cstdio>
#include <cstring>

#include <spdlog/spdlog.h>

namespace OpenAPT::Compressor {

    #define CHUNK 16384

    /**
     * 对单个文件进行压缩
     * @param file_name 待压缩的文件名（包含路径）
     * @return 是否压缩成功
     */
    bool compress_file(const char *file_name) {
        char outfile_name[256];
        sprintf(outfile_name, "%s.gz", file_name);

        FILE *in = fopen(file_name, "rb");
        gzFile out = gzopen(outfile_name, "wb");
        if (!in || !out) {
            spdlog::error("Failed to open file {} or create compressed file {}", file_name, outfile_name);
            return false;
        }

        char buf[CHUNK];
        int len;
        while ((len = fread(buf, 1, CHUNK, in)) > 0) {
            if (gzwrite(out, buf, len) != len) {
                spdlog::error("Failed to compress file {}", file_name);
                fclose(in);
                gzclose(out);
                return false;
            }
        }

        fclose(in);
        gzclose(out);
        spdlog::info("Compressed file {} -> {}", file_name, outfile_name);
        return true;
    }

    /**
     * 对单个文件进行解压缩
     * @param file_name 待解压的文件名（包含路径）
     * @return 是否解压成功
     */
    bool decompress_file(const char *file_name) {
        char outfile_name[256];
        sprintf(outfile_name, "%s.out", file_name);

        gzFile in = gzopen(file_name, "rb");
        FILE *out = fopen(outfile_name, "wb");
        if (!in || !out) {
            spdlog::error("Failed to open compressed file {} or create decompressed file {}", file_name, outfile_name);
            return false;
        }

        char buf[CHUNK];
        int len;
        while ((len = gzread(in, buf, CHUNK)) > 0) {
            if (fwrite(buf, 1, len, out) != len) {
                spdlog::error("Failed to decompress file {}", file_name);
                gzclose(in);
                fclose(out);
                return false;
            }
        }

        gzclose(in);
        fclose(out);
        spdlog::info("Decompressed file {} -> {}", file_name, outfile_name);
        return true;
    }

    /**
     * 对指定目录下的文件进行压缩
     * @param folder_name 待压缩的目录名（绝对路径）
     * @return 是否压缩成功
     */
    bool compress_folder(const char *folder_name) {
        char outfile_name[256];
        sprintf(outfile_name, "%s.gz", folder_name);

        gzFile out = gzopen(outfile_name, "wb");
        if (!out) {
            spdlog::error("Failed to create compressed file {}", outfile_name);
            return false;
        }

    #ifdef _WIN32  // Windows下的实现
        HANDLE hFind;  // 文件句柄
        WIN32_FIND_DATAA findData;

        char searchPath[256];
        sprintf(searchPath, "%s\\*", folder_name);

        hFind = FindFirstFileA(searchPath, &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            spdlog::error("Failed to open folder {}", folder_name);
            return false;
        }

        do {
            // 忽略"."和".."目录
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
                continue;

            // 拼接文件路径
            char file_name[256];
            sprintf(file_name, "%s%c%s", folder_name, DIR_SEPARATOR, findData.cFileName);

            // 如果是目录，则递归调用
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                bool res = compress_folder(file_name);
                if (!res) {
                    FindClose(hFind);
                    gzclose(out);
                    return false;
                }
                continue;
            }

            // 否则是文件，进行压缩
            if (access(file_name, F_OK) == -1) {  // 文件不存在
                continue;
            }

            char absolute_path[256];
            realpath(file_name, absolute_path);

            FILE *in = fopen(absolute_path, "rb");
            if (!in)
                continue;

            char buf[CHUNK];
            int len;
            while ((len = fread(buf, 1, CHUNK, in)) > 0) {
                if (gzwrite(out, buf, len) != len) {
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
    #else  // Linux下的实现
        DIR *dir;
        struct dirent *entry;
        char file_name[256];

        dir = opendir(folder_name);
        if (!dir) {
            spdlog::error("Failed to open folder {}", folder_name);
            return false;
        }

        while ((entry = readdir(dir)) != NULL) {
            // 忽略"."和".."目录
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            // 拼接文件路径
            char file_name[512];
            int ret = snprintf(file_name, sizeof(file_name), "%s/%s", folder_name, entry->d_name);
            if (ret < 0 || ret >= sizeof(file_name)) { // 判断是否发生截断
                spdlog::error("Failed to compress file {} because the output was truncated or an error occurred in snprintf()", entry->d_name);
                return false;
            }
            //sprintf(file_name, "%s/%s", folder_name, entry->d_name);

            // 如果是目录，则递归调用
            struct stat st;
            if (stat(file_name, &st) == 0 && S_ISDIR(st.st_mode)) {
                bool res = compress_folder(file_name);
                if (!res) {
                    closedir(dir);
                    gzclose(out);
                    return false;
                }
                continue;
            }

            // 否则是文件，进行压缩
            if (access(file_name, F_OK) == -1) {  // 文件不存在
                continue;
            }

            char absolute_path[256];
            char* abs_path = realpath(file_name, absolute_path);
            if (abs_path == NULL) {
                spdlog::error("Path not found");
                return false;
            }


            FILE *in = fopen(absolute_path, "rb");
            if (!in)
                continue;

            char buf[CHUNK];
            int len;
            while ((len = fread(buf, 1, CHUNK, in)) > 0) {
                if (gzwrite(out, buf, len) != len) {
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

