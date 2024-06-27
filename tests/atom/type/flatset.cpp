#include "atom/type/flatset.hpp"
#include <gtest/gtest.h>

TEST(FlatSetTest, Insert) {
    atom::type::FlatSet<int> flatSet;
    flatSet.insert(1);
    flatSet.insert(2);
    flatSet.insert(3);

    ASSERT_EQ(flatSet.size(), 3);
    ASSERT_TRUE(flatSet.contains(1));
    ASSERT_TRUE(flatSet.contains(2));
    ASSERT_TRUE(flatSet.contains(3));
}

TEST(FlatSetTest, InsertDuplicate) {
    atom::type::FlatSet<int> flatSet;
    flatSet.insert(1);
    flatSet.insert(1);

    ASSERT_EQ(flatSet.size(), 1);
    ASSERT_TRUE(flatSet.contains(1));
}

TEST(FlatSetTest, Emplace) {
    atom::type::FlatSet<std::string> flatSet;
    flatSet.emplace("hello");
    flatSet.emplace("world");

    ASSERT_EQ(flatSet.size(), 2);
    ASSERT_TRUE(flatSet.contains("hello"));
    ASSERT_TRUE(flatSet.contains("world"));
}

TEST(FlatSetTest, Find) {
    atom::type::FlatSet<int> flatSet{1, 2, 3};

    ASSERT_EQ(*flatSet.find(2), 2);
    ASSERT_EQ(flatSet.find(4), flatSet.end());
}

TEST(FlatSetTest, EqualRange) {
    atom::type::FlatSet<int> flatSet{1, 2, 3, 4, 5};

    auto range = flatSet.equalRange(3);
    ASSERT_EQ(range.first, 3);
    ASSERT_EQ(range.second, 3);

    range = flatSet.equalRange(6);
    ASSERT_EQ(range.first, flatSet.end());
    ASSERT_EQ(range.second, flatSet.end());
}

TEST(FlatSetTest, LowerBound) {
    atom::type::FlatSet<int> flatSet{1, 2, 3, 4, 5};

    ASSERT_EQ(*flatSet.lowerBound(3), 3);
    ASSERT_EQ(flatSet.lowerBound(6), flatSet.end());
}

TEST(FlatSetTest, UpperBound) {
    atom::type::FlatSet<int> flatSet{1, 2, 3, 4, 5};

    ASSERT_EQ(*flatSet.upperBound(3), 4);
    ASSERT_EQ(flatSet.upperBound(6), flatSet.end());
}

TEST(FlatSetTest, Swap) {
    atom::type::FlatSet<int> flatSet1{1, 2, 3};
    atom::type::FlatSet<int> flatSet2{4, 5, 6};

    flatSet1.swap(flatSet2);

    ASSERT_EQ(flatSet1.size(), 3);
    ASSERT_TRUE(flatSet1.contains(4));
    ASSERT_TRUE(flatSet1.contains(5));
    ASSERT_TRUE(flatSet1.contains(6));

    ASSERT_EQ(flatSet2.size(), 3);
    ASSERT_TRUE(flatSet2.contains(1));
    ASSERT_TRUE(flatSet2.contains(2));
    ASSERT_TRUE(flatSet2.contains(3));
}