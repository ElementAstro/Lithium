/*
 * compress.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-31

Description: Compressor using ZLib

**************************************************/

#include "io.hpp"
#include "compress.hpp"

#include <fstream>
#include <stdexcept>
#include <cstring>
#include <zlib.h>
#if __cplusplus >= 201703L
#include <filesystem>
#endif
#ifdef __cpp_lib_format
#include <format>
#else
#include <fmt/format.h>
#endif
#ifdef _WIN32
#include <windows.h>
#define PATH_SEPARATOR '\\'
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH_SEPARATOR '/'
#endif

#include "atom/log/loguru.hpp"
#include "libzippp/libzippp.h"
#include "atom/type/json.hpp"

using json = nlohmann::json;
using namespace libzippp;
namespace fs = std::filesystem;

constexpr int CHUNK = 16384;

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

namespace Atom::IO
{
    bool compress_file(const std::string &file_name, const std::string &output_folder)
    {
        fs::path input_path(file_name);
        if (!fs::exists(input_path))
        {
            LOG_F(ERROR, "Input file {} does not exist.", file_name);
            return false;
        }

        fs::path output_path = fs::path(output_folder) / input_path.filename().concat(".gz");
        gzFile out = gzopen(output_path.string().c_str(), "wb");
        if (!out)
        {
            LOG_F(ERROR, "Failed to create compressed file {}", output_path.string());
            return false;
        }

        std::ifstream in(file_name, std::ios::binary);
        if (!in)
        {
            LOG_F(ERROR, "Failed to open input file {}", file_name);
            gzclose(out);
            return false;
        }

        char buf[CHUNK];
        while (in.read(buf, CHUNK))
        {
            if (gzwrite(out, buf, static_cast<unsigned>(in.gcount())) == 0)
            {
                LOG_F(ERROR, "Failed to compress file {}", file_name);
                in.close();
                gzclose(out);
                return false;
            }
        }

        in.close();
        gzclose(out);
        DLOG_F(INFO, "Compressed file {} -> {}", file_name, output_path.string());
        return true;
    }

    bool compress_file_(const fs::path &file, gzFile out)
    {
        std::ifstream in(file, std::ios::binary);
        if (!in)
        {
            LOG_F(ERROR, "Failed to open file {}", file.string());
            return false;
        }

        char buf[CHUNK];
        while (in.read(buf, sizeof(buf)) || in.gcount())
        {
            if (gzwrite(out, buf, static_cast<unsigned>(in.gcount())) != static_cast<int>(in.gcount()))
            {
                in.close();
                gzclose(out);
                DLOG_F(ERROR, "Failed to compress file {}", file.string());
                return false;
            }
        }

        in.close();
        return true;
    }

    bool decompress_file(const std::string &file_name, const std::string &output_folder)
    {
        fs::path input_path(file_name);
        if (!fs::exists(input_path))
        {
            LOG_F(ERROR, "Input file {} does not exist.", file_name);
            return false;
        }

        fs::path output_path = fs::path(output_folder) / input_path.filename().stem().concat(".out");
        FILE *out = fopen(output_path.string().c_str(), "wb");
        if (!out)
        {
            LOG_F(ERROR, "Failed to create decompressed file {}", output_path.string());
            return false;
        }

        gzFile in = gzopen(file_name.c_str(), "rb");
        if (!in)
        {
            LOG_F(ERROR, "Failed to open compressed file {}", file_name);
            fclose(out);
            return false;
        }

        char buf[CHUNK];
        int bytesRead;
        while ((bytesRead = gzread(in, buf, CHUNK)) > 0)
        {
            if (fwrite(buf, 1, bytesRead, out) != static_cast<size_t>(bytesRead))
            {
                LOG_F(ERROR, "Failed to decompress file {}", file_name);
                fclose(out);
                gzclose(in);
                return false;
            }
        }

        fclose(out);
        gzclose(in);
        DLOG_F(INFO, "Decompressed file {} -> {}", file_name, output_path.string());
        return true;
    }

