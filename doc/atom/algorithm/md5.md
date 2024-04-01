# MD5 Class

The MD5 class provides functionality to calculate the MD5 hash of input data using the MD5 algorithm.

## Member Functions

### Public Methods

#### `static std::string encrypt(const std::string &input)`

Encrypts the input string using the MD5 algorithm and returns the MD5 hash of the input string.

### Private Methods

#### `void init()`

Initializes internal variables and a buffer for MD5 computation.

#### `void update(const std::string &input)`

Updates the MD5 computation with additional input data.

#### `std::string finalize()`

Finalizes the MD5 computation and returns the resulting hash of all the input data provided so far.

#### `void processBlock(const uint8_t *block)`

Processes a 64-byte block of input data.

### Static Helper Functions

#### `static uint32_t F(uint32_t x, uint32_t y, uint32_t z)`

The F function for the MD5 algorithm.

#### `static uint32_t G(uint32_t x, uint32_t y, uint32_t z)`

The G function for the MD5 algorithm.

#### `static uint32_t H(uint32_t x, uint32_t y, uint32_t z)`

The H function for the MD5 algorithm.

#### `static uint32_t I(uint32_t x, uint32_t y, uint32_t z)`

The I function for the MD5 algorithm.

#### `static uint32_t leftRotate(uint32_t x, uint32_t n)`

Performs a left rotate operation on the input.

#### `static uint32_t reverseBytes(uint32_t x)`

Reverses the bytes in the input.

## Member Variables

- `uint32_t _a, _b, _c, _d`: Internal state variables for MD5 computation.
- `uint64_t _count`: Total count of input bits.
- `std::vector<uint8_t> _buffer`: Buffer for input data.

## Example Usage

```cpp
#include "MD5.h"
#include <iostream>

int main() {
    std::string input = "Hello, World!";
    std::string md5Hash = MD5::encrypt(input);
    std::cout << "MD5 Hash: " << md5Hash << std::endl;
    return 0;
}
```
