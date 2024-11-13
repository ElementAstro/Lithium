#include "async_compress.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

#include <minizip-ng/mz_compat.h>
#include <minizip-ng/mz_strm.h>
#include <minizip-ng/mz_strm_buf.h>
#include <minizip-ng/mz_strm_mem.h>
#include <minizip-ng/mz_strm_split.h>
#include <minizip-ng/mz_strm_zlib.h>
#include <minizip-ng/mz_zip.h>

namespace atom::async::io {
BaseCompressor::BaseCompressor(asio::io_context& io_context,
                               const fs::path& output_file)
    : io_context_(io_context), output_stream_(io_context) {
    LOG_F(INFO, "BaseCompressor constructor called with output_file: {}",
          output_file.string());
    openOutputFile(output_file);
    zlib_stream_.zalloc = Z_NULL;
    zlib_stream_.zfree = Z_NULL;
    zlib_stream_.opaque = Z_NULL;

    if (deflateInit2(&zlib_stream_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16,
                     8, Z_DEFAULT_STRATEGY) != Z_OK) {
        LOG_F(ERROR, "Failed to initialize zlib.");
        throw std::runtime_error("Failed to initialize zlib.");
    }
    LOG_F(INFO, "BaseCompressor initialized successfully");
}

void BaseCompressor::openOutputFile(const fs::path& output_file) {
    LOG_F(INFO, "Opening output file: {}", output_file.string());
#ifdef _WIN32
    HANDLE fileHandle =
        CreateFile(output_file.string().c_str(), GENERIC_WRITE, 0, NULL,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to open output file: {}", output_file.string());
        throw std::runtime_error("Failed to open output file.");
    }
    output_stream_.assign(fileHandle);
#else
    int file_descriptor = ::open(output_file.string().c_str(),
                                 O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor == -1) {
        LOG_F(ERROR, "Failed to open output file: {}", output_file.string());
        throw std::runtime_error("Failed to open output file.");
    }
    output_stream_.assign(file_descriptor);
#endif
    LOG_F(INFO, "Output file opened successfully: {}", output_file.string());
}

void BaseCompressor::doCompress() {
    LOG_F(INFO, "Starting compression");
    zlib_stream_.avail_out = out_buffer_.size();
    zlib_stream_.next_out = reinterpret_cast<Bytef*>(out_buffer_.data());

    int ret = deflate(&zlib_stream_, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR) {
        LOG_F(ERROR, "Zlib stream error during compression.");
        throw std::runtime_error("Zlib stream error.");
    }

    std::size_t bytesToWrite = out_buffer_.size() - zlib_stream_.avail_out;
    LOG_F(INFO, "Writing {} bytes to output file", bytesToWrite);
    asio::async_write(
        output_stream_, asio::buffer(out_buffer_, bytesToWrite),
        [this](std::error_code ec, std::size_t /*bytes_written*/) {
            if (!ec) {
                LOG_F(INFO, "Write to output file successful");
                if (zlib_stream_.avail_in > 0) {
                    doCompress();
                } else {
                    onAfterWrite();
                }
            } else {
                LOG_F(ERROR, "Error during file write: {}", ec.message());
            }
        });
}

void BaseCompressor::finishCompression() {
    LOG_F(INFO, "Finishing compression");
    zlib_stream_.avail_in = 0;
    zlib_stream_.next_in = Z_NULL;

    int ret;
    do {
        zlib_stream_.avail_out = out_buffer_.size();
        zlib_stream_.next_out = reinterpret_cast<Bytef*>(out_buffer_.data());
        ret = deflate(&zlib_stream_, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            LOG_F(ERROR, "Zlib stream error during finish compression.");
            throw std::runtime_error("Zlib stream error.");
        }

        std::size_t bytesToWrite = out_buffer_.size() - zlib_stream_.avail_out;
        LOG_F(INFO, "Writing {} bytes to output file during finish",
              bytesToWrite);
        asio::async_write(
            output_stream_, asio::buffer(out_buffer_, bytesToWrite),
            [this, ret](std::error_code ec, std::size_t /*bytes_written*/) {
                if (!ec && ret == Z_FINISH) {
                    deflateEnd(&zlib_stream_);
                    LOG_F(INFO, "Compression finished successfully.");
                } else {
                    LOG_F(ERROR,
                          "Error during file write or compression finish: {}",
                          ec.message());
                }
            });

    } while (ret != Z_STREAM_END);
}

SingleFileCompressor::SingleFileCompressor(asio::io_context& io_context,
                                           const fs::path& input_file,
                                           const fs::path& output_file)
    : BaseCompressor(io_context, output_file), input_stream_(io_context) {
    LOG_F(INFO,
          "SingleFileCompressor constructor called with input_file: {}, "
          "output_file: {}",
          input_file.string(), output_file.string());
    openInputFile(input_file);
}

void SingleFileCompressor::start() {
    LOG_F(INFO, "Starting SingleFileCompressor");
    doRead();
}

void SingleFileCompressor::openInputFile(const fs::path& input_file) {
    LOG_F(INFO, "Opening input file: {}", input_file.string());
#ifdef _WIN32
    HANDLE fileHandle =
        CreateFile(input_file.string().c_str(), GENERIC_READ, 0, NULL,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to open input file: {}", input_file.string());
        throw std::runtime_error("Failed to open input file.");
    }
    input_stream_.assign(fileHandle);
#else
    int file_descriptor = ::open(input_file.string().c_str(), O_RDONLY);
    if (file_descriptor == -1) {
        LOG_F(ERROR, "Failed to open input file: {}", input_file.string());
        throw std::runtime_error("Failed to open input file.");
    }
    input_stream_.assign(file_descriptor);
#endif
    LOG_F(INFO, "Input file opened successfully: {}", input_file.string());
}

void SingleFileCompressor::doRead() {
    LOG_F(INFO, "Starting to read from input file");
    input_stream_.async_read_some(
        asio::buffer(in_buffer_),
        [this](std::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                LOG_F(INFO, "Read {} bytes from input file", bytes_transferred);
                zlib_stream_.avail_in = bytes_transferred;
                zlib_stream_.next_in =
                    reinterpret_cast<Bytef*>(in_buffer_.data());
                doCompress();
            } else {
                if (ec != asio::error::eof) {
                    LOG_F(ERROR, "Error during file read: {}", ec.message());
                }
                finishCompression();
            }
        });
}

