/*
 * mhash.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-16

Description: Implementation of murmur3 hash and quick hash

**************************************************/

#ifndef ATOM_ALGORITHM_MHASH_HPP
#define ATOM_ALGORITHM_MHASH_HPP

#include <stdint.h>
#include <cstdint>
#include <functional>
#include <limits>
#include <ranges>
#include <string>
#include <vector>

#include "macro.hpp"

namespace atom::algorithm {
/**
 * @brief Converts a string to a hexadecimal string representation.
 *
 * @param data The input string.
 * @return std::string The hexadecimal string representation.
 */
ATOM_NODISCARD auto hexstringFromData(const std::string& data) -> std::string;

/**
 * @brief Converts a hexadecimal string representation to binary data.
 *
 * @param data The input hexadecimal string.
 * @return std::string The binary data.
 * @throw std::invalid_argument If the input hexstring is not a valid
 * hexadecimal string.
 */
ATOM_NODISCARD auto dataFromHexstring(const std::string& data) -> std::string;

/**
 * @brief Implements the MinHash algorithm for estimating Jaccard similarity.
 *
 * The MinHash algorithm generates hash signatures for sets and estimates the
 * Jaccard index between sets based on these signatures.
 */
class MinHash {
public:
    /**
     * @brief Type definition for a hash function used in MinHash.
     */
    using HashFunction = std::function<size_t(size_t)>;

    /**
     * @brief Constructs a MinHash object with a specified number of hash
     * functions.
     *
     * @param num_hashes The number of hash functions to use for MinHash.
     */
    explicit MinHash(size_t num_hashes);

    /**
     * @brief Computes the MinHash signature (hash values) for a given set.
     *
     * @tparam Range Type of the range representing the set elements.
     * @param set The set for which to compute the MinHash signature.
     * @return std::vector<size_t> MinHash signature (hash values) for the set.
     */
    template <std::ranges::range Range>
    auto computeSignature(const Range& set) const -> std::vector<size_t> {
        std::vector<size_t> signature(hash_functions_.size(),
                                      std::numeric_limits<size_t>::max());

        for (const auto& element : set) {
            size_t elementHash =
                std::hash<typename Range::value_type>{}(element);
            for (size_t i = 0; i < hash_functions_.size(); ++i) {
                signature[i] =
                    std::min(signature[i], hash_functions_[i](elementHash));
            }
        }

        return signature;
    }

    /**
     * @brief Computes the Jaccard index between two sets based on their MinHash
     * signatures.
     *
     * @param sig1 MinHash signature of the first set.
     * @param sig2 MinHash signature of the second set.
     * @return double Estimated Jaccard index between the two sets.
     */
    static auto jaccardIndex(const std::vector<size_t>& sig1,
                             const std::vector<size_t>& sig2) -> double;

private:
    /**
     * @brief Vector of hash functions used for MinHash.
     */
    std::vector<HashFunction> hash_functions_;

    /**
     * @brief Generates a hash function suitable for MinHash.
     *
     * @return HashFunction Generated hash function.
     */
    static auto generateHashFunction() -> HashFunction;
};

}  // namespace atom::algorithm

#endif
