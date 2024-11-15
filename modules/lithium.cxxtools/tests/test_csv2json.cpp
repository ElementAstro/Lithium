// test_csv2json.cpp
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "csv2json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace lithium::cxxtools::detail;

class Csv2JsonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary CSV file for testing
        csvFilePath = fs::temp_directory_path() / "test.csv";
        std::ofstream csvFile(csvFilePath);
        csvFile << "name,age,city\n";
        csvFile << "Alice,30,New York\n";
        csvFile << "Bob,25,Los Angeles\n";
        csvFile << "Charlie,35,Chicago\n";
        csvFile.close();
    }

    void TearDown() override {
        // Remove the temporary CSV file
        fs::remove(csvFilePath);
    }

    fs::path csvFilePath;
};

TEST_F(Csv2JsonTest, ConvertImpl_ValidCsvFile_ReturnsJson) {
    Csv2Json converter;
    json result = converter.convertImpl(csvFilePath.string());

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0]["name"], "Alice");
    EXPECT_EQ(result[0]["age"], "30");
    EXPECT_EQ(result[0]["city"], "New York");
    EXPECT_EQ(result[1]["name"], "Bob");
    EXPECT_EQ(result[1]["age"], "25");
    EXPECT_EQ(result[1]["city"], "Los Angeles");
    EXPECT_EQ(result[2]["name"], "Charlie");
    EXPECT_EQ(result[2]["age"], "35");
    EXPECT_EQ(result[2]["city"], "Chicago");
}

TEST_F(Csv2JsonTest, ConvertImpl_InvalidCsvFile_ThrowsException) {
    Csv2Json converter;
    fs::path invalidCsvFilePath = fs::temp_directory_path() / "invalid.csv";

    EXPECT_THROW(converter.convertImpl(invalidCsvFilePath.string()),
                 std::runtime_error);
}

TEST_F(Csv2JsonTest, SaveToFileImpl_ValidJsonData_SavesToFile) {
    Csv2Json converter;
    json jsonData = json::array(
        {{{"name", "Alice"}, {"age", "30"}, {"city", "New York"}},
         {{"name", "Bob"}, {"age", "25"}, {"city", "Los Angeles"}},
         {{"name", "Charlie"}, {"age", "35"}, {"city", "Chicago"}}});

    fs::path jsonFilePath = fs::temp_directory_path() / "test.json";
    bool result = converter.saveToFileImpl(jsonData, jsonFilePath.string());

    ASSERT_TRUE(result);
    std::ifstream jsonFile(jsonFilePath);
    ASSERT_TRUE(jsonFile.is_open());

    json savedData;
    jsonFile >> savedData;
    jsonFile.close();

    EXPECT_EQ(savedData, jsonData);

    // Clean up
    fs::remove(jsonFilePath);
}

TEST_F(Csv2JsonTest, SaveToFileImpl_InvalidFilePath_ThrowsException) {
    Csv2Json converter;
    json jsonData = json::array(
        {{{"name", "Alice"}, {"age", "30"}, {"city", "New York"}},
         {{"name", "Bob"}, {"age", "25"}, {"city", "Los Angeles"}},
         {{"name", "Charlie"}, {"age", "35"}, {"city", "Chicago"}}});

    fs::path invalidJsonFilePath = "/invalid/path/test.json";

    EXPECT_THROW(
        converter.saveToFileImpl(jsonData, invalidJsonFilePath.string()),
        std::runtime_error);
}
