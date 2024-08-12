#include "atom/utils/aligned.hpp"
#include <gtest/gtest.h>

// 测试存储大小和对齐方式符合条件的情况
TEST(ValidateAlignedStorageTest, ValidStorage) {
    EXPECT_NO_THROW((atom::utils::ValidateAlignedStorage<32, 16, 64, 16>()));
}

// 测试存储大小和对齐方式不符合条件的情况
TEST(ValidateAlignedStorageTest, InvalidStorageSize) {
// 这段代码将导致编译错误，因此无法直接运行
#ifdef STATIC_ASSERT_TEST
    atom::utils::ValidateAlignedStorage<32, 16, 16, 16> invalidStorageSize;
#endif
}

TEST(ValidateAlignedStorageTest, InvalidStorageAlign) {
// 这段代码将导致编译错误，因此无法直接运行
#ifdef STATIC_ASSERT_TEST
    atom::utils::ValidateAlignedStorage<32, 16, 64, 8> invalidStorageAlign;
#endif
}