void SingleFileCompressor::onAfterWrite() {
    LOG_F(INFO, "SingleFileCompressor onAfterWrite called");
    doRead();
}

DirectoryCompressor::DirectoryCompressor(asio::io_context& io_context,
                                         fs::path input_dir,
                                         const fs::path& output_file)
    : BaseCompressor(io_context, output_file),
      input_dir_(std::move(input_dir)) {
    LOG_F(INFO,
          "DirectoryCompressor constructor called with input_dir: {}, "
          "output_file: {}",
          input_dir_.string(), output_file.string());
}

void DirectoryCompressor::start() {
    LOG_F(INFO, "Starting DirectoryCompressor");
    for (const auto& entry : fs::recursive_directory_iterator(input_dir_)) {
        if (entry.is_regular_file()) {
            files_to_compress_.push_back(entry.path());
            LOG_F(INFO, "Added file to compress: {}", entry.path().string());
        }
    }
    if (!files_to_compress_.empty()) {
        doCompressNextFile();
    } else {
        LOG_F(WARNING, "No files to compress in directory: {}",
              input_dir_.string());
    }
}

void DirectoryCompressor::doCompressNextFile() {
    LOG_F(INFO, "Starting compression of next file");
    if (files_to_compress_.empty()) {
        LOG_F(INFO, "No more files to compress, finishing compression");
        finishCompression();
        return;
    }

    current_file_ = files_to_compress_.back();
    files_to_compress_.pop_back();
    LOG_F(INFO, "Compressing file: {}", current_file_.string());

    input_stream_.open(current_file_, std::ios::binary);
    if (!input_stream_) {
        LOG_F(ERROR, "Failed to open file: {}", current_file_.string());
        doCompressNextFile();
        return;
    }

    doRead();
}

void DirectoryCompressor::doRead() {
    LOG_F(INFO, "Starting to read from file: {}", current_file_.string());
    input_stream_.read(in_buffer_.data(), in_buffer_.size());
    auto bytesRead = input_stream_.gcount();
    if (bytesRead > 0) {
        LOG_F(INFO, "Read {} bytes from file: {}", bytesRead,
              current_file_.string());
        zlib_stream_.avail_in = bytesRead;
        zlib_stream_.next_in = reinterpret_cast<Bytef*>(in_buffer_.data());
        doCompress();
    } else {
        LOG_F(INFO, "Finished reading file: {}", current_file_.string());
        input_stream_.close();
        doCompressNextFile();
    }
}

