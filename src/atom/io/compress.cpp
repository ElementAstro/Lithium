/*
 * compress.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-31

Description: Compressor using ZLib

**************************************************/

#include "compress.hpp"
#include "io.hpp"

#include <minizip-ng/mz_compat.h>
#include <minizip-ng/mz_strm.h>
#include <minizip-ng/mz_strm_buf.h>
#include <minizip-ng/mz_strm_mem.h>
#include <minizip-ng/mz_strm_split.h>
#include <minizip-ng/mz_strm_zlib.h>
#include <minizip-ng/mz_zip.h>
#include <minizip-ng/unzip.h>
#include <minizip-ng/zip.h>
#include <zlib.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
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
#include "atom/type/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

constexpr int CHUNK = 16384;

namespace atom::io {
bool compress_file(const std::string &file_name,
                   const std::string &output_folder) {
    fs::path input_path(file_name);
    if (!fs::exists(input_path)) {
        LOG_F(ERROR, "Input file {} does not exist.", file_name);
        return false;
    }

    fs::path output_path =
        fs::path(output_folder) / input_path.filename().concat(".gz");
    gzFile out = gzopen(output_path.string().c_str(), "wb");
    if (!out) {
        LOG_F(ERROR, "Failed to create compressed file {}",
              output_path.string());
        return false;
    }

    std::ifstream in(file_name, std::ios::binary);
    if (!in) {
        LOG_F(ERROR, "Failed to open input file {}", file_name);
        gzclose(out);
        return false;
    }

    char buf[CHUNK];
    while (in.read(buf, sizeof(buf))) {
        if (gzwrite(out, buf, static_cast<unsigned>(in.gcount())) == 0) {
            LOG_F(ERROR, "Failed to compress file {}", file_name);
            gzclose(out);
            return false;
        }
    }

    if (in.eof()) {
        if (gzwrite(out, buf, static_cast<unsigned>(in.gcount())) == 0) {
            LOG_F(ERROR, "Failed to compress file {}", file_name);
            gzclose(out);
            return false;
        }
    } else if (in.bad()) {
        LOG_F(ERROR, "Failed to read input file {}", file_name);
        gzclose(out);
        return false;
    }

    gzclose(out);
    DLOG_F(INFO, "Compressed file {} -> {}", file_name, output_path.string());
    return true;
}

bool compress_file_(const fs::path &file, gzFile out) {
    std::ifstream in(file, std::ios::binary);
    if (!in) {
        LOG_F(ERROR, "Failed to open file {}", file.string());
        return false;
    }

    char buf[CHUNK];
    while (in.read(buf, sizeof(buf))) {
        if (gzwrite(out, buf, static_cast<unsigned>(in.gcount())) !=
            static_cast<int>(in.gcount())) {
            LOG_F(ERROR, "Failed to compress file {}", file.string());
            return false;
        }
    }

    if (in.eof()) {
        if (gzwrite(out, buf, static_cast<unsigned>(in.gcount())) !=
            static_cast<int>(in.gcount())) {
            LOG_F(ERROR, "Failed to compress file {}", file.string());
            return false;
        }
    } else if (in.bad()) {
        LOG_F(ERROR, "Failed to read file {}", file.string());
        return false;
    }

    return true;
}

bool decompress_file(const std::string &file_name,
                     const std::string &output_folder) {
    fs::path input_path(file_name);
    if (!fs::exists(input_path)) {
        LOG_F(ERROR, "Input file {} does not exist.", file_name);
        return false;
    }

    fs::path output_path =
        fs::path(output_folder) / input_path.filename().stem().concat(".out");
    FILE *out = fopen(output_path.string().c_str(), "wb");
    if (!out) {
        LOG_F(ERROR, "Failed to create decompressed file {}",
              output_path.string());
        return false;
    }

    gzFile in = gzopen(file_name.c_str(), "rb");
    if (!in) {
        LOG_F(ERROR, "Failed to open compressed file {}", file_name);
        fclose(out);
        return false;
    }

    char buf[CHUNK];
    int bytesRead;
    while ((bytesRead = gzread(in, buf, sizeof(buf))) > 0) {
        if (fwrite(buf, 1, bytesRead, out) != static_cast<size_t>(bytesRead)) {
            LOG_F(ERROR, "Failed to decompress file {}", file_name);
            fclose(out);
            gzclose(in);
            return false;
        }
    }

    if (bytesRead < 0) {
        LOG_F(ERROR, "Failed to read compressed file {}", file_name);
        fclose(out);
        gzclose(in);
        return false;
    }

    fclose(out);
    gzclose(in);
    DLOG_F(INFO, "Decompressed file {} -> {}", file_name, output_path.string());
    return true;
}

bool compress_folder_(const fs::path &folder_name) {
    auto outfile_name = folder_name.string() + ".gz";
    gzFile out = gzopen(outfile_name.c_str(), "wb");
    if (!out) {
        LOG_F(ERROR, "Failed to create compressed file {}", outfile_name);
        return false;
    }

    for (const auto &entry : fs::recursive_directory_iterator(folder_name)) {
        if (entry.is_directory()) {
            std::string file_pattern = entry.path().string() + "/*";
            for (const auto &sub_entry : fs::directory_iterator(file_pattern)) {
                if (sub_entry.is_regular_file()) {
                    if (!compress_file_(sub_entry.path(), out)) {
                        gzclose(out);
                        return false;
                    }
                }
            }
        } else if (entry.is_regular_file()) {
            if (!compress_file_(entry.path(), out)) {
                gzclose(out);
                return false;
            }
        }
    }

    gzclose(out);
    DLOG_F(INFO, "Compressed folder {} -> {}", folder_name.string(),
           outfile_name);
    return true;
}

bool compress_folder(const char *folder_name) {
    return compress_folder_(fs::path(folder_name));
}

bool extract_zip(const std::string &zip_file,
                 const std::string &destination_folder) {
    void *zip_reader = unzOpen(zip_file.c_str());
    if (zip_reader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    if (unzGoToFirstFile(zip_reader) != UNZ_OK) {
        LOG_F(ERROR, "Failed to read first file in ZIP: {}", zip_file);
        unzClose(zip_reader);
        return false;
    }

    do {
        char filename[256];
        unz_file_info file_info;
        if (unzGetCurrentFileInfo(zip_reader, &file_info, filename,
                                  sizeof(filename), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zip_reader);
            return false;
        }

        std::string file_path =
            "./" + fs::path(destination_folder).string() + "/" + filename;
        if (filename[strlen(filename) - 1] == '/') {
            fs::create_directories(file_path);
            continue;
        }

        if (unzOpenCurrentFile(zip_reader) != UNZ_OK) {
            LOG_F(ERROR, "Failed to open file in ZIP: {}", filename);
            unzClose(zip_reader);
            return false;
        }

        std::ofstream out_file(file_path, std::ios::binary);
        if (!out_file) {
            LOG_F(ERROR, "Failed to create file: {}", file_path);
            unzCloseCurrentFile(zip_reader);
            unzClose(zip_reader);
            return false;
        }

        char buffer[8192];
        int read_size = 0;
        while ((read_size = unzReadCurrentFile(zip_reader, buffer,
                                               sizeof(buffer))) > 0) {
            out_file.write(buffer, read_size);
        }

        unzCloseCurrentFile(zip_reader);
        out_file.close();
        DLOG_F(INFO, "Extracted file {}", file_path);
    } while (unzGoToNextFile(zip_reader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zip_reader);
    DLOG_F(INFO, "Extracted ZIP file {}", zip_file);
    return true;
}

bool create_zip(const std::string &source_folder, const std::string &zip_file,
                int compression_level) {
    void *zip_writer = zipOpen(zip_file.c_str(), APPEND_STATUS_CREATE);
    if (zip_writer == nullptr) {
        LOG_F(ERROR, "Failed to create ZIP file: {}", zip_file);
        return false;
    }

    try {
        for (const auto &entry :
             fs::recursive_directory_iterator(source_folder)) {
            if (fs::is_regular_file(entry)) {
                std::string file_path = entry.path().string();
                std::string relative_path =
                    fs::relative(file_path, source_folder).string();

                zip_fileinfo file_info = {};
                if (zipOpenNewFileInZip(zip_writer, relative_path.c_str(),
                                        &file_info, nullptr, 0, nullptr, 0,
                                        nullptr, Z_DEFLATED,
                                        compression_level) != ZIP_OK) {
                    LOG_F(ERROR, "Failed to add file to ZIP: {}",
                          relative_path);
                    zipClose(zip_writer, nullptr);
                    return false;
                }

                std::ifstream in_file(file_path, std::ios::binary);
                if (!in_file) {
                    LOG_F(ERROR, "Failed to open file for reading: {}",
                          file_path);
                    zipCloseFileInZip(zip_writer);
                    zipClose(zip_writer, nullptr);
                    return false;
                }

                char buffer[8192];
                while (in_file.read(buffer, sizeof(buffer))) {
                    zipWriteInFileInZip(zip_writer, buffer, in_file.gcount());
                }
                if (in_file.gcount() > 0) {
                    zipWriteInFileInZip(zip_writer, buffer, in_file.gcount());
                }

                in_file.close();
                zipCloseFileInZip(zip_writer);
            }
        }

        zipClose(zip_writer, nullptr);
        DLOG_F(INFO, "ZIP file created successfully: {}", zip_file);
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to create ZIP file: {}", zip_file);
        zipClose(zip_writer, nullptr);
        return false;
    }
}

std::vector<std::string> list_files_in_zip(const std::string &zip_file) {
    std::vector<std::string> file_list;
    void *zip_reader = unzOpen(zip_file.c_str());
    if (zip_reader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return file_list;
    }

    if (unzGoToFirstFile(zip_reader) != UNZ_OK) {
        LOG_F(ERROR, "Failed to read first file in ZIP: {}", zip_file);
        unzClose(zip_reader);
        return file_list;
    }

    do {
        char filename[256];
        unz_file_info file_info;
        if (unzGetCurrentFileInfo(zip_reader, &file_info, filename,
                                  sizeof(filename), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zip_reader);
            return file_list;
        }
        file_list.push_back(filename);
    } while (unzGoToNextFile(zip_reader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zip_reader);
    return file_list;
}

bool file_exists_in_zip(const std::string &zip_file,
                        const std::string &file_name) {
    void *zip_reader = unzOpen(zip_file.c_str());
    if (zip_reader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    if (unzLocateFile(zip_reader, file_name.c_str(), 0) == UNZ_OK) {
        unzClose(zip_reader);
        return true;
    }

    unzClose(zip_reader);
    return false;
}

bool remove_file_from_zip(const std::string &zip_file,
                          const std::string &file_name) {
    void *zip_reader = unzOpen(zip_file.c_str());
    if (zip_reader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    if (unzLocateFile(zip_reader, file_name.c_str(), 0) != UNZ_OK) {
        LOG_F(ERROR, "File not found in ZIP: {}", file_name);
        unzClose(zip_reader);
        return false;
    }

    // This is a simplified method that recreates the ZIP file without the
    // specified file.
    std::string temp_zip_file = zip_file + ".tmp";
    void *zip_writer = zipOpen(temp_zip_file.c_str(), APPEND_STATUS_CREATE);

    if (zip_writer == nullptr) {
        LOG_F(ERROR, "Failed to create temporary ZIP file: {}", temp_zip_file);
        unzClose(zip_reader);
        return false;
    }

    if (unzGoToFirstFile(zip_reader) != UNZ_OK) {
        LOG_F(ERROR, "Failed to read first file in ZIP: {}", zip_file);
        unzClose(zip_reader);
        zipClose(zip_writer, nullptr);
        return false;
    }

    do {
        char filename[256];
        unz_file_info file_info;
        if (unzGetCurrentFileInfo(zip_reader, &file_info, filename,
                                  sizeof(filename), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zip_reader);
            zipClose(zip_writer, nullptr);
            return false;
        }

        if (file_name == filename) {
            continue;
        }

        if (unzOpenCurrentFile(zip_reader) != UNZ_OK) {
            LOG_F(ERROR, "Failed to open file in ZIP: {}", filename);
            unzClose(zip_reader);
            zipClose(zip_writer, nullptr);
            return false;
        }

        zip_fileinfo file_info_out = {};
        if (zipOpenNewFileInZip(zip_writer, filename, &file_info_out, nullptr,
                                0, nullptr, 0, nullptr, Z_DEFLATED,
                                Z_DEFAULT_COMPRESSION) != ZIP_OK) {
            LOG_F(ERROR, "Failed to add file to temporary ZIP: {}", filename);
            unzCloseCurrentFile(zip_reader);
            unzClose(zip_reader);
            zipClose(zip_writer, nullptr);
            return false;
        }

        char buffer[8192];
        int read_size = 0;
        while ((read_size = unzReadCurrentFile(zip_reader, buffer,
                                               sizeof(buffer))) > 0) {
            zipWriteInFileInZip(zip_writer, buffer, read_size);
        }

        unzCloseCurrentFile(zip_reader);
        zipCloseFileInZip(zip_writer);
    } while (unzGoToNextFile(zip_reader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zip_reader);
    zipClose(zip_writer, nullptr);

    // Replace original ZIP file with the new one
    fs::remove(zip_file);
    fs::rename(temp_zip_file, zip_file);

    return true;
}

size_t get_zip_file_size(const std::string &zip_file) {
    std::ifstream in(zip_file, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}
}  // namespace atom::io
