#include "atom/type//no_offset_ptr.hpp"
#include <gtest/gtest.h>

// Test fixture for UnshiftedPtr
class UnshiftedPtrTest : public ::testing::Test {
protected:
    void SetUp() override { ptr = new UnshiftedPtr<int>(5); }

    void TearDown() override { delete ptr; }

    // Pointer to UnshiftedPtr instance for testing
    UnshiftedPtr<int>* ptr;
};

// Test the constructor
TEST_F(UnshiftedPtrTest, ConstructorTest) {
    UnshiftedPtr<int> p(10);
    EXPECT_EQ(*p, 10);
}

// Test the copy constructor
TEST_F(UnshiftedPtrTest, CopyConstructorTest) {
    UnshiftedPtr<int> p(*ptr);
    EXPECT_EQ(*p, 5);
}

// Test the move constructor
TEST_F(UnshiftedPtrTest, MoveConstructorTest) {
    UnshiftedPtr<int> p(std::move(*ptr));
    EXPECT_EQ(*p, 5);
}

// Test the destructor
TEST_F(UnshiftedPtrTest, DestructorTest) {
    // Destructor should be called after TearDown()
    // Verify that there are no memory leaks or errors
}

// Test the copy assignment operator
TEST_F(UnshiftedPtrTest, CopyAssignmentOperatorTest) {
    UnshiftedPtr<int> p;
    p = *ptr;
    EXPECT_EQ(*p, 5);
}

// Test the move assignment operator
TEST_F(UnshiftedPtrTest, MoveAssignmentOperatorTest) {
    UnshiftedPtr<int> p;
    p = std::move(*ptr);
    EXPECT_EQ(*p, 5);
}

// Test the dereference operators
TEST_F(UnshiftedPtrTest, DereferenceOperatorsTest) {
    EXPECT_EQ(*ptr, 5);
    EXPECT_EQ((ptr)++, 5);
    EXPECT_EQ(*ptr, 6);
}