void DirectoryCompressor::onAfterWrite() {
    LOG_F(INFO, "DirectoryCompressor onAfterWrite called");
    doRead();
}

BaseDecompressor::BaseDecompressor(asio::io_context& io_context)
    : io_context_(io_context) {
    LOG_F(INFO, "BaseDecompressor constructor called");
}

void BaseDecompressor::decompress(gzFile source, StreamHandle& output_stream) {
    LOG_F(INFO, "BaseDecompressor::decompress called");
    in_file_ = source;
    out_stream_ = &output_stream;
    doRead();
}

void BaseDecompressor::doRead() {
    LOG_F(INFO, "BaseDecompressor::doRead called");
    std::size_t bytesTransferred =
        gzread(in_file_, in_buffer_.data(), in_buffer_.size());
    if (bytesTransferred > 0) {
        LOG_F(INFO, "Read {} bytes from compressed file", bytesTransferred);
        asio::async_write(
            *out_stream_, asio::buffer(in_buffer_, bytesTransferred),
            [this](std::error_code ec, std::size_t /*bytes_written*/) {
                if (!ec) {
                    LOG_F(INFO, "Write to output stream successful");
                    doRead();
                } else {
                    LOG_F(ERROR, "Error during file write: {}", ec.message());
                    done();
                }
            });
    } else {
        if (bytesTransferred < 0) {
            LOG_F(ERROR, "Error during file read");
        }
        gzclose(in_file_);
        done();
    }
}

SingleFileDecompressor::SingleFileDecompressor(asio::io_context& io_context,
                                               fs::path input_file,
                                               fs::path output_folder)
    : BaseDecompressor(io_context),
      input_file_(std::move(input_file)),
      output_folder_(std::move(output_folder)),
      output_stream_(io_context) {
    LOG_F(INFO,
          "SingleFileDecompressor constructor called with input_file: {}, "
          "output_folder: {}",
          input_file_.string(), output_folder_.string());
}

void SingleFileDecompressor::start() {
    LOG_F(INFO, "SingleFileDecompressor::start called");
    if (!fs::exists(input_file_)) {
        LOG_F(ERROR, "Input file does not exist: {}", input_file_.string());
        return;
    }

    fs::path outputFilePath =
        output_folder_ / input_file_.filename().stem().concat(".out");
    gzFile inputHandle = gzopen(input_file_.string().c_str(), "rb");
    if (inputHandle == nullptr) {
        LOG_F(ERROR, "Failed to open compressed file: {}",
              input_file_.string());
        return;
    }

#ifdef _WIN32
    HANDLE file_handle =
        CreateFile(outputFilePath.string().c_str(), GENERIC_WRITE, 0, NULL,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        gzclose(inputHandle);
        LOG_F(ERROR, "Failed to create decompressed file: {}",
              outputFilePath.string());
        return;
    }
    output_stream_.assign(file_handle);
#else
    int file_descriptor = ::open(outputFilePath.string().c_str(),
                                 O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor == -1) {
        gzclose(inputHandle);
        LOG_F(ERROR, "Failed to create decompressed file: {}",
              outputFilePath.string());
        return;
    }
    output_stream_.assign(file_descriptor);
#endif

    decompress(inputHandle, output_stream_);
}

void SingleFileDecompressor::done() {
    LOG_F(INFO, "SingleFileDecompressor::done called");
    output_stream_.close();
    LOG_F(INFO, "Decompressed file successfully: {}", input_file_.string());
}

DirectoryDecompressor::DirectoryDecompressor(asio::io_context& io_context,
                                             const fs::path& input_dir,
                                             const fs::path& output_folder)
    : BaseDecompressor(io_context),
      input_dir_(input_dir),
      output_folder_(output_folder),
      output_stream_(io_context) {
    LOG_F(INFO,
          "DirectoryDecompressor constructor called with input_dir: {}, "
          "output_folder: {}",
          input_dir_.string(), output_folder_.string());
}

void DirectoryDecompressor::start() {
    LOG_F(INFO, "DirectoryDecompressor::start called");
    for (const auto& entry : fs::recursive_directory_iterator(input_dir_)) {
        if (entry.is_regular_file()) {
            files_to_decompress_.push_back(entry.path());
            LOG_F(INFO, "Added file to decompress: {}", entry.path().string());
        }
    }
    if (!files_to_decompress_.empty()) {
        decompressNextFile();
    } else {
        LOG_F(WARNING, "No files to decompress in directory: {}",
              input_dir_.string());
    }
}

