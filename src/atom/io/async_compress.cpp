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


BaseCompressor::BaseCompressor(asio::io_context& io_context, const fs::path& output_file)
    : io_context_(io_context), output_stream_(io_context) {
    openOutputFile(output_file);
    zlib_stream_.zalloc = Z_NULL;
    zlib_stream_.zfree = Z_NULL;
    zlib_stream_.opaque = Z_NULL;

    if (deflateInit2(&zlib_stream_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw std::runtime_error("Failed to initialize zlib.");
    }
}

void BaseCompressor::openOutputFile(const fs::path& output_file) {
#ifdef _WIN32
    HANDLE fileHandle = CreateFile(output_file.string().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open output file.");
    }
    output_stream_.assign(fileHandle);
#else
    int file_descriptor = ::open(output_file.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor == -1) {
        throw std::runtime_error("Failed to open output file.");
    }
    output_stream_.assign(file_descriptor);
#endif
}

void BaseCompressor::doCompress() {
    zlib_stream_.avail_out = out_buffer_.size();
    zlib_stream_.next_out = reinterpret_cast<Bytef*>(out_buffer_.data());

    int ret = deflate(&zlib_stream_, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR) {
        throw std::runtime_error("Zlib stream error.");
    }

    std::size_t bytesToWrite = out_buffer_.size() - zlib_stream_.avail_out;
    asio::async_write(output_stream_, asio::buffer(out_buffer_, bytesToWrite),
        [this](std::error_code ec, std::size_t /*bytes_written*/) {
            if (!ec) {
                if (zlib_stream_.avail_in > 0) {
                    doCompress();
                } else {
                    onAfterWrite();
                }
            } else {
                std::cerr << "Error during file write: " << ec.message() << "\n";
            }
        });
}

void BaseCompressor::finishCompression() {
    zlib_stream_.avail_in = 0;
    zlib_stream_.next_in = Z_NULL;

    int ret;
    do {
        zlib_stream_.avail_out = out_buffer_.size();
        zlib_stream_.next_out = reinterpret_cast<Bytef*>(out_buffer_.data());
        ret = deflate(&zlib_stream_, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            throw std::runtime_error("Zlib stream error.");
        }

        std::size_t bytesToWrite = out_buffer_.size() - zlib_stream_.avail_out;
        asio::async_write(output_stream_, asio::buffer(out_buffer_, bytesToWrite),
            [this, ret](std::error_code ec, std::size_t /*bytes_written*/) {
                if (!ec && ret == Z_FINISH) {
                    deflateEnd(&zlib_stream_);
                    std::cout << "Compression finished successfully.\n";
                } else {
                    std::cerr << "Error during file write or compression finish: " << ec.message() << "\n";
                }
            });

    } while (ret != Z_STREAM_END);
}

SingleFileCompressor::SingleFileCompressor(asio::io_context& io_context, const fs::path& input_file, const fs::path& output_file)
    : BaseCompressor(io_context, output_file), input_stream_(io_context) {
    openInputFile(input_file);
}

void SingleFileCompressor::start() {
    doRead();
}

void SingleFileCompressor::openInputFile(const fs::path& input_file) {
#ifdef _WIN32
    HANDLE fileHandle = CreateFile(input_file.string().c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open input file.");
    }
    input_stream_.assign(fileHandle);
#else
    int file_descriptor = ::open(input_file.string().c_str(), O_RDONLY);
    if (file_descriptor == -1) {
        throw std::runtime_error("Failed to open input file.");
    }
    input_stream_.assign(file_descriptor);
#endif
}

void SingleFileCompressor::doRead() {
    input_stream_.async_read_some(asio::buffer(in_buffer_),
        [this](std::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                zlib_stream_.avail_in = bytes_transferred;
                zlib_stream_.next_in = reinterpret_cast<Bytef*>(in_buffer_.data());
                doCompress();
            } else {
                if (ec != asio::error::eof) {
                    std::cerr << "Error during file read: " << ec.message() << "\n";
                }
                finishCompression();
            }
        });
}

