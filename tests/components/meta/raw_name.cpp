#include "atom/function/raw_name.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

template <typename T, typename U>
struct Pair {};

template <auto V>
struct ValueWrapper {};

enum class TestEnum { VALUE1, VALUE2 };

struct TestStruct {
    int member;
};

template <typename T>
struct TemplateClass {
    T value;
};

TEST(RawNameTest, RawNameOfTypeTest) {
    ASSERT_EQ(raw_name_of<int>(), "int");
    ASSERT_EQ(raw_name_of<TestStruct>(), "TestStruct");
}

TEST(RawNameTest, RawNameOfTemplateTest) {
    ASSERT_EQ(raw_name_of_template<TemplateClass<int>>(), "TemplateClass<int>");
}

TEST(RawNameTest, RawNameOfEnumTest) {
    ASSERT_EQ(raw_name_of_enum<TestEnum::VALUE1>(), "VALUE1");
    ASSERT_EQ(raw_name_of_enum<TestEnum::VALUE2>(), "VALUE2");
}

TEST(RawNameTest, RawNameMember) {
    std::cout << raw_name_of_member<&TestStruct::member>() << std::endl;
    EXPECT_EQ(raw_name_of_member<&TestStruct::member>(), "member");
}
