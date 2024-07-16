#include "atom/function/type_info.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

struct TestStruct {
    int a;
    float b;
};

// Helper function to simulate demangling
namespace DemangleHelper {
std::string Demangle(const std::string& name) {
    return name;  // No real demangling for simplification
}
}  // namespace DemangleHelper

TEST(TypeInfoTest, BasicTypeTest) {
    TypeInfo intInfo = TypeInfo::fromType<int>();
    EXPECT_EQ(intInfo.name(), "int");
    EXPECT_TRUE(intInfo.isArithmetic());
    EXPECT_FALSE(intInfo.isConst());
    EXPECT_FALSE(intInfo.isReference());
    EXPECT_FALSE(intInfo.isPointer());

    TypeInfo constIntRefInfo = TypeInfo::fromType<const int&>();
    EXPECT_EQ(constIntRefInfo.name(), "int");
    EXPECT_FALSE(constIntRefInfo.isArithmetic());
    EXPECT_TRUE(constIntRefInfo.isConst());
    EXPECT_TRUE(constIntRefInfo.isReference());
    EXPECT_FALSE(constIntRefInfo.isPointer());
}

TEST(TypeInfoTest, ClassTypeTest) {
    TypeInfo structInfo = TypeInfo::fromType<TestStruct>();
    EXPECT_EQ(structInfo.name(), "TestStruct");
    EXPECT_TRUE(structInfo.isClass());
    EXPECT_FALSE(structInfo.isArithmetic());
    EXPECT_FALSE(structInfo.isVoid());
    EXPECT_FALSE(structInfo.isConst());
}

TEST(TypeInfoTest, SharedPtrTypeTest) {
    TypeInfo sharedPtrInfo = GetTypeInfo<std::shared_ptr<int>>::get();
    EXPECT_EQ(sharedPtrInfo.name(), "std::shared_ptr<int>");
    EXPECT_TRUE(sharedPtrInfo.isPointer());
    EXPECT_TRUE(sharedPtrInfo.isArithmetic());
}

TEST(TypeInfoTest, ReferenceWrapperTypeTest) {
    TypeInfo refWrapperInfo = GetTypeInfo<std::reference_wrapper<int>>::get();
    EXPECT_EQ(refWrapperInfo.name(), "std::reference_wrapper<int>");
    EXPECT_FALSE(refWrapperInfo.isReference());  // std::reference_wrapper
                                                 // itself is not a reference
    EXPECT_FALSE(refWrapperInfo.isConst());
}

TEST(TypeInfoTest, NameEqualityTest) {
    TypeInfo intInfo1 = TypeInfo::fromType<int>();
    TypeInfo intInfo2 = TypeInfo::fromType<int>();
    TypeInfo floatInfo = TypeInfo::fromType<float>();

    EXPECT_TRUE(intInfo1 == intInfo2);
    EXPECT_FALSE(intInfo1 == floatInfo);
}

TEST(TypeInfoTest, BareTypeEqualityTest) {
    TypeInfo constIntInfo = TypeInfo::fromType<const int>();
    TypeInfo intInfo = TypeInfo::fromType<int>();

    EXPECT_TRUE(constIntInfo.bareEqual(intInfo));
    EXPECT_TRUE(constIntInfo.isConst());
}

TEST(TypeInfoTest, UserTypeFunctionTest) {
    TypeInfo intInfo = userType<int>();
    TypeInfo structInfo = userType<TestStruct>();

    EXPECT_EQ(intInfo.name(), "int");
    EXPECT_EQ(structInfo.name(), "TestStruct");
}

TEST(TypeInfoTest, TypeRegistryTest) {
    registerType<int>("int");
    registerType<TestStruct>("TestStruct");

    auto intInfoOpt = getTypeInfo("int");
    auto structInfoOpt = getTypeInfo("TestStruct");
    auto unknownInfoOpt = getTypeInfo("unknown");

    ASSERT_TRUE(intInfoOpt.has_value());
    ASSERT_TRUE(structInfoOpt.has_value());
    ASSERT_FALSE(unknownInfoOpt.has_value());

    EXPECT_EQ(intInfoOpt->name(), "int");
    EXPECT_EQ(structInfoOpt->name(), "TestStruct");
}
