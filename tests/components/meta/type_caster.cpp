// test_type_caster.hpp
#include "atom/function/type_caster.hpp"
#include <gtest/gtest.h>
#include <thread>

namespace atom::meta::test {

// Test helper types
struct TestStruct {
    int value;
    explicit TestStruct(int v = 0) : value(v) {}
};

enum class TestEnum { Value1, Value2, Value3 };

class TypeCasterTest : public ::testing::Test {
protected:
    std::shared_ptr<TypeCaster> caster;

    void SetUp() override { caster = TypeCaster::createShared(); }

    // Helper to create simple conversion functions
    template <typename From, typename To>
    static auto makeConvertFunc() -> TypeCaster::ConvertFunc {
        return [](const std::any& value) -> std::any {
            return std::any(static_cast<To>(std::any_cast<From>(value)));
        };
    }
};

// Constructor Tests
TEST_F(TypeCasterTest, CreateInstance) {
    EXPECT_NE(caster, nullptr);
    auto types = caster->getRegisteredTypes();
    EXPECT_FALSE(types.empty());
}

// Basic Type Registration Tests
TEST_F(TypeCasterTest, RegisterBasicType) {
    caster->registerType<TestStruct>("TestStruct");
    auto types = caster->getRegisteredTypes();
    EXPECT_TRUE(std::find(types.begin(), types.end(), "TestStruct") !=
                types.end());
}

// Type Conversion Tests
TEST_F(TypeCasterTest, BasicConversion) {
    caster->registerType<int>("int");
    caster->registerType<double>("double");
    caster->registerConversion<int, double>(makeConvertFunc<int, double>());

    std::any input = 42;
    auto result = caster->convert<double>(input);
    EXPECT_EQ(std::any_cast<double>(result), 42.0);
}

// Multi-stage Conversion Tests
TEST_F(TypeCasterTest, MultiStageConversion) {
    caster->registerType<int>("int");
    caster->registerType<double>("double");
    caster->registerType<std::string>("string");

    caster->registerMultiStageConversion<double, int, std::string>(
        makeConvertFunc<int, double>(), [](const std::any& value) -> std::any {
            return std::to_string(std::any_cast<double>(value));
        });

    std::any input = 42;
    auto result = caster->convert<std::string>(input);
    EXPECT_EQ(std::any_cast<std::string>(result), "42");
}

// Type Alias Tests
TEST_F(TypeCasterTest, TypeAlias) {
    caster->registerType<int>("int");
    caster->registerAlias<int>("Integer");
    auto types = caster->getRegisteredTypes();
    EXPECT_TRUE(std::find(types.begin(), types.end(), "Integer") !=
                types.end());
}

// Type Group Tests
TEST_F(TypeCasterTest, TypeGroup) {
    std::vector<std::string> numericTypes = {"int", "double", "float"};
    caster->registerTypeGroup("numeric", numericTypes);
    // Verify group registration (implementation specific)
}

// Enum Tests
TEST_F(TypeCasterTest, EnumRegistration) {
    caster->registerEnumValue<TestEnum>("TestEnum", "Value1", TestEnum::Value1);
    caster->registerEnumValue<TestEnum>("TestEnum", "Value2", TestEnum::Value2);

    auto enumStr = caster->enumToString(TestEnum::Value1, "TestEnum");
    EXPECT_EQ(enumStr, "Value1");

    auto enumVal = caster->stringToEnum<TestEnum>("Value2", "TestEnum");
    EXPECT_EQ(enumVal, TestEnum::Value2);
}

// Error Handling Tests
TEST_F(TypeCasterTest, ConversionNotFound) {
    caster->registerType<int>("int");
    caster->registerType<std::string>("string");

    std::any input = 42;
    EXPECT_THROW(caster->convert<std::string>(input), std::runtime_error);
}

TEST_F(TypeCasterTest, InvalidEnumValue) {
    caster->registerEnumValue<TestEnum>("TestEnum", "Value1", TestEnum::Value1);
    EXPECT_THROW(caster->enumToString(TestEnum::Value2, "TestEnum"),
                 std::invalid_argument);
    EXPECT_THROW(caster->stringToEnum<TestEnum>("InvalidValue", "TestEnum"),
                 std::invalid_argument);
}

// Thread Safety Tests
TEST_F(TypeCasterTest, ConcurrentTypeRegistration) {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([this, i]() {
            std::string typeName = "Type" + std::to_string(i);
            caster->registerType<int>(typeName);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto types = caster->getRegisteredTypes();
    EXPECT_EQ(std::count_if(types.begin(), types.end(),
                            [](const std::string& name) {
                                return name.find("Type") != std::string::npos;
                            }),
              10);
}

// Cache Tests
TEST_F(TypeCasterTest, ConversionPathCache) {
    caster->registerType<int>("int");
    caster->registerType<double>("double");
    caster->registerType<std::string>("string");

    caster->registerConversion<int, double>(makeConvertFunc<int, double>());
    caster->registerConversion<double, std::string>(
        [](const std::any& value) -> std::any {
            return std::to_string(std::any_cast<double>(value));
        });

    // First conversion should build the path
    std::any input = 42;
    auto result1 = caster->convert<std::string>(input);

    // Second conversion should use cached path
    auto result2 = caster->convert<std::string>(input);

    EXPECT_EQ(std::any_cast<std::string>(result1),
              std::any_cast<std::string>(result2));
}

// Complex Conversion Path Tests
TEST_F(TypeCasterTest, ComplexConversionPath) {
    // Register types
    caster->registerType<int>("int");
    caster->registerType<float>("float");
    caster->registerType<double>("double");
    caster->registerType<std::string>("string");

    // Register conversions
    caster->registerConversion<int, float>(makeConvertFunc<int, float>());
    caster->registerConversion<float, double>(makeConvertFunc<float, double>());
    caster->registerConversion<double, std::string>(
        [](const std::any& value) -> std::any {
            return std::to_string(std::any_cast<double>(value));
        });

    std::any input = 42;
    auto result = caster->convert<std::string>(input);
    EXPECT_EQ(std::any_cast<std::string>(result), "42");
}

}  // namespace atom::meta::test