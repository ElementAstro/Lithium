// FILE: src/atom/web/test_minetype.hpp

#include "atom/web/minetype.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include "atom/type/json.hpp"
using namespace nlohmann;

// Test constructor
TEST(MimeTypesTest, Constructor) {
    std::vector<std::string> knownFiles = {"file1.txt", "file2.txt"};
    MimeTypes mimeTypes(knownFiles, true);
    // Add assertions if there are any public methods to verify the state
}

// Test readJson method
TEST(MimeTypesTest, ReadJson) {
    std::vector<std::string> knownFiles;
    MimeTypes mimeTypes(knownFiles);

    // Create a temporary JSON file
    std::string jsonFile = "test_mime.json";
    std::ofstream file(jsonFile);
    file << R"({
        "mimeTypes": {
            "text/plain": ["txt", "text"],
            "image/jpeg": ["jpg", "jpeg"]
        }
    })";
    file.close();

    mimeTypes.readJson(jsonFile);

    // Clean up
    std::remove(jsonFile.c_str());

    // Add assertions if there are any public methods to verify the state
}

// Test guessType method
TEST(MimeTypesTest, GuessType) {
    std::vector<std::string> knownFiles;
    MimeTypes mimeTypes(knownFiles);

    auto result = mimeTypes.guessType("http://example.com/file.txt");
    EXPECT_EQ(result.first.value(), "text/plain");
    EXPECT_EQ(result.second, std::nullopt);
}

// Test guessAllExtensions method
TEST(MimeTypesTest, GuessAllExtensions) {
    std::vector<std::string> knownFiles;
    MimeTypes mimeTypes(knownFiles);

    auto extensions = mimeTypes.guessAllExtensions("text/plain");
    EXPECT_EQ(extensions.size(), 2);
    EXPECT_EQ(extensions[0], "txt");
    EXPECT_EQ(extensions[1], "text");
}

// Test guessExtension method
TEST(MimeTypesTest, GuessExtension) {
    std::vector<std::string> knownFiles;
    MimeTypes mimeTypes(knownFiles);

    auto extension = mimeTypes.guessExtension("text/plain");
    EXPECT_EQ(extension.value(), "txt");
}

// Test addType method
TEST(MimeTypesTest, AddType) {
    std::vector<std::string> knownFiles;
    MimeTypes mimeTypes(knownFiles);

    mimeTypes.addType("application/json", "json");

    auto extension = mimeTypes.guessExtension("application/json");
    EXPECT_EQ(extension.value(), "json");
}

// Test listAllTypes method
TEST(MimeTypesTest, ListAllTypes) {
    std::vector<std::string> knownFiles;
    MimeTypes mimeTypes(knownFiles);

    mimeTypes.addType("application/json", "json");

    testing::internal::CaptureStdout();
    mimeTypes.listAllTypes();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.find("application/json") != std::string::npos);
    EXPECT_TRUE(output.find("json") != std::string::npos);
}

// Test guessTypeByContent method
TEST(MimeTypesTest, GuessTypeByContent) {
    std::vector<std::string> knownFiles;
    MimeTypes mimeTypes(knownFiles);

    // Create a temporary file with known content
    std::string filePath = "test_file.txt";
    std::ofstream file(filePath);
    file << "This is a test file.";
    file.close();

    auto mimeType = mimeTypes.guessTypeByContent(filePath);
    EXPECT_EQ(mimeType.value(), "text/plain");

    // Clean up
    std::remove(filePath.c_str());
}