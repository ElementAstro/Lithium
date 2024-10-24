#ifndef ATOM_ALGORITHM_TEA_HPP
#define ATOM_ALGORITHM_TEA_HPP

#include <array>
#include <cstdint>
#include <vector>

namespace atom::algorithm {
using XTEAKey = std::array<uint32_t, 4>;

/**
 * @brief Encrypts two 32-bit values using the TEA algorithm.
 *
 * @param value0 The first 32-bit value to be encrypted.
 * @param value1 The second 32-bit value to be encrypted.
 * @param key The 128-bit key used for encryption.
 */
auto teaEncrypt(uint32_t &value0, uint32_t &value1,
                const std::array<uint32_t, 4> &key) -> void;

/**
 * @brief Decrypts two 32-bit values using the TEA algorithm.
 *
 * @param value0 The first 32-bit value to be decrypted.
 * @param value1 The second 32-bit value to be decrypted.
 * @param key The 128-bit key used for decryption.
 */
auto teaDecrypt(uint32_t &value0, uint32_t &value1,
                const std::array<uint32_t, 4> &key) -> void;

/**
 * @brief Encrypts a vector of 32-bit values using the XXTEA algorithm.
 *
 * @param inputData The vector of 32-bit values to be encrypted.
 * @param inputKey The 128-bit key used for encryption.
 * @return A vector of encrypted 32-bit values.
 */
auto xxteaEncrypt(const std::vector<uint32_t> &inputData,
                  const std::vector<uint32_t> &inputKey)
    -> std::vector<uint32_t>;

/**
 * @brief Decrypts a vector of 32-bit values using the XXTEA algorithm.
 *
 * @param inputData The vector of 32-bit values to be decrypted.
 * @param inputKey The 128-bit key used for decryption.
 * @return A vector of decrypted 32-bit values.
 */
auto xxteaDecrypt(const std::vector<uint32_t> &inputData,
                  const std::vector<uint32_t> &inputKey)
    -> std::vector<uint32_t>;

/**
 * @brief Encrypts two 32-bit values using the XTEA algorithm.
 *
 * @param value0 The first 32-bit value to be encrypted.
 * @param value1 The second 32-bit value to be encrypted.
 * @param key The 128-bit key used for encryption.
 */
auto xteaEncrypt(uint32_t &value0, uint32_t &value1,
                 const XTEAKey &key) -> void;

/**
 * @brief Decrypts two 32-bit values using the XTEA algorithm.
 *
 * @param value0 The first 32-bit value to be decrypted.
 * @param value1 The second 32-bit value to be decrypted.
 * @param key The 128-bit key used for decryption.
 */
auto xteaDecrypt(uint32_t &value0, uint32_t &value1,
                 const XTEAKey &key) -> void;

/**
 * @brief Converts a byte array to a vector of 32-bit unsigned integers.
 *
 * @param data The byte array to be converted.
 * @return A vector of 32-bit unsigned integers.
 */
auto toUint32Vector(const std::vector<uint8_t> &data) -> std::vector<uint32_t>;

/**
 * @brief Converts a vector of 32-bit unsigned integers back to a byte array.
 *
 * @param data The vector of 32-bit unsigned integers to be converted.
 * @return A byte array.
 */
auto toByteArray(const std::vector<uint32_t> &data) -> std::vector<uint8_t>;
}  // namespace atom::algorithm

#endif