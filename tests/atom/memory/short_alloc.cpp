#include "atom/memory/short_alloc.hpp"

#include <gtest/gtest.h>

#include <list>
#include <set>
#include <string>
#include <vector>

// Test fixture for ShortAlloc
template <std::size_t N, std::size_t Align = alignof(std::max_align_t)>
class ShortAllocTest : public ::testing::Test {
protected:
    Arena<N, Align> arena;
    ShortAlloc<int, N, Align> allocator;

    ShortAllocTest() : allocator(arena) {}
};

// Test allocation and deallocation
TYPED_TEST_SUITE_P(ShortAllocTest);

TYPED_TEST_P(ShortAllocTest, BasicAllocation) {
    std::vector<int, ShortAlloc<int, TypeParam::first_type::SIZE,
                                TypeParam::first_type::ALIGNMENT>>
        vec(this->allocator);
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    ASSERT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

TYPED_TEST_P(ShortAllocTest, AllocateUnique) {
    auto uniquePtr =
        allocate_unique<ShortAlloc<int, TypeParam::first_type::SIZE,
                                   TypeParam::first_type::ALIGNMENT>,
                        int>(this->allocator, 10);
    EXPECT_EQ(*uniquePtr, 10);
}

TYPED_TEST_P(ShortAllocTest, LargeAllocation) {
    EXPECT_THROW(this->allocator.allocate(TypeParam::first_type::SIZE + 1),
                 std::bad_alloc);
}

TYPED_TEST_P(ShortAllocTest, MultipleAllocations) {
    auto p1 = this->allocator.allocate(1);
    auto p2 = this->allocator.allocate(1);
    EXPECT_NE(p1, p2);
    this->allocator.deallocate(p1, 1);
    this->allocator.deallocate(p2, 1);
}

TYPED_TEST_P(ShortAllocTest, DeallocateInReverseOrder) {
    auto p1 = this->allocator.allocate(1);
    auto p2 = this->allocator.allocate(1);
    this->allocator.deallocate(p2, 1);
    this->allocator.deallocate(p1, 1);
    EXPECT_NO_THROW(this->allocator.deallocate(p1, 1));
}

TYPED_TEST_P(ShortAllocTest, VectorWithCustomAllocator) {
    std::vector<int, ShortAlloc<int, TypeParam::first_type::SIZE,
                                TypeParam::first_type::ALIGNMENT>>
        vec(this->allocator);
    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
    }
    EXPECT_EQ(vec.size(), 100);
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

TYPED_TEST_P(ShortAllocTest, ListWithCustomAllocator) {
    std::list<int, ShortAlloc<int, TypeParam::first_type::SIZE,
                              TypeParam::first_type::ALIGNMENT>>
        lst(this->allocator);
    for (int i = 0; i < 100; ++i) {
        lst.push_back(i);
    }
    EXPECT_EQ(lst.size(), 100);
    int i = 0;
    for (const auto& val : lst) {
        EXPECT_EQ(val, i++);
    }
}

TYPED_TEST_P(ShortAllocTest, SetWithCustomAllocator) {
    std::set<int, std::less<int>,
             ShortAlloc<int, TypeParam::first_type::SIZE,
                        TypeParam::first_type::ALIGNMENT>>
        s(this->allocator);
    for (int i = 0; i < 100; ++i) {
        s.insert(i);
    }
    EXPECT_EQ(s.size(), 100);
    int i = 0;
    for (const auto& val : s) {
        EXPECT_EQ(val, i++);
    }
}

TYPED_TEST_P(ShortAllocTest, StringWithCustomAllocator) {
    std::basic_string<char, std::char_traits<char>,
                      ShortAlloc<char, TypeParam::first_type::SIZE,
                                 TypeParam::first_type::ALIGNMENT>>
        str(this->allocator);
    std::string testStr = "Hello, ShortAlloc!";
    str.assign(testStr);
    EXPECT_EQ(str, testStr);
}

REGISTER_TYPED_TEST_SUITE_P(ShortAllocTest, BasicAllocation, AllocateUnique,
                            LargeAllocation, MultipleAllocations,
                            DeallocateInReverseOrder, VectorWithCustomAllocator,
                            ListWithCustomAllocator, SetWithCustomAllocator,
                            StringWithCustomAllocator);

using MyTypes = ::testing::Types<
    std::pair<Arena<128>, std::integral_constant<std::size_t, 128>>,
    std::pair<Arena<256>, std::integral_constant<std::size_t, 256>>,
    std::pair<Arena<512>, std::integral_constant<std::size_t, 512>>,
    std::pair<Arena<1024>, std::integral_constant<std::size_t, 1024>>>;
INSTANTIATE_TYPED_TEST_SUITE_P(My, ShortAllocTest, MyTypes);