#include "atom/system/stat.hpp"
#include <gtest/gtest.h>

#include <fstream>


using namespace atom::system;
namespace fs = std::filesystem;

// Helper function to create a sample file for testing
void createSampleFile(const fs::path& path) {
    std::ofstream file(path);
    file << "Sample content";
    file.close();
}

// Helper function to create a sample directory for testing
void createSampleDirectory(const fs::path& path) { fs::create_directory(path); }

// Test fixture for setting up common test environment
class StatTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a sample file and directory for testing
        createSampleFile(testFilePath);
        createSampleDirectory(testDirPath);
    }

    void TearDown() override {
        // Remove the sample file and directory
        fs::remove(testFilePath);
        fs::remove(testDirPath);
    }

    fs::path testFilePath = "test_file.txt";
    fs::path testDirPath = "test_directory";
};

// Test constructor and update function
TEST_F(StatTest, ConstructorAndUpdate) {
    Stat fileStat(testFilePath);
    EXPECT_NO_THROW(fileStat.update());

    Stat dirStat(testDirPath);
    EXPECT_NO_THROW(dirStat.update());
}

// Test type function
TEST_F(StatTest, Type) {
    Stat fileStat(testFilePath);
    EXPECT_EQ(fileStat.type(), fs::file_type::regular);

    Stat dirStat(testDirPath);
    EXPECT_EQ(dirStat.type(), fs::file_type::directory);
}

// Test size function
TEST_F(StatTest, Size) {
    Stat fileStat(testFilePath);
    EXPECT_GT(fileStat.size(), 0);

    Stat dirStat(testDirPath);
    EXPECT_EQ(dirStat.size(), 0);  // Directory size is implementation-dependent
}

// Test atime function
TEST_F(StatTest, Atime) {
    Stat fileStat(testFilePath);
    EXPECT_GT(fileStat.atime(), 0);

    Stat dirStat(testDirPath);
    EXPECT_GT(dirStat.atime(), 0);
}

// Test mtime function
TEST_F(StatTest, Mtime) {
    Stat fileStat(testFilePath);
    EXPECT_GT(fileStat.mtime(), 0);

    Stat dirStat(testDirPath);
    EXPECT_GT(dirStat.mtime(), 0);
}

// Test ctime function
TEST_F(StatTest, Ctime) {
    Stat fileStat(testFilePath);
    EXPECT_GT(fileStat.ctime(), 0);

    Stat dirStat(testDirPath);
    EXPECT_GT(dirStat.ctime(), 0);
}

// Test mode function
TEST_F(StatTest, Mode) {
    Stat fileStat(testFilePath);
    EXPECT_GT(fileStat.mode(), 0);

    Stat dirStat(testDirPath);
    EXPECT_GT(dirStat.mode(), 0);
}

// Test uid function
TEST_F(StatTest, Uid) {
#ifndef _WIN32
    Stat fileStat(testFilePath);
    EXPECT_GE(fileStat.uid(), 0);

    Stat dirStat(testDirPath);
    EXPECT_GE(dirStat.uid(), 0);
#else
    GTEST_SKIP() << "Skipping UID test on Windows";
#endif
}

// Test gid function
TEST_F(StatTest, Gid) {
#ifndef _WIN32
    Stat fileStat(testFilePath);
    EXPECT_GE(fileStat.gid(), 0);

    Stat dirStat(testDirPath);
    EXPECT_GE(dirStat.gid(), 0);
#else
    GTEST_SKIP() << "Skipping GID test on Windows";
#endif
}

// Test path function
TEST_F(StatTest, Path) {
    Stat fileStat(testFilePath);
    EXPECT_EQ(fileStat.path(), testFilePath);

    Stat dirStat(testDirPath);
    EXPECT_EQ(dirStat.path(), testDirPath);
}