void SingleFileCompressor::onAfterWrite() {
    doRead();
}

DirectoryCompressor::DirectoryCompressor(asio::io_context& io_context, fs::path input_dir, const fs::path& output_file)
    : BaseCompressor(io_context, output_file), input_dir_(std::move(input_dir)) {
}

void DirectoryCompressor::start() {
    for (const auto& entry : fs::recursive_directory_iterator(input_dir_)) {
        if (entry.is_regular_file()) {
            files_to_compress_.push_back(entry.path());
        }
    }
    if (!files_to_compress_.empty()) {
        doCompressNextFile();
    }
}

void DirectoryCompressor::doCompressNextFile() {
    if (files_to_compress_.empty()) {
        finishCompression();
        return;
    }

    current_file_ = files_to_compress_.back();
    files_to_compress_.pop_back();

    input_stream_.open(current_file_, std::ios::binary);
    if (!input_stream_) {
        std::cerr << "Failed to open file: " << current_file_ << "\n";
        doCompressNextFile();
        return;
    }

    doRead();
}

void DirectoryCompressor::doRead() {
    input_stream_.read(in_buffer_.data(), in_buffer_.size());
    auto bytesRead = input_stream_.gcount();
    if (bytesRead > 0) {
        zlib_stream_.avail_in = bytesRead;
        zlib_stream_.next_in = reinterpret_cast<Bytef*>(in_buffer_.data());
        doCompress();
    } else {
        input_stream_.close();
        doCompressNextFile();
    }
}

void DirectoryCompressor::onAfterWrite() {
    doRead();
}

BaseDecompressor::BaseDecompressor(asio::io_context& io_context)
    : io_context_(io_context) {
}

void BaseDecompressor::decompress(gzFile source, StreamHandle& output_stream) {
    in_file_ = source;
    out_stream_ = &output_stream;
    doRead();
}

void BaseDecompressor::doRead() {
    std::size_t bytesTransferred = gzread(in_file_, in_buffer_.data(), in_buffer_.size());
    if (bytesTransferred > 0) {
        asio::async_write(*out_stream_, asio::buffer(in_buffer_, bytesTransferred),
            [this](std::error_code ec, std::size_t /*bytes_written*/) {
                if (!ec) {
                    doRead();
                } else {
                    std::cerr << "Error during file write: " << ec.message() << "\n";
                    done();
                }
            });
    } else {
        if (bytesTransferred < 0) {
            std::cerr << "Error during file read\n";
        }
        gzclose(in_file_);
        done();
    }
}

SingleFileDecompressor::SingleFileDecompressor(asio::io_context& io_context, fs::path input_file, fs::path output_folder)
    : BaseDecompressor(io_context), input_file_(std::move(input_file)), output_folder_(std::move(output_folder)), output_stream_(io_context) {
}

void SingleFileDecompressor::start() {
    if (!fs::exists(input_file_)) {
        std::cerr << "Input file does not exist: " << input_file_ << "\n";
        return;
    }

    fs::path outputFilePath = output_folder_ / input_file_.filename().stem().concat(".out");
    gzFile inputHandle = gzopen(input_file_.string().c_str(), "rb");
    if (inputHandle == nullptr) {
        std::cerr << "Failed to open compressed file: " << input_file_ << "\n";
        return;
    }

#ifdef _WIN32
    HANDLE file_handle = CreateFile(outputFilePath.string().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        gzclose(inputHandle);
        std::cerr << "Failed to create decompressed file: " << outputFilePath << "\n";
        return;
    }
    output_stream_.assign(file_handle);
#else
    int file_descriptor = ::open(outputFilePath.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor == -1) {
        gzclose(inputHandle);
        std::cerr << "Failed to create decompressed file: " << outputFilePath << "\n";
        return;
    }
    output_stream_.assign(file_descriptor);
#endif

    decompress(inputHandle, output_stream_);
}

