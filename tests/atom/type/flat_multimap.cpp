#include "atom/type/flat_multimap.hpp"
#include <gtest/gtest.h>

TEST(FlatMultimapTest, EmplaceTest) {
    FlatMultimap<int, std::string> m;
    m.emplace(1, "one");
    m.emplace(2, "two");
    m.emplace(3, "three");

    ASSERT_EQ(m.size(), 3);
    ASSERT_EQ(m[1], "one");
    ASSERT_EQ(m[2], "two");
    ASSERT_EQ(m[3], "three");
}

TEST(FlatMultimapTest, InsertTest) {
    FlatMultimap<int, std::string> m;
    m.insert({1, "one"});
    m.insert({2, "two"});
    m.insert({3, "three"});

    ASSERT_EQ(m.size(), 3);
    ASSERT_EQ(m[1], "one");
    ASSERT_EQ(m[2], "two");
    ASSERT_EQ(m[3], "three");
}

TEST(FlatMultimapTest, TryEmplaceTest) {
    FlatMultimap<int, std::string> m;
    m.tryEmplace(1, "one");
    m.tryEmplace(2, "two");
    m.tryEmplace(3, "three");

    ASSERT_EQ(m.size(), 3);
    ASSERT_EQ(m[1], "one");
    ASSERT_EQ(m[2], "two");
    ASSERT_EQ(m[3], "three");
}

TEST(FlatMultimapTest, FindTest) {
    FlatMultimap<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};

    ASSERT_EQ(m.find(1)->second, "one");
    ASSERT_EQ(m.find(2)->second, "two");
    ASSERT_EQ(m.find(3)->second, "three");
    ASSERT_EQ(m.find(4), m.end());
}

TEST(FlatMultimapTest, CountTest) {
    FlatMultimap<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};

    ASSERT_EQ(m.count(1), 1);
    ASSERT_EQ(m.count(2), 1);
    ASSERT_EQ(m.count(3), 1);
    ASSERT_EQ(m.count(4), 0);
}

TEST(FlatMultimapTest, LowerBoundTest) {
    FlatMultimap<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};

    ASSERT_EQ(m.lowerBound(1)->first, 1);
    ASSERT_EQ(m.lowerBound(2)->first, 2);
    ASSERT_EQ(m.lowerBound(3)->first, 3);
    ASSERT_EQ(m.lowerBound(4), m.end());
}

TEST(FlatMultimapTest, UpperBoundTest) {
    FlatMultimap<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};

    ASSERT_EQ(m.upperBound(1)->first, 2);
    ASSERT_EQ(m.upperBound(2)->first, 3);
    ASSERT_EQ(m.upperBound(3)->first, 4);
    ASSERT_EQ(m.upperBound(4), m.end());
}

TEST(FlatMultimapTest, EqualRangeTest) {
    FlatMultimap<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};

    auto range = m.equalRange(2);
    ASSERT_EQ(range.first->first, 2);
    ASSERT_EQ(range.first->second, "two");
    ASSERT_EQ(range.second->first, 3);

    range = m.equalRange(4);
    ASSERT_EQ(range.first, m.end());
    ASSERT_EQ(range.second, m.end());
}

TEST(FlatMultimapTest, SwapTest) {
    FlatMultimap<int, std::string> m1{{1, "one"}, {2, "two"}};
    FlatMultimap<int, std::string> m2{{3, "three"}, {4, "four"}};

    m1.swap(m2);

    ASSERT_EQ(m1.size(), 2);
    ASSERT_EQ(m1[3], "three");
    ASSERT_EQ(m1[4], "four");

    ASSERT_EQ(m2.size(), 2);
    ASSERT_EQ(m2[1], "one");
    ASSERT_EQ(m2[2], "two");
}

TEST(FlatMultimapTest, ClearTest) {
    FlatMultimap<int, std::string> m{{1, "one"}, {2, "two"}, {3, "three"}};

    m.clear();

    ASSERT_EQ(m.size(), 0);
    ASSERT_EQ(m.begin(), m.end());
}