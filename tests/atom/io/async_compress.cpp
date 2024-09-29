#include <gtest/gtest.h>
#include <asio.hpp>
#include <filesystem>
#include <string_view>

#include "atom/io/async_compress.hpp"

namespace fs = std::filesystem;
using namespace atom::async::io;

class AsyncCompressTest : public ::testing::Test {
protected:
    asio::io_context io_context;
    fs::path test_file = "test_file.txt";
    fs::path test_dir = "test_dir";
    fs::path output_file = "output_file.gz";
    fs::path output_folder = "output_folder";

    void SetUp() override {
        // Create test files and directories
        std::ofstream(test_file) << "test content";
        fs::create_directory(test_dir);
        std::ofstream(test_dir / "file1.txt") << "file1 content";
        std::ofstream(test_dir / "file2.txt") << "file2 content";
    }

    void TearDown() override {
        // Clean up test files and directories
        fs::remove(test_file);
        fs::remove_all(test_dir);
        fs::remove(output_file);
        fs::remove_all(output_folder);
    }
};

TEST_F(AsyncCompressTest, SingleFileCompressorTest) {
    SingleFileCompressor compressor(io_context, test_file, output_file);
    compressor.start();
    io_context.run();
    ASSERT_TRUE(fs::exists(output_file));
}

TEST_F(AsyncCompressTest, DirectoryCompressorTest) {
    DirectoryCompressor compressor(io_context, test_dir, output_file);
    compressor.start();
    io_context.run();
    ASSERT_TRUE(fs::exists(output_file));
}

TEST_F(AsyncCompressTest, SingleFileDecompressorTest) {
    SingleFileCompressor compressor(io_context, test_file, output_file);
    compressor.start();
    io_context.run();

    SingleFileDecompressor decompressor(io_context, output_file, output_folder);
    decompressor.start();
    io_context.run();
    ASSERT_TRUE(fs::exists(output_folder / "test_file.txt"));
}

TEST_F(AsyncCompressTest, DirectoryDecompressorTest) {
    DirectoryCompressor compressor(io_context, test_dir, output_file);
    compressor.start();
    io_context.run();

    DirectoryDecompressor decompressor(io_context, output_file, output_folder);
    decompressor.start();
    io_context.run();
    ASSERT_TRUE(fs::exists(output_folder / "file1.txt"));
    ASSERT_TRUE(fs::exists(output_folder / "file2.txt"));
}

TEST_F(AsyncCompressTest, ListFilesInZipTest) {
    ListFilesInZip listFiles(io_context, output_file.string());
    listFiles.start();
    io_context.run();
    auto files = listFiles.getFileList();
    ASSERT_FALSE(files.empty());
}

TEST_F(AsyncCompressTest, FileExistsInZipTest) {
    FileExistsInZip fileExists(io_context, output_file.string(), "file1.txt");
    fileExists.start();
    io_context.run();
    ASSERT_TRUE(fileExists.found());
}

TEST_F(AsyncCompressTest, RemoveFileFromZipTest) {
    RemoveFileFromZip removeFile(io_context, output_file.string(), "file1.txt");
    removeFile.start();
    io_context.run();
    ASSERT_TRUE(removeFile.isSuccessful());
}

TEST_F(AsyncCompressTest, GetZipFileSizeTest) {
    GetZipFileSize getSize(io_context, output_file.string());
    getSize.start();
    io_context.run();
    ASSERT_GT(getSize.getSizeValue(), 0);
}