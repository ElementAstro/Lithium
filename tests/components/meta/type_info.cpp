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
    Type_Info intInfo = Type_Info::from_type<int>();
    EXPECT_EQ(intInfo.name(), "int");
    EXPECT_TRUE(intInfo.is_arithmetic());
    EXPECT_FALSE(intInfo.is_const());
    EXPECT_FALSE(intInfo.is_reference());
    EXPECT_FALSE(intInfo.is_pointer());

    Type_Info constIntRefInfo = Type_Info::from_type<const int&>();
    EXPECT_EQ(constIntRefInfo.name(), "int");
    EXPECT_FALSE(constIntRefInfo.is_arithmetic());
    EXPECT_TRUE(constIntRefInfo.is_const());
    EXPECT_TRUE(constIntRefInfo.is_reference());
    EXPECT_FALSE(constIntRefInfo.is_pointer());
}

TEST(TypeInfoTest, ClassTypeTest) {
    Type_Info structInfo = Type_Info::from_type<TestStruct>();
    EXPECT_EQ(structInfo.name(), "TestStruct");
    EXPECT_TRUE(structInfo.is_class());
    EXPECT_FALSE(structInfo.is_arithmetic());
    EXPECT_FALSE(structInfo.is_void());
    EXPECT_FALSE(structInfo.is_const());
}

TEST(TypeInfoTest, SharedPtrTypeTest) {
    Type_Info sharedPtrInfo = Get_Type_Info<std::shared_ptr<int>>::get();
    EXPECT_EQ(sharedPtrInfo.name(), "std::shared_ptr<int>");
    EXPECT_TRUE(sharedPtrInfo.is_pointer());
    EXPECT_TRUE(sharedPtrInfo.is_arithmetic());
}

TEST(TypeInfoTest, ReferenceWrapperTypeTest) {
    Type_Info refWrapperInfo =
        Get_Type_Info<std::reference_wrapper<int>>::get();
    EXPECT_EQ(refWrapperInfo.name(), "std::reference_wrapper<int>");
    EXPECT_FALSE(refWrapperInfo.is_reference());  // std::reference_wrapper
                                                  // itself is not a reference
    EXPECT_FALSE(refWrapperInfo.is_const());
}

TEST(TypeInfoTest, NameEqualityTest) {
    Type_Info intInfo1 = Type_Info::from_type<int>();
    Type_Info intInfo2 = Type_Info::from_type<int>();
    Type_Info floatInfo = Type_Info::from_type<float>();

    EXPECT_TRUE(intInfo1 == intInfo2);
    EXPECT_FALSE(intInfo1 == floatInfo);
}

TEST(TypeInfoTest, BareTypeEqualityTest) {
    Type_Info constIntInfo = Type_Info::from_type<const int>();
    Type_Info intInfo = Type_Info::from_type<int>();

    EXPECT_TRUE(constIntInfo.bare_equal(intInfo));
    EXPECT_TRUE(constIntInfo.is_const());
}

TEST(TypeInfoTest, UserTypeFunctionTest) {
    Type_Info intInfo = user_type<int>();
    Type_Info structInfo = user_type<TestStruct>();

    EXPECT_EQ(intInfo.name(), "int");
    EXPECT_EQ(structInfo.name(), "TestStruct");
}

TEST(TypeInfoTest, TypeRegistryTest) {
    register_type<int>("int");
    register_type<TestStruct>("TestStruct");

    auto intInfoOpt = get_type_info("int");
    auto structInfoOpt = get_type_info("TestStruct");
    auto unknownInfoOpt = get_type_info("unknown");

    ASSERT_TRUE(intInfoOpt.has_value());
    ASSERT_TRUE(structInfoOpt.has_value());
    ASSERT_FALSE(unknownInfoOpt.has_value());

    EXPECT_EQ(intInfoOpt->name(), "int");
    EXPECT_EQ(structInfoOpt->name(), "TestStruct");
}
