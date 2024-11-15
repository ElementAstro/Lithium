// test_symbol.cpp
#include "atom/type/json.hpp"
#include "symbol.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>

#include "atom/log/loguru.hpp"
#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

class SymbolAnalyzerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary library file for testing
        libraryPath = fs::temp_directory_path() / "test_library.so";
        std::ofstream libraryFile(libraryPath);
        libraryFile << "dummy content";
        libraryFile.close();
    }

    void TearDown() override {
        // Remove the temporary library file
        fs::remove(libraryPath);
    }

    fs::path libraryPath;
};

TEST_F(SymbolAnalyzerTest, AnalyzeLibrary_ValidLibrary_CsvOutput) {
    std::string outputFormat = "csv";
    int threadCount = 2;

    analyzeLibrary(libraryPath.string(), outputFormat, threadCount);

    fs::path csvFilePath = "symbols.csv";
    ASSERT_TRUE(fs::exists(csvFilePath));

    std::ifstream csvFile(csvFilePath);
    ASSERT_TRUE(csvFile.is_open());

    std::string line;
    std::getline(csvFile, line);
    EXPECT_EQ(line, "Address,Type,Bind,Visibility,Name,Demangled Name");

    csvFile.close();
    fs::remove(csvFilePath);
}

TEST_F(SymbolAnalyzerTest, AnalyzeLibrary_ValidLibrary_JsonOutput) {
    std::string outputFormat = "json";
    int threadCount = 2;

    analyzeLibrary(libraryPath.string(), outputFormat, threadCount);

    fs::path jsonFilePath = "symbols.json";
    ASSERT_TRUE(fs::exists(jsonFilePath));

    std::ifstream jsonFile(jsonFilePath);
    ASSERT_TRUE(jsonFile.is_open());

    nlohmann::json jsonData;
    jsonFile >> jsonData;
    EXPECT_TRUE(jsonData.is_array());

    jsonFile.close();
    fs::remove(jsonFilePath);
}

TEST_F(SymbolAnalyzerTest, AnalyzeLibrary_ValidLibrary_YamlOutput) {
    std::string outputFormat = "yaml";
    int threadCount = 2;

    analyzeLibrary(libraryPath.string(), outputFormat, threadCount);

    fs::path yamlFilePath = "symbols.yaml";
    ASSERT_TRUE(fs::exists(yamlFilePath));

    std::ifstream yamlFile(yamlFilePath);
    ASSERT_TRUE(yamlFile.is_open());

    YAML::Node yamlData = YAML::LoadFile(yamlFilePath.string());
    EXPECT_TRUE(yamlData.IsSequence());

    yamlFile.close();
    fs::remove(yamlFilePath);
}

TEST_F(SymbolAnalyzerTest, AnalyzeLibrary_InvalidLibraryPath_ThrowsException) {
    std::string invalidLibraryPath = "/invalid/path/to/library.so";
    std::string outputFormat = "json";
    int threadCount = 2;

    EXPECT_THROW(analyzeLibrary(invalidLibraryPath, outputFormat, threadCount),
                 std::runtime_error);
}

TEST_F(SymbolAnalyzerTest,
       AnalyzeLibrary_UnsupportedOutputFormat_ThrowsException) {
    std::string outputFormat = "unsupported_format";
    int threadCount = 2;

    EXPECT_THROW(
        analyzeLibrary(libraryPath.string(), outputFormat, threadCount),
        std::runtime_error);
}

TEST_F(SymbolAnalyzerTest, Main_InvalidArguments_ReturnsFailure) {
    int argc = 2;
    const char* argv[] = {"symbol_analyzer", "arg1"};
    EXPECT_EQ(main(argc, const_cast<char**>(argv)), EXIT_FAILURE);
}

TEST_F(SymbolAnalyzerTest, Main_ValidArguments_ReturnsSuccess) {
    int argc = 4;
    const char* argv[] = {"symbol_analyzer", libraryPath.string().c_str(),
                          "json", "2"};
    EXPECT_EQ(main(argc, const_cast<char**>(argv)), EXIT_SUCCESS);

    fs::path jsonFilePath = "symbols.json";
    ASSERT_TRUE(fs::exists(jsonFilePath));
    fs::remove(jsonFilePath);
}

TEST_F(SymbolAnalyzerTest, Main_InvalidThreadCount_ReturnsFailure) {
    int argc = 4;
    const char* argv[] = {"symbol_analyzer", libraryPath.string().c_str(),
                          "json", "-1"};
    EXPECT_EQ(main(argc, const_cast<char**>(argv)), EXIT_FAILURE);
}

TEST_F(SymbolAnalyzerTest, Main_UnsupportedOutputFormat_ReturnsFailure) {
    int argc = 4;
    const char* argv[] = {"symbol_analyzer", libraryPath.string().c_str(),
                          "unsupported_format", "2"};
    EXPECT_EQ(main(argc, const_cast<char**>(argv)), EXIT_FAILURE);
}

int main(int argc, char** argv) {
    loguru::init(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}