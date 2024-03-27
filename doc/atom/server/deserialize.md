# DeserializationEngine Class Documentation

The `DeserializationEngine` class is responsible for deserializing data using different deserialization engines. It allows adding multiple deserialization engines and selecting the current engine for deserialization.

## Class Declaration

```cpp
class DeserializationEngine {
public:
    DeserializationEngine() = default;
    ~DeserializationEngine() = default;

    void addDeserializeEngine(const std::string &name, const std::shared_ptr<DeserializeEngine> &engine);

    bool setCurrentDeserializeEngine(const std::string &name);

    template <typename T>
    std::optional<T> deserialize(const std::string &data) const;

private:
    std::unordered_map<std::string, std::shared_ptr<DeserializeEngine>> deserializationEngines_;
    std::string currentDeserializationEngine_;
    mutable std::mutex mutex_;
};
```

## Usage Examples

### 1. Adding a DeserializeEngine

```cpp
DeserializationEngine engine;

// Create a JsonDeserializer instance
std::shared_ptr<JsonDeserializer> jsonEngine = std::make_shared<JsonDeserializer>();

// Add JsonDeserializer as a deserialization engine
engine.addDeserializeEngine("json", jsonEngine);
```

### 2. Setting the Current DeserializeEngine

```cpp
// Set the current deserialization engine to "json"
bool success = engine.setCurrentDeserializeEngine("json");

if (success) {
    std::cout << "Current deserialization engine set to 'json'." << std::endl;
} else {
    std::cout << "Failed to set current deserialization engine." << std::endl;
}
```

### 3. Deserializing Data

```cpp
// Assuming 'engine' has a valid deserialization engine set

// Deserialize JSON data into a specific type
std::optional<std::string> result = engine.deserialize<std::string>("{\"key\": \"value\"}");

if (result.has_value()) {
    std::cout << "Deserialization successful. Result: " << result.value() << std::endl;
} else {
    std::cout << "Deserialization failed." << std::endl;
}
```

## Additional Notes

- The `ENABLE_FASTHASH` conditional compilation flag is used to select the appropriate hash map implementation.
- The `deserialize` method uses the current deserialization engine set in the class.

This concludes the documentation for the `DeserializationEngine` class, showcasing how to add deserialization engines, set the current engine, and deserialize data using the selected engine.
