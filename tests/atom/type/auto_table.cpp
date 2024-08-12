#include "atom/type/auto_table.hpp"
#include <gtest/gtest.h>

using namespace atom::type;

// Test fixture for CountingHashTable
class CountingHashTableTest : public ::testing::Test {
protected:
    CountingHashTable<int, std::string> table;

    void SetUp() override {
        // Initialize table with some values
        table.insert(1, "one");
        table.insert(2, "two");
        table.insert(3, "three");
    }
};

TEST_F(CountingHashTableTest, InsertTest) {
    table.insert(4, "four");
    auto value = table.get(4);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "four");
}

TEST_F(CountingHashTableTest, GetTest) {
    auto value = table.get(1);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "one");

    value = table.get(2);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "two");

    value = table.get(3);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(value.value(), "three");

    value = table.get(99);  // non-existing key
    EXPECT_FALSE(value.has_value());
}

TEST_F(CountingHashTableTest, EraseTest) {
    bool erased = table.erase(2);
    EXPECT_TRUE(erased);
    auto value = table.get(2);
    EXPECT_FALSE(value.has_value());

    erased = table.erase(99);  // non-existing key
    EXPECT_FALSE(erased);
}

TEST_F(CountingHashTableTest, ClearTest) {
    table.clear();
    auto entries = table.getAllEntries();
    EXPECT_TRUE(entries.empty());
}

TEST_F(CountingHashTableTest, GetAllEntriesTest) {
    auto entries = table.getAllEntries();
    ASSERT_EQ(entries.size(), 3);
    EXPECT_EQ(entries[0].second.value, "one");
    EXPECT_EQ(entries[1].second.value, "two");
    EXPECT_EQ(entries[2].second.value, "three");
}

TEST_F(CountingHashTableTest, SortEntriesByCountDescTest) {
    table.get(1);
    table.get(1);
    table.get(3);
    table.sortEntriesByCountDesc();

    auto entries = table.getAllEntries();
    ASSERT_EQ(entries.size(), 3);
    EXPECT_EQ(entries[0].second.value, "one");
    EXPECT_EQ(entries[1].second.value, "three");
    EXPECT_EQ(entries[2].second.value, "two");
}

TEST_F(CountingHashTableTest, AutoSortingTest) {
    table.get(1);
    table.get(1);
    table.get(3);

    table.startAutoSorting(std::chrono::milliseconds(100));

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    table.stopAutoSorting();

    auto entries = table.getAllEntries();
    ASSERT_EQ(entries.size(), 3);
    EXPECT_EQ(entries[0].second.value, "one");
    EXPECT_EQ(entries[1].second.value, "three");
    EXPECT_EQ(entries[2].second.value, "two");
}
