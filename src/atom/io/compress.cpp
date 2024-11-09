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
    LOG_F(INFO,
          "compressFile called with input_file_name: {}, output_folder: {}",
          input_file_name, output_folder);
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
    LOG_F(INFO, "Compressed file {} -> {}", input_file_name,
          outputPath.string());
    return true;
}

auto compressFile(const fs::path &file, gzFile out) -> bool {
    LOG_F(INFO, "compressFile called with file: {}", file.string());
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

    LOG_F(INFO, "Compressed file {}", file.string());
    return true;
}

auto decompressFile(std::string_view input_file_name,
                    std::string_view output_folder) -> bool {
    LOG_F(INFO,
          "decompressFile called with input_file_name: {}, output_folder: {}",
          input_file_name, output_folder);
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
    LOG_F(INFO, "Decompressed file {} -> {}", input_file_name,
          outputPath.string());
    return true;
}

auto compressFolder(const fs::path &folder_name) -> bool {
    LOG_F(INFO, "compressFolder called with folder_name: {}",
          folder_name.string());
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
    LOG_F(INFO, "Compressed folder {} -> {}", folder_name.string(),
          outfileName);
    return true;
}

auto compressFolder(const char *folder_name) -> bool {
    LOG_F(INFO, "compressFolder called with folder_name: {}", folder_name);
    return compressFolder(fs::path(folder_name));
}

void compressFileSlice(const std::string &inputFile, size_t sliceSize) {
    std::ifstream inFile(inputFile, std::ios::binary);
    if (!inFile) {
        LOG_F(ERROR, "Failed to open input file.");
        return;
    }

    std::vector<char> buffer(sliceSize);
    size_t bytesRead;
    int fileIndex = 0;

    while (inFile) {
        // Read a slice of the file
        inFile.read(buffer.data(), sliceSize);
        bytesRead = inFile.gcount();

        if (bytesRead > 0) {
            // Prepare compressed data
            std::vector<char> compressedData(compressBound(bytesRead));
            uLongf compressedSize = compressedData.size();

            // Compress the data
            if (compress(reinterpret_cast<Bytef *>(compressedData.data()),
                         &compressedSize,
                         reinterpret_cast<const Bytef *>(buffer.data()),
                         bytesRead) != Z_OK) {
                LOG_F(ERROR, "Compression failed.");
                inFile.close();
                return;
            }

            // Write the compressed data to a new file
            std::string compressedFileName =
                "slice_" + std::to_string(fileIndex++) + ".zlib";
            std::ofstream outFile(compressedFileName, std::ios::binary);
            if (!outFile) {
                LOG_F(ERROR, "Failed to open output file.");
                inFile.close();
                return;
            }

            // Write the size of the compressed data and the data itself
            outFile.write(reinterpret_cast<char *>(&compressedSize),
                          sizeof(compressedSize));
            outFile.write(compressedData.data(), compressedSize);
            outFile.close();
        }
    }

    inFile.close();
    LOG_F(INFO, "File sliced and compressed successfully.");
}

void decompressFileSlice(const std::string &sliceFile, size_t sliceSize) {
    std::ifstream inFile(sliceFile, std::ios::binary);
    if (!inFile) {
        LOG_F(ERROR, "Failed to open compressed file: {}", sliceFile);
        return;
    }

    // Read the compressed size
    uLongf compressedSize;
    inFile.read(reinterpret_cast<char *>(&compressedSize),
                sizeof(compressedSize));

    // Prepare buffer for compressed data
    std::vector<char> compressedData(compressedSize);
    inFile.read(compressedData.data(), compressedSize);
    inFile.close();

    // Prepare buffer for decompressed data
    std::vector<char> decompressedData(
        sliceSize);  // Adjust sliceSize for max expected original size
    uLongf decompressedSize = sliceSize;

    // Decompress the data
    if (uncompress(reinterpret_cast<Bytef *>(decompressedData.data()),
                   &decompressedSize,
                   reinterpret_cast<const Bytef *>(compressedData.data()),
                   compressedSize) != Z_OK) {
        LOG_F(ERROR, "Decompression failed for file: {}", sliceFile);
        return;
    }

    // Write the decompressed data to a new file
    std::string decompressedFileName = "decompressed_" + sliceFile;
    std::ofstream outFile(decompressedFileName, std::ios::binary);
    if (!outFile) {
        LOG_F(ERROR, "Failed to open decompressed output file.");
        return;
    }

    outFile.write(decompressedData.data(), decompressedSize);
    outFile.close();
    LOG_F(INFO, "Decompressed file created: {}", decompressedFileName);
}

void listCompressedFiles() {
    for (const auto &entry : std::filesystem::directory_iterator(".")) {
        if (entry.path().extension() == ".zlib") {
            LOG_F(INFO, "{}", entry.path().filename().string());
        }
    }
}

void deleteCompressedFiles() {
    for (const auto &entry : std::filesystem::directory_iterator(".")) {
        if (entry.path().extension() == ".zlib") {
            std::filesystem::remove(entry.path());
            LOG_F(INFO, "Deleted: {}", entry.path().filename().string());
        }
    }
}

