#include "atom/type/stack_vector.hpp"
#include <gtest/gtest.h>

// Test fixture for StackVector
class StackVectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Code here will be called immediately after the constructor (right
        // before each test).
    }

    void TearDown() override {
        // Code here will be called immediately after each test (right
        // before the destructor).
    }

    // Helper function to compare two StackVectors
    static auto compareStackVectors(const StackVector<int, 10>& sv1,
                                    const StackVector<int, 10>& sv2) -> bool {
        if (sv1.size() != sv2.size()) {
            return false;
        }

        for (size_t i = 0; i < sv1.size(); ++i) {
            if (sv1[i] != sv2[i]) {
                return false;
            }
        }

        return true;
    }
};

// Tests for StackVector
TEST_F(StackVectorTest, EmplaceBack) {
    StackVector<int, 10> sv;

    sv.emplaceBack(1);
    EXPECT_EQ(sv.size(), 1);
    EXPECT_EQ(sv[0], 1);

    sv.emplaceBack(2, 3);
    EXPECT_EQ(sv.size(), 3);
    EXPECT_EQ(sv[0], 1);
    EXPECT_EQ(sv[1], 2);
    EXPECT_EQ(sv[2], 3);
}

TEST_F(StackVectorTest, CopyConstructor) {
    StackVector<int, 10> sv1;
    sv1.emplaceBack(1);
    sv1.emplaceBack(2);

    StackVector<int, 10> sv2(sv1);

    EXPECT_TRUE(compareStackVectors(sv1, sv2));
}

TEST_F(StackVectorTest, MoveConstructor) {
    StackVector<int, 10> sv1;
    sv1.emplaceBack(1);
    sv1.emplaceBack(2);

    StackVector<int, 10> sv2(std::move(sv1));

    EXPECT_TRUE(compareStackVectors(sv1, StackVector<int, 10>()));
    EXPECT_TRUE(compareStackVectors(sv2, sv1));
}

TEST_F(StackVectorTest, Destructor) {
    StackVector<int, 10> sv;
    sv.emplaceBack(1);
    sv.emplaceBack(2);

    // Use a scope to trigger destructor
    {
        StackVector<int, 10> sv1;
        sv1.emplaceBack(3);
        sv1.emplaceBack(4);
    }

    // Check if sv still has its elements
    EXPECT_EQ(sv.size(), 2);
    EXPECT_EQ(sv[0], 1);
    EXPECT_EQ(sv[1], 2);
}

TEST_F(StackVectorTest, Resize) {
    StackVector<int, 10> sv;
    sv.emplaceBack(1);
    sv.emplaceBack(2);

    sv.resize(3);
    EXPECT_EQ(sv.size(), 3);

    sv.resize(1);
    EXPECT_EQ(sv.size(), 1);
    EXPECT_EQ(sv[0], 1);
}

TEST_F(StackVectorTest, CopyAssignmentOperator) {
    StackVector<int, 10> sv1;
    sv1.emplaceBack(1);
    sv1.emplaceBack(2);

    StackVector<int, 10> sv2;
    sv2 = sv1;

    EXPECT_TRUE(compareStackVectors(sv1, sv2));
}

TEST_F(StackVectorTest, MoveAssignmentOperator) {
    StackVector<int, 10> sv1;
    sv1.emplaceBack(1);
    sv1.emplaceBack(2);

    StackVector<int, 10> sv2;
    sv2 = std::move(sv1);

    EXPECT_TRUE(compareStackVectors(sv1, StackVector<int, 10>()));
    EXPECT_TRUE(compareStackVectors(sv2, sv1));
}