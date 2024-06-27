#include <gtest/gtest.h>

#include "atom/type/pod_vector.hpp"

namespace atom::type {

// Test fixture for PodVector
class PodVectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialization before each test case
        podVector = new PodVector<int>();
    }

    void TearDown() override {
        // Clean up after each test case
        delete podVector;
    }

    // Pointer to PodVector instance for testing
    PodVector<int>* podVector;
};

TEST_F(PodVectorTest, Constructor_Default) {
    EXPECT_EQ(podVector->size(), 0);
    EXPECT_EQ(podVector->capacity(), 4);
}

TEST_F(PodVectorTest, Constructor_InitializerList) {
    PodVector<int> vec{1, 2, 3, 4};
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec.capacity(), 4);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(vec[i], i + 1);
    }
}

TEST_F(PodVectorTest, Constructor_Size) {
    PodVector<int> vec(5);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.capacity(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec[i], 0);  // Default value of int is 0
    }
}

TEST_F(PodVectorTest, CopyConstructor) {
    PodVector<int> vec{1, 2, 3, 4};
    PodVector<int> copy(vec);
    EXPECT_EQ(copy.size(), 4);
    EXPECT_EQ(copy.capacity(), 4);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(copy[i], vec[i]);
    }
}

TEST_F(PodVectorTest, MoveConstructor) {
    PodVector<int> vec{1, 2, 3, 4};
    PodVector<int> move(std::move(vec));
    EXPECT_EQ(move.size(), 4);
    EXPECT_EQ(move.capacity(), 4);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(move[i], i + 1);
    }
    EXPECT_EQ(vec.size(), 0);  // vec should be empty after move
}

}  // namespace atom::type