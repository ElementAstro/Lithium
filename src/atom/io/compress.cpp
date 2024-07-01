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
auto compressFile(const std::string &file_name,
                  const std::string &output_folder) -> bool {
    fs::path input_path(file_name);
    if (!fs::exists(input_path)) {
        LOG_F(ERROR, "Input file {} does not exist.", file_name);
        return false;
    }

    fs::path outputPath =
        fs::path(output_folder) / input_path.filename().concat(".gz");
    gzFile out = gzopen(outputPath.string().c_str(), "wb");
    if (out == nullptr) {
        LOG_F(ERROR, "Failed to create compressed file {}",
              outputPath.string());
        return false;
    }

    std::ifstream input(file_name, std::ios::binary);
    if (!input) {
        LOG_F(ERROR, "Failed to open input file {}", file_name);
        gzclose(out);
        return false;
    }

    char buf[CHUNK];
    while (input.read(buf, sizeof(buf))) {
        if (gzwrite(out, buf, static_cast<unsigned>(input.gcount())) == 0) {
            LOG_F(ERROR, "Failed to compress file {}", file_name);
            gzclose(out);
            return false;
        }
    }

    if (input.eof()) {
        if (gzwrite(out, buf, static_cast<unsigned>(input.gcount())) == 0) {
            LOG_F(ERROR, "Failed to compress file {}", file_name);
            gzclose(out);
            return false;
        }
    } else if (input.bad()) {
        LOG_F(ERROR, "Failed to read input file {}", file_name);
        gzclose(out);
        return false;
    }

    gzclose(out);
    DLOG_F(INFO, "Compressed file {} -> {}", file_name, outputPath.string());
    return true;
}

auto compressFile(const fs::path &file, gzFile out) -> bool {
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

auto decompressFile(const std::string &file_name,
                    const std::string &output_folder) -> bool {
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

auto compressFolder(const fs::path &folder_name) -> bool {
    auto outfileName = folder_name.string() + ".gz";
    gzFile out = gzopen(outfileName.c_str(), "wb");
    if (out == nullptr) {
        LOG_F(ERROR, "Failed to create compressed file {}", outfileName);
        return false;
    }

    for (const auto &entry : fs::recursive_directory_iterator(folder_name)) {
        if (entry.is_directory()) {
            std::string filePattern = entry.path().string() + "/*";
            for (const auto &subEntry : fs::directory_iterator(filePattern)) {
                if (subEntry.is_regular_file()) {
                    if (!compressFile(subEntry.path(), out)) {
                        gzclose(out);
                        return false;
                    }
                }
            }
        } else if (entry.is_regular_file()) {
            if (!compressFile(entry.path(), out)) {
                gzclose(out);
                return false;
            }
        }
    }

    gzclose(out);
    DLOG_F(INFO, "Compressed folder {} -> {}", folder_name.string(),
           outfileName);
    return true;
}

auto compressFolder(const char *folder_name) -> bool {
    return compressFolder(fs::path(folder_name));
}

auto extractZip(const std::string &zip_file,
                const std::string &destination_folder) -> bool {
    void *zipReader = unzOpen(zip_file.c_str());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    if (unzGoToFirstFile(zipReader) != UNZ_OK) {
        LOG_F(ERROR, "Failed to read first file in ZIP: {}", zip_file);
        unzClose(zipReader);
        return false;
    }

    do {
        char filename[256];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename,
                                  sizeof(filename), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zipReader);
            return false;
        }

        std::string filePath =
            "./" + fs::path(destination_folder).string() + "/" + filename;
        if (filename[strlen(filename) - 1] == '/') {
            fs::create_directories(filePath);
            continue;
        }

        if (unzOpenCurrentFile(zipReader) != UNZ_OK) {
            LOG_F(ERROR, "Failed to open file in ZIP: {}", filename);
            unzClose(zipReader);
            return false;
        }

        std::ofstream out_file(filePath, std::ios::binary);
        if (!out_file) {
            LOG_F(ERROR, "Failed to create file: {}", filePath);
            unzCloseCurrentFile(zipReader);
            unzClose(zipReader);
            return false;
        }

        char buffer[8192];
        int readSize = 0;
        while ((readSize = unzReadCurrentFile(zipReader, buffer,
                                              sizeof(buffer))) > 0) {
            out_file.write(buffer, readSize);
        }

        unzCloseCurrentFile(zipReader);
        out_file.close();
        DLOG_F(INFO, "Extracted file {}", filePath);
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
    DLOG_F(INFO, "Extracted ZIP file {}", zip_file);
    return true;
}

auto createZip(const std::string &source_folder, const std::string &zip_file,
               int compression_level) -> bool {
    void *zipWriter = zipOpen(zip_file.c_str(), APPEND_STATUS_CREATE);
    if (zipWriter == nullptr) {
        LOG_F(ERROR, "Failed to create ZIP file: {}", zip_file);
        return false;
    }

    try {
        for (const auto &entry :
             fs::recursive_directory_iterator(source_folder)) {
            if (fs::is_regular_file(entry)) {
                std::string filePath = entry.path().string();
                std::string relativePath =
                    fs::relative(filePath, source_folder).string();

                zip_fileinfo file_info = {};
                if (zipOpenNewFileInZip(zipWriter, relativePath.c_str(),
                                        &file_info, nullptr, 0, nullptr, 0,
                                        nullptr, Z_DEFLATED,
                                        compression_level) != ZIP_OK) {
                    LOG_F(ERROR, "Failed to add file to ZIP: {}", relativePath);
                    zipClose(zipWriter, nullptr);
                    return false;
                }

                std::ifstream inFile(filePath, std::ios::binary);
                if (!inFile) {
                    LOG_F(ERROR, "Failed to open file for reading: {}",
                          filePath);
                    zipCloseFileInZip(zipWriter);
                    zipClose(zipWriter, nullptr);
                    return false;
                }

                char buffer[8192];
                while (inFile.read(buffer, sizeof(buffer))) {
                    zipWriteInFileInZip(zipWriter, buffer, inFile.gcount());
                }
                if (inFile.gcount() > 0) {
                    zipWriteInFileInZip(zipWriter, buffer, inFile.gcount());
                }

                inFile.close();
                zipCloseFileInZip(zipWriter);
            }
        }

        zipClose(zipWriter, nullptr);
        DLOG_F(INFO, "ZIP file created successfully: {}", zip_file);
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to create ZIP file: {}", zip_file);
        zipClose(zipWriter, nullptr);
        return false;
    }
}

auto listFilesInZip(const std::string &zip_file) -> std::vector<std::string> {
    std::vector<std::string> fileList;
    void *zipReader = unzOpen(zip_file.c_str());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return fileList;
    }

    if (unzGoToFirstFile(zipReader) != UNZ_OK) {
        LOG_F(ERROR, "Failed to read first file in ZIP: {}", zip_file);
        unzClose(zipReader);
        return fileList;
    }

    do {
        char filename[256];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename,
                                  sizeof(filename), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zipReader);
            return fileList;
        }
        fileList.emplace_back(filename);
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
    return fileList;
}

