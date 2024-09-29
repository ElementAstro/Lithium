#ifndef ASYNC_COMPRESS_HPP
#define ASYNC_COMPRESS_HPP

#include <zlib.h>
#include <array>
#include <asio.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;
#ifdef _WIN32
#include <windows.h>
using StreamHandle = asio::windows::stream_handle;
#else
#include <fcntl.h>
using StreamHandle = asio::posix::stream_descriptor;
#endif

namespace atom::async::io {
constexpr std::size_t CHUNK = 16384;

class BaseCompressor {
public:
    BaseCompressor(asio::io_context& io_context, const fs::path& output_file);
    virtual void start() = 0;

protected:
    void openOutputFile(const fs::path& output_file);
    void doCompress();
    virtual void onAfterWrite() = 0;
    void finishCompression();

    asio::io_context& io_context_;
    StreamHandle output_stream_;
    std::array<char, CHUNK> out_buffer_{};
    z_stream zlib_stream_{};
};

class SingleFileCompressor : public BaseCompressor {
public:
    SingleFileCompressor(asio::io_context& io_context,
                         const fs::path& input_file,
                         const fs::path& output_file);
    void start() override;

private:
    void openInputFile(const fs::path& input_file);
    void doRead();
    void onAfterWrite() override;

    StreamHandle input_stream_;
    std::array<char, CHUNK> in_buffer_{};
};

class DirectoryCompressor : public BaseCompressor {
public:
    DirectoryCompressor(asio::io_context& io_context, fs::path input_dir,
                        const fs::path& output_file);
    void start() override;

private:
    void doCompressNextFile();
    void doRead();
    void onAfterWrite() override;

    fs::path input_dir_;
    std::vector<fs::path> files_to_compress_;
    fs::path current_file_;
    std::ifstream input_stream_;
    std::array<char, CHUNK> in_buffer_{};
};

class BaseDecompressor {
public:
    explicit BaseDecompressor(asio::io_context& io_context);
    virtual void start() = 0;

protected:
    void decompress(gzFile source, StreamHandle& output_stream);
    void doRead();
    virtual void done() = 0;

    asio::io_context& io_context_;
    StreamHandle* out_stream_{};
    std::array<char, CHUNK> in_buffer_{};
    gzFile in_file_{};
};

class SingleFileDecompressor : public BaseDecompressor {
public:
    SingleFileDecompressor(asio::io_context& io_context, fs::path input_file,
                           fs::path output_folder);
    void start() override;

private:
    void done() override;

    fs::path input_file_;
    fs::path output_folder_;
    StreamHandle output_stream_;
};

class DirectoryDecompressor : public BaseDecompressor {
public:
    DirectoryDecompressor(asio::io_context& io_context,
                          const fs::path& input_dir,
                          const fs::path& output_folder);
    void start() override;

private:
    void decompressNextFile();
    void done() override;

    fs::path input_dir_;
    fs::path output_folder_;
    StreamHandle output_stream_;
    std::vector<fs::path> files_to_decompress_;
    fs::path current_file_;
};

class ZipOperation {
public:
    virtual void start() = 0;
};

class ListFilesInZip : public ZipOperation {
public:
    ListFilesInZip(asio::io_context& io_context, std::string_view zip_file);
    void start() override;
    [[nodiscard]] auto getFileList() const -> std::vector<std::string>;

private:
    void listFiles();
    asio::io_context& io_context_;
    std::string zip_file_;
    std::vector<std::string> fileList_;
};

class FileExistsInZip : public ZipOperation {
public:
    FileExistsInZip(asio::io_context& io_context, std::string_view zip_file,
                    std::string_view file_name);
    void start() override;
    [[nodiscard]] auto found() const -> bool;

private:
    void checkFileExists();
    asio::io_context& io_context_;
    std::string zip_file_;
    std::string file_name_;
    bool fileExists_ = false;
};

class RemoveFileFromZip : public ZipOperation {
public:
    RemoveFileFromZip(asio::io_context& io_context, std::string_view zip_file,
                      std::string_view file_name);
    void start() override;
    [[nodiscard]] auto isSuccessful() const -> bool;

private:
    void removeFile();
    asio::io_context& io_context_;
    std::string zip_file_;
    std::string file_name_;
    bool success_ = false;
};

class GetZipFileSize : public ZipOperation {
public:
    GetZipFileSize(asio::io_context& io_context, std::string_view zip_file);
    void start() override;
    [[nodiscard]] auto getSizeValue() const -> size_t;

private:
    void getSize();
    asio::io_context& io_context_;
    std::string zip_file_;
    size_t size_ = 0;
};
}  // namespace atom::async::io

#endif  // ASYNC_COMPRESS_HPP