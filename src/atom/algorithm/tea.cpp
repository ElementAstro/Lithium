#include "tea.hpp"

namespace atom::algorithm {
// Constants for TEA
constexpr uint32_t DELTA = 0x9E3779B9;
constexpr int NUM_ROUNDS = 32;
constexpr int SHIFT_4 = 4;
constexpr int SHIFT_5 = 5;
constexpr int BYTE_SHIFT = 8;
constexpr size_t MIN_ROUNDS = 6;
constexpr size_t MAX_ROUNDS = 52;
constexpr int SHIFT_3 = 3;
constexpr int SHIFT_2 = 2;
constexpr uint32_t KEY_MASK = 3;
constexpr int SHIFT_11 = 11;

// TEA encryption function
auto teaEncrypt(uint32_t &value0, uint32_t &value1,
                const std::array<uint32_t, 4> &key) -> void {
    uint32_t sum = 0;
    for (int i = 0; i < NUM_ROUNDS; ++i) {
        sum += DELTA;
        value0 += ((value1 << SHIFT_4) + key[0]) ^ (value1 + sum) ^
                  ((value1 >> SHIFT_5) + key[1]);
        value1 += ((value0 << SHIFT_4) + key[2]) ^ (value0 + sum) ^
                  ((value0 >> SHIFT_5) + key[3]);
    }
}

// TEA decryption function
auto teaDecrypt(uint32_t &value0, uint32_t &value1,
                const std::array<uint32_t, 4> &key) -> void {
    uint32_t sum = DELTA * NUM_ROUNDS;
    for (int i = 0; i < NUM_ROUNDS; ++i) {
        value1 -= ((value0 << SHIFT_4) + key[2]) ^ (value0 + sum) ^
                  ((value0 >> SHIFT_5) + key[3]);
        value0 -= ((value1 << SHIFT_4) + key[0]) ^ (value1 + sum) ^
                  ((value1 >> SHIFT_5) + key[1]);
        sum -= DELTA;
    }
}

// Helper function to convert a byte array to a vector of uint32_t
auto toUint32Vector(const std::vector<uint8_t> &data) -> std::vector<uint32_t> {
    size_t numElements = (data.size() + 3) / 4;
    std::vector<uint32_t> result(numElements);

    for (size_t index = 0; index < data.size(); ++index) {
        result[index / 4] |= static_cast<uint32_t>(data[index])
                             << ((index % 4) * BYTE_SHIFT);
    }

    return result;
}

// Helper function to convert a vector of uint32_t back to a byte array
auto toByteArray(const std::vector<uint32_t> &data) -> std::vector<uint8_t> {
    std::vector<uint8_t> result(data.size() * 4);

    for (size_t index = 0; index < data.size() * 4; ++index) {
        result[index] =
            static_cast<uint8_t>(data[index / 4] >> ((index % 4) * BYTE_SHIFT));
    }

    return result;
}

// XXTEA encrypt function
auto xxteaEncrypt(const std::vector<uint32_t> &inputData,
                  const std::vector<uint32_t> &inputKey)
    -> std::vector<uint32_t> {
    size_t numElements = inputData.size();
    if (numElements < 2) {
        return inputData;
    }

    uint32_t sum = 0;
    uint32_t lastElement = inputData[numElements - 1];
    uint32_t currentElement;
    size_t numRounds = MIN_ROUNDS + MAX_ROUNDS / numElements;

    std::vector<uint32_t> result = inputData;

    for (size_t roundIndex = 0; roundIndex < numRounds; ++roundIndex) {
        sum += DELTA;
        uint32_t keyIndex = (sum >> SHIFT_2) & KEY_MASK;
        for (size_t elementIndex = 0; elementIndex < numElements - 1;
             ++elementIndex) {
            currentElement = result[elementIndex + 1];
            result[elementIndex] +=
                ((lastElement >> SHIFT_5) ^ (currentElement << SHIFT_2)) +
                    ((currentElement >> SHIFT_3) ^ (lastElement << SHIFT_4)) ^
                ((sum ^ currentElement) +
                 (inputKey[(elementIndex & KEY_MASK) ^ keyIndex] ^
                  lastElement));
            lastElement = result[elementIndex];
        }
        currentElement = result[0];
        result[numElements - 1] +=
            ((lastElement >> SHIFT_5) ^ (currentElement << SHIFT_2)) +
                ((currentElement >> SHIFT_3) ^ (lastElement << SHIFT_4)) ^
            ((sum ^ currentElement) +
             (inputKey[((numElements - 1) & KEY_MASK) ^ keyIndex] ^
              lastElement));
        lastElement = result[numElements - 1];
    }

    return result;
}

// XXTEA decrypt function
auto xxteaDecrypt(const std::vector<uint32_t> &inputData,
                  const std::vector<uint32_t> &inputKey)
    -> std::vector<uint32_t> {
    size_t numElements = inputData.size();
    if (numElements < 2) {
        return inputData;
    }

    uint32_t sum = (MIN_ROUNDS + MAX_ROUNDS / numElements) * DELTA;
    uint32_t lastElement = inputData[numElements - 1];
    uint32_t currentElement;

    std::vector<uint32_t> result = inputData;

    for (size_t roundIndex = 0;
         roundIndex < MIN_ROUNDS + MAX_ROUNDS / numElements; ++roundIndex) {
        uint32_t keyIndex = (sum >> SHIFT_2) & KEY_MASK;
        for (size_t elementIndex = numElements - 1; elementIndex > 0;
             --elementIndex) {
            lastElement = result[elementIndex - 1];
            result[elementIndex] -=
                ((lastElement >> SHIFT_5) ^ (currentElement << SHIFT_2)) +
                    ((currentElement >> SHIFT_3) ^ (lastElement << SHIFT_4)) ^
                ((sum ^ currentElement) +
                 (inputKey[(elementIndex & KEY_MASK) ^ keyIndex] ^
                  lastElement));
            currentElement = result[elementIndex];
        }
        lastElement = result[numElements - 1];
        result[0] -=
            ((lastElement >> SHIFT_5) ^ (currentElement << SHIFT_2)) +
                ((currentElement >> SHIFT_3) ^ (lastElement << SHIFT_4)) ^
            ((sum ^ currentElement) +
             (inputKey[((numElements - 1) & KEY_MASK) ^ keyIndex] ^
              lastElement));
        sum -= DELTA;
        currentElement = result[0];
    }

    return result;
}

// XTEA encryption function
auto xteaEncrypt(uint32_t &value0, uint32_t &value1,
                 const XTEAKey &key) -> void {
    uint32_t sum = 0;
    for (int i = 0; i < NUM_ROUNDS; ++i) {
        value0 += ((value1 << SHIFT_4) ^ (value1 >> SHIFT_5)) + value1 ^
                  (sum + key[sum & KEY_MASK]);
        sum += DELTA;
        value1 += ((value0 << SHIFT_4) ^ (value0 >> SHIFT_5)) + value0 ^
                  (sum + key[(sum >> SHIFT_11) & KEY_MASK]);
    }
}

// XTEA decryption function
auto xteaDecrypt(uint32_t &value0, uint32_t &value1,
                 const XTEAKey &key) -> void {
    uint32_t sum = DELTA * NUM_ROUNDS;
    for (int i = 0; i < NUM_ROUNDS; ++i) {
        value1 -= ((value0 << SHIFT_4) ^ (value0 >> SHIFT_5)) + value0 ^
                  (sum + key[(sum >> SHIFT_11) & KEY_MASK]);
        sum -= DELTA;
        value0 -= ((value1 << SHIFT_4) ^ (value1 >> SHIFT_5)) + value1 ^
                  (sum + key[sum & KEY_MASK]);
    }
}
}  // namespace atom::algorithm