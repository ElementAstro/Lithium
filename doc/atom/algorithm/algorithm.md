# Atom Algorithm Library

## Overview

The `atom::algorithm` namespace provides implementations of various algorithms, focusing on string searching and data structures. This module is part of the Atom library and is defined in the `ATOM_ALGORITHM_ALGORITHM_HPP` header file.

## Classes

### KMP (Knuth-Morris-Pratt)

The `KMP` class implements the Knuth-Morris-Pratt string searching algorithm.

#### Constructor

```cpp
explicit KMP(std::string_view pattern);
```

Constructs a KMP object with the given pattern to search for.

#### Methods

1. `search`

   ```cpp
   [[nodiscard]] auto search(std::string_view text) const -> std::vector<int>;
   ```

   Searches for occurrences of the pattern in the given text.

   - Returns: A vector of integers representing the starting positions of pattern matches in the text.

2. `setPattern`
   ```cpp
   void setPattern(std::string_view pattern);
   ```
   Sets a new pattern for searching.

#### Private Method

- `computeFailureFunction`
  ```cpp
  auto computeFailureFunction(std::string_view pattern) -> std::vector<int>;
  ```
  Computes the failure function (partial match table) for the given pattern.

### BloomFilter

The `BloomFilter` class implements a Bloom filter data structure.

#### Template Parameter

- `N`: The size of the Bloom filter (number of bits).

#### Constructor

```cpp
explicit BloomFilter(std::size_t num_hash_functions);
```

Constructs a new BloomFilter object with the specified number of hash functions.

#### Methods

1. `insert`

   ```cpp
   void insert(std::string_view element);
   ```

   Inserts an element into the Bloom filter.

2. `contains`

   ```cpp
   [[nodiscard]] auto contains(std::string_view element) const -> bool;
   ```

   Checks if an element might be present in the Bloom filter.

   - Returns: `true` if the element might be present, `false` otherwise.

#### Private Method

- `hash`
  ```cpp
  auto hash(std::string_view element, std::size_t seed) const -> std::size_t;
  ```
  Computes the hash value of an element using a specific seed.

### BoyerMoore

The `BoyerMoore` class implements the Boyer-Moore string searching algorithm.

#### Constructor

```cpp
explicit BoyerMoore(std::string_view pattern);
```

Constructs a BoyerMoore object with the given pattern to search for.

#### Methods

1. `search`

   ```cpp
   auto search(std::string_view text) const -> std::vector<int>;
   ```

   Searches for occurrences of the pattern in the given text.

   - Returns: A vector of integers representing the starting positions of pattern matches in the text.

2. `setPattern`
   ```cpp
   void setPattern(std::string_view pattern);
   ```
   Sets a new pattern for searching.

#### Private Methods

1. `computeBadCharacterShift`

   ```cpp
   void computeBadCharacterShift();
   ```

   Computes the bad character shift table for the current pattern.

2. `computeGoodSuffixShift`
   ```cpp
   void computeGoodSuffixShift();
   ```
   Computes the good suffix shift table for the current pattern.

## Usage Examples

### KMP

```cpp
atom::algorithm::KMP kmp("pattern");
std::vector<int> matches = kmp.search("This is a text with a pattern in it.");
```

### BloomFilter

```cpp
atom::algorithm::BloomFilter<1000> filter(3);
filter.insert("apple");
bool contains = filter.contains("apple");
```

### BoyerMoore

```cpp
atom::algorithm::BoyerMoore bm("pattern");
std::vector<int> matches = bm.search("This is a text with a pattern in it.");
```

## Notes

- The library uses C++17 features, particularly `std::string_view`.
- The Bloom filter implementation is templated, allowing for customization of the filter size.
- Both KMP and Boyer-Moore algorithms are efficient string searching algorithms with different performance characteristics depending on the input.

## Dependencies

- C++17 or later
- Standard library headers: `<bitset>`, `<string>`, `<string_view>`, `<unordered_map>`, `<vector>`
