#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>

#include "atom/function/refl_yaml.hpp"

// A sample struct to use with Reflectable
struct Person {
    std::string name;
    int age;
    bool is_student;
};

namespace {

using atom::meta::make_field;
using atom::meta::Reflectable;

class ReflectableYamlTest : public ::testing::Test {
protected:
    Reflectable<Person, decltype(make_field("name", &Person::name)),
                decltype(make_field("age", &Person::age)),
                decltype(make_field("is_student", &Person::is_student))>
        reflectable;

    ReflectableYamlTest()
        : reflectable(
              make_field("name", &Person::name),
              make_field("age", &Person::age, true, 0,
                         [](int age) { return age >= 0; }),
              make_field("is_student", &Person::is_student, false, false)) {}
};

// Test for successful YAML deserialization
TEST_F(ReflectableYamlTest, FromYamlSuccess) {
    YAML::Node node = YAML::Load(R"(
        name: John Doe
        age: 25
    )");

    Person p = reflectable.from_yaml(node);

    EXPECT_EQ(p.name, "John Doe");
    EXPECT_EQ(p.age, 25);
    EXPECT_FALSE(p.is_student);  // Default value
}

// Test for missing required field
TEST_F(ReflectableYamlTest, FromYamlMissingRequiredField) {
    YAML::Node node = YAML::Load(R"(
        age: 25
    )");  // Missing "name" field

    EXPECT_THROW(reflectable.from_yaml(node), atom::error::InvalidArgument);
}

// Test for validation failure
TEST_F(ReflectableYamlTest, FromYamlValidationFailure) {
    YAML::Node node = YAML::Load(R"(
        name: John Doe
        age: -5
    )");  // Invalid age

    EXPECT_THROW(reflectable.from_yaml(node), atom::error::InvalidArgument);
}

// Test for optional field with default value
TEST_F(ReflectableYamlTest, FromYamlOptionalField) {
    YAML::Node node = YAML::Load(R"(
        name: John Doe
        age: 25
    )");

    Person p = reflectable.from_yaml(node);

    EXPECT_EQ(p.name, "John Doe");
    EXPECT_EQ(p.age, 25);
    EXPECT_FALSE(p.is_student);  // Default value since "is_student" is optional
                                 // and not provided
}

// Test for successful serialization to YAML
TEST_F(ReflectableYamlTest, ToYamlSuccess) {
    Person p{"Jane Doe", 22, true};

    YAML::Node node = reflectable.to_yaml(p);

    EXPECT_EQ(node["name"].as<std::string>(), "Jane Doe");
    EXPECT_EQ(node["age"].as<int>(), 22);
    EXPECT_EQ(node["is_student"].as<bool>(), true);
}

// Test for serialization of object with default values
TEST_F(ReflectableYamlTest, ToYamlWithDefaultValues) {
    Person p{"Alice", 0};  // "age" set to 0 and "is_student" uses default value

    YAML::Node node = reflectable.to_yaml(p);

    EXPECT_EQ(node["name"].as<std::string>(), "Alice");
    EXPECT_EQ(node["age"].as<int>(), 0);
    EXPECT_EQ(node["is_student"].as<bool>(), false);
}

}  // namespace
