# JsonChecker Class

## Brief

Json Checker class is used to check if the JSON data matches the specified type.

## Details

This class offers the ability to add default and custom rules for specific types, as well as to check if the JSON data matches the specified type or expected value.

## Usage Example

```cpp
// Create an instance of JsonChecker
JsonChecker checker;

// Check if the JSON data matches the specified type
json jsonData = ...; // JSON data to be checked
bool result = checker.checkType<int>(jsonData, "integer");
```

## Expected Output

The `result` variable should contain `true` or `false` based on whether the JSON data matches the "integer" type.

## Constructor

```cpp
/**
 * Default constructor for JsonChecker.
 */
JsonChecker();
```

## Adding Default Rule

```cpp
// Adding a default rule for a specific type
checker.addDefaultRule<int>("integer", [](const json &data) {
    return data.is_number_integer();
});
```

## Adding Custom Rule

```cpp
// Adding a custom rule for a specific type
checker.addCustomRule<std::string>("customString", [](const json &data) {
    // Add custom validation logic here
    return true; // Replace with actual validation logic
});
```

## Checking Type

```cpp
// Checking if the JSON data matches the specified type
json jsonData = ...; // JSON data to be checked
bool result = checker.checkType<int>(jsonData, "integer");
```

## Checking Value

```cpp
// Checking if the JSON data matches the expected value
json jsonData = ...; // JSON data to be checked
int expectedValue = 10; // Expected value
bool result = checker.checkValue<int>(jsonData, expectedValue);
```

## Validating Format

```cpp
// Validating the format of the JSON string data using a regular expression
json jsonData = ...; // JSON data to be validated
std::string format = R"(\d{4}-\d{2}-\d{2})"; // Regular expression format
bool result = checker.validateFormat<std::string>(jsonData, format);
```

## Setting Failure Message

```cpp
// Setting the failure message
checker.onFailure("Validation failed!");
```

## Setting Failure Callback

```cpp
// Setting the failure callback function
checker.setFailureCallback([](const std::string &message) {
    // Custom failure handling logic
});
```

## Notes

- Replace `...` with actual JSON data in the examples.
- These examples assume the presence of a `json` type, which can be replaced with the appropriate JSON library or type used in the project.
- The `ENABLE_FASTHASH` preprocessor directive is assumed to be defined appropriately.
