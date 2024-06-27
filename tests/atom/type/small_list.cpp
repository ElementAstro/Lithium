#include "atom/type/small_list.hpp"
#include <gtest/gtest.h>

TEST(SmallListTest, PushBack) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(list.front(), 1);
    ASSERT_EQ(list.back(), 3);
}

TEST(SmallListTest, PushFront) {
    SmallList<int> list;
    list.pushFront(1);
    list.pushFront(2);
    list.pushFront(3);

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(list.front(), 3);
    ASSERT_EQ(list.back(), 1);
}

TEST(SmallListTest, PopBack) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.popBack();

    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(list.back(), 2);
}

TEST(SmallListTest, PopFront) {
    SmallList<int> list;
    list.pushFront(1);
    list.pushFront(2);
    list.pushFront(3);
    list.popFront();

    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(list.front(), 2);
}

TEST(SmallListTest, Empty) {
    SmallList<int> list;

    ASSERT_TRUE(list.empty());
    list.pushBack(1);
    ASSERT_FALSE(list.empty());
}

TEST(SmallListTest, Size) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);

    ASSERT_EQ(list.size(), 3);
}

TEST(SmallListTest, Clear) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    list.clear();

    ASSERT_TRUE(list.empty());
}

TEST(SmallListTest, Iterator) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);

    SmallList<int>::Iterator it = list.begin();
    ASSERT_EQ(*it, 1);
    ++it;
    ASSERT_EQ(*it, 2);
    ++it;
    ASSERT_EQ(*it, 3);
}

TEST(SmallListTest, Insert) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(3);
    list.insert(list.begin(), 2);

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(list.front(), 1);
    ASSERT_EQ(list.back(), 3);
}

TEST(SmallListTest, Erase) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(3);
    auto it = list.begin();  // 获取开始迭代器
    std::advance(it, 1);     // 移动到第二个元素
    list.erase(it);          // 删除第二个元素

    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(list.front(), 1);
    ASSERT_EQ(list.back(), 3);
}

TEST(SmallListTest, Remove) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(1);
    list.remove(1);

    ASSERT_EQ(list.size(), 1);
    ASSERT_EQ(list.front(), 2);
}

TEST(SmallListTest, Unique) {
    SmallList<int> list;
    list.pushBack(1);
    list.pushBack(2);
    list.pushBack(1);
    list.unique();

    ASSERT_EQ(list.size(), 2);
    ASSERT_EQ(list.front(), 1);
    ASSERT_EQ(list.back(), 2);
}

TEST(SmallListTest, Sort) {
    SmallList<int> list;
    list.pushBack(3);
    list.pushBack(1);
    list.pushBack(2);
    list.sort();

    ASSERT_EQ(list.size(), 3);
    ASSERT_EQ(list.front(), 1);
    ASSERT_EQ(list.back(), 3);
}

TEST(SmallListTest, Swap) {
    SmallList<int> list1;
    list1.pushBack(1);
    list1.pushBack(2);

    SmallList<int> list2;
    list2.pushBack(3);
    list2.pushBack(4);

    list1.swap(list2);

    ASSERT_EQ(list1.size(), 2);
    ASSERT_EQ(list1.front(), 3);
    ASSERT_EQ(list1.back(), 4);

    ASSERT_EQ(list2.size(), 2);
    ASSERT_EQ(list2.front(), 1);
    ASSERT_EQ(list2.back(), 2);
}