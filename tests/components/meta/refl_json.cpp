#include <gtest/gtest.h>

#include "atom/error/exception.hpp"
#include "atom/function/refl_json.hpp"

// A sample struct to use with Reflectable
struct Person {
    std::string name;
    int age;
    bool is_student;
};

namespace {

using atom::meta::make_field;
using atom::meta::Reflectable;

class ReflectableTest : public ::testing::Test {
protected:
    Reflectable<Person, decltype(make_field("name", &Person::name)),
                decltype(make_field("age", &Person::age)),
                decltype(make_field("is_student", &Person::is_student))>
        reflectable;

    ReflectableTest()
        : reflectable(
              make_field("name", &Person::name),
              make_field("age", &Person::age, true, 0,
                         [](int age) { return age >= 0; }),
              make_field("is_student", &Person::is_student, false, false)) {}
};

// Test for successful JSON deserialization
TEST_F(ReflectableTest, FromJsonSuccess) {
    json j = R"({"name": "John Doe", "age": 25})"_json;

    Person p = reflectable.from_json(j);

    EXPECT_EQ(p.name, "John Doe");
    EXPECT_EQ(p.age, 25);
    EXPECT_FALSE(p.is_student);  // Default value
}

// Test for missing required field
TEST_F(ReflectableTest, FromJsonMissingRequiredField) {
    json j = R"({"age": 25})"_json;  // Missing "name" field

    EXPECT_THROW(reflectable.from_json(j), atom::error::MissingArgument);
}

// Test for validation failure
TEST_F(ReflectableTest, FromJsonValidationFailure) {
    json j = R"({"name": "John Doe", "age": -5})"_json;  // Invalid age

    EXPECT_THROW(reflectable.from_json(j), atom::error::InvalidArgument);
}

// Test for optional field with default value
TEST_F(ReflectableTest, FromJsonOptionalField) {
    json j = R"({"name": "John Doe", "age": 25})"_json;

    Person p = reflectable.from_json(j);

    EXPECT_EQ(p.name, "John Doe");
    EXPECT_EQ(p.age, 25);
    EXPECT_FALSE(p.is_student);  // Default value since "is_student" is optional
                                 // and not provided
}

// Test for successful serialization to JSON
TEST_F(ReflectableTest, ToJsonSuccess) {
    Person p{"Jane Doe", 22, true};

    json j = reflectable.to_json(p);

    EXPECT_EQ(j["name"], "Jane Doe");
    EXPECT_EQ(j["age"], 22);
    EXPECT_EQ(j["is_student"], true);
}

// Test for serialization of object with default values
TEST_F(ReflectableTest, ToJsonWithDefaultValues) {
    Person p{"Alice", 0};  // "age" set to 0 and "is_student" uses default value

    json j = reflectable.to_json(p);

    EXPECT_EQ(j["name"], "Alice");
    EXPECT_EQ(j["age"], 0);
    EXPECT_EQ(j["is_student"], false);
}

}  // namespace
