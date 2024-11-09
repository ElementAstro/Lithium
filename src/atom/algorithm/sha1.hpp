#ifndef ATOM_ALGORITHM_SHA1_HPP
#define ATOM_ALGORITHM_SHA1_HPP

#include <array>
#include <cstdint>
#include <string>

namespace atom::algorithm {
class SHA1 {
public:
    SHA1();

    void update(const uint8_t* data, size_t length);
    auto digest() -> std::array<uint8_t, 20>;
    void reset();

    static constexpr size_t DIGEST_SIZE = 20;

private:
    void processBlock(const uint8_t* block);
    static auto rotateLeft(uint32_t value, size_t bits) -> uint32_t;

    static constexpr size_t BLOCK_SIZE = 64;
    static constexpr size_t HASH_SIZE = 5;
    static constexpr size_t SCHEDULE_SIZE = 80;
    static constexpr size_t LENGTH_SIZE = 8;
    static constexpr size_t BITS_PER_BYTE = 8;
    static constexpr uint8_t PADDING_BYTE = 0x80;
    static constexpr uint8_t BYTE_MASK = 0xFF;
    static constexpr size_t WORD_SIZE = 32;

    std::array<uint32_t, HASH_SIZE> hash_;
    std::array<uint8_t, BLOCK_SIZE> buffer_;
    uint64_t bitCount_;
};

auto bytesToHex(const std::array<uint8_t, 20>& bytes) -> std::string;

}  // namespace atom::algorithm

#endif  // ATOM_ALGORITHM_SHA1_HPP
