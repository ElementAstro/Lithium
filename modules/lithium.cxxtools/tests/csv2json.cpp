#include "csv2json.hpp"

#include <gtest/gtest.h>
#include <fstream>

#include "atom/type/json.hpp"

using namespace lithium::cxxtools;

TEST(CSV2JSONTest, BasicConversion) {
    std::string csvContent =
        "name,age,city\nJohn,30,New York\nJane,25,Los Angeles\n";
    std::string csvFilePath = "test.csv";
    std::ofstream csvFile(csvFilePath);
    csvFile << csvContent;
    csvFile.close();

    std::string jsonFilePath = "test.json";
    EXPECT_NO_THROW({
        auto jsonData = detail::csvToJson(csvFilePath);
        detail::saveJsonToFile(jsonData, jsonFilePath);
    });

    std::ifstream jsonFile(jsonFilePath);
    ASSERT_TRUE(jsonFile.is_open());

    nlohmann::json expectedJson = R"([
        {"name": "John", "age": "30", "city": "New York"},
        {"name": "Jane", "age": "25", "city": "Los Angeles"}
    ])"_json;

    nlohmann::json actualJson;
    jsonFile >> actualJson;
    EXPECT_EQ(expectedJson, actualJson);
}

TEST(CSV2JSONTest, MissingCSVFile) {
    EXPECT_THROW({ detail::csvToJson("nonexistent.csv"); }, std::runtime_error);
}

TEST(CSV2JSONTest, InvalidCSVContent) {
    std::string csvContent = "name,age,city\nJohn,30\nJane,25,Los Angeles\n";
    std::string csvFilePath = "invalid.csv";
    std::ofstream csvFile(csvFilePath);
    csvFile << csvContent;
    csvFile.close();

    EXPECT_THROW(
        { auto jsonData = detail::csvToJson(csvFilePath); }, std::exception);
}
