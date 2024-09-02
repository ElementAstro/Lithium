#include "atom/io/pushd.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

std::filesystem::path getUniquePath() {
    auto tempDir = std::filesystem::temp_directory_path();
    auto timestamp =
        std::chrono::system_clock::now().time_since_epoch().count();
    return tempDir / ("file_" + std::to_string(timestamp) + ".tmp");
}

class DirectoryStackTest : public ::testing::Test {
protected:
    DirectoryStack dir_stack;

    // Helper function to create a temporary directory for testing
    static auto createTempDirectory() -> std::filesystem::path {
        auto tempDir = getUniquePath();
        std::filesystem::create_directory(tempDir);
        return tempDir;
    }

    // Helper function to remove a temporary directory after testing
    static void removeTempDirectory(const std::filesystem::path& temp_dir) {
        std::filesystem::remove_all(temp_dir);
    }

    // Helper function to set the current path for testing
    static void setCurrentPath(const std::filesystem::path& path) {
        std::filesystem::current_path(path);
    }
};

// Test pushd and popd functionality
TEST_F(DirectoryStackTest, PushdAndPopd) {
    auto original_dir = std::filesystem::current_path();
    auto temp_dir1 = createTempDirectory();
    auto temp_dir2 = createTempDirectory();

    // Test pushd
    dir_stack.pushd(temp_dir1);
    EXPECT_EQ(std::filesystem::current_path(), temp_dir1);
    EXPECT_EQ(dir_stack.size(), 1);

    dir_stack.pushd(temp_dir2);
    EXPECT_EQ(std::filesystem::current_path(), temp_dir2);
    EXPECT_EQ(dir_stack.size(), 2);

    // Test popd
    dir_stack.popd();
    EXPECT_EQ(std::filesystem::current_path(), temp_dir1);
    EXPECT_EQ(dir_stack.size(), 1);

    dir_stack.popd();
    EXPECT_EQ(std::filesystem::current_path(), original_dir);
    EXPECT_EQ(dir_stack.size(), 0);

    // Cleanup
    removeTempDirectory(temp_dir1);
    removeTempDirectory(temp_dir2);
}

// Test swap functionality
TEST_F(DirectoryStackTest, Swap) {
    auto temp_dir1 = createTempDirectory();
    auto temp_dir2 = createTempDirectory();

    dir_stack.pushd(temp_dir1);
    dir_stack.pushd(temp_dir2);

    // Swap directories in the stack
    dir_stack.swap(0, 1);
    dir_stack.goto_index(0);
    EXPECT_EQ(std::filesystem::current_path(), temp_dir1);

    // Cleanup
    removeTempDirectory(temp_dir1);
    removeTempDirectory(temp_dir2);
}

// Test remove functionality
TEST_F(DirectoryStackTest, Remove) {
    auto temp_dir1 = createTempDirectory();
    auto temp_dir2 = createTempDirectory();

    dir_stack.pushd(temp_dir1);
    dir_stack.pushd(temp_dir2);

    // Remove the first directory from the stack
    dir_stack.remove(0);
    EXPECT_EQ(dir_stack.size(), 1);
    dir_stack.goto_index(0);
    EXPECT_EQ(std::filesystem::current_path(), temp_dir2);

    // Cleanup
    removeTempDirectory(temp_dir1);
    removeTempDirectory(temp_dir2);
}

// Test goto_index functionality
TEST_F(DirectoryStackTest, GotoIndex) {
    auto temp_dir1 = createTempDirectory();
    auto temp_dir2 = createTempDirectory();

    dir_stack.pushd(temp_dir1);
    dir_stack.pushd(temp_dir2);

    // Go to the first directory in the stack
    dir_stack.goto_index(0);
    EXPECT_EQ(std::filesystem::current_path(), temp_dir1);

    // Go to the second directory in the stack
    dir_stack.goto_index(1);
    EXPECT_EQ(std::filesystem::current_path(), temp_dir2);

    // Cleanup
    removeTempDirectory(temp_dir1);
    removeTempDirectory(temp_dir2);
}

// Test save_stack_to_file and load_stack_from_file functionality
TEST_F(DirectoryStackTest, SaveAndLoadStackFromFile) {
    auto temp_dir1 = createTempDirectory();
    auto temp_dir2 = createTempDirectory();
    std::string filename = "stack_test.txt";

    dir_stack.pushd(temp_dir1);
    dir_stack.pushd(temp_dir2);

    // Save stack to file
    dir_stack.save_stack_to_file(filename);

    // Clear the current stack and load from file
    dir_stack.clear();
    dir_stack.load_stack_from_file(filename);

    // Check if the stack was loaded correctly
    EXPECT_EQ(dir_stack.size(), 2);
    dir_stack.goto_index(0);
    EXPECT_EQ(std::filesystem::current_path(), temp_dir1);
    dir_stack.goto_index(1);
    EXPECT_EQ(std::filesystem::current_path(), temp_dir2);

    // Cleanup
    removeTempDirectory(temp_dir1);
    removeTempDirectory(temp_dir2);
    std::filesystem::remove(filename);
}

// Test the clear functionality
TEST_F(DirectoryStackTest, ClearStack) {
    auto temp_dir1 = createTempDirectory();
    auto temp_dir2 = createTempDirectory();

    dir_stack.pushd(temp_dir1);
    dir_stack.pushd(temp_dir2);

    // Clear the stack
    dir_stack.clear();
    EXPECT_EQ(dir_stack.size(), 0);
    EXPECT_TRUE(dir_stack.is_empty());

    // Cleanup
    removeTempDirectory(temp_dir1);
    removeTempDirectory(temp_dir2);
}

// Test peek functionality
TEST_F(DirectoryStackTest, Peek) {
    auto temp_dir1 = createTempDirectory();

    dir_stack.pushd(temp_dir1);
    dir_stack.peek();
    EXPECT_EQ(dir_stack.size(), 1);

    // Cleanup
    removeTempDirectory(temp_dir1);
}

// Test edge case: popd on empty stack
TEST_F(DirectoryStackTest, PopdOnEmptyStack) {
    dir_stack.popd();
    EXPECT_TRUE(dir_stack.is_empty());
}

// Test edge case: swap with invalid indices
TEST_F(DirectoryStackTest, SwapWithInvalidIndices) {
    auto temp_dir1 = createTempDirectory();

    dir_stack.pushd(temp_dir1);
    dir_stack.swap(0, 1);  // Invalid swap, should not change anything
    EXPECT_EQ(dir_stack.size(), 1);

    // Cleanup
    removeTempDirectory(temp_dir1);
}

// Test edge case: remove with invalid index
TEST_F(DirectoryStackTest, RemoveWithInvalidIndex) {
    auto temp_dir1 = createTempDirectory();

    dir_stack.pushd(temp_dir1);
    dir_stack.remove(1);  // Invalid remove, should not change anything
    EXPECT_EQ(dir_stack.size(), 1);

    // Cleanup
    removeTempDirectory(temp_dir1);
}