void SingleFileDecompressor::done() {
    output_stream_.close();
    std::cout << "Decompressed file successfully: " << input_file_ << "\n";
}

DirectoryDecompressor::DirectoryDecompressor(asio::io_context& io_context, const fs::path& input_dir, const fs::path& output_folder)
    : BaseDecompressor(io_context), input_dir_(input_dir), output_folder_(output_folder), output_stream_(io_context) {
}

void DirectoryDecompressor::start() {
    for (const auto& entry : fs::recursive_directory_iterator(input_dir_)) {
        if (entry.is_regular_file()) {
            files_to_decompress_.push_back(entry.path());
        }
    }
    if (!files_to_decompress_.empty()) {
        decompressNextFile();
    }
}

void DirectoryDecompressor::decompressNextFile() {
    if (files_to_decompress_.empty()) {
        std::cout << "All files decompressed successfully.\n";
        return;
    }

    current_file_ = files_to_decompress_.back();
    files_to_decompress_.pop_back();

    fs::path outputFilePath = output_folder_ / current_file_.filename().stem().concat(".out");
    gzFile inputHandle = gzopen(current_file_.string().c_str(), "rb");
    if (inputHandle == nullptr) {
        std::cerr << "Failed to open compressed file: " << current_file_ << "\n";
        decompressNextFile();
        return;
    }

#ifdef _WIN32
    HANDLE fileHandle = CreateFile(outputFilePath.string().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        gzclose(inputHandle);
        std::cerr << "Failed to create decompressed file: " << outputFilePath << "\n";
        decompressNextFile();
        return;
    }
    output_stream_.assign(fileHandle);
#else
    int file_descriptor = ::open(output_file_path.string().c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor == -1) {
        gzclose(input_handle);
        std::cerr << "Failed to create decompressed file: " << output_file_path << "\n";
        decompress_next_file();
        return;
    }
    output_stream_.assign(file_descriptor);
#endif

    decompress(inputHandle, output_stream_);
}

void DirectoryDecompressor::done() {
    output_stream_.close();
    std::cout << "Decompressed file successfully: " << current_file_ << "\n";
    decompressNextFile();
}

// Utility function for error logging
void log_error(const std::string& message) {
    std::cerr << "ERROR: " << message << std::endl;
}

// ListFilesInZip implementation
ListFilesInZip::ListFilesInZip(asio::io_context& io_context, std::string_view zip_file)
    : io_context_(io_context), zip_file_(zip_file) {}

void ListFilesInZip::start() {
    auto result = std::async(std::launch::deferred, &ListFilesInZip::listFiles, this);
    io_context_.post([result = std::move(result)]() mutable { result.get(); });
}

std::vector<std::string> ListFilesInZip::getFileList() {
    return fileList_;
}

void ListFilesInZip::listFiles() {
    unzFile zipReader = unzOpen(zip_file_.data());
    if (zipReader == nullptr) {
        log_error("Failed to open ZIP file: " + std::string(zip_file_));
        return;
    }

    if (unzGoToFirstFile(zipReader) != UNZ_OK) {
        log_error("Failed to read first file in ZIP: " + std::string(zip_file_));
        unzClose(zipReader);
        return;
    }

    do {
        std::array<char, 256> filename;
        unz_file_info fileInfo;
        if (unzGetCurrentFileInfo(zipReader, &fileInfo, filename.data(),
                                  filename.size(), nullptr, 0, nullptr,
                                  0) != UNZ_OK) {
            log_error("Failed to get file info in ZIP: " + std::string(zip_file_));
            unzClose(zipReader);
            return;
        }
        fileList_.emplace_back(filename.data());
    } while (unzGoToNextFile(zipReader) != UNZ_END_OF_LIST_OF_FILE);

    unzClose(zipReader);
}

