#include "atom/io/io.hpp"
#include <gtest/gtest.h>

#include <fstream>
using namespace std::literals;

TEST(IO_Test, CreateDirectory_Test) {
    std::string path = "test_directory";
    bool result = atom::io::createDirectory(path);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove_all(path);
}

TEST(IO_Test, CreateDirectoriesRecursive_Test) {
    std::string basePath = "test_base";
    std::vector<std::string> subdirs = {"subdir1", "subdir2"};
    atom::io::CreateDirectoriesOptions options;
    bool result =
        atom::io::createDirectoriesRecursive(basePath, subdirs, options);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove_all(basePath);
}

TEST(IO_Test, RemoveDirectory_Test) {
    std::string path = "test_directory";
    fs::create_directory(path);
    bool result = atom::io::removeDirectory(path);
    EXPECT_TRUE(result);
}

TEST(IO_Test, RemoveDirectoriesRecursive_Test) {
    std::string basePath = "test_base";
    std::vector<std::string> subdirs = {"subdir1", "subdir2"};
    fs::create_directories(basePath + subdirs[0]);
    fs::create_directories(basePath + subdirs[1]);
    atom::io::CreateDirectoriesOptions options;
    bool result =
        atom::io::removeDirectoriesRecursive(basePath, subdirs, options);
    EXPECT_TRUE(result);
}

TEST(IO_Test, RenameDirectory_Test) {
    std::string oldPath = "test_directory";
    std::string newPath = "renamed_directory";
    fs::create_directory(oldPath);
    bool result = atom::io::renameDirectory(oldPath, newPath);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove_all(newPath);
}

TEST(IO_Test, MoveDirectory_Test) {
    std::string oldPath = "test_directory";
    std::string newPath = "moved_directory";
    fs::create_directory(oldPath);
    bool result = atom::io::moveDirectory(oldPath, newPath);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove_all(newPath);
}

TEST(IO_Test, CopyFile_Test) {
    std::string srcPath = "test_file.txt";
    std::string dstPath = "copied_file.txt";
    std::ofstream srcFile(srcPath);
    srcFile << "This is a test file.";
    srcFile.close();
    bool result = atom::io::copyFile(srcPath, dstPath);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove(srcPath);
    fs::remove(dstPath);
}

TEST(IO_Test, MoveFile_Test) {
    std::string srcPath = "test_file.txt";
    std::string dstPath = "moved_file.txt";
    std::ofstream srcFile(srcPath);
    srcFile << "This is a test file.";
    srcFile.close();
    bool result = atom::io::moveFile(srcPath, dstPath);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove(dstPath);
}

TEST(IO_Test, RenameFile_Test) {
    std::string oldPath = "test_file.txt";
    std::string newPath = "renamed_file.txt";
    std::ofstream oldFile(oldPath);
    oldFile << "This is a test file.";
    oldFile.close();
    bool result = atom::io::renameFile(oldPath, newPath);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove(newPath);
}

TEST(IO_Test, RemoveFile_Test) {
    std::string path = "test_file.txt";
    std::ofstream file(path);
    file << "This is a test file.";
    file.close();
    bool result = atom::io::removeFile(path);
    EXPECT_TRUE(result);
}

TEST(IO_Test, CreateSymlink_Test) {
    std::string targetPath = "target_file.txt";
    std::string symlinkPath = "symlink_file.txt";
    std::ofstream targetFile(targetPath);
    targetFile << "This is a target file.";
    targetFile.close();
    bool result = atom::io::createSymlink(targetPath, symlinkPath);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove(targetPath);
    fs::remove(symlinkPath);
}

TEST(IO_Test, RemoveSymlink_Test) {
    std::string path = "symlink_file.txt";
    bool result = atom::io::removeSymlink(path);
    EXPECT_TRUE(result);
}

TEST(IO_Test, FileSize_Test) {
    std::string path = "test_file.txt";
    std::ofstream file(path);
    file << "This is a test file.";
    file.close();
    uintmax_t size = atom::io::fileSize(path);
    EXPECT_EQ(size, 22);

    // Clean up
    fs::remove(path);
}

TEST(IO_Test, TruncateFile_Test) {
    std::string path = "test_file.txt";
    std::ofstream file(path);
    file << "This is a test file.";
    file.close();
    bool result = atom::io::truncateFile(path, 10);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove(path);
}

TEST(IO_Test, JWalk_Test) {
    std::string root = "test_directory";
    fs::create_directory(root);
    atom::io::jwalk(root);

    // Clean up
    fs::remove_all(root);
}

TEST(IO_Test, FWalk_Test) {
    std::string root = "test_directory";
    fs::create_directory(root);
    atom::io::fwalk(root, [](const fs::path& path) { EXPECT_TRUE(true); });

    // Clean up
    fs::remove_all(root);
}

