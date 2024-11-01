#include "atom/function/any.hpp"

#include <gtest/gtest.h>

using namespace atom::meta;

const int INITIAL_VALUE = 42;
const int UPDATED_VALUE = 100;
const int SECOND_VALUE = 200;

class BoxedValueTest : public ::testing::Test {};

TEST_F(BoxedValueTest, DefaultConstructor) {
    BoxedValue boxedValue;
    EXPECT_TRUE(boxedValue.isUndef());
    EXPECT_TRUE(boxedValue.isNull());
}

TEST_F(BoxedValueTest, ConstructWithValue) {
    BoxedValue boxedValue(INITIAL_VALUE);
    EXPECT_FALSE(boxedValue.isUndef());
    EXPECT_TRUE(boxedValue.canCast<int>());
    EXPECT_EQ(boxedValue.tryCast<int>().value(), INITIAL_VALUE);
}

TEST_F(BoxedValueTest, ConstructWithConstValue) {
    const int VALUE = INITIAL_VALUE;
    BoxedValue boxedValue = VALUE;
    // TODO: FIX ME - Now we can't detect constant value
    // EXPECT_TRUE(boxedValue.isConst());
    EXPECT_TRUE(boxedValue.isReadonly());
    EXPECT_EQ(boxedValue.tryCast<int>().value(), INITIAL_VALUE);
}

TEST_F(BoxedValueTest, CopyConstructor) {
    BoxedValue boxedValue1(INITIAL_VALUE);
    BoxedValue boxedValue2 = boxedValue1;
    EXPECT_EQ(boxedValue2.tryCast<int>().value(), INITIAL_VALUE);
    boxedValue1 = UPDATED_VALUE;
    EXPECT_EQ(boxedValue2.tryCast<int>().value(),
              INITIAL_VALUE);  // Copy should not affect original
}

TEST_F(BoxedValueTest, MoveConstructor) {
    BoxedValue boxedValue1(INITIAL_VALUE);            // 构造默认对象
    BoxedValue boxedValue2 = std::move(boxedValue1);  // 移动构造
    EXPECT_EQ(boxedValue2.tryCast<int>().value(),
              INITIAL_VALUE);            // 验证移动对象的值
    EXPECT_TRUE(boxedValue1.isUndef());  // 原对象应被置于无效状态
}

TEST_F(BoxedValueTest, CopyAssignment) {
    BoxedValue boxedValue1(INITIAL_VALUE);  // 创建一个 BoxedValue
    BoxedValue boxedValue2;                 // 默认构造
    boxedValue2 = boxedValue1;              // 拷贝赋值
    EXPECT_EQ(boxedValue2.tryCast<int>().value(), INITIAL_VALUE);
    boxedValue1 =
        makeBoxedValue(UPDATED_VALUE);  // 使用 makeBoxedValue 来避免直接赋值
    EXPECT_EQ(boxedValue2.tryCast<int>().value(),
              INITIAL_VALUE);  // boxedValue2 不应该受影响
}

TEST_F(BoxedValueTest, MoveAssignment) {
    BoxedValue boxedValue1(INITIAL_VALUE);
    BoxedValue boxedValue2;
    boxedValue2 = std::move(boxedValue1);
    EXPECT_TRUE(boxedValue2.canCast<int>());
    EXPECT_EQ(boxedValue2.tryCast<int>().value(), INITIAL_VALUE);
    EXPECT_TRUE(
        boxedValue1
            .isUndef());  // Original should be in a valid but undefined state
}

TEST_F(BoxedValueTest, Swap) {
    BoxedValue boxedValue1(INITIAL_VALUE);
    BoxedValue boxedValue2(UPDATED_VALUE);
    boxedValue1.swap(boxedValue2);
    EXPECT_EQ(boxedValue1.tryCast<int>().value(), UPDATED_VALUE);
    EXPECT_EQ(boxedValue2.tryCast<int>().value(), INITIAL_VALUE);
}

TEST_F(BoxedValueTest, CheckAttributes) {
    BoxedValue boxedValue(INITIAL_VALUE);
    boxedValue.setAttr("key", BoxedValue(UPDATED_VALUE));
    EXPECT_TRUE(boxedValue.hasAttr("key"));
    EXPECT_EQ(boxedValue.getAttr("key").tryCast<int>().value(), UPDATED_VALUE);
    boxedValue.removeAttr("key");
    EXPECT_FALSE(boxedValue.hasAttr("key"));
}

TEST_F(BoxedValueTest, ListAttributes) {
    BoxedValue boxedValue(INITIAL_VALUE);
    boxedValue.setAttr("key1", BoxedValue(UPDATED_VALUE));
    boxedValue.setAttr("key2", BoxedValue(SECOND_VALUE));
    auto attrs = boxedValue.listAttrs();
    EXPECT_EQ(attrs.size(), 2);
    EXPECT_NE(std::find(attrs.begin(), attrs.end(), "key1"), attrs.end());
    EXPECT_NE(std::find(attrs.begin(), attrs.end(), "key2"), attrs.end());
}

TEST_F(BoxedValueTest, TryCastValid) {
    BoxedValue boxedValue(INITIAL_VALUE);
    auto result = boxedValue.tryCast<int>();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), INITIAL_VALUE);
}

TEST_F(BoxedValueTest, TryCastInvalid) {
    BoxedValue boxedValue(INITIAL_VALUE);
    auto result = boxedValue.tryCast<std::string>();
    EXPECT_FALSE(result.has_value());
}

TEST_F(BoxedValueTest, CanCast) {
    BoxedValue boxedValue(INITIAL_VALUE);
    EXPECT_TRUE(boxedValue.canCast<int>());
    EXPECT_FALSE(boxedValue.canCast<std::string>());
}

TEST_F(BoxedValueTest, DebugString) {
    BoxedValue boxedValue(INITIAL_VALUE);
    EXPECT_EQ(boxedValue.debugString(), "BoxedValue<int>: 42");
}

TEST_F(BoxedValueTest, VoidTypeCheck) {
    BoxedValue boxedValue;
    EXPECT_TRUE(boxedValue.isUndef());
    EXPECT_FALSE(boxedValue.canCast<int>());
    EXPECT_TRUE(boxedValue.isNull());
}

// TODO: FIX ME - Now we can't detect constant value
/*
TEST_F(BoxedValueTest, ConstDataPtrCheck) {
    const int value = INITIAL_VALUE;
    BoxedValue boxedValue = constVar(value);
    EXPECT_TRUE(boxedValue.isConstDataPtr());
    EXPECT_EQ(*static_cast<const int*>(boxedValue.getPtr()), INITIAL_VALUE);
}
*/

TEST_F(BoxedValueTest, ReadonlyCheck) {
    int value = INITIAL_VALUE;
    BoxedValue boxedValue = makeBoxedValue(value, false, true);
    EXPECT_TRUE(boxedValue.isReadonly());
    boxedValue.resetReturnValue();
    EXPECT_TRUE(boxedValue.isReadonly());
}

/*

*/
TEST_F(BoxedValueTest, ReferenceHandling) {
    int value = INITIAL_VALUE;
    BoxedValue boxedValue = makeBoxedValue(std::ref(value));
    // TODO: FIX ME - Now we can't detect reference
    // EXPECT_TRUE(boxedValue.isRef());
    EXPECT_EQ(boxedValue.tryCast<int>().value(), INITIAL_VALUE);
    value = UPDATED_VALUE;
    EXPECT_EQ(boxedValue.tryCast<int>().value(), UPDATED_VALUE);
}
