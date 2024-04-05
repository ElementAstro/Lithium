#include <gtest/gtest.h>
#include "atom/experiment/memory.hpp"

struct TestType {
  int value;
};

using TestPool = MemoryPool<TestType>;

class MemoryPoolTest : public ::testing::Test {
protected:
  TestPool pool;
};

TEST_F(MemoryPoolTest, Allocate) {
  auto p = pool.allocate(1);
  EXPECT_NE(p, nullptr);
  pool.deallocate(p, 1);
}

TEST_F(MemoryPoolTest, AllocateMultiple) {
  auto p1 = pool.allocate(1);
  auto p2 = pool.allocate(2);
  auto p3 = pool.allocate(3);
  EXPECT_NE(p1, nullptr);
  EXPECT_NE(p2, nullptr);
  EXPECT_NE(p3, nullptr);
  pool.deallocate(p1, 1);
  pool.deallocate(p2, 2);
  pool.deallocate(p3, 3);
}

TEST_F(MemoryPoolTest, DeallocateOrder) {
  auto p1 = pool.allocate(1);
  auto p2 = pool.allocate(2);
  pool.deallocate(p2, 2);
  pool.deallocate(p1, 1);
}

TEST_F(MemoryPoolTest, ReuseChunk) {
  auto p1 = pool.allocate(1);
  pool.deallocate(p1, 1);
  auto p2 = pool.allocate(1);
  EXPECT_EQ(p1, p2);
  pool.deallocate(p2, 1);
}

TEST_F(MemoryPoolTest, AllocateLarge) {
  auto p = pool.allocate(pool.block_size() / sizeof(TestType) + 1);
  EXPECT_NE(p, nullptr);
  pool.deallocate(p, pool.block_size() / sizeof(TestType) + 1);
}

TEST_F(MemoryPoolTest, AllocateHuge) {
  EXPECT_THROW(pool.allocate(std::numeric_limits<size_t>::max()), std::bad_alloc);
}

TEST_F(MemoryPoolTest, Compare) {
  TestPool pool2;
  EXPECT_TRUE(pool.is_equal(pool));
  EXPECT_FALSE(pool.is_equal(pool2));
}

TEST_F(MemoryPoolTest, AsMemoryResource) {
  std::pmr::vector<TestType> vec{&pool};
  for (int i = 0; i < 100; ++i) {
    vec.emplace_back(TestType{i});
  }
  EXPECT_EQ(vec.size(), 100);
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(vec[i].value, i);
  }
}

#ifndef __MAIN__
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