// FileExistsInZip implementation
FileExistsInZip::FileExistsInZip(asio::io_context& io_context, std::string_view zip_file, std::string_view file_name)
    : io_context_(io_context), zip_file_(zip_file), file_name_(file_name) {}

void FileExistsInZip::start() {
    auto result = std::async(std::launch::deferred, &FileExistsInZip::checkFileExists, this);
    io_context_.post([result = std::move(result)]() mutable { result.get(); });
}

bool FileExistsInZip::found() const {
    return fileExists_;
}

void FileExistsInZip::checkFileExists() {
    unzFile zipReader = unzOpen(zip_file_.data());
    if (zipReader == nullptr) {
        log_error("Failed to open ZIP file: " + std::string(zip_file_));
        return;
    }

    fileExists_ = unzLocateFile(zipReader, file_name_.data(), 0) == UNZ_OK;
    unzClose(zipReader);
}

// RemoveFileFromZip implementation
RemoveFileFromZip::RemoveFileFromZip(asio::io_context& io_context, std::string_view zip_file, std::string_view file_name)
    : io_context_(io_context), zip_file_(zip_file), file_name_(file_name) {}

void RemoveFileFromZip::start() {
    auto result = std::async(std::launch::deferred, &RemoveFileFromZip::removeFile, this);
    io_context_.post([result = std::move(result)]() mutable { result.get(); });
}

bool RemoveFileFromZip::isSuccessful() const {
    return success_;
}

void RemoveFileFromZip::removeFile() {
    unzFile zipReader = unzOpen(zip_file_.data());
    if (zipReader == nullptr) {
        log_error("Failed to open ZIP file: " + zip_file_);
        return;
    }

    if (unzLocateFile(zipReader, file_name_.data(), 0) != UNZ_OK) {
        log_error("File not found in ZIP: " + file_name_);
        unzClose(zipReader);
        return;
    }

    std::string tempZipFile = std::string(zip_file_) + ".tmp";
    zipFile zipWriter = zipOpen(tempZipFile.c_str(), APPEND_STATUS_CREATE);
    if (zipWriter == nullptr) {
        log_error("Failed to create temporary ZIP file: " + tempZipFile);
        unzClose(zipReader);
        return;
    }

    if (unzGoToFirstFile(zipReader) != UNZ_OK) {
        log_error("Failed to read first file in ZIP: " + zip_file_);
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
            log_error("Failed to get file info in ZIP: " + zip_file_);
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return;
        }

        if (file_name_ == filename.data()) {
            continue;
        }

        if (unzOpenCurrentFile(zipReader) != UNZ_OK) {
            log_error("Failed to open file in ZIP: " + std::string(filename.data()));
            unzClose(zipReader);
            zipClose(zipWriter, nullptr);
            return;
        }

        zip_fileinfo fileInfoOut = {};
        if (zipOpenNewFileInZip(zipWriter, filename.data(), &fileInfoOut,
                                nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED,
                                Z_DEFAULT_COMPRESSION) != ZIP_OK) {
            log_error("Failed to add file to temporary ZIP: " + std::string(filename.data()));
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
}

// GetZipFileSize implementation
GetZipFileSize::GetZipFileSize(asio::io_context& io_context, std::string_view zip_file)
    : io_context_(io_context), zip_file_(zip_file) {}

void GetZipFileSize::start() {
    auto result = std::async(std::launch::deferred, &GetZipFileSize::getSize, this);
    io_context_.post([result = std::move(result)]() mutable { result.get(); });
}

size_t GetZipFileSize::getSizeValue() const {
    return size_;
}

void GetZipFileSize::getSize() {
    std::ifstream inputFile(zip_file_.data(), std::ifstream::ate | std::ifstream::binary);
    if (!inputFile) {
        log_error("Failed to open ZIP file to get size: " + std::string(zip_file_));
        return;
    }
    size_ = static_cast<size_t>(inputFile.tellg());
}
