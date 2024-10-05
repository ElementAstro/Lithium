# Atom Algorithm Library: Base64 and XOR Encryption

This document provides a detailed explanation of the Base64 encoding/decoding and XOR encryption/decryption functions implemented in the Atom Algorithm Library.

## Table of Contents

1. [Overview](#overview)
2. [Base64 Encoding and Decoding](#base64-encoding-and-decoding)
   - [Runtime Functions](#runtime-functions)
   - [Compile-time Functions](#compile-time-functions)
3. [XOR Encryption and Decryption](#xor-encryption-and-decryption)
4. [Usage Examples](#usage-examples)

## Overview

The Atom Algorithm Library provides implementations for Base64 encoding/decoding and XOR encryption/decryption. It includes both runtime and compile-time versions of Base64 functions.

## Base64 Encoding and Decoding

### Runtime Functions

#### Base64 Encode

```cpp
[[nodiscard]] auto base64Encode(std::string_view bytes_to_encode) -> std::string;
```

This function encodes the input data using the Base64 algorithm.

- **Parameters:**
  - `bytes_to_encode`: A string view of the data to be encoded.
- **Returns:** A string containing the Base64 encoded data.

#### Base64 Decode

```cpp
[[nodiscard]] auto base64Decode(std::string_view encoded_string) -> std::string;
```

This function decodes Base64 encoded data.

- **Parameters:**
  - `encoded_string`: A string view of the Base64 encoded data.
- **Returns:** A string containing the decoded data.

#### Fast Base64 Encode

```cpp
auto fbase64Encode(std::span<const unsigned char> input) -> std::string;
```

An optimized version of Base64 encoding.

- **Parameters:**
  - `input`: A span of unsigned chars to be encoded.
- **Returns:** A string containing the Base64 encoded data.

#### Fast Base64 Decode

```cpp
auto fbase64Decode(std::span<const char> input) -> std::vector<unsigned char>;
```

An optimized version of Base64 decoding.

- **Parameters:**
  - `input`: A span of chars containing Base64 encoded data.
- **Returns:** A vector of unsigned chars containing the decoded data.

### Compile-time Functions

#### Compile-time Base64 Encode

```cpp
template <size_t N>
constexpr auto cbase64Encode(const StaticString<N>& input);
```

This function performs Base64 encoding at compile-time.

- **Template Parameters:**
  - `N`: The size of the input string.
- **Parameters:**
  - `input`: A `StaticString` containing the data to be encoded.
- **Returns:** A `StaticString` containing the Base64 encoded data.

#### Compile-time Base64 Decode

```cpp
template <size_t N>
constexpr auto cbase64Decode(const StaticString<N>& input);
```

This function performs Base64 decoding at compile-time.

- **Template Parameters:**
  - `N`: The size of the input string.
- **Parameters:**
  - `input`: A `StaticString` containing the Base64 encoded data.
- **Returns:** A `StaticString` containing the decoded data.

## XOR Encryption and Decryption

### XOR Encrypt

```cpp
[[nodiscard]] auto xorEncrypt(std::string_view plaintext, uint8_t key) -> std::string;
```

This function encrypts the input data using the XOR algorithm.

- **Parameters:**
  - `plaintext`: A string view of the data to be encrypted.
  - `key`: An 8-bit unsigned integer used as the encryption key.
- **Returns:** A string containing the encrypted data.

### XOR Decrypt

```cpp
[[nodiscard]] auto xorDecrypt(std::string_view ciphertext, uint8_t key) -> std::string;
```

This function decrypts XOR encrypted data.

- **Parameters:**
  - `ciphertext`: A string view of the encrypted data.
  - `key`: An 8-bit unsigned integer used as the decryption key (must be the same as the encryption key).
- **Returns:** A string containing the decrypted data.

## Usage Examples

### Base64 Encoding and Decoding

```cpp
#include "atom/algorithm/base.hpp"
#include <iostream>

int main() {
    std::string original = "Hello, World!";

    // Runtime Base64 encoding
    std::string encoded = atom::algorithm::base64Encode(original);
    std::cout << "Encoded: " << encoded << std::endl;

    // Runtime Base64 decoding
    std::string decoded = atom::algorithm::base64Decode(encoded);
    std::cout << "Decoded: " << decoded << std::endl;

    // Compile-time Base64 encoding
    constexpr auto staticOriginal = atom::StaticString("Hello, World!");
    constexpr auto staticEncoded = atom::algorithm::cbase64Encode(staticOriginal);
    std::cout << "Static Encoded: " << staticEncoded.data() << std::endl;

    // Compile-time Base64 decoding
    constexpr auto staticDecoded = atom::algorithm::cbase64Decode(staticEncoded);
    std::cout << "Static Decoded: " << staticDecoded.data() << std::endl;

    return 0;
}
```

### XOR Encryption and Decryption

```cpp
#include "atom/algorithm/base.hpp"
#include <iostream>

int main() {
    std::string original = "Secret message";
    uint8_t key = 42;

    // XOR encryption
    std::string encrypted = atom::algorithm::xorEncrypt(original, key);
    std::cout << "Encrypted: " << encrypted << std::endl;

    // XOR decryption
    std::string decrypted = atom::algorithm::xorDecrypt(encrypted, key);
    std::cout << "Decrypted: " << decrypted << std::endl;

    return 0;
}
```

These examples demonstrate how to use the Base64 encoding/decoding and XOR encryption/decryption functions provided by the Atom Algorithm Library. The library offers both runtime and compile-time options for Base64 operations, allowing for flexibility in different use cases.