auto fileExistsInZip(const std::string &zip_file,
                     const std::string &file_name) -> bool {
    void *zipReader = unzOpen(zip_file.c_str());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    if (unzLocateFile(zipReader, file_name.c_str(), 0) == UNZ_OK) {
        unzClose(zipReader);
        return true;
    }

    unzClose(zipReader);
    return false;
}

auto removeFileFromZip(const std::string &zip_file,
                       const std::string &file_name) -> bool {
    void *zipReader = unzOpen(zip_file.c_str());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    if (unzLocateFile(zipReader, file_name.c_str(), 0) != UNZ_OK) {
        LOG_F(ERROR, "File not found in ZIP: {}", file_name);
        unzClose(zipReader);
        return false;
    }

    // This is a simplified method that recreates the ZIP file without the
    // specified file.
    std::string tempZipFile = zip_file + ".tmp";
    void *zipWriter = zipOpen(tempZipFile.c_str(), APPEND_STATUS_CREATE);

    if (zipWriter == nullptr) {
        LOG_F(ERROR, "Failed to create temporary ZIP file: {}", tempZipFile);
        unzClose(zipReader);
        return false;
    }

    if (unzGoToFirstFile(zipReader) != UNZ_OK) {
        LOG_F(ERROR, "Failed to read first file in ZIP: {}", zip_file);
        unzClose(zipReader);
        zipClose(zipWriter, nullptr);
        return false;
    }

    do {
        char filename[256];
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename,
                                  sizeof(filename), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return false;
        }

        if (file_name == filename) {
            continue;
        }

        if (unzOpenCurrentFile(zipReader) != UNZ_OK) {
            LOG_F(ERROR, "Failed to open file in ZIP: {}", filename);
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return false;
        }

        zip_fileinfo fileInfoOut = {};
        if (zipOpenNewFileInZip(zipWriter, filename, &fileInfoOut, nullptr, 0,
                                nullptr, 0, nullptr, Z_DEFLATED,
                                Z_DEFAULT_COMPRESSION) != ZIP_OK) {
            LOG_F(ERROR, "Failed to add file to temporary ZIP: {}", filename);
            unzCloseCurrentFile(zipReader);
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return false;
        }

        char buffer[8192];
        int readSize = 0;
        while ((readSize = unzReadCurrentFile(zipReader, buffer,
                                              sizeof(buffer))) > 0) {
            zipWriteInFileInZip(zipWriter, buffer, readSize);
        }

        unzCloseCurrentFile(zipReader);
        zipCloseFileInZip(zipWriter);
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
    zipClose(zipWriter, nullptr);

    // Replace original ZIP file with the new one
    fs::remove(zip_file);
    fs::rename(tempZipFile, zip_file);

    return true;
}

auto getZipFileSize(const std::string &zip_file) -> size_t {
    std::ifstream in(zip_file, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}
}  // namespace atom::io