    bool compress_folder_(const fs::path &folder_name)
    {
        auto outfile_name = fmt::format("{}.gz", folder_name.string());
        gzFile out = gzopen(outfile_name.c_str(), "wb");
        if (!out)
        {
            LOG_F(ERROR, "Failed to create compressed file {}", outfile_name);
            return false;
        }

        for (const auto &entry : fs::recursive_directory_iterator(folder_name))
        {
            if (entry.is_directory())
            {
#ifdef _WIN32
                std::string file_name = fmt::format("{}\\{}", entry.path().string(), "*");
#else
                std::string file_name = fmt::format("{}/{}", entry.path().string(), "*");
#endif
                for (const auto &sub_entry : fs::directory_iterator(file_name))
                {
                    if (sub_entry.is_regular_file())
                    {
                        std::ifstream in(sub_entry.path(), std::ios::binary);
                        if (!in)
                        {
                            LOG_F(ERROR, "Failed to open file {}", sub_entry.path().string());
                            continue;
                        }

                        char buf[CHUNK];
                        while (in.read(buf, sizeof(buf)) || in.gcount())
                        {
                            if (gzwrite(out, buf, static_cast<unsigned>(in.gcount())) != static_cast<int>(in.gcount()))
                            {
                                in.close();
                                gzclose(out);
                                LOG_F(ERROR, "Failed to compress file {}", sub_entry.path().string());
                                return false;
                            }
                        }

                        in.close();
                    }
                }
            }
            else if (entry.is_regular_file())
            {
                if (!compress_file_(entry.path(), out))
                {
                    gzclose(out);
                    return false;
                }
            }
        }
        gzclose(out);
        DLOG_F(INFO, "Compressed folder {} -> {}", folder_name.string(), outfile_name);
        return true;
    }

    bool compress_folder(const char *folder_name)
    {
        return compress_folder_(fs::path(folder_name));
    }

    bool extract_zip(const std::string &zip_file, const std::string &destination_folder)
    {
        std::ifstream file(zip_file, std::ios::binary);
        if (!file)
        {
            LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
            return false;
        }
        file.close();

        ZipArchive zip("archive.zip");
        zip.setErrorHandlerCallback([](const std::string &message,
                                       const std::string &strerror,
                                       int zip_error_code,
                                       int system_error_code)
                                    { LOG_F(ERROR, "Exract zip file failed : {} {}", message, strerror); });
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
                LOG_F(ERROR, "Extracting file: {}, size: %d", name, size);
                std::string textData = entry.readAsText();
                fs::path file_path = fs::path(destination_folder) / name;
                std::ofstream file(file_path);
                if (file.is_open())
                {
                    file << textData;
                    file.close();
                    DLOG_F(INFO, "File extracted: {}", file_path.string());
                }
                else
                {
                    LOG_F(ERROR, "Failed to create file: {}", file_path.string());
                }
            }
            zip.close();
            DLOG_F(INFO, "ZIP file extracted successfully.");
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to extract ZIP file: {}", e.what());
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
                                        { LOG_F(ERROR, "Create zip file failed : {} {}", message, strerror); });

            zip.open(ZipArchive::Write);
            ProgressListener pl;
            zip.addProgressListener(&pl);
            zip.setProgressPrecision(0.1);

            for (const auto &entry : fs::recursive_directory_iterator(source_folder))
            {
                if (fs::is_regular_file(entry))
                {
                    std::string file_path = entry.path().string();
                    std::string relative_path = fs::relative(file_path, source_folder).string();

                    zip.addFile(file_path, relative_path);
                }
            }

            // 关闭 ZIP 文件
            zip.close();
            DLOG_F(INFO, "ZIP file created successfully: {}", zip_file);
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to create ZIP file: {}", e.what());
            return false;
        }
    }
}
