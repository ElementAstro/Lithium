#include "atom/function/any.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

class BoxedValueTest : public ::testing::Test {};

TEST_F(BoxedValueTest, DefaultConstructor) {
    BoxedValue bv;
    EXPECT_TRUE(bv.isUndef());
    EXPECT_TRUE(bv.isNull());
}

TEST_F(BoxedValueTest, ConstructWithValue) {
    BoxedValue bv(42);
    EXPECT_FALSE(bv.isUndef());
    EXPECT_TRUE(bv.canCast<int>());
    EXPECT_EQ(bv.tryCast<int>().value(), 42);
}

TEST_F(BoxedValueTest, ConstructWithConstValue) {
    const int x = 42;
    BoxedValue bv = x;
    // TODO: FIX ME - Now we can't detect constant value
    // EXPECT_TRUE(bv.isConst());
    EXPECT_TRUE(bv.isReadonly());
    EXPECT_EQ(bv.tryCast<int>().value(), 42);
}

TEST_F(BoxedValueTest, CopyConstructor) {
    BoxedValue bv1(42);
    BoxedValue bv2 = bv1;
    EXPECT_EQ(bv2.tryCast<int>().value(), 42);
    bv1 = 100;
    EXPECT_EQ(bv2.tryCast<int>().value(),
              42);  // Copy should not affect original
}

TEST_F(BoxedValueTest, MoveConstructor) {
    BoxedValue bv1(42);                         // 构造默认对象
    BoxedValue bv2 = std::move(bv1);            // 移动构造
    EXPECT_EQ(bv2.tryCast<int>().value(), 42);  // 验证移动对象的值
    EXPECT_TRUE(bv1.isUndef());  // 原对象应被置于无效状态
}

TEST_F(BoxedValueTest, CopyAssignment) {
    BoxedValue bv1(42);  // 创建一个 BoxedValue
    BoxedValue bv2;      // 默认构造
    bv2 = bv1;           // 拷贝赋值
    EXPECT_EQ(bv2.tryCast<int>().value(), 42);
    bv1 = makeBoxedValue(100);  // 使用 makeBoxedValue 来避免直接赋值
    EXPECT_EQ(bv2.tryCast<int>().value(), 42);  // bv2 不应该受影响
}

TEST_F(BoxedValueTest, MoveAssignment) {
    BoxedValue bv1(42);
    BoxedValue bv2;
    bv2 = std::move(bv1);
    EXPECT_TRUE(bv2.canCast<int>());
    EXPECT_EQ(bv2.tryCast<int>().value(), 42);
    EXPECT_TRUE(
        bv1.isUndef());  // Original should be in a valid but undefined state
}

TEST_F(BoxedValueTest, Swap) {
    BoxedValue bv1(42);
    BoxedValue bv2(100);
    bv1.swap(bv2);
    EXPECT_EQ(bv1.tryCast<int>().value(), 100);
    EXPECT_EQ(bv2.tryCast<int>().value(), 42);
}

TEST_F(BoxedValueTest, CheckAttributes) {
    BoxedValue bv(42);
    bv.setAttr("key", BoxedValue(100));
    EXPECT_TRUE(bv.hasAttr("key"));
    EXPECT_EQ(bv.getAttr("key").tryCast<int>().value(), 100);
    bv.removeAttr("key");
    EXPECT_FALSE(bv.hasAttr("key"));
}

TEST_F(BoxedValueTest, ListAttributes) {
    BoxedValue bv(42);
    bv.setAttr("key1", BoxedValue(100));
    bv.setAttr("key2", BoxedValue(200));
    auto attrs = bv.listAttrs();
    EXPECT_EQ(attrs.size(), 2);
    EXPECT_NE(std::find(attrs.begin(), attrs.end(), "key1"), attrs.end());
    EXPECT_NE(std::find(attrs.begin(), attrs.end(), "key2"), attrs.end());
}

TEST_F(BoxedValueTest, TryCastValid) {
    BoxedValue bv(42);
    auto result = bv.tryCast<int>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 42);
}

TEST_F(BoxedValueTest, TryCastInvalid) {
    BoxedValue bv(42);
    auto result = bv.tryCast<std::string>();
    EXPECT_FALSE(result.has_value());
}

TEST_F(BoxedValueTest, CanCast) {
    BoxedValue bv(42);
    EXPECT_TRUE(bv.canCast<int>());
    EXPECT_FALSE(bv.canCast<std::string>());
}

TEST_F(BoxedValueTest, DebugString) {
    BoxedValue bv(42);
    EXPECT_EQ(bv.debugString(), "BoxedValue<int>: 42");
}

TEST_F(BoxedValueTest, VoidTypeCheck) {
    BoxedValue bv;
    EXPECT_TRUE(bv.isUndef());
    EXPECT_FALSE(bv.canCast<int>());
    EXPECT_TRUE(bv.isNull());
}

// TODO: FIX ME - Now we can't detect constant value
/*
TEST_F(BoxedValueTest, ConstDataPtrCheck) {
    const int x = 42;
    BoxedValue bv = constVar(x);
    EXPECT_TRUE(bv.isConstDataPtr());
    EXPECT_EQ(*static_cast<const int*>(bv.getPtr()), 42);
}
*/

TEST_F(BoxedValueTest, ReadonlyCheck) {
    int x = 42;
    BoxedValue bv = makeBoxedValue(x, false, true);
    EXPECT_TRUE(bv.isReadonly());
    bv.resetReturnValue();
    EXPECT_TRUE(bv.isReadonly());
}

/*

*/
TEST_F(BoxedValueTest, ReferenceHandling) {
    int x = 42;
    BoxedValue bv = makeBoxedValue(std::ref(x));
    // TODO: FIX ME - Now we can't detect reference
    // EXPECT_TRUE(bv.isRef());
    EXPECT_EQ(bv.tryCast<int>().value(), 42);
    x = 100;
    EXPECT_EQ(bv.tryCast<int>().value(), 100);
}
