#include "atom/type/auto_table.hpp"
#include <gtest/gtest.h>

TEST(CountingHashTableTest, InsertTest) {
    atom::type::CountingHashTable<int, std::string> table;

    table.insert(1, "one");
    table.insert(2, "two");
    table.insert(3, "three");

    ASSERT_EQ(table.getAllEntries(),
              std::vector<std::pair<
                  int, atom::type::CountingHashTable<int, std::string>::Entry>>{
                  {1, {"one", 1}}, {2, {"two", 1}}, {3, {"three", 1}}});
}

TEST(CountingHashTableTest, GetTest) {
    atom::type::CountingHashTable<int, std::string> table;

    table.insert(1, "one");

    ASSERT_EQ(table.get(1), std::optional<std::string>("one"));
    ASSERT_EQ(table.get(2), std::optional<std::string>(std::nullopt));
}

TEST(CountingHashTableTest, EraseTest) {
    atom::type::CountingHashTable<int, std::string> table;

    table.insert(1, "one");
    table.insert(2, "two");

    ASSERT_TRUE(table.erase(1));
    ASSERT_FALSE(table.erase(3));
    ASSERT_EQ(table.getAllEntries(),
              std::vector<std::pair<
                  int, atom::type::CountingHashTable<int, std::string>::Entry>>{
                  {2, {"two", 1}}});
}

TEST(CountingHashTableTest, ClearTest) {
    atom::type::CountingHashTable<int, std::string> table;

    table.insert(1, "one");
    table.insert(2, "two");
    table.clear();

    ASSERT_EQ(
        table.getAllEntries(),
        std::vector<std::pair<
            int, atom::type::CountingHashTable<int, std::string>::Entry>>());
}

TEST(CountingHashTableTest, SortEntriesByCountDescTest) {
    atom::type::CountingHashTable<int, std::string> table;

    table.insert(1, "one");
    table.insert(2, "two");
    table.insert(3, "three");
    table.get(1);
    table.get(1);
    table.get(3);

    table.sortEntriesByCountDesc();
    ASSERT_EQ(table.getAllEntries(),
              std::vector<std::pair<
                  int, atom::type::CountingHashTable<int, std::string>::Entry>>{
                  {1, {"one", 2}}, {3, {"three", 1}}, {2, {"two", 1}}});
}

TEST(CountingHashTableTest, StartAutoSortingTest) {
    atom::type::CountingHashTable<int, std::string> table;

    table.insert(1, "one");
    table.insert(2, "two");
    table.insert(3, "three");

    // Test that auto sorting is working by checking if the entries are sorted
    // after the specified interval
    table.startAutoSorting(std::chrono::milliseconds(10));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    table.stopAutoSorting();

    ASSERT_EQ(table.getAllEntries(),
              std::vector<std::pair<
                  int, atom::type::CountingHashTable<int, std::string>::Entry>>{
                  {1, {"one", 1}}, {2, {"two", 1}}, {3, {"three", 1}}});
}