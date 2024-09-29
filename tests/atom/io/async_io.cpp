#include "atom/io/async_io.hpp"

#include <gtest/gtest.h>
#include <asio.hpp>
#include <filesystem>
#include <fstream>

using namespace atom::async::io;

class AsyncIOTest : public ::testing::Test {
protected:
    asio::io_context io_context;
    AsyncFile asyncFile{io_context};
    AsyncDirectory asyncDirectory{io_context};

    void SetUp() override {
        // Create test files and directories
        std::ofstream("test_file.txt") << "test content";
        std::filesystem::create_directory("test_dir");
    }

    void TearDown() override {
        // Clean up test files and directories
        std::filesystem::remove("test_file.txt");
        std::filesystem::remove_all("test_dir");
    }
};

// AsyncFile Tests

TEST_F(AsyncIOTest, AsyncRead_ValidFile) {
    bool callback_called = false;
    asyncFile.asyncRead("test_file.txt", [&](const std::string& content) {
        EXPECT_EQ(content, "test content");
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncRead_NonExistentFile) {
    bool callback_called = false;
    asyncFile.asyncRead("non_existent.txt", [&](const std::string& content) {
        EXPECT_TRUE(content.empty());
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncWrite_ValidContent) {
    bool callback_called = false;
    asyncFile.asyncWrite("test_write.txt", "write content", [&](bool success) {
        EXPECT_TRUE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
    std::ifstream file("test_write.txt");
    std::string content;
    file >> content;
    EXPECT_EQ(content, "write content");
    std::filesystem::remove("test_write.txt");
}

TEST_F(AsyncIOTest, AsyncWrite_InvalidPath) {
    bool callback_called = false;
    asyncFile.asyncWrite("/invalid_path/test_write.txt", "write content",
                         [&](bool success) {
                             EXPECT_FALSE(success);
                             callback_called = true;
                         });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncDelete_ValidFile) {
    bool callback_called = false;
    asyncFile.asyncDelete("test_file.txt", [&](bool success) {
        EXPECT_TRUE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
    EXPECT_FALSE(std::filesystem::exists("test_file.txt"));
}

TEST_F(AsyncIOTest, AsyncDelete_NonExistentFile) {
    bool callback_called = false;
    asyncFile.asyncDelete("non_existent.txt", [&](bool success) {
        EXPECT_FALSE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncCopy_ValidSourceAndDestination) {
    bool callback_called = false;
    asyncFile.asyncCopy("test_file.txt", "test_copy.txt", [&](bool success) {
        EXPECT_TRUE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
    EXPECT_TRUE(std::filesystem::exists("test_copy.txt"));
    std::filesystem::remove("test_copy.txt");
}

TEST_F(AsyncIOTest, AsyncCopy_InvalidSource) {
    bool callback_called = false;
    asyncFile.asyncCopy("non_existent.txt", "test_copy.txt", [&](bool success) {
        EXPECT_FALSE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncReadWithTimeout_ValidFile_SufficientTimeout) {
    bool callback_called = false;
    asyncFile.asyncReadWithTimeout("test_file.txt", 1000,
                                   [&](const std::string& content) {
                                       EXPECT_EQ(content, "test content");
                                       callback_called = true;
                                   });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncReadWithTimeout_ValidFile_InsufficientTimeout) {
    bool callback_called = false;
    asyncFile.asyncReadWithTimeout("test_file.txt", 1,
                                   [&](const std::string& content) {
                                       EXPECT_TRUE(content.empty());
                                       callback_called = true;
                                   });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncBatchRead_MultipleValidFiles) {
    std::ofstream("test_file2.txt") << "test content 2";
    bool callback_called = false;
    asyncFile.asyncBatchRead({"test_file.txt", "test_file2.txt"},
                             [&](const std::vector<std::string>& contents) {
                                 EXPECT_EQ(contents.size(), 2);
                                 EXPECT_EQ(contents[0], "test content");
                                 EXPECT_EQ(contents[1], "test content 2");
                                 callback_called = true;
                             });
    io_context.run();
    EXPECT_TRUE(callback_called);
    std::filesystem::remove("test_file2.txt");
}

TEST_F(AsyncIOTest, AsyncBatchRead_SomeInvalidFiles) {
    bool callback_called = false;
    asyncFile.asyncBatchRead({"test_file.txt", "non_existent.txt"},
                             [&](const std::vector<std::string>& contents) {
                                 EXPECT_EQ(contents.size(), 2);
                                 EXPECT_EQ(contents[0], "test content");
                                 EXPECT_TRUE(contents[1].empty());
                                 callback_called = true;
                             });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncStat_ValidFile) {
    bool callback_called = false;
    asyncFile.asyncStat("test_file.txt", [&](bool exists, std::uintmax_t size,
                                             std::time_t mtime) {
        EXPECT_TRUE(exists);
        EXPECT_GT(size, 0);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncStat_NonExistentFile) {
    bool callback_called = false;
    asyncFile.asyncStat(
        "non_existent.txt",
        [&](bool exists, std::uintmax_t size, std::time_t mtime) {
            EXPECT_FALSE(exists);
            callback_called = true;
        });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncMove_ValidSourceAndDestination) {
    bool callback_called = false;
    asyncFile.asyncMove("test_file.txt", "test_move.txt", [&](bool success) {
        EXPECT_TRUE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
    EXPECT_TRUE(std::filesystem::exists("test_move.txt"));
    std::filesystem::remove("test_move.txt");
}

TEST_F(AsyncIOTest, AsyncMove_InvalidSource) {
    bool callback_called = false;
    asyncFile.asyncMove("non_existent.txt", "test_move.txt", [&](bool success) {
        EXPECT_FALSE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncChangePermissions_ValidFile) {
    bool callback_called = false;
    asyncFile.asyncChangePermissions(
        "test_file.txt", std::filesystem::perms::owner_all, [&](bool success) {
            EXPECT_TRUE(success);
            callback_called = true;
        });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncChangePermissions_InvalidFile) {
    bool callback_called = false;
    asyncFile.asyncChangePermissions("non_existent.txt",
                                     std::filesystem::perms::owner_all,
                                     [&](bool success) {
                                         EXPECT_FALSE(success);
                                         callback_called = true;
                                     });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncCreateDirectory_ValidPath) {
    bool callback_called = false;
    asyncFile.asyncCreateDirectory("test_create_dir", [&](bool success) {
        EXPECT_TRUE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
    EXPECT_TRUE(std::filesystem::exists("test_create_dir"));
    std::filesystem::remove("test_create_dir");
}

TEST_F(AsyncIOTest, AsyncCreateDirectory_InvalidPath) {
    bool callback_called = false;
    asyncFile.asyncCreateDirectory("/invalid_path/test_create_dir",
                                   [&](bool success) {
                                       EXPECT_FALSE(success);
                                       callback_called = true;
                                   });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncExists_ValidFile) {
    bool callback_called = false;
    asyncFile.asyncExists("test_file.txt", [&](bool exists) {
        EXPECT_TRUE(exists);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncExists_NonExistentFile) {
    bool callback_called = false;
    asyncFile.asyncExists("non_existent.txt", [&](bool exists) {
        EXPECT_FALSE(exists);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

// AsyncDirectory Tests

TEST_F(AsyncIOTest, AsyncDirectoryCreate_ValidPath) {
    bool callback_called = false;
    asyncDirectory.asyncCreate("test_create_dir", [&](bool success) {
        EXPECT_TRUE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
    EXPECT_TRUE(std::filesystem::exists("test_create_dir"));
    std::filesystem::remove("test_create_dir");
}

TEST_F(AsyncIOTest, AsyncDirectoryCreate_InvalidPath) {
    bool callback_called = false;
    asyncDirectory.asyncCreate("/invalid_path/test_create_dir",
                               [&](bool success) {
                                   EXPECT_FALSE(success);
                                   callback_called = true;
                               });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncDirectoryRemove_ValidPath) {
    std::filesystem::create_directory("test_remove_dir");
    bool callback_called = false;
    asyncDirectory.asyncRemove("test_remove_dir", [&](bool success) {
        EXPECT_TRUE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
    EXPECT_FALSE(std::filesystem::exists("test_remove_dir"));
}

TEST_F(AsyncIOTest, AsyncDirectoryRemove_NonExistentPath) {
    bool callback_called = false;
    asyncDirectory.asyncRemove("non_existent_dir", [&](bool success) {
        EXPECT_FALSE(success);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncDirectoryListContents_ValidDirectory) {
    std::ofstream("test_dir/file1.txt") << "content1";
    std::ofstream("test_dir/file2.txt") << "content2";
    bool callback_called = false;
    asyncDirectory.asyncListContents(
        "test_dir", [&](const std::vector<std::string>& contents) {
            EXPECT_EQ(contents.size(), 2);
            EXPECT_NE(std::find(contents.begin(), contents.end(), "file1.txt"),
                      contents.end());
            EXPECT_NE(std::find(contents.begin(), contents.end(), "file2.txt"),
                      contents.end());
            callback_called = true;
        });
    io_context.run();
    EXPECT_TRUE(callback_called);
    std::filesystem::remove("test_dir/file1.txt");
    std::filesystem::remove("test_dir/file2.txt");
}

TEST_F(AsyncIOTest, AsyncDirectoryListContents_InvalidDirectory) {
    bool callback_called = false;
    asyncDirectory.asyncListContents(
        "non_existent_dir", [&](const std::vector<std::string>& contents) {
            EXPECT_TRUE(contents.empty());
            callback_called = true;
        });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncDirectoryExists_ValidDirectory) {
    bool callback_called = false;
    asyncDirectory.asyncExists("test_dir", [&](bool exists) {
        EXPECT_TRUE(exists);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}

TEST_F(AsyncIOTest, AsyncDirectoryExists_NonExistentDirectory) {
    bool callback_called = false;
    asyncDirectory.asyncExists("non_existent_dir", [&](bool exists) {
        EXPECT_FALSE(exists);
        callback_called = true;
    });
    io_context.run();
    EXPECT_TRUE(callback_called);
}