auto extractZip(std::string_view zip_file,
                std::string_view destination_folder) -> bool {
    LOG_F(INFO, "extractZip called with zip_file: {}, destination_folder: {}",
          zip_file, destination_folder);
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
        LOG_F(INFO, "Extracted file {}", filePath);
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
    LOG_F(INFO, "Extracted ZIP file {}", zip_file);
    return true;
}

auto createZip(std::string_view source_folder, std::string_view zip_file,
               int compression_level) -> bool {
    LOG_F(INFO,
          "createZip called with source_folder: {}, zip_file: {}, "
          "compression_level: {}",
          source_folder, zip_file, compression_level);
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
        LOG_F(INFO, "ZIP file created successfully: {}", zip_file);
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to create ZIP file: {}", zip_file);
        zipClose(zipWriter, nullptr);
        return false;
    }
}

auto listFilesInZip(std::string_view zip_file) -> std::vector<std::string> {
    LOG_F(INFO, "listFilesInZip called with zip_file: {}", zip_file);
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
        LOG_F(INFO, "Found file in ZIP: {}", filename.data());
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
    LOG_F(INFO, "Listed files in ZIP: {}", zip_file);
    return fileList;
}

auto fileExistsInZip(std::string_view zip_file,
                     std::string_view file_name) -> bool {
    LOG_F(INFO, "fileExistsInZip called with zip_file: {}, file_name: {}",
          zip_file, file_name);
    void *zipReader = unzOpen(zip_file.data());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file);
        return false;
    }

    bool exists = unzLocateFile(zipReader, file_name.data(), 0) == UNZ_OK;
    if (exists) {
        LOG_F(INFO, "File found in ZIP: {}", file_name);
    } else {
        LOG_F(WARNING, "File not found in ZIP: {}", file_name);
    }

    unzClose(zipReader);
    return exists;
}

auto removeFileFromZip(std::string_view zip_file,
                       std::string_view file_name) -> bool {
    LOG_F(INFO, "removeFileFromZip called with zip_file: {}, file_name: {}",
          zip_file, file_name);
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
            LOG_F(INFO, "Skipping file: {} for removal", filename.data());
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

    fs::remove(zip_file);
    fs::rename(tempZipFile, zip_file);

    LOG_F(INFO, "File removed from ZIP: {}", file_name);
    return true;
}

auto getZipFileSize(std::string_view zip_file) -> size_t {
    LOG_F(INFO, "getZipFileSize called with zip_file: {}", zip_file);
    std::ifstream inputFile(zip_file.data(),
                            std::ifstream::ate | std::ifstream::binary);
    size_t size = inputFile.tellg();
    LOG_F(INFO, "Size of ZIP file {}: {}", zip_file, size);
    return size;
}

bool decompressChunk(const std::vector<unsigned char> &chunkData,
                     std::vector<unsigned char> &outputBuffer) {
    LOG_F(INFO, "decompressChunk called");
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
    LOG_F(INFO, "Chunk decompressed successfully");
    return true;
}

constexpr int CHUNK_SIZE = 4096;

void processFilesInParallel(const std::vector<std::string> &filenames) {
    LOG_F(INFO, "processFilesInParallel called with {} files",
          filenames.size());
    std::vector<std::future<void>> futures;

    for (const auto &filename : filenames) {
        futures.push_back(std::async(std::launch::async, [filename]() {
            LOG_F(INFO, "Processing file: {}", filename);
            std::ifstream file(filename, std::ios::binary);

            if (!file) {
                LOG_F(ERROR, "Failed to open file: {}", filename);
                return;
            }

            std::vector<unsigned char> chunkData(CHUNK_SIZE);
            std::vector<unsigned char> outputBuffer(CHUNK_SIZE * 2);

            while (file.read(reinterpret_cast<char *>(chunkData.data()),
                             CHUNK_SIZE) ||
                   file.gcount() > 0) {
                if (!decompressChunk(chunkData, outputBuffer)) {
                    LOG_F(ERROR, "Failed to decompress chunk for file: {}",
                          filename);
                }
            }
            LOG_F(INFO, "Finished processing file: {}", filename);
        }));
    }

    for (auto &future : futures) {
        future.get();
    }
    LOG_F(INFO, "All files processed in parallel");
}

bool createBackup(const std::string &originalFile,
                  const std::string &backupFile) {
    LOG_F(INFO, "createBackup called with originalFile: {}, backupFile: {}",
          originalFile, backupFile);
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

bool restoreBackup(const std::string &backupFile,
                   const std::string &originalFile) {
    LOG_F(INFO, "restoreBackup called with backupFile: {}, originalFile: {}",
          backupFile, originalFile);
    try {
        std::filesystem::copy(
            backupFile, originalFile,
            std::filesystem::copy_options::overwrite_existing);
        LOG_F(INFO, "Backup restored: {}", originalFile);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to restore backup: {}", e.what());
        return false;
    }
}
}  // namespace atom::io
