#include "atom/io/async_glob.hpp"

#include <gtest/gtest.h>
#include <asio.hpp>
#include <filesystem>
#include <vector>

namespace atom::io::test {

class AsyncGlobTest : public ::testing::Test {
protected:
    asio::io_context io_context;
    AsyncGlob* async_glob;

    void SetUp() override { async_glob = new AsyncGlob(io_context); }

    void TearDown() override { delete async_glob; }
};

TEST_F(AsyncGlobTest, ConstructorTest) { ASSERT_NE(async_glob, nullptr); }

TEST_F(AsyncGlobTest, SimpleGlobTest) {
    std::vector<fs::path> result;
    async_glob->glob("*.txt",
                     [&](std::vector<fs::path> paths) { result = paths; });
    io_context.run();
    ASSERT_FALSE(result.empty());
}

TEST_F(AsyncGlobTest, RecursiveGlobTest) {
    std::vector<fs::path> result;
    async_glob->glob(
        "**/*.txt", [&](std::vector<fs::path> paths) { result = paths; }, true);
    io_context.run();
    ASSERT_FALSE(result.empty());
}

TEST_F(AsyncGlobTest, DirectoryOnlyGlobTest) {
    std::vector<fs::path> result;
    async_glob->glob(
        "*", [&](std::vector<fs::path> paths) { result = paths; }, false, true);
    io_context.run();
    for (const auto& path : result) {
        ASSERT_TRUE(fs::is_directory(path));
    }
}

TEST_F(AsyncGlobTest, SpecialCharacterGlobTest) {
    std::vector<fs::path> result;
    async_glob->glob("file[0-9].txt",
                     [&](std::vector<fs::path> paths) { result = paths; });
    io_context.run();
    ASSERT_FALSE(result.empty());
}

TEST_F(AsyncGlobTest, HiddenFilesGlobTest) {
    std::vector<fs::path> result;
    async_glob->glob(".*",
                     [&](std::vector<fs::path> paths) { result = paths; });
    io_context.run();
    for (const auto& path : result) {
        ASSERT_TRUE(path.filename().string().starts_with("."));
    }
}

}  // namespace atom::io::test