TEST(IO_Test, ConvertToLinuxPath_Test) {
    std::string windowsPath = "C:\\Windows\\System32";
    std::string linuxPath = atom::io::convertToLinuxPath(windowsPath);
    EXPECT_EQ(linuxPath, "C:/Windows/System32");
}

TEST(IO_Test, ConvertToWindowsPath_Test) {
    std::string linuxPath = "/home/user/Documents";
    std::string windowsPath = atom::io::convertToWindowsPath(linuxPath);
    EXPECT_EQ(windowsPath, "C:\\Users\\User\\Documents");
}

TEST(IO_Test, NormPath_Test) {
    std::string rawPath = "../test_directory/./subdir/";
    std::string normPath = atom::io::normPath(rawPath);
    EXPECT_EQ(normPath, "../test_directory/subdir/");
}

TEST(IO_Test, IsFolderNameValid_Test) {
    std::string validFolderName = "test_folder";
    std::string invalidFolderName = "test/folder";
    EXPECT_TRUE(atom::io::isFolderNameValid(validFolderName));
    EXPECT_FALSE(atom::io::isFolderNameValid(invalidFolderName));
}

TEST(IO_Test, IsFileNameValid_Test) {
    std::string validFileName = "test_file.txt";
    std::string invalidFileName = "test/file.txt";
    EXPECT_TRUE(atom::io::isFileNameValid(validFileName));
    EXPECT_FALSE(atom::io::isFileNameValid(invalidFileName));
}

TEST(IO_Test, IsFolderExists_Test) {
    std::string folderPath = "test_directory";
    fs::create_directory(folderPath);
    EXPECT_TRUE(atom::io::isFolderExists(folderPath));
    EXPECT_FALSE(atom::io::isFolderExists("nonexistent_directory"s));

    // Clean up
    fs::remove_all(folderPath);
}

TEST(IO_Test, IsFileExists_Test) {
    std::string filePath = "test_file.txt";
    std::ofstream file(filePath);
    file << "This is a test file.";
    file.close();
    EXPECT_TRUE(atom::io::isFileExists(filePath));
    EXPECT_FALSE(atom::io::isFileExists("nonexistent_file.txt"s));
}

TEST(IO_Test, IsFolderEmpty_Test) {
    std::string folderPath = "test_directory";
    fs::create_directory(folderPath);
    EXPECT_TRUE(atom::io::isFolderEmpty(folderPath));

    // Add a file to the folder
    std::string filePath = folderPath + "/test_file.txt";
    std::ofstream file(filePath);
    file << "This is a test file.";
    file.close();
    EXPECT_FALSE(atom::io::isFolderEmpty(folderPath));

    // Clean up
    fs::remove_all(folderPath);
}

TEST(IO_Test, IsAbsolutePath_Test) {
    std::string absolutePath = "/home/user/Documents";
    std::string relativePath = "test_directory";
    EXPECT_TRUE(atom::io::isAbsolutePath(absolutePath));
    EXPECT_FALSE(atom::io::isAbsolutePath(relativePath));
}

TEST(IO_Test, ChangeWorkingDirectory_Test) {
    std::string directoryPath = "test_directory";
    fs::create_directory(directoryPath);
    bool result = atom::io::changeWorkingDirectory(directoryPath);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove_all(directoryPath);
}

TEST(IO_Test, GetFileTimes_Test) {
    std::string filePath = "test_file.txt";
    std::ofstream file(filePath);
    file << "This is a test file.";
    file.close();
    std::pair<std::string, std::string> fileTimes =
        atom::io::getFileTimes(filePath);
    EXPECT_FALSE(fileTimes.first.empty());
    EXPECT_FALSE(fileTimes.second.empty());

    // Clean up
    fs::remove(filePath);
}

TEST(IO_Test, CheckFileTypeInFolder_Test) {
    std::string folderPath = "test_directory";
    std::string fileType = ".txt";
    atom::io::FileOption fileOption = atom::io::FileOption::NAME;
    fs::create_directory(folderPath);
    std::string filePath = folderPath + "/test_file.txt";
    std::ofstream file(filePath);
    file << "This is a test file.";
    file.close();
    std::vector<std::string> filePaths =
        atom::io::checkFileTypeInFolder(folderPath, fileType, fileOption);
    EXPECT_EQ(filePaths.size(), 1);

    // Clean up
    fs::remove_all(folderPath);
}

TEST(IO_Test, IsExecutableFile_Test) {
    std::string fileName = "test_file";
    std::string fileExt = ".exe";
    std::string filePath = fileName + fileExt;
    std::ofstream file(filePath);
    file << "This is a test file.";
    file.close();
    bool result = atom::io::isExecutableFile(fileName, fileExt);
    EXPECT_TRUE(result);

    // Clean up
    fs::remove(filePath);
}
