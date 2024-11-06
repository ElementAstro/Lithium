#ifndef ATOM_META_TEST_TYPE_CASTER_HPP
#define ATOM_META_TEST_TYPE_CASTER_HPP

#include "atom/function/type_caster.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

class TypeCasterTest : public ::testing::Test {
protected:
    TypeCaster typeCaster;

    void SetUp() override {
        // Register some custom types and conversions for testing
        typeCaster.registerType<int>("int");
        typeCaster.registerType<double>("double");
        typeCaster.registerConversion<int, double>([](const std::any& input) {
            return std::any_cast<int>(input) * 1.0;
        });
        typeCaster.registerConversion<double, int>([](const std::any& input) {
            return static_cast<int>(std::any_cast<double>(input));
        });
    }
};

TEST_F(TypeCasterTest, ConvertIntToDouble) {
    std::any input = 42;
    std::any result = typeCaster.convert<double>(input);
    EXPECT_EQ(std::any_cast<double>(result), 42.0);
}

TEST_F(TypeCasterTest, ConvertDoubleToInt) {
    std::any input = 42.0;
    std::any result = typeCaster.convert<int>(input);
    EXPECT_EQ(std::any_cast<int>(result), 42);
}

TEST_F(TypeCasterTest, RegisterAndConvertCustomType) {
    struct CustomType {
        int value;
    };

    typeCaster.registerType<CustomType>("CustomType");
    typeCaster.registerConversion<CustomType, int>([](const std::any& input) {
        return std::any_cast<CustomType>(input).value;
    });

    CustomType customValue{123};
    std::any input = customValue;
    std::any result = typeCaster.convert<int>(input);
    EXPECT_EQ(std::any_cast<int>(result), 123);
}

TEST_F(TypeCasterTest, RegisterMultiStageConversion) {
    typeCaster.registerMultiStageConversion<int, double, std::string>(
        [](const std::any& input) { return std::any_cast<int>(input) * 1.0; },
        [](const std::any& input) {
            return std::to_string(std::any_cast<double>(input));
        });

    std::any input = 42;
    std::any result = typeCaster.convert<std::string>(input);
    EXPECT_EQ(std::any_cast<std::string>(result), "42.000000");
}

TEST_F(TypeCasterTest, GetRegisteredTypes) {
    auto types = typeCaster.getRegisteredTypes();
    EXPECT_NE(std::find(types.begin(), types.end(), "int"), types.end());
    EXPECT_NE(std::find(types.begin(), types.end(), "double"), types.end());
}

TEST_F(TypeCasterTest, EnumToString) {
    enum class TestEnum { VALUE1, VALUE2 };
    typeCaster.registerEnumValue<TestEnum>("TestEnum", "VALUE1",
                                           TestEnum::VALUE1);
    typeCaster.registerEnumValue<TestEnum>("TestEnum", "VALUE2",
                                           TestEnum::VALUE2);

    EXPECT_EQ(typeCaster.enumToString(TestEnum::VALUE1, "TestEnum"), "VALUE1");
    EXPECT_EQ(typeCaster.enumToString(TestEnum::VALUE2, "TestEnum"), "VALUE2");
}

TEST_F(TypeCasterTest, StringToEnum) {
    enum class TestEnum { VALUE1, VALUE2 };
    typeCaster.registerEnumValue<TestEnum>("TestEnum", "VALUE1",
                                           TestEnum::VALUE1);
    typeCaster.registerEnumValue<TestEnum>("TestEnum", "VALUE2",
                                           TestEnum::VALUE2);

    EXPECT_EQ(typeCaster.stringToEnum<TestEnum>("VALUE1", "TestEnum"),
              TestEnum::VALUE1);
    EXPECT_EQ(typeCaster.stringToEnum<TestEnum>("VALUE2", "TestEnum"),
              TestEnum::VALUE2);
}

#endif  // ATOM_META_TEST_TYPE_CASTER_HPP
