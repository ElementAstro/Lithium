#include <gtest/gtest.h>
#include <memory>
#include "atom/type/pointer.hpp"

// Test fixture for PointerSentinel class
class PointerSentinelTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test data
        intValue = 10;
        ptrInt = std::make_shared<int>(intValue);
        uniquePtrInt = std::make_unique<int>(intValue);
        weakPtrInt = ptrInt;
    }

    void TearDown() override {
        // Clean up test data
        ptrInt.reset();
        uniquePtrInt.reset();
    }

    // Test data
    int intValue;
    std::shared_ptr<int> ptrInt;
    std::unique_ptr<int> uniquePtrInt;
    std::weak_ptr<int> weakPtrInt;
};

// Test case for constructing PointerSentinel with shared_ptr
TEST_F(PointerSentinelTest, ConstructorSharedPtr) {
    PointerSentinel<int> ps(ptrInt);
    ASSERT_EQ(ps.get(), ptrInt.get());
}

// Test case for constructing PointerSentinel with unique_ptr
TEST_F(PointerSentinelTest, ConstructorUniquePtr) {
    PointerSentinel<int> ps(std::move(uniquePtrInt));
    ASSERT_EQ(ps.get(), uniquePtrInt.get());
}

// Test case for constructing PointerSentinel with weak_ptr
TEST_F(PointerSentinelTest, ConstructorWeakPtr) {
    PointerSentinel<int> ps(weakPtrInt);
    ASSERT_EQ(ps.get(), ptrInt.get());
}

// Test case for constructing PointerSentinel with raw pointer
TEST_F(PointerSentinelTest, ConstructorRawPtr) {
    PointerSentinel<int> ps(&intValue);
    ASSERT_EQ(ps.get(), &intValue);
}

// Test case for copying PointerSentinel
TEST_F(PointerSentinelTest, CopyConstructor) {
    PointerSentinel<int> ps(ptrInt);
    PointerSentinel<int> psCopy(ps);
    ASSERT_EQ(psCopy.get(), ptrInt.get());
}

// Test case for moving PointerSentinel
TEST_F(PointerSentinelTest, MoveConstructor) {
    PointerSentinel<int> ps(ptrInt);
    PointerSentinel<int> psMove(std::move(ps));
    ASSERT_EQ(psMove.get(), ptrInt.get());
}

// Test case for copying PointerSentinel using copy assignment operator
TEST_F(PointerSentinelTest, CopyAssignmentOperator) {
    PointerSentinel<int> ps(ptrInt);
    PointerSentinel<int> psCopy;
    psCopy = ps;
    ASSERT_EQ(psCopy.get(), ptrInt.get());
}

// Test case for moving PointerSentinel using move assignment operator
TEST_F(PointerSentinelTest, MoveAssignmentOperator) {
    PointerSentinel<int> ps(ptrInt);
    PointerSentinel<int> psMove;
    psMove = std::move(ps);
    ASSERT_EQ(psMove.get(), ptrInt.get());
}

// Test case for applying callable object to PointerSentinel
TEST_F(PointerSentinelTest, ApplyCallableObject) {
    PointerSentinel<int> ps(ptrInt);
    auto callable = [](const int* ptr) { return *ptr * 2; };
    ASSERT_EQ(ps.apply(callable), intValue * 2);
}

// Test case for applying void function to PointerSentinel
TEST_F(PointerSentinelTest, ApplyVoidFunction) {
    PointerSentinel<int> ps(ptrInt);
    auto voidFunc = [](int* ptr, int value) { *ptr += value; };
    ps.applyVoid(voidFunc, 5);
    ASSERT_EQ(*ptrInt, intValue + 5);
}
