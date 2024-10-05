# UUID Class Documentation

## Overview

The `UUID` class is a C++ wrapper around Boost's UUID implementation, providing a convenient interface for generating, manipulating, and comparing Universally Unique Identifiers (UUIDs). It is defined in the `atom::extra::boost` namespace and offers methods for creating different versions of UUIDs, converting between various formats, and performing UUID-related operations.

## Table of Contents

1. [Constructors](#constructors)
2. [Basic Operations](#basic-operations)
3. [Conversion Methods](#conversion-methods)
4. [UUID Generation](#uuid-generation)
5. [UUID Properties](#uuid-properties)
6. [Advanced Operations](#advanced-operations)
7. [Namespace UUIDs](#namespace-uuids)
8. [Comparison and Hashing](#comparison-and-hashing)

## Constructors

```cpp
UUID()
explicit UUID(const std::string& str)
explicit UUID(const ::boost::uuids::uuid& uuid)
```

The `UUID` class provides three constructors:

1. Default constructor: Generates a new random UUID (version 4).
2. String constructor: Creates a UUID from a string representation.
3. Boost UUID constructor: Creates a UUID from a Boost UUID object.

**Examples:**

```cpp
atom::extra::boost::UUID uuid1;  // Random UUID
atom::extra::boost::UUID uuid2("123e4567-e89b-12d3-a456-426614174000");  // From string
boost::uuids::uuid boost_uuid = boost::uuids::random_generator()();
atom::extra::boost::UUID uuid3(boost_uuid);  // From Boost UUID
```

## Basic Operations

### toString()

```cpp
[[nodiscard]] auto toString() const -> std::string
```

Converts the UUID to its string representation.

**Example:**

```cpp
atom::extra::boost::UUID uuid;
std::cout << uuid.toString() << std::endl;  // e.g., "123e4567-e89b-12d3-a456-426614174000"
```

### isNil()

```cpp
[[nodiscard]] auto isNil() const -> bool
```

Checks if the UUID is nil (all zeros).

**Example:**

```cpp
atom::extra::boost::UUID uuid;
bool is_nil = uuid.isNil();
```

### format()

```cpp
[[nodiscard]] auto format() const -> std::string
```

Returns a formatted string representation of the UUID, enclosed in curly braces.

**Example:**

```cpp
atom::extra::boost::UUID uuid;
std::cout << uuid.format() << std::endl;  // e.g., "{123e4567-e89b-12d3-a456-426614174000}"
```

## Conversion Methods

### toBytes()

```cpp
[[nodiscard]] auto toBytes() const -> std::vector<uint8_t>
```

Converts the UUID to a vector of bytes.

**Example:**

```cpp
atom::extra::boost::UUID uuid;
std::vector<uint8_t> bytes = uuid.toBytes();
```

### fromBytes()

```cpp
static auto fromBytes(const std::span<const uint8_t>& bytes) -> UUID
```

Creates a UUID from a span of bytes.

**Example:**

```cpp
std::vector<uint8_t> bytes = {/* ... 16 bytes ... */};
atom::extra::boost::UUID uuid = atom::extra::boost::UUID::fromBytes(bytes);
```

### toUint64()

```cpp
[[nodiscard]] auto toUint64() const -> uint64_t
```

Converts the UUID to a 64-bit unsigned integer.

**Example:**

```cpp
atom::extra::boost::UUID uuid;
uint64_t value = uuid.toUint64();
```

### toBase64()

```cpp
[[nodiscard]] auto toBase64() const -> std::string
```

Converts the UUID to a Base64 encoded string.

**Example:**

```cpp
atom::extra::boost::UUID uuid;
std::string base64 = uuid.toBase64();
```

## UUID Generation

### v1()

```cpp
[[nodiscard]] static auto v1() -> UUID
```

Generates a version 1 (time-based) UUID.

**Example:**

```cpp
atom::extra::boost::UUID uuid = atom::extra::boost::UUID::v1();
```

### v3()

```cpp
static auto v3(const UUID& namespace_uuid, const std::string& name) -> UUID
```

Generates a version 3 (name-based, MD5) UUID.

**Example:**

```cpp
atom::extra::boost::UUID namespace_uuid = atom::extra::boost::UUID::namespaceURL();
atom::extra::boost::UUID uuid = atom::extra::boost::UUID::v3(namespace_uuid, "example.com");
```

### v4()

```cpp
[[nodiscard]] static auto v4() -> UUID
```

Generates a version 4 (random) UUID.

**Example:**

```cpp
atom::extra::boost::UUID uuid = atom::extra::boost::UUID::v4();
```

### v5()

```cpp
static auto v5(const UUID& namespace_uuid, const std::string& name) -> UUID
```

Generates a version 5 (name-based, SHA1) UUID.

**Example:**

```cpp
atom::extra::boost::UUID namespace_uuid = atom::extra::boost::UUID::namespaceURL();
atom::extra::boost::UUID uuid = atom::extra::boost::UUID::v5(namespace_uuid, "example.com");
```

## UUID Properties

### version()

```cpp
[[nodiscard]] auto version() const -> int
```

Returns the version of the UUID.

**Example:**

```cpp
atom::extra::boost::UUID uuid;
int version = uuid.version();
```

### variant()

```cpp
[[nodiscard]] auto variant() const -> int
```

Returns the variant of the UUID.

**Example:**

```cpp
atom::extra::boost::UUID uuid;
int variant = uuid.variant();
```

## Advanced Operations

### getTimestamp()

```cpp
[[nodiscard]] auto getTimestamp() const -> std::chrono::system_clock::time_point
```

Returns the timestamp of a version 1 UUID.

**Example:**

```cpp
atom::extra::boost::UUID uuid = atom::extra::boost::UUID::v1();
auto timestamp = uuid.getTimestamp();
std::cout << "UUID timestamp: " << std::chrono::system_clock::to_time_t(timestamp) << std::endl;
```

## Namespace UUIDs

The `UUID` class provides static methods to access predefined namespace UUIDs:

```cpp
static auto namespaceDNS() -> UUID
static auto namespaceURL() -> UUID
static auto namespaceOID() -> UUID
```

**Example:**

```cpp
atom::extra::boost::UUID dns_namespace = atom::extra::boost::UUID::namespaceDNS();
atom::extra::boost::UUID url_namespace = atom::extra::boost::UUID::namespaceURL();
atom::extra::boost::UUID oid_namespace = atom::extra::boost::UUID::namespaceOID();
```

## Comparison and Hashing

The `UUID` class supports comparison operations and can be used as a key in standard containers.

**Example:**

```cpp
atom::extra::boost::UUID uuid1, uuid2;
if (uuid1 < uuid2) {
    std::cout << "uuid1 is less than uuid2" << std::endl;
}

std::unordered_set<atom::extra::boost::UUID> uuid_set;
uuid_set.insert(uuid1);
uuid_set.insert(uuid2);
```

## Complete Usage Example

Here's a comprehensive example demonstrating various features of the `UUID` class:

```cpp
#include <iostream>
#include <unordered_set>
#include <chrono>

int main() {
    // Generate a random UUID (v4)
    atom::extra::boost::UUID uuid1;
    std::cout << "Random UUID: " << uuid1.toString() << std::endl;

    // Create a UUID from string
    atom::extra::boost::UUID uuid2("123e4567-e89b-12d3-a456-426614174000");
    std::cout << "UUID from string: " << uuid2.toString() << std::endl;

    // Generate a v1 (time-based) UUID
    atom::extra::boost::UUID uuid3 = atom::extra::boost::UUID::v1();
    std::cout << "V1 UUID: " << uuid3.toString() << std::endl;

    // Get timestamp from v1 UUID
    auto timestamp = uuid3.getTimestamp();
    std::cout << "V1 UUID timestamp: " << std::chrono::system_clock::to_time_t(timestamp) << std::endl;

    // Generate v3 and v5 UUIDs
    atom::extra::boost::UUID namespace_uuid = atom::extra::boost::UUID::namespaceURL();
    atom::extra::boost::UUID uuid4 = atom::extra::boost::UUID::v3(namespace_uuid, "example.com");
    atom::extra::boost::UUID uuid5 = atom::extra::boost::UUID::v5(namespace_uuid, "example.com");
    std::cout << "V3 UUID: " << uuid4.toString() << std::endl;
    std::cout << "V5 UUID: " << uuid5.toString() << std::endl;

    // Convert UUID to different formats
    std::cout << "UUID as bytes: ";
    for (auto byte : uuid1.toBytes()) {
        std::cout << std::hex << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;

    std::cout << "UUID as uint64: " << uuid1.toUint64() << std::endl;
    std::cout << "UUID as Base64: " << uuid1.toBase64() << std::endl;

    // Check UUID properties
    std::cout << "UUID version: " << uuid1.version() << std::endl;
    std::cout << "UUID variant: " << uuid1.variant() << std::endl;

    // Compare UUIDs
    if (uuid1 < uuid2) {
        std::cout << "uuid1 is less than uuid2" << std::endl;
    }

    // Use UUID in standard containers
    std::unordered_set<atom::extra::boost::UUID> uuid_set;
    uuid_set.insert(uuid1);
    uuid_set.insert(uuid2);
    std::cout << "UUID set size: " << uuid_set.size() << std::endl;

    // Format UUID
    std::cout << "Formatted UUID: " << uuid1.format() << std::endl;

    // Check if UUID is nil
    atom::extra::boost::UUID nil_uuid("00000000-0000-0000-0000-000000000000");
    std::cout << "Is nil UUID nil? " << (nil_uuid.isNil() ? "Yes" : "No") << std::endl;

    return 0;
}
```

This example demonstrates the following features of the `UUID` class:

1. Generating random (v4) UUIDs
2. Creating UUIDs from strings
3. Generating time-based (v1) UUIDs and retrieving their timestamps
4. Generating name-based (v3 and v5) UUIDs
5. Converting UUIDs to different formats (bytes, uint64, Base64)
6. Checking UUID properties (version and variant)
7. Comparing UUIDs
8. Using UUIDs in standard containers (e.g., `std::unordered_set`)
9. Formatting UUIDs
10. Checking if a UUID is nil

## Best Practices

When working with the `UUID` class, consider the following best practices:

1. Use the appropriate UUID version for your use case:

   - V1 for time-based UUIDs
   - V3 or V5 for name-based UUIDs (prefer V5 for better collision resistance)
   - V4 for random UUIDs

2. When generating name-based UUIDs (V3 or V5), use the appropriate namespace UUID for your domain (DNS, URL, or OID).

3. Always check for errors when creating UUIDs from strings or bytes to ensure the input is valid.

4. Use the `isNil()` method to check for nil UUIDs when necessary.

5. When working with V1 UUIDs, remember that the timestamp is based on the time the UUID was generated, not the current time.

6. Use the `format()` method when you need to display UUIDs in a standardized format (e.g., in logs or user interfaces).

7. Leverage the comparison operators and hash function support to use UUIDs in sorted containers or hash-based containers efficiently.

## Conclusion

The `UUID` class provides a robust and feature-rich implementation for working with UUIDs in C++. It offers a wide range of functionality, including generation of different UUID versions, conversion between various formats, and utility methods for UUID manipulation and comparison. By utilizing this class, developers can easily integrate UUID support into their applications, ensuring unique identification across distributed systems and databases.
