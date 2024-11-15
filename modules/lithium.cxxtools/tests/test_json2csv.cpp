// test_json2csv.cpp
#include "json2csv.hpp"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "atom/type/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace lithium::cxxtools::converters;

class JsonToCsvConverterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary JSON file for testing
        jsonFilePath = fs::temp_directory_path() / "test.json";
        json jsonData = json::array(
            {{{"name", "Alice"}, {"age", 30}, {"city", "New York"}},
             {{"name", "Bob"}, {"age", 25}, {"city", "Los Angeles"}},
             {{"name", "Charlie"}, {"age", 35}, {"city", "Chicago"}}});
        std::ofstream jsonFile(jsonFilePath);
        jsonFile << std::setw(4) << jsonData;
        jsonFile.close();
    }

    void TearDown() override {
        // Remove the temporary JSON file
        fs::remove(jsonFilePath);
    }

    fs::path jsonFilePath;
};

TEST_F(JsonToCsvConverterTest, ConvertImpl_ValidJsonArray_ReturnsCsv) {
    JsonToCsvConverter converter;
    json jsonData =
        json::array({{{"name", "Alice"}, {"age", 30}, {"city", "New York"}},
                     {{"name", "Bob"}, {"age", 25}, {"city", "Los Angeles"}},
                     {{"name", "Charlie"}, {"age", 35}, {"city", "Chicago"}}});

    fs::path csvFilePath = fs::temp_directory_path() / "test.csv";
    bool result = converter.convertImpl(jsonData, csvFilePath);

    ASSERT_TRUE(result);
    std::ifstream csvFile(csvFilePath);
    ASSERT_TRUE(csvFile.is_open());

    std::string line;
    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"name\",\"age\",\"city\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Alice\",\"30\",\"New York\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Bob\",\"25\",\"Los Angeles\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Charlie\",\"35\",\"Chicago\"");

    csvFile.close();

    // Clean up
    fs::remove(csvFilePath);
}

TEST_F(JsonToCsvConverterTest, ConvertImpl_InvalidJsonData_ThrowsException) {
    JsonToCsvConverter converter;
    json invalidJsonData = {
        {"name", "Alice"}, {"age", 30}, {"city", "New York"}};

    fs::path csvFilePath = fs::temp_directory_path() / "test.csv";

    EXPECT_THROW(converter.convertImpl(invalidJsonData, csvFilePath),
                 std::runtime_error);
}

TEST_F(JsonToCsvConverterTest, ConvertImpl_EmptyJsonArray_ReturnsEmptyCsv) {
    JsonToCsvConverter converter;
    json emptyJsonData = json::array();

    fs::path csvFilePath = fs::temp_directory_path() / "test.csv";
    bool result = converter.convertImpl(emptyJsonData, csvFilePath);

    ASSERT_TRUE(result);
    std::ifstream csvFile(csvFilePath);
    ASSERT_TRUE(csvFile.is_open());

    std::string line;
    std::getline(csvFile, line);
    EXPECT_TRUE(line.empty());

    csvFile.close();

    // Clean up
    fs::remove(csvFilePath);
}

TEST_F(JsonToCsvConverterTest,
       ConvertImpl_NestedJsonArray_ReturnsFlattenedCsv) {
    JsonToCsvConverter converter;
    json nestedJsonData = json::array(
        {{{"name", "Alice"}, {"details", {{"age", 30}, {"city", "New York"}}}},
         {{"name", "Bob"}, {"details", {{"age", 25}, {"city", "Los Angeles"}}}},
         {{"name", "Charlie"},
          {"details", {{"age", 35}, {"city", "Chicago"}}}}});

    fs::path csvFilePath = fs::temp_directory_path() / "test.csv";
    bool result = converter.convertImpl(nestedJsonData, csvFilePath);

    ASSERT_TRUE(result);
    std::ifstream csvFile(csvFilePath);
    ASSERT_TRUE(csvFile.is_open());

    std::string line;
    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"name\",\"details_age\",\"details_city\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Alice\",\"30\",\"New York\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Bob\",\"25\",\"Los Angeles\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Charlie\",\"35\",\"Chicago\"");

    csvFile.close();

    // Clean up
    fs::remove(csvFilePath);
}

TEST_F(JsonToCsvConverterTest, ConvertImpl_ArrayInJson_ReturnsFlattenedCsv) {
    JsonToCsvConverter converter;
    json arrayJsonData = json::array(
        {{{"name", "Alice"}, {"hobbies", {"reading", "swimming"}}},
         {{"name", "Bob"}, {"hobbies", {"cycling", "hiking"}}},
         {{"name", "Charlie"}, {"hobbies", {"running", "gaming"}}}});

    fs::path csvFilePath = fs::temp_directory_path() / "test.csv";
    bool result = converter.convertImpl(arrayJsonData, csvFilePath);

    ASSERT_TRUE(result);
    std::ifstream csvFile(csvFilePath);
    ASSERT_TRUE(csvFile.is_open());

    std::string line;
    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"name\",\"hobbies_0\",\"hobbies_1\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Alice\",\"reading\",\"swimming\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Bob\",\"cycling\",\"hiking\"");

    std::getline(csvFile, line);
    EXPECT_EQ(line, "\"Charlie\",\"running\",\"gaming\"");

    csvFile.close();

    // Clean up
    fs::remove(csvFilePath);
}
