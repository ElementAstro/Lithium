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
#include <zlib.h>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <string>
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
constexpr int BUFFER_SIZE = 8192;
constexpr int FILENAME_SIZE = 256;

namespace atom::io {
auto compressFile(std::string_view input_file_name,
                  std::string_view output_folder) -> bool {
    fs::path inputPath(input_file_name);
    if (!fs::exists(inputPath)) {
        LOG_F(ERROR, "Input file {} does not exist.", input_file_name);
        return false;
    }

    fs::path outputPath =
        fs::path(output_folder) / inputPath.filename().concat(".gz");
    gzFile out = gzopen(outputPath.string().data(), "wb");
    if (out == nullptr) {
        LOG_F(ERROR, "Failed to create compressed file {}",
              outputPath.string());
        return false;
    }

    std::ifstream input(input_file_name.data(), std::ios::binary);
    if (!input) {
        LOG_F(ERROR, "Failed to open input file {}", input_file_name);
        gzclose(out);
        return false;
    }

    std::array<char, CHUNK> buf;
    while (input.read(buf.data(), buf.size())) {
        if (gzwrite(out, buf.data(), static_cast<unsigned>(input.gcount())) ==
            0) {
            LOG_F(ERROR, "Failed to compress file {}", input_file_name);
            gzclose(out);
            return false;
        }
    }

    if (input.eof()) {
        if (gzwrite(out, buf.data(), static_cast<unsigned>(input.gcount())) ==
            0) {
            LOG_F(ERROR, "Failed to compress file {}", input_file_name);
            gzclose(out);
            return false;
        }
    } else if (input.bad()) {
        LOG_F(ERROR, "Failed to read input file {}", input_file_name);
        gzclose(out);
        return false;
    }

    gzclose(out);
    DLOG_F(INFO, "Compressed file {} -> {}", input_file_name,
           outputPath.string());
    return true;
}

auto compressFile(const fs::path &file, gzFile out) -> bool {
    std::ifstream input_file(file, std::ios::binary);
    if (!input_file) {
        LOG_F(ERROR, "Failed to open file {}", file.string());
        return false;
    }

    std::array<char, CHUNK> buf;
    while (input_file.read(buf.data(), buf.size())) {
        if (gzwrite(out, buf.data(),
                    static_cast<unsigned>(input_file.gcount())) !=
            static_cast<int>(input_file.gcount())) {
            LOG_F(ERROR, "Failed to compress file {}", file.string());
            return false;
        }
    }

    if (input_file.eof()) {
        if (gzwrite(out, buf.data(),
                    static_cast<unsigned>(input_file.gcount())) !=
            static_cast<int>(input_file.gcount())) {
            LOG_F(ERROR, "Failed to compress file {}", file.string());
            return false;
        }
    } else if (input_file.bad()) {
        LOG_F(ERROR, "Failed to read file {}", file.string());
        return false;
    }

    return true;
}

auto decompressFile(std::string_view input_file_name,
                    std::string_view output_folder) -> bool {
    fs::path inputPath(input_file_name);
    if (!fs::exists(inputPath)) {
        LOG_F(ERROR, "Input file {} does not exist.", input_file_name);
        return false;
    }

    fs::path outputPath =
        fs::path(output_folder) / inputPath.filename().stem().concat(".out");
    FILE *out = fopen(outputPath.string().data(), "wb");
    if (out == nullptr) {
        LOG_F(ERROR, "Failed to create decompressed file {}",
              outputPath.string());
        return false;
    }

    gzFile in = gzopen(input_file_name.data(), "rb");
    if (in == nullptr) {
        LOG_F(ERROR, "Failed to open compressed file {}", input_file_name);
        if (fclose(out) != 0) {
            LOG_F(ERROR, "Failed to close file {}", outputPath.string());
            gzclose(in);
            return false;
        }
        return false;
    }

    std::array<char, CHUNK> buf;
    int bytesRead;
    while ((bytesRead = gzread(in, buf.data(), buf.size())) > 0) {
        if (fwrite(buf.data(), 1, bytesRead, out) !=
            static_cast<size_t>(bytesRead)) {
            LOG_F(ERROR, "Failed to decompress file {}", input_file_name);
            fclose(out);
            gzclose(in);
            return false;
        }
    }

    if (bytesRead < 0) {
        LOG_F(ERROR, "Failed to read compressed file {}", input_file_name);
        fclose(out);
        gzclose(in);
        return false;
    }

    fclose(out);
    gzclose(in);
    DLOG_F(INFO, "Decompressed file {} -> {}", input_file_name,
           outputPath.string());
    return true;
}

auto compressFolder(const fs::path &folder_name) -> bool {
    auto outfileName = folder_name.string() + ".gz";
    gzFile out = gzopen(outfileName.data(), "wb");
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

auto extractZip(std::string_view zip_file,
                std::string_view destination_folder) -> bool {
    void *zipReader = unzOpen(zip_file.data());
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
        std::array<char, FILENAME_SIZE> filename;
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename.data(),
                                  filename.size(), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zipReader);
            return false;
        }

