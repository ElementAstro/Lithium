#include <gtest/gtest.h>

#include "atom/type/json-schema.hpp"

using namespace atom::type;

class JsonValidatorTest : public ::testing::Test {
protected:
    JsonValidator validator;
};

TEST_F(JsonValidatorTest, SetRootSchema) {
    json schema = R"({
        "type": "object",
        "properties": {
            "name": { "type": "string" }
        },
        "required": ["name"]
    })"_json;

    validator.setRootSchema(schema);
    EXPECT_TRUE(validator.getErrors().empty());
}

TEST_F(JsonValidatorTest, ValidateValidInstance) {
    json schema = R"({
        "type": "object",
        "properties": {
            "name": { "type": "string" }
        },
        "required": ["name"]
    })"_json;

    json instance = R"({
        "name": "John Doe"
    })"_json;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_TRUE(result);
    EXPECT_TRUE(validator.getErrors().empty());
}

TEST_F(JsonValidatorTest, ValidateInvalidInstance) {
    json schema = R"({
        "type": "object",
        "properties": {
            "name": { "type": "string" }
        },
        "required": ["name"]
    })"_json;

    json instance = R"({
        "age": 30
    })"_json;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message, "Missing required field: name");
}

TEST_F(JsonValidatorTest, ValidateTypeMismatch) {
    json schema = R"({
        "type": "object",
        "properties": {
            "age": { "type": "integer" }
        }
    })"_json;

    json instance = R"({
        "age": "thirty"
    })"_json;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "Type mismatch, expected type: integer");
}

TEST_F(JsonValidatorTest, ValidateEnum) {
    json schema = R"({
        "type": "string",
        "enum": ["red", "green", "blue"]
    })"_json;

    json instance = "yellow";

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message, "Value not in enum range");
}

TEST_F(JsonValidatorTest, ValidateMinimum) {
    json schema = R"({
        "type": "number",
        "minimum": 10
    })"_json;

    json instance = 5;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message, "Value less than minimum: 10");
}

TEST_F(JsonValidatorTest, ValidateMaximum) {
    json schema = R"({
        "type": "number",
        "maximum": 10
    })"_json;

    json instance = 15;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "Value greater than maximum: 10");
}

TEST_F(JsonValidatorTest, ValidatePattern) {
    json schema = R"({
        "type": "string",
        "pattern": "^[a-z]+$"
    })"_json;

    json instance = "123abc";

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "String does not match pattern: ^[a-z]+$");
}

TEST_F(JsonValidatorTest, ValidateMinLength) {
    json schema = R"({
        "type": "string",
        "minLength": 5
    })"_json;

    json instance = "abc";

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "String length less than minimum length: 5");
}

TEST_F(JsonValidatorTest, ValidateMaxLength) {
    json schema = R"({
        "type": "string",
        "maxLength": 5
    })"_json;

    json instance = "abcdef";

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "String length greater than maximum length: 5");
}

TEST_F(JsonValidatorTest, ValidateMinItems) {
    json schema = R"({
        "type": "array",
        "minItems": 3
    })"_json;

    json instance = R"([1, 2])"_json;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "Array size less than minimum items: 3");
}

TEST_F(JsonValidatorTest, ValidateMaxItems) {
    json schema = R"({
        "type": "array",
        "maxItems": 3
    })"_json;

    json instance = R"([1, 2, 3, 4])"_json;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "Array size greater than maximum items: 3");
}

TEST_F(JsonValidatorTest, ValidateUniqueItems) {
    json schema = R"({
        "type": "array",
        "uniqueItems": true
    })"_json;

    json instance = R"([1, 2, 2])"_json;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message, "Array items are not unique");
}

TEST_F(JsonValidatorTest, ValidateConst) {
    json schema = R"({
        "type": "string",
        "const": "constant"
    })"_json;

    json instance = "not_constant";

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "Value does not match const value");
}

TEST_F(JsonValidatorTest, ValidateDependencies) {
    json schema = R"({
        "type": "object",
        "properties": {
            "name": { "type": "string" },
            "age": { "type": "integer" }
        },
        "dependencies": {
            "name": ["age"]
        }
    })"_json;

    json instance = R"({
        "name": "John Doe"
    })"_json;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message, "Missing dependency: age");
}

TEST_F(JsonValidatorTest, ValidateAllOf) {
    json schema = R"({
        "allOf": [
            { "type": "string" },
            { "minLength": 5 }
        ]
    })"_json;

    json instance = "abc";

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "String length less than minimum length: 5");
}

TEST_F(JsonValidatorTest, ValidateAnyOf) {
    json schema = R"({
        "anyOf": [
            { "type": "string" },
            { "type": "number" }
        ]
    })"_json;

    json instance = true;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "Value does not match any of the schemas in anyOf");
}

TEST_F(JsonValidatorTest, ValidateOneOf) {
    json schema = R"({
        "oneOf": [
            { "type": "string" },
            { "type": "number" }
        ]
    })"_json;

    json instance = true;

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message,
              "Value does not match exactly one of the schemas in oneOf");
}

TEST_F(JsonValidatorTest, ValidateNot) {
    json schema = R"({
        "not": { "type": "string" }
    })"_json;

    json instance = "abc";

    validator.setRootSchema(schema);
    bool result = validator.validate(instance);
    EXPECT_FALSE(result);
    ASSERT_EQ(validator.getErrors().size(), 1);
    EXPECT_EQ(validator.getErrors()[0].message, "Value matches schema in not");
}
