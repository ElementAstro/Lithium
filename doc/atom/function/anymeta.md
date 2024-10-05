# TypeMetadata and TypeRegistry Documentation

## Overview

This documentation covers the `TypeMetadata` and `TypeRegistry` classes, along with related helper functions and the `TypeRegistrar` template class. These components are part of the `atom::meta` namespace and provide a system for dynamic reflection, method overloading, and event handling in C++.

## TypeMetadata Class

The `TypeMetadata` class stores metadata about a type, including its methods, properties, constructors, and events.

### Key Components

- `MethodFunction`: A function type for methods that take a vector of `BoxedValue` arguments and return a `BoxedValue`.
- `GetterFunction`: A function type for property getters.
- `SetterFunction`: A function type for property setters.
- `ConstructorFunction`: A function type for constructors.
- `EventCallback`: A function type for event listeners.

### Public Methods

```cpp
void addMethod(const std::string& name, MethodFunction method);
void removeMethod(const std::string& name);
void addProperty(const std::string& name, GetterFunction getter, SetterFunction setter);
void removeProperty(const std::string& name);
void addConstructor(const std::string& type_name, ConstructorFunction constructor);
void addEvent(const std::string& event_name);
void removeEvent(const std::string& event_name);
void addEventListener(const std::string& event_name, EventCallback callback);
void fireEvent(BoxedValue& obj, const std::string& event_name, const std::vector<BoxedValue>& args) const;
auto getMethods(const std::string& name) const -> std::optional<const std::vector<MethodFunction>*>;
auto getProperty(const std::string& name) const -> std::optional<Property>;
auto getConstructor(const std::string& type_name, size_t index = 0) const -> std::optional<ConstructorFunction>;
auto getEvent(const std::string& name) const -> std::optional<const Event*>;
```

## TypeRegistry Class

The `TypeRegistry` class is a singleton that manages a global registry of type metadata.

### Public Methods

```cpp
static auto instance() -> TypeRegistry&;
void registerType(const std::string& name, TypeMetadata metadata);
auto getMetadata(const std::string& name) const -> std::optional<TypeMetadata>;
```

## Helper Functions

### callMethod

```cpp
auto callMethod(BoxedValue& obj, const std::string& method_name, std::vector<BoxedValue> args) -> BoxedValue;
```

Dynamically calls an overloaded method on a `BoxedValue` object.

### getProperty

```cpp
auto getProperty(const BoxedValue& obj, const std::string& property_name) -> BoxedValue;
```

Dynamically retrieves a property value from a `BoxedValue` object.

### setProperty

```cpp
void setProperty(BoxedValue& obj, const std::string& property_name, const BoxedValue& value);
```

Dynamically sets a property value on a `BoxedValue` object.

### fireEvent

```cpp
void fireEvent(BoxedValue& obj, const std::string& event_name, const std::vector<BoxedValue>& args);
```

Fires an event on a `BoxedValue` object.

### createInstance

```cpp
auto createInstance(const std::string& type_name, std::vector<BoxedValue> args) -> BoxedValue;
```

Dynamically constructs an object by type name.

## TypeRegistrar Template Class

The `TypeRegistrar` class provides a way to register types with the `TypeRegistry`.

### Public Methods

```cpp
static void registerType(const std::string& type_name);
```

Registers a type with metadata, including a default constructor, events, and methods.

## Usage Examples

### Registering a Type

```cpp
class MyClass {};

atom::meta::TypeRegistrar<MyClass>::registerType("MyClass");
```

### Calling a Method

```cpp
atom::meta::BoxedValue obj = atom::meta::createInstance("MyClass", {});
atom::meta::callMethod(obj, "print", {atom::meta::BoxedValue("Hello, World!")});
```

### Getting and Setting Properties

```cpp
auto value = atom::meta::getProperty(obj, "someProperty");
atom::meta::setProperty(obj, "someProperty", atom::meta::BoxedValue(42));
```

### Firing an Event

```cpp
atom::meta::fireEvent(obj, "onCreate", {});
```

## Notes

- The system supports method overloading, but the current implementation doesn't check argument types for matching overloads.
- The `TypeRegistry` is thread-safe, using a shared mutex for concurrent access.
- Error handling is implemented using custom exceptions (e.g., `THROW_NOT_FOUND`).
- The `TypeRegistrar` provides a basic implementation for registering types. You may need to extend it for more complex type registrations.
- The system relies on the `BoxedValue` class (not shown in this documentation) for type-erased value storage.
