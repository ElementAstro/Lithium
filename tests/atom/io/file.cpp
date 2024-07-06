#include "atom/io/file.hpp"
#include <gtest/gtest.h>

using namespace atom::io;

class FileManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试用例执行前的设置代码
        // 可以在这里初始化 FileManager 对象
        fileManager = new FileManager();
    }

    void TearDown() override {
        // 在每个测试用例执行后的清理代码
        // 可以在这里释放 FileManager 对象
        delete fileManager;
    }

    FileManager* fileManager;
};

TEST_F(FileManagerTest, CreateFileTest) {
    std::string filename = "testfile.txt";
    bool result = fileManager->createFile(filename);
    // 验证文件是否成功创建
    EXPECT_TRUE(result);
    // 验证文件是否存在
    std::ifstream file(filename);
    EXPECT_TRUE(file.good());
    file.close();
}

TEST_F(FileManagerTest, OpenFileTest) {
    std::string filename = "testfile.txt";
    bool result = fileManager->openFile(filename);
    // 验证文件是否成功打开
    EXPECT_TRUE(result);
}

TEST_F(FileManagerTest, ReadFileTest) {
    std::string filename = "testfile.txt";
    std::string contents;
    bool result = fileManager->readFile(contents);
    // 验证文件是否成功读取
    EXPECT_TRUE(result);
    // 验证读取的内容是否为空
    EXPECT_FALSE(contents.empty());
}

TEST_F(FileManagerTest, WriteFileTest) {
    std::string filename = "testfile.txt";
    std::string contents = "Hello, World!";
    bool result = fileManager->writeFile(contents);
    // 验证文件是否成功写入
    EXPECT_TRUE(result);
    // 验证文件内容是否与写入的内容一致
    std::ifstream file(filename);
    std::string fileContents((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    EXPECT_EQ(fileContents, contents);
    file.close();
}

TEST_F(FileManagerTest, MoveFileTest) {
    std::string oldFilename = "testfile.txt";
    std::string newFilename = "movedfile.txt";
    bool result = fileManager->moveFile(oldFilename, newFilename);
    // 验证文件是否成功移动
    EXPECT_TRUE(result);
    // 验证原文件是否已删除
    std::ifstream file(oldFilename);
    EXPECT_FALSE(file.good());
    file.close();
    // 验证新文件是否存在
    std::ifstream newFile(newFilename);
    EXPECT_TRUE(newFile.good());
    newFile.close();
}

TEST_F(FileManagerTest, DeleteFileTest) {
    std::string filename = "testfile.txt";
    bool result = fileManager->deleteFile(filename);
    // 验证文件是否成功删除
    EXPECT_TRUE(result);
    // 验证文件是否已删除
    std::ifstream file(filename);
    EXPECT_FALSE(file.good());
    file.close();
}

TEST_F(FileManagerTest, GetFileSizeTest) {
    std::string filename = "testfile.txt";
    long fileSize = fileManager->getFileSize();
    // 验证获取的文件大小是否正确
    EXPECT_EQ(fileSize, 0);  // 假设文件为空
}

TEST_F(FileManagerTest, GetFileDirectoryTest) {
    std::string filename = "testfile.txt";
    std::string fileDirectory = FileManager::getFileDirectory(filename);
    // 验证获取的文件目录是否正确
    // 这里的验证需要根据实际的文件系统路径进行调整
    EXPECT_EQ(fileDirectory, "./");  // 假设文件位于当前目
}