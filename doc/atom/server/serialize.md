# Serialization Engine

## Introduction

Serialization engine is designed to provide a way to render data into different formats such as JSON, XML, YAML, and INI. It allows flexibility in choosing the serialization format and provides an easy way to add new serialization engines.

## Serialization Class

The `Serialization` class serves as the base class for all specific serialization engines. It provides a virtual function `serialize` to render data into a string.

```cpp
// Create a JSON serialization engine
std::shared_ptr<JsonSerializationEngine> jsonEngine = std::make_shared<JsonSerializationEngine>();
SerializationEngine engine;
engine.addSerializationEngine("json", jsonEngine);
```

## JsonSerializationEngine

This class is responsible for serializing data into JSON format.

```cpp
// Serialize data using JSON serialization engine
std::string jsonData = engine.serialize(myData);
```

**Expected Output:**  
A JSON-formatted string representing the serialized data.

## XmlSerializationEngine

This class is responsible for serializing data into XML format.

```cpp
// Serialize data using XML serialization engine
std::string xmlData = engine.serialize(myData);
```

**Expected Output:**  
An XML-formatted string representing the serialized data.

## YamlSerializationEngine

This class is responsible for serializing data into YAML format.

```cpp
// Serialize data using YAML serialization engine
std::string yamlData = engine.serialize(myData);
```

**Expected Output:**  
A YAML-formatted string representing the serialized data.

## IniSerializationEngine

This class is responsible for serializing data into INI format.

```cpp
// Serialize data using INI serialization engine
std::string iniData = engine.serialize(myData);
```

**Expected Output:**  
An INI-formatted string representing the serialized data.

## SerializationEngine Class

The `SerializationEngine` class manages the serialization engines and provides an interface to add, select, and use different serialization engines.

```cpp
// Add a serialization engine to the SerializationEngine
engine.addSerializationEngine("yaml", yamlEngine);

// Set the current serialization engine to JSON
bool success = engine.setCurrentSerializationEngine("json");

// Serialize data using the selected serialization engine
std::optional<std::string> serializedData = engine.serialize(myData);
if (serializedData.has_value()) {
    std::cout << "Serialized data: " << *serializedData << std::endl;
} else {
    std::cerr << "Serialization failed." << std::endl;
}
```

**Expected Output:**  
If serialization is successful, it prints the serialized data; otherwise, it outputs a serialization failure message.

## Notes

- The `SerializationEngine` class uses a mutex to ensure thread safety when accessing and modifying the serialization engines.
- The usage of `std::optional` allows handling potential serialization failures gracefully.
