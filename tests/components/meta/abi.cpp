#include "atom/function/abi.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

// Helper functions for testing
template <typename T>
void test_demangle_type(const std::string& expected) {
    EXPECT_EQ(DemangleHelper::demangleType<T>(), expected);
}

template <typename T>
void test_demangle_instance(const T& instance, const std::string& expected) {
    EXPECT_EQ(DemangleHelper::demangleType(instance), expected);
}

void test_demangle(const std::string_view& mangled_name,
                   const std::string& expected) {
    EXPECT_EQ(DemangleHelper::demangle(mangled_name), expected);
}

void test_demangle_with_location(const std::string_view& mangled_name,
                                 const std::string& expected,
                                 const std::source_location& location) {
    std::string full_expected = expected + " (" + location.file_name() + ":" +
                                std::to_string(location.line()) + ")";
    EXPECT_EQ(DemangleHelper::demangle(mangled_name, location), full_expected);
}

void test_demangle_many(const std::vector<std::string_view>& mangled_names,
                        const std::vector<std::string>& expected) {
    EXPECT_EQ(DemangleHelper::demangleMany(mangled_names), expected);
}

void test_demangle_many_with_location(
    const std::vector<std::string_view>& mangled_names,
    const std::vector<std::string>& expected,
    const std::source_location& location) {
    std::vector<std::string> full_expected;
    for (const auto& name : expected) {
        full_expected.push_back(name + " (" + location.file_name() + ":" +
                                std::to_string(location.line()) + ")");
    }
    EXPECT_EQ(DemangleHelper::demangleMany(mangled_names, location),
              full_expected);
}

// Test cases
TEST(DemangleHelperTest, DemangleTypeTest) {
    test_demangle_type<int>("int");
    test_demangle_type<double>("double");
    test_demangle_type<std::string>(
        "std::__cxx11::basic_string<char, std::char_traits<char>, "
        "std::allocator<char> >");
}

TEST(DemangleHelperTest, DemangleInstanceTest) {
    int int_instance = 42;
    double double_instance = 3.14;
    std::string string_instance = "hello";

    test_demangle_instance(int_instance, "int");
    test_demangle_instance(double_instance, "double");
    test_demangle_instance(string_instance,
                           "std::__cxx11::basic_string<char, "
                           "std::char_traits<char>, std::allocator<char> >");
}

TEST(DemangleHelperTest, DemangleTest) {
    test_demangle(typeid(int).name(), "int");
    test_demangle(typeid(double).name(), "double");
    test_demangle(typeid(std::string).name(),
                  "std::__cxx11::basic_string<char, std::char_traits<char>, "
                  "std::allocator<char> >");
}

TEST(DemangleHelperTest, DemangleWithLocationTest) {
    auto location = std::source_location::current();
    test_demangle_with_location(typeid(int).name(), "int", location);
    test_demangle_with_location(typeid(double).name(), "double", location);
    test_demangle_with_location(
        typeid(std::string).name(),
        "std::__cxx11::basic_string<char, "
        "std::char_traits<char>, std::allocator<char> >",
        location);
}

TEST(DemangleHelperTest, DemangleManyTest) {
    std::vector<std::string_view> mangled_names = {
        typeid(int).name(), typeid(double).name(), typeid(std::string).name()};
    std::vector<std::string> expected = {
        "int", "double",
        "std::__cxx11::basic_string<char, std::char_traits<char>, "
        "std::allocator<char> >"};
    test_demangle_many(mangled_names, expected);
}

TEST(DemangleHelperTest, DemangleManyWithLocationTest) {
    auto location = std::source_location::current();
    std::vector<std::string_view> mangled_names = {
        typeid(int).name(), typeid(double).name(), typeid(std::string).name()};
    std::vector<std::string> expected = {
        "int", "double",
        "std::__cxx11::basic_string<char, std::char_traits<char>, "
        "std::allocator<char> >"};
    test_demangle_many_with_location(mangled_names, expected, location);
}
