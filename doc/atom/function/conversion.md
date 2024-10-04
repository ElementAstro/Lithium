# Type Conversion System Documentation

## Overview

This document covers the type conversion system implemented in the `atom::meta` namespace. The system provides a flexible and extensible way to perform various type conversions, including static and dynamic conversions, as well as conversions for container types.

## Key Components

### TypeConversionBase

An abstract base class for all type conversions.

```cpp
class TypeConversionBase {
public:
    virtual auto convert(const std::any& from) const -> std::any = 0;
    virtual auto convertDown(const std::any& toAny) const -> std::any = 0;
    virtual auto to() const ATOM_NOEXCEPT -> const TypeInfo&;
    virtual auto from() const ATOM_NOEXCEPT -> const TypeInfo&;
    virtual auto bidir() const ATOM_NOEXCEPT -> bool;
    // ... other methods ...
};
```

### StaticConversion

Handles static (compile-time) conversions between types.

```cpp
template <typename From, typename To>
class StaticConversion : public TypeConversionBase {
    // ... implementation ...
};
```

### DynamicConversion

Handles dynamic (runtime) conversions between polymorphic types.

```cpp
template <typename From, typename To>
class DynamicConversion : public TypeConversionBase {
    // ... implementation ...
};
```

### Container Conversions

Specialized conversion classes for various container types:

- `VectorConversion`: Converts between `std::vector` of different types.
- `MapConversion`: Converts between map-like containers of different types.
- `SequenceConversion`: Converts between sequence containers of different types.
- `SetConversion`: Converts between set-like containers of different types.

### TypeConversions

A class that manages and performs type conversions.

```cpp
class TypeConversions {
public:
    static auto createShared() -> std::shared_ptr<TypeConversions>;
    void addConversion(const std::shared_ptr<TypeConversionBase>& conversion);
    template <typename To, typename From>
    auto convert(const std::any& from) const -> std::any;
    auto canConvert(const TypeInfo& fromTypeInfo, const TypeInfo& toTypeInfo) const -> bool;
    // ... other methods ...
};
```

## Usage Examples

### Adding a Base Class Conversion

```cpp
TypeConversions conversions;
conversions.addBaseClass<Base, Derived>();
```

This adds both static and dynamic conversions between `Base` and `Derived`.

### Adding a Vector Conversion

```cpp
conversions.addVectorConversion<From, To>();
```

This adds a conversion between `std::vector<std::shared_ptr<From>>` and `std::vector<std::shared_ptr<To>>`.

### Performing a Conversion

```cpp
std::any fromValue = /* ... */;
auto toValue = conversions.convert<To, From>(fromValue);
```

This converts a value of type `From` to type `To`.

### Checking If Conversion Is Possible

```cpp
bool canConvert = conversions.canConvert(userType<From>(), userType<To>());
```

This checks if a conversion from `From` to `To` is possible.

## Detailed Descriptions

### StaticConversion

Handles upcasting for pointer and reference types. It uses `static_cast` for the conversion.

### DynamicConversion

Handles dynamic casting for polymorphic types. It uses `dynamic_cast` for the conversion and checks for null pointers in case of failed conversions.

### VectorConversion

Converts between vectors of different types. It performs element-wise conversion using `dynamic_pointer_cast`.

### MapConversion

Converts between map-like containers with different key or value types. It performs key conversion using `static_cast` and value conversion using `dynamic_pointer_cast`.

### SequenceConversion

Converts between sequence containers (like `std::list` or `std::deque`) of different types. It performs element-wise conversion using `dynamic_pointer_cast`.

### SetConversion

Converts between set-like containers of different types. It performs element-wise conversion using `dynamic_pointer_cast`.

## Error Handling

The system uses a custom `BadConversionException` for reporting conversion errors. The `THROW_CONVERSION_ERROR` macro is used to throw these exceptions with detailed error messages.

## Performance Considerations

- The system uses `std::any` for type erasure, which may have some performance overhead.
- Dynamic conversions use `dynamic_cast`, which has a runtime cost.
- The `TypeConversions` class uses a hash map for storing conversions, providing fast lookup times.
- An optional fast hash implementation (`emhash8::HashMap`) can be used by defining `ENABLE_FASTHASH`.

## Thread Safety

The `TypeConversions` class does not provide built-in thread safety. If used in a multi-threaded environment, external synchronization should be applied.

## Extensibility

The system is designed to be extensible. New conversion types can be added by deriving from `TypeConversionBase` and implementing the required virtual functions.

## Notes

- The system heavily relies on C++17 features like `std::any` and `if constexpr`.
- It supports both value semantics and reference semantics for conversions.
- The conversion system is particularly useful for scenarios involving polymorphic types and container conversions.
- Care should be taken when using dynamic conversions, as they can throw exceptions if the conversion fails.
