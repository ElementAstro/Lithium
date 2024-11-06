#include "sha1.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace atom::algorithm {
SHA1::SHA1() { reset(); }

void SHA1::update(const uint8_t* data, size_t length) {
    size_t remaining = length;
    size_t offset = 0;

    while (remaining > 0) {
        size_t bufferOffset = (bitCount_ / 8) % BLOCK_SIZE;

        size_t bytesToFill = BLOCK_SIZE - bufferOffset;
        size_t bytesToCopy = std::min(remaining, bytesToFill);

        std::copy(data + offset, data + offset + bytesToCopy,
                  buffer_.data() + bufferOffset);
        offset += bytesToCopy;
        remaining -= bytesToCopy;
        bitCount_ += bytesToCopy * BITS_PER_BYTE;

        if (bufferOffset + bytesToCopy == BLOCK_SIZE) {
            processBlock(buffer_.data());
        }
    }
}

std::array<uint8_t, SHA1::DIGEST_SIZE> SHA1::digest() {
    uint64_t bitLength = bitCount_;

    // Padding
    size_t bufferOffset = (bitCount_ / 8) % BLOCK_SIZE;
    buffer_[bufferOffset] = PADDING_BYTE;  // Append the bit '1'

    if (bufferOffset >= BLOCK_SIZE - LENGTH_SIZE) {
        // Not enough space for the length, process the block
        processBlock(buffer_.data());
        std::fill(buffer_.begin(), buffer_.end(), 0);
    }

    // Append the length of the message
    for (size_t i = 0; i < LENGTH_SIZE; ++i) {
        buffer_[BLOCK_SIZE - LENGTH_SIZE + i] =
            (bitLength >> (LENGTH_SIZE * BITS_PER_BYTE - i * BITS_PER_BYTE)) &
            BYTE_MASK;
    }
    processBlock(buffer_.data());

    // Produce the final hash value
    std::array<uint8_t, DIGEST_SIZE> result;
    for (size_t i = 0; i < HASH_SIZE; ++i) {
        result[i * 4] = (hash_[i] >> 24) & BYTE_MASK;
        result[i * 4 + 1] = (hash_[i] >> 16) & BYTE_MASK;
        result[i * 4 + 2] = (hash_[i] >> 8) & BYTE_MASK;
        result[i * 4 + 3] = hash_[i] & BYTE_MASK;
    }

    return result;
}

void SHA1::reset() {
    bitCount_ = 0;
    hash_.fill(0);
    hash_[0] = 0x67452301;
    hash_[1] = 0xEFCDAB89;
    hash_[2] = 0x98BADCFE;
    hash_[3] = 0x10325476;
    hash_[4] = 0xC3D2E1F0;
    buffer_.fill(0);
}

void SHA1::processBlock(const uint8_t* block) {
    std::array<uint32_t, SCHEDULE_SIZE> schedule{};
    for (size_t i = 0; i < 16; ++i) {
        schedule[i] = (block[i * 4] << 24) | (block[i * 4 + 1] << 16) |
                      (block[i * 4 + 2] << 8) | block[i * 4 + 3];
    }

    for (size_t i = 16; i < SCHEDULE_SIZE; ++i) {
        schedule[i] = rotateLeft(schedule[i - 3] ^ schedule[i - 8] ^
                                     schedule[i - 14] ^ schedule[i - 16],
                                 1);
    }

    uint32_t a = hash_[0];
    uint32_t b = hash_[1];
    uint32_t c = hash_[2];
    uint32_t d = hash_[3];
    uint32_t e = hash_[4];

    for (size_t i = 0; i < SCHEDULE_SIZE; ++i) {
        uint32_t f;
        uint32_t k;
        if (i < 20) {
            f = (b & c) | (~b & d);
            k = 0x5A827999;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
        }

        uint32_t temp = rotateLeft(a, 5) + f + e + k + schedule[i];
        e = d;
        d = c;
        c = rotateLeft(b, 30);
        b = a;
        a = temp;
    }

    hash_[0] += a;
    hash_[1] += b;
    hash_[2] += c;
    hash_[3] += d;
    hash_[4] += e;
}

auto SHA1::rotateLeft(uint32_t value, size_t bits) -> uint32_t {
    return (value << bits) | (value >> (WORD_SIZE - bits));
}

auto bytesToHex(const std::array<uint8_t, SHA1::DIGEST_SIZE>& bytes) -> std::string {
    std::ostringstream oss;
    for (uint8_t byte : bytes) {
        oss << std::setw(2) << std::setfill('0') << std::hex << (int)byte;
    }
    return oss.str();
}
}  // namespace atom::algorithm
