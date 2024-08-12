#include "atom/type/iter.hpp"
#include <gtest/gtest.h>

TEST(PointerIteratorTest, BasicTest) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto begin = v.begin();
    auto end = v.end();
    auto pointerRange = makePointerRange(begin, end);

    ASSERT_EQ(pointerRange.first, &*begin);
    ASSERT_EQ(pointerRange.second, &*end);
}

TEST(processContainerTest, EraseElements) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    processContainer(v);

    ASSERT_EQ(v.size(), 2);
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 5);
}

TEST(EarlyIncIteratorTest, BasicTest) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto begin = v.begin();
    auto end = v.end();
    auto earlyIncIterator = makeEarlyIncIterator(begin);

    ASSERT_EQ(*earlyIncIterator, *begin);
    ++earlyIncIterator;
    ASSERT_EQ(*earlyIncIterator, *std::next(begin));
}

TEST(TransformIteratorTest, BasicTest) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto begin = v.begin();
    auto end = v.end();
    auto transformIterator =
        makeTransformIterator(begin, [](int x) { return x * 2; });

    ASSERT_EQ(*transformIterator, *begin * 2);
    ++transformIterator;
    ASSERT_EQ(*transformIterator, *std::next(begin) * 2);
}

TEST(FilterIteratorTest, BasicTest) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto begin = v.begin();
    auto end = v.end();
    auto filterIterator =
        makeFilterIterator(begin, end, [](int x) { return x % 2 == 0; });

    ASSERT_EQ(*filterIterator, *std::next(begin));
    ++filterIterator;
    ASSERT_EQ(*filterIterator, *std::next(begin, 2));
}

TEST(ReverseIteratorTest, BasicTest) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto begin = v.rbegin();
    auto end = v.rend();
    ReverseIterator<decltype(begin)> reverseIterator(begin);

    ASSERT_EQ(*reverseIterator, *begin);
    ++reverseIterator;
    ASSERT_EQ(*reverseIterator, *std::next(begin));
}
