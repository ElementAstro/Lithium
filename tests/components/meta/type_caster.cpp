#include "atom/function/type_caster.hpp"
#include <gtest/gtest.h>

TEST(TypeCasterTest, RegisterConversion) {
    atom::meta::TypeCaster caster;
    bool conversionRegistered = false;

    // Define a conversion function from int to double
    auto intToDoubleFunc = [](const std::any& value) {
        return static_cast<double>(std::any_cast<int>(value));
    };

    // Attempt to register the conversion
    try {
        caster.register_conversion<int, double>(intToDoubleFunc);
        conversionRegistered = true;
    } catch (const std::exception&) {
        conversionRegistered = false;
    }

    // Verify that the conversion was registered
    ASSERT_TRUE(conversionRegistered);
    ASSERT_TRUE((caster.has_conversion<int, double>()));
}

TEST(TypeCasterTest, Convert) {
    atom::meta::TypeCaster caster;

    // Register conversions
    caster.register_conversion<int, double>([](const std::any& value) {
        return static_cast<double>(std::any_cast<int>(value));
    });
    caster.register_conversion<double, std::string>([](const std::any& value) {
        std::stringstream ss;
        ss << std::any_cast<double>(value);
        return ss.str();
    });

    // Create input vector
    std::vector<std::any> input = {1, 2.0, 3.14};

    // Define target type names
    std::vector<std::string> targetTypeNames = {
        "int", "double",
        atom::meta::DemangleHelper::DemangleType<std::string>()};

    // Perform conversion
    std::vector<std::any> output = caster.convert(input, targetTypeNames);

    // Verify output
    ASSERT_EQ(output.size(), input.size());
    ASSERT_TRUE(std::any_cast<int>(output[0]) == 1);
    ASSERT_TRUE(std::any_cast<double>(output[1]) == 2.0);
    ASSERT_TRUE(std::any_cast<std::string>(output[2]) == "3.14");
}

TEST(TypeCasterTest, InvalidArgument) {
    atom::meta::TypeCaster caster;

    // Create input vector with mismatched size
    std::vector<std::any> input = {1, 2.0};
    std::vector<std::string> targetTypeNames = {
        "int", "double",
        atom::meta::DemangleHelper::DemangleType<std::string>()};

    // Verify that an exception is thrown for mismatched sizes
    ASSERT_THROW(caster.convert(input, targetTypeNames),
                 atom::error::Exception);
}

TEST(TypeCasterTest, UnknownType) {
    atom::meta::TypeCaster caster;

    // Define target type name of an unknown type
    std::vector<std::string> targetTypeNames = {"unknown"};

    // Verify that an exception is thrown for unknown type
    ASSERT_THROW(caster.convert({}, targetTypeNames), atom::error::Exception);
}
