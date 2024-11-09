
#include <gtest/gtest.h>
#include <sstream>

#include "target/reader.hpp"

using namespace lithium::target;

// Test writing a CSV file
TEST(DictWriterTest, WriteCSV) {
    std::ostringstream oss;
    Dialect dialect;
    DictWriter writer(oss, {"Name", "Age", "City"}, dialect, true);

    std::unordered_map<std::string, std::string> row1 = {
        {"Name", "Alice"}, {"Age", "30"}, {"City", "New York"}};
    std::unordered_map<std::string, std::string> row2 = {
        {"Name", "Bob"}, {"Age", "25"}, {"City", "Los Angeles"}};

    writer.writeRow(row1);
    writer.writeRow(row2);

    std::string expected =
        "\"Name\",\"Age\",\"City\"\n\"Alice\",\"30\",\"New "
        "York\"\n\"Bob\",\"25\",\"Los Angeles\"\n";
    ASSERT_EQ(oss.str(), expected);
}

// Test reading a CSV file
TEST(DictReaderTest, ReadCSV) {
    std::istringstream iss(
        "\"Name\",\"Age\",\"City\"\n\"Alice\",\"30\",\"New "
        "York\"\n\"Bob\",\"25\",\"Los Angeles\"\n");
    Dialect dialect;
    DictReader reader(iss, {"Name", "Age", "City"}, dialect, Encoding::UTF8);

    std::unordered_map<std::string, std::string> row;
    ASSERT_TRUE(reader.next(row));
    ASSERT_EQ(row["Name"], "Alice");
    ASSERT_EQ(row["Age"], "30");
    ASSERT_EQ(row["City"], "New York");

    ASSERT_TRUE(reader.next(row));
    ASSERT_EQ(row["Name"], "Bob");
    ASSERT_EQ(row["Age"], "25");
    ASSERT_EQ(row["City"], "Los Angeles");

    ASSERT_FALSE(reader.next(row));
}

// Test writing and reading UTF16 encoded CSV file
TEST(DictWriterReaderTest, WriteReadUTF16CSV) {
    std::ostringstream oss;
    Dialect dialect;
    DictWriter writer(oss, {"Name", "Age", "City"}, dialect, true,
                      Encoding::UTF16);

    std::unordered_map<std::string, std::string> row1 = {
        {"Name", "Alice"}, {"Age", "30"}, {"City", "New York"}};
    std::unordered_map<std::string, std::string> row2 = {
        {"Name", "Bob"}, {"Age", "25"}, {"City", "Los Angeles"}};

    writer.writeRow(row1);
    writer.writeRow(row2);

    std::string utf16_csv = oss.str();

    std::istringstream iss(utf16_csv);
    DictReader reader(iss, {"Name", "Age", "City"}, dialect, Encoding::UTF16);

    std::unordered_map<std::string, std::string> row;
    ASSERT_TRUE(reader.next(row));
    ASSERT_EQ(row["Name"], "Alice");
    ASSERT_EQ(row["Age"], "30");
    ASSERT_EQ(row["City"], "New York");

    ASSERT_TRUE(reader.next(row));
    ASSERT_EQ(row["Name"], "Bob");
    ASSERT_EQ(row["Age"], "25");
    ASSERT_EQ(row["City"], "Los Angeles");

    ASSERT_FALSE(reader.next(row));
}

// Test detecting dialect
TEST(DictReaderTest, DetectDialect) {
    std::istringstream iss(
        "Name;Age;City\nAlice;30;New York\nBob;25;Los Angeles\n");
    Dialect dialect;
    DictReader reader(iss, {"Name", "Age", "City"}, dialect, Encoding::UTF8);

    std::unordered_map<std::string, std::string> row;
    ASSERT_TRUE(reader.next(row));
    ASSERT_EQ(row["Name"], "Alice");
    ASSERT_EQ(row["Age"], "30");
    ASSERT_EQ(row["City"], "New York");

    ASSERT_TRUE(reader.next(row));
    ASSERT_EQ(row["Name"], "Bob");
    ASSERT_EQ(row["Age"], "25");
    ASSERT_EQ(row["City"], "Los Angeles");

    ASSERT_FALSE(reader.next(row));
}