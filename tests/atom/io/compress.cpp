#include "atom/io/compress.hpp"
#include <gtest/gtest.h>

#include <filesystem>
namespace fs = std::filesystem;

TEST(CompressTest, CompressFile) {
    // Test compressing a file
    std::string fileName = "test.txt";
    std::string outputFolder = "output";
    bool result = atom::io::compressFile(fileName, outputFolder);
    EXPECT_TRUE(result);  // Expect compression to succeed

    // Test compressing a file that is already compressed
    bool result2 = atom::io::compressFile(fileName + ".gz", outputFolder);
    EXPECT_FALSE(result2);  // Expect compression to fail

    // Cleanup
    fs::remove(fileName + ".gz");
}

TEST(CompressTest, DecompressFile) {
    // Test decompressing a file
    std::string fileName = "test.txt.gz";
    std::string outputFolder = "output";
    bool result = atom::io::decompressFile(fileName, outputFolder);
    EXPECT_TRUE(result);  // Expect decompression to succeed

    // Test decompressing a file that is not compressed
    bool result2 = atom::io::decompressFile("test.txt", outputFolder);
    EXPECT_FALSE(result2);  // Expect decompression to fail

    // Cleanup
    fs::remove("test.txt");
}

TEST(CompressTest, CompressFolder) {
    // Test compressing a folder
    std::string folderName = "test_folder";
    bool result = atom::io::compressFolder(folderName.c_str());
    EXPECT_TRUE(result);  // Expect compression to succeed

    // Cleanup
    fs::remove(folderName + ".gz");
    fs::remove(folderName);
}

TEST(CompressTest, ExtractZip) {
    // Test extracting a zip file
    std::string zipFile = "test.zip";
    std::string destinationFolder = "output";
    bool result = atom::io::extractZip(zipFile, destinationFolder);
    EXPECT_TRUE(result);  // Expect extraction to succeed

    // Test extracting a zip file that does not exist
    bool result2 = atom::io::extractZip("nonexistent.zip", destinationFolder);
    EXPECT_FALSE(result2);  // Expect extraction to fail

    // Cleanup
    fs::remove(zipFile);
    fs::remove(destinationFolder);
}

TEST(CompressTest, CreateZip) {
    // Test creating a zip file
    std::string sourceFolder = "test_folder";
    std::string zipFile = "test.zip";
    bool result = atom::io::createZip(sourceFolder, zipFile);
    EXPECT_TRUE(result);  // Expect creation to succeed

    // Test creating a zip file with a nonexistent source folder
    bool result2 = atom::io::createZip("nonexistent_folder", zipFile);
    EXPECT_FALSE(result2);  // Expect creation to fail

    // Cleanup
    fs::remove(zipFile);
    fs::remove(sourceFolder);
}

TEST(CompressTest, ListFilesInZip) {
    // Test listing files in a zip file
    std::string zipFile = "test.zip";
    std::vector<std::string> result = atom::io::listFilesInZip(zipFile);
    EXPECT_EQ(result.size(), 0);  // Expect empty list since zip file is empty

    // Test listing files in a nonexistent zip file
    std::vector<std::string> result2 =
        atom::io::listFilesInZip("nonexistent.zip");
    EXPECT_EQ(result2.size(),
              0);  // Expect empty list since zip file does not exist

    // Cleanup
    fs::remove(zipFile);
}

TEST(CompressTest, FileExistsInZip) {
    // Test checking if a file exists in a zip file
    std::string zipFile = "test.zip";
    std::string fileName = "file.txt";
    bool result = atom::io::fileExistsInZip(zipFile, fileName);
    EXPECT_FALSE(result);  // Expect file to not exist since zip file is empty

    // Test checking if a file exists in a nonexistent zip file
    bool result2 = atom::io::fileExistsInZip("nonexistent.zip", fileName);
    EXPECT_FALSE(
        result2);  // Expect file to not exist since zip file does not exist

    // Cleanup
    fs::remove(zipFile);
}

TEST(CompressTest, RemoveFileFromZip) {
    // Test removing a file from a zip file
    std::string zipFile = "test.zip";
    std::string fileName = "file.txt";
    bool result = atom::io::removeFileFromZip(zipFile, fileName);
    EXPECT_FALSE(result);  // Expect removal to fail since zip file is empty

    // Test removing a file from a nonexistent zip file
    bool result2 = atom::io::removeFileFromZip("nonexistent.zip", fileName);
    EXPECT_FALSE(
        result2);  // Expect removal to fail since zip file does not exist

    // Cleanup
    fs::remove(zipFile);
}

TEST(CompressTest, GetZipFileSize) {
    // Test getting the size of a zip file
    std::string zipFile = "test.zip";
    size_t result = atom::io::getZipFileSize(zipFile);
    EXPECT_EQ(result, 0);  // Expect size to be 0 since zip file is empty

    // Test getting the size of a nonexistent zip file
    size_t result2 = atom::io::getZipFileSize("nonexistent.zip");
    EXPECT_EQ(result2, 0);  // Expect size to be 0 since zip file does not exist

    // Cleanup
    fs::remove(zipFile);
}
