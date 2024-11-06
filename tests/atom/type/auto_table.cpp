#include <gtest/gtest.h>

#include "atom/type/auto_table.hpp"

using namespace atom::type;

TEST(CountingHashTableTest, InsertAndGet) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    auto result = table.get(1);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "one");
}

TEST(CountingHashTableTest, InsertBatchAndGetBatch) {
    CountingHashTable<int, std::string> table;
    std::vector<std::pair<int, std::string>> items = {{1, "one"}, {2, "two"}};
    table.insertBatch(items);
    auto results = table.getBatch({1, 2, 3});
    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0].value(), "one");
    EXPECT_EQ(results[1].value(), "two");
    EXPECT_FALSE(results[2].has_value());
}

TEST(CountingHashTableTest, GetAccessCount) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    table.get(1);
    table.get(1);
    auto count = table.getAccessCount(1);
    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(count.value(), 2);
}

TEST(CountingHashTableTest, Erase) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    EXPECT_TRUE(table.erase(1));
    EXPECT_FALSE(table.get(1).has_value());
}

TEST(CountingHashTableTest, Clear) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    table.insert(2, "two");
    table.clear();
    EXPECT_FALSE(table.get(1).has_value());
    EXPECT_FALSE(table.get(2).has_value());
}

TEST(CountingHashTableTest, GetAllEntries) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    table.insert(2, "two");
    auto entries = table.getAllEntries();
    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries[0].second.value, "one");
    EXPECT_EQ(entries[1].second.value, "two");
}

TEST(CountingHashTableTest, SortEntriesByCountDesc) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    table.insert(2, "two");
    table.get(1);
    table.get(1);
    table.get(2);
    table.sortEntriesByCountDesc();
    auto entries = table.getAllEntries();
    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries[0].second.value, "one");
    EXPECT_EQ(entries[1].second.value, "two");
}

TEST(CountingHashTableTest, GetTopNEntries) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    table.insert(2, "two");
    table.get(1);
    table.get(1);
    table.get(2);
    auto topEntries = table.getTopNEntries(1);
    ASSERT_EQ(topEntries.size(), 1);
    EXPECT_EQ(topEntries[0].second.value, "one");
}

TEST(CountingHashTableTest, AutoSorting) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    table.insert(2, "two");
    table.get(1);
    table.get(1);
    table.get(2);
    table.startAutoSorting(std::chrono::milliseconds(100));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    table.stopAutoSorting();
    auto entries = table.getAllEntries();
    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries[0].second.value, "one");
    EXPECT_EQ(entries[1].second.value, "two");
}

TEST(CountingHashTableTest, SerializeToJson) {
    CountingHashTable<int, std::string> table;
    table.insert(1, "one");
    table.insert(2, "two");
    auto json = table.serializeToJson();
    EXPECT_EQ(json.size(), 2);
    EXPECT_EQ(json[0]["value"], "one");
    EXPECT_EQ(json[1]["value"], "two");
}

TEST(CountingHashTableTest, DeserializeFromJson) {
    CountingHashTable<int, std::string> table;
    nlohmann::json json = {{{"key", 1}, {"value", "one"}, {"count", 2}},
                           {{"key", 2}, {"value", "two"}, {"count", 1}}};
    table.deserializeFromJson(json);
    auto result1 = table.get(1);
    auto result2 = table.get(2);
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result1.value(), "one");
    EXPECT_EQ(result2.value(), "two");
    EXPECT_EQ(table.getAccessCount(1).value(), 2);
    EXPECT_EQ(table.getAccessCount(2).value(), 1);
}