void DirectoryDecompressor::decompressNextFile() {
    LOG_F(INFO, "DirectoryDecompressor::decompressNextFile called");
    if (files_to_decompress_.empty()) {
        LOG_F(INFO, "All files decompressed successfully.");
        return;
    }

    current_file_ = files_to_decompress_.back();
    files_to_decompress_.pop_back();
    LOG_F(INFO, "Decompressing file: {}", current_file_.string());

    fs::path outputFilePath =
        output_folder_ / current_file_.filename().stem().concat(".out");
    gzFile inputHandle = gzopen(current_file_.string().c_str(), "rb");
    if (inputHandle == nullptr) {
        LOG_F(ERROR, "Failed to open compressed file: {}",
              current_file_.string());
        decompressNextFile();
        return;
    }

#ifdef _WIN32
    HANDLE fileHandle =
        CreateFile(outputFilePath.string().c_str(), GENERIC_WRITE, 0, NULL,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        gzclose(inputHandle);
        LOG_F(ERROR, "Failed to create decompressed file: {}",
              outputFilePath.string());
        decompressNextFile();
        return;
    }
    output_stream_.assign(fileHandle);
#else
    int file_descriptor = ::open(outputFilePath.string().c_str(),
                                 O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor == -1) {
        gzclose(inputHandle);
        LOG_F(ERROR, "Failed to create decompressed file: {}",
              outputFilePath.string());
        decompressNextFile();
        return;
    }
    output_stream_.assign(file_descriptor);
#endif

    decompress(inputHandle, output_stream_);
}

void DirectoryDecompressor::done() {
    LOG_F(INFO, "DirectoryDecompressor::done called");
    output_stream_.close();
    LOG_F(INFO, "Decompressed file successfully: {}", current_file_.string());
    decompressNextFile();
}

// ListFilesInZip implementation
ListFilesInZip::ListFilesInZip(asio::io_context& io_context,
                               std::string_view zip_file)
    : io_context_(io_context), zip_file_(zip_file) {
    LOG_F(INFO, "ListFilesInZip constructor called with zip_file: {}",
          zip_file);
}

void ListFilesInZip::start() {
    LOG_F(INFO, "ListFilesInZip::start called");
    auto result = std::make_shared<std::future<void>>(
        std::async(std::launch::deferred, &ListFilesInZip::listFiles, this));
    io_context_.post([result]() mutable { result->get(); });
}

std::vector<std::string> ListFilesInZip::getFileList() const {
    LOG_F(INFO, "ListFilesInZip::getFileList called");
    return fileList_;
}

void ListFilesInZip::listFiles() {
    LOG_F(INFO, "ListFilesInZip::listFiles called");
    unzFile zipReader = unzOpen(zip_file_.data());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file_);
        return;
    }

    if (unzGoToFirstFile(zipReader) != UNZ_OK) {
        LOG_F(ERROR, "Failed to read first file in ZIP: {}", zip_file_);
        unzClose(zipReader);
        return;
    }

    do {
        std::array<char, 256> filename;
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename.data(),
                                  filename.size(), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file_);
            unzClose(zipReader);
            return;
        }
        fileList_.emplace_back(filename.data());
        LOG_F(INFO, "Found file in ZIP: {}", filename.data());
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
    LOG_F(INFO, "ListFilesInZip::listFiles completed");
}

// FileExistsInZip implementation
FileExistsInZip::FileExistsInZip(asio::io_context& io_context,
                                 std::string_view zip_file,
                                 std::string_view file_name)
    : io_context_(io_context), zip_file_(zip_file), file_name_(file_name) {
    LOG_F(INFO,
          "FileExistsInZip constructor called with zip_file: {}, file_name: {}",
          zip_file, file_name);
}

void FileExistsInZip::start() {
    LOG_F(INFO, "FileExistsInZip::start called");
    auto result = std::make_shared<std::future<void>>(std::async(
        std::launch::deferred, &FileExistsInZip::checkFileExists, this));
    io_context_.post([result]() mutable { result->get(); });
}

bool FileExistsInZip::found() const {
    LOG_F(INFO, "FileExistsInZip::found called, returning: {}", fileExists_);
    return fileExists_;
}

void FileExistsInZip::checkFileExists() {
    LOG_F(INFO, "FileExistsInZip::checkFileExists called");
    unzFile zipReader = unzOpen(zip_file_.data());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file_);
        return;
    }

    fileExists_ = unzLocateFile(zipReader, file_name_.data(), 0) == UNZ_OK;
    if (fileExists_) {
        LOG_F(INFO, "File found in ZIP: {}", file_name_);
    } else {
        LOG_F(WARNING, "File not found in ZIP: {}", file_name_);
    }
    unzClose(zipReader);
    LOG_F(INFO, "FileExistsInZip::checkFileExists completed");
}

