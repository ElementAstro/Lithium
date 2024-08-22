#include "atom/algorithm/base.hpp"

#include <iostream>

int main() {
    {
        std::string originalText = "Hello, World!";
        std::string encodedText = atom::algorithm::base64Encode(originalText);

        std::cout << "Original: " << originalText << std::endl;
        std::cout << "Encoded: " << encodedText << std::endl;
    }
    {
        std::string encodedText = "SGVsbG8sIFdvcmxkIQ==";
        std::string decodedText = atom::algorithm::base64Decode(encodedText);

        std::cout << "Encoded: " << encodedText << std::endl;
        std::cout << "Decoded: " << decodedText << std::endl;
    }
    {
        std::vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o'};
        std::string encodedText = atom::algorithm::fbase64Encode(data);

        std::cout << "Encoded: " << encodedText << std::endl;
    }
    {
        std::string encodedText = "SGVsbG8=";
        std::vector<unsigned char> decodedData =
            atom::algorithm::fbase64Decode(encodedText);

        std::cout << "Decoded: ";
        for (unsigned char c : decodedData) {
            std::cout << c;
        }
        std::cout << std::endl;
    }
    {
        std::string plaintext = "EncryptMe";
        uint8_t key = 0xAA;
        std::string encryptedText = atom::algorithm::xorEncrypt(plaintext, key);

        std::cout << "Plaintext: " << plaintext << std::endl;
        std::cout << "Encrypted: " << encryptedText << std::endl;
    }
    {
        std::string encryptedText = "EncryptedStringHere";
        uint8_t key = 0xAA;
        std::string decryptedText =
            atom::algorithm::xorDecrypt(encryptedText, key);

        std::cout << "Encrypted: " << encryptedText << std::endl;
        std::cout << "Decrypted: " << decryptedText << std::endl;
    }
    {
        constexpr StaticString<5> INPUT = "Hello";
        constexpr auto ENCODED = atom::algorithm::cbase64Encode(INPUT);

        std::cout << "Compile-time Encoded: " << ENCODED.cStr() << std::endl;
    }
    {
        constexpr StaticString<8> INPUT = "SGVsbG8=";
        constexpr auto DECODED = atom::algorithm::cbase64Decode(INPUT);

        std::cout << "Compile-time Decoded: " << DECODED.cStr() << std::endl;
    }
    return 0;
}