        std::string filePath = "./" + fs::path(destination_folder).string() +
                               "/" + filename.data();
        if (filename[filename.size() - 1] == '/') {
            fs::create_directories(filePath);
            continue;
        }

        if (unzOpenCurrentFile(zipReader) != UNZ_OK) {
            LOG_F(ERROR, "Failed to open file in ZIP: {}", filename.data());
            unzClose(zipReader);
            return false;
        }

        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            LOG_F(ERROR, "Failed to create file: {}", filePath);
            unzCloseCurrentFile(zipReader);
            unzClose(zipReader);
            return false;
        }

        std::array<char, BUFFER_SIZE> buffer;
        int readSize = 0;
        while ((readSize = unzReadCurrentFile(zipReader, buffer.data(),
                                              buffer.size())) > 0) {
            outFile.write(buffer.data(), readSize);
        }

        unzCloseCurrentFile(zipReader);
        outFile.close();
        DLOG_F(INFO, "Extracted file {}", filePath);
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
    DLOG_F(INFO, "Extracted ZIP file {}", zip_file);
    return true;
}

auto createZip(std::string_view source_folder, std::string_view zip_file,
               int compression_level) -> bool {
    void *zipWriter = zipOpen(zip_file.data(), APPEND_STATUS_CREATE);
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

                zip_fileinfo fileInfo = {};
                if (zipOpenNewFileInZip(zipWriter, relativePath.data(),
                                        &fileInfo, nullptr, 0, nullptr, 0,
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

                std::array<char, BUFFER_SIZE> buffer;
                while (inFile.read(buffer.data(), buffer.size())) {
                    zipWriteInFileInZip(zipWriter, buffer.data(),
                                        inFile.gcount());
                }
                if (inFile.gcount() > 0) {
                    zipWriteInFileInZip(zipWriter, buffer.data(),
                                        inFile.gcount());
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

auto listFilesInZip(std::string_view zip_file) -> std::vector<std::string> {
    std::vector<std::string> fileList;
    void *zipReader = unzOpen(zip_file.data());
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
        std::array<char, FILENAME_SIZE> filename;
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename.data(),
                                  filename.size(), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zipReader);
            return fileList;
        }
        fileList.emplace_back(filename.data());
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
    return fileList;
}

auto fileExistsInZip(std::string_view zip_file,
                     std::string_view file_name) -> bool {
    void *zipReader = unzOpen(zip_file.data());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    if (unzLocateFile(zipReader, file_name.data(), 0) == UNZ_OK) {
        unzClose(zipReader);
        return true;
    }

    unzClose(zipReader);
    return false;
}

auto removeFileFromZip(std::string_view zip_file,
                       std::string_view file_name) -> bool {
    void *zipReader = unzOpen(zip_file.data());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    if (unzLocateFile(zipReader, file_name.data(), 0) != UNZ_OK) {
        LOG_F(ERROR, "File not found in ZIP: {}", file_name);
        unzClose(zipReader);
        return false;
    }

    // This is a simplified method that recreates the ZIP file without the
    // specified file.
    std::string tempZipFile = std::string(zip_file) + ".tmp";
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
        std::array<char, FILENAME_SIZE> filename;
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename.data(),
                                  filename.size(), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file);
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return false;
        }

        if (file_name == filename.data()) {
            continue;
        }

        if (unzOpenCurrentFile(zipReader) != UNZ_OK) {
            LOG_F(ERROR, "Failed to open file in ZIP: {}", filename.data());
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return false;
        }

        zip_fileinfo fileInfoOut = {};
        if (zipOpenNewFileInZip(zipWriter, filename.data(), &fileInfoOut,
                                nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED,
                                Z_DEFAULT_COMPRESSION) != ZIP_OK) {
            LOG_F(ERROR, "Failed to add file to temporary ZIP: {}",
                  filename.data());
            unzCloseCurrentFile(zipReader);
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return false;
        }

        std::array<char, BUFFER_SIZE> buffer;
        int readSize = 0;
        while ((readSize = unzReadCurrentFile(zipReader, buffer.data(),
                                              buffer.size())) > 0) {
            zipWriteInFileInZip(zipWriter, buffer.data(), readSize);
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

auto getZipFileSize(std::string_view zip_file) -> size_t {
    std::ifstream inputFile(zip_file.data(),
                            std::ifstream::ate | std::ifstream::binary);
    return inputFile.tellg();
}

bool decompressChunk(const std::vector<unsigned char> &chunkData,
                     std::vector<unsigned char> &outputBuffer) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    if (inflateInit(&stream) != Z_OK) {
        LOG_F(ERROR, "Error initializing zlib inflate.");
        return false;
    }

    stream.avail_in = static_cast<uInt>(chunkData.size());
    stream.next_in = const_cast<Bytef *>(chunkData.data());

    do {
        stream.avail_out = static_cast<uInt>(outputBuffer.size());
        stream.next_out = outputBuffer.data();

        int ret = inflate(&stream, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR) {
            LOG_F(ERROR, "Data error detected. Skipping corrupted chunk.");
            inflateEnd(&stream);
            return false;
        }

    } while (stream.avail_out == 0);

    inflateEnd(&stream);
    return true;
}
constexpr int CHUNK_SIZE = 4096;
// Function to process files in parallel
void processFilesInParallel(const std::vector<std::string> &filenames) {
    std::vector<std::future<void>> futures;

    for (const auto &filename : filenames) {
        futures.push_back(std::async(std::launch::async, [filename]() {
            LOG_F(INFO, "Processing file: {}", filename);
            std::ifstream file(filename, std::ios::binary);

            if (!file) {
                LOG_F(INFO, "Failed to open file: {}", filename);
                return;
            }

            std::vector<unsigned char> chunkData(CHUNK_SIZE);
            std::vector<unsigned char> outputBuffer(
                CHUNK_SIZE * 2);  // Preallocate a larger buffer to avoid
                                  // multiple allocations

            while (file.read(reinterpret_cast<char *>(chunkData.data()),
                             CHUNK_SIZE) ||
                   file.gcount() > 0) {
                if (!decompressChunk(chunkData, outputBuffer)) {
                    LOG_F(ERROR, "Failed to decompress chunk for file: {}",
                          filename);
                }
            }
        }));
    }

    for (auto &future : futures) {
        future.get();
    }
}

// Function to create a backup of a file
bool createBackup(const std::string &originalFile,
                  const std::string &backupFile) {
    try {
        std::filesystem::copy(
            originalFile, backupFile,
            std::filesystem::copy_options::overwrite_existing);
        LOG_F(INFO, "Backup created: {}", backupFile);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to create backup: {}", e.what());
        return false;
    }
}

// Function to restore a backup
bool restoreBackup(const std::string &backupFile,
                   const std::string &originalFile) {
    try {
        std::filesystem::copy(
            backupFile, originalFile,
            std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        return false;
    }
}
}  // namespace atom::io