// RemoveFileFromZip implementation
RemoveFileFromZip::RemoveFileFromZip(asio::io_context& io_context,
                                     std::string_view zip_file,
                                     std::string_view file_name)
    : io_context_(io_context), zip_file_(zip_file), file_name_(file_name) {
    LOG_F(
        INFO,
        "RemoveFileFromZip constructor called with zip_file: {}, file_name: {}",
        zip_file, file_name);
}

void RemoveFileFromZip::start() {
    LOG_F(INFO, "RemoveFileFromZip::start called");
    auto result = std::make_shared<std::future<void>>(std::async(
        std::launch::deferred, &RemoveFileFromZip::removeFile, this));
    io_context_.post([result]() mutable { result->get(); });
}

bool RemoveFileFromZip::isSuccessful() const {
    LOG_F(INFO, "RemoveFileFromZip::isSuccessful called, returning: {}",
          success_);
    return success_;
}

void RemoveFileFromZip::removeFile() {
    LOG_F(INFO, "RemoveFileFromZip::removeFile called");
    unzFile zipReader = unzOpen(zip_file_.data());
    if (zipReader == nullptr) {
        LOG_F(ERROR, "Failed to open ZIP file: {}", zip_file_);
        return;
    }

    if (unzLocateFile(zipReader, file_name_.data(), 0) != UNZ_OK) {
        LOG_F(ERROR, "File not found in ZIP: {}", file_name_);
        unzClose(zipReader);
        return;
    }

    std::string tempZipFile = std::string(zip_file_) + ".tmp";
    zipFile zipWriter = zipOpen(tempZipFile.c_str(), APPEND_STATUS_CREATE);
    if (zipWriter == nullptr) {
        LOG_F(ERROR, "Failed to create temporary ZIP file: {}", tempZipFile);
        unzClose(zipReader);
        return;
    }

    if (unzGoToFirstFile(zipReader) != UNZ_OK) {
        LOG_F(ERROR, "Failed to read first file in ZIP: {}", zip_file_);
        unzClose(zipReader);
        zipClose(zipWriter, nullptr);
        return;
    }

    do {
        std::array<char, 256> filename;
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename.data(),
                                  filename.size(), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            LOG_F(ERROR, "Failed to get file info in ZIP: {}", zip_file_);
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return;
        }

        if (file_name_ == filename.data()) {
            LOG_F(INFO, "Skipping file: {} for removal", filename.data());
            continue;
        }

        if (unzOpenCurrentFile(zipReader) != UNZ_OK) {
            LOG_F(ERROR, "Failed to open file in ZIP: {}", filename.data());
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return;
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
            return;
        }

        std::array<char, 1024> buffer;
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

    fs::remove(zip_file_);
    fs::rename(tempZipFile, zip_file_);

    success_ = true;
    LOG_F(INFO, "RemoveFileFromZip::removeFile completed successfully");
}

// GetZipFileSize implementation
GetZipFileSize::GetZipFileSize(asio::io_context& io_context,
                               std::string_view zip_file)
    : io_context_(io_context), zip_file_(zip_file) {
    LOG_F(INFO, "GetZipFileSize constructor called with zip_file: {}",
          zip_file);
}

void GetZipFileSize::start() {
    LOG_F(INFO, "GetZipFileSize::start called");
    auto result = std::make_shared<std::future<void>>(
        std::async(std::launch::deferred, &GetZipFileSize::getSize, this));
    io_context_.post([result]() mutable { result->get(); });
}

size_t GetZipFileSize::getSizeValue() const {
    LOG_F(INFO, "GetZipFileSize::getSizeValue called, returning: {}", size_);
    return size_;
}

void GetZipFileSize::getSize() {
    LOG_F(INFO, "GetZipFileSize::getSize called");
    std::ifstream inputFile(zip_file_.data(),
                            std::ifstream::ate | std::ifstream::binary);
    if (!inputFile) {
        LOG_F(ERROR, "Failed to open ZIP file to get size: {}", zip_file_);
        return;
    }
    size_ = static_cast<size_t>(inputFile.tellg());
    LOG_F(INFO, "GetZipFileSize::getSize completed, size: {}", size_);
}

}  // namespace atom::async::io
