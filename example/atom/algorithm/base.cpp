#include "atom/algorithm/base.hpp"

#include <iostream>

using namespace Atom::Algorithm;

int main() {
    std::string input = "Hello, world!";
    auto encoded = base32Encode(reinterpret_cast<const uint8_t*>(input.data()),
                                 input.size());
    std::cout << "Encoded: " << encoded << std::endl;

    try {
        auto decoded = base32Decode(encoded);
        std::cout << "Decoded: " << decoded << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::string input = "Hello, world!";
    auto encoded = base128Encode(
        reinterpret_cast<const uint8_t*>(input.data()), input.size());
    std::cout << "Encoded: " << encoded << std::endl;

    try {
        auto decoded = base128Decode(encoded);
        std::cout << "Decoded: " << decoded << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    uint8_t key = 0x42;
    auto encrypted = xorEncrypt(input, key);
    std::cout << "Encrypted: "
              << base128Encode(
                     reinterpret_cast<const uint8_t*>(encrypted.data()),
                     encrypted.size())
              << std::endl;

    auto decrypted = xorDecrypt(encrypted, key);
    std::cout << "Decrypted: " << decrypted << std::endl;

    return 0;
}