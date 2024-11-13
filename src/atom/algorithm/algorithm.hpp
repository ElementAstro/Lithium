/*
 * algorithm.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-5

Description: A collection of algorithms for C++

**************************************************/

#ifndef ATOM_ALGORITHM_ALGORITHM_HPP
#define ATOM_ALGORITHM_ALGORITHM_HPP

#include <bitset>
#include <exception>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace atom::algorithm {
/**
 * @brief Implements the Knuth-Morris-Pratt (KMP) string searching algorithm.
 *
 * This class provides methods to search for occurrences of a pattern within a
 * text using the KMP algorithm, which preprocesses the pattern to achieve
 * efficient string searching.
 */
class KMP {
public:
    /**
     * @brief Constructs a KMP object with the given pattern.
     *
     * @param pattern The pattern to search for in text.
     */
    explicit KMP(std::string_view pattern);

    /**
     * @brief Searches for occurrences of the pattern in the given text.
     *
     * @param text The text to search within.
     * @return std::vector<int> Vector containing positions where the pattern
     * starts in the text.
     */
    [[nodiscard]] auto search(std::string_view text) const -> std::vector<int>;

    /**
     * @brief Sets a new pattern for searching.
     *
     * @param pattern The new pattern to search for.
     */
    void setPattern(std::string_view pattern);

private:
    /**
     * @brief Computes the failure function (partial match table) for the given
     * pattern.
     *
     * This function preprocesses the pattern to determine the length of the
     * longest proper prefix which is also a suffix at each position in the
     * pattern.
     *
     * @param pattern The pattern for which to compute the failure function.
     * @return std::vector<int> The computed failure function (partial match
     * table).
     */
    auto computeFailureFunction(std::string_view pattern) -> std::vector<int>;

    std::string pattern_;  ///< The pattern to search for.
    std::vector<int>
        failure_;  ///< Failure function (partial match table) for the pattern.

    mutable std::shared_mutex mutex_;  ///< Mutex for thread-safe operations
};

/**
 * @brief The BloomFilter class implements a Bloom filter data structure.
 * @tparam N The size of the Bloom filter (number of bits).
 */
template <std::size_t N>
class BloomFilter {
public:
    /**
     * @brief Constructs a new BloomFilter object with the specified number of
     * hash functions.
     * @param num_hash_functions The number of hash functions to use for the
     * Bloom filter.
     */
    explicit BloomFilter(std::size_t num_hash_functions);

    /**
     * @brief Inserts an element into the Bloom filter.
     * @param element The element to insert.
     */
    void insert(std::string_view element);

    /**
     * @brief Checks if an element might be present in the Bloom filter.
     * @param element The element to check.
     * @return True if the element might be present, false otherwise.
     */
    [[nodiscard]] auto contains(std::string_view element) const -> bool;

private:
    std::bitset<N> m_bits_; /**< The bitset representing the Bloom filter. */
    std::size_t
        m_num_hash_functions_; /**< The number of hash functions used. */

    /**
     * @brief Computes the hash value of an element using a specific seed.
     * @param element The element to hash.
     * @param seed The seed value for the hash function.
     * @return The hash value of the element.
     */
    auto hash(std::string_view element, std::size_t seed) const -> std::size_t;
};

/**
 * @brief Implements the Boyer-Moore string searching algorithm.
 *
 * This class provides methods to search for occurrences of a pattern within a
 * text using the Boyer-Moore algorithm, which preprocesses the pattern to
 * achieve efficient string searching.
 */
class BoyerMoore {
public:
    /**
     * @brief Constructs a BoyerMoore object with the given pattern.
     *
     * @param pattern The pattern to search for in text.
     */
    explicit BoyerMoore(std::string_view pattern);

    /**
     * @brief Searches for occurrences of the pattern in the given text.
     *
     * @param text The text to search within.
     * @return std::vector<int> Vector containing positions where the pattern
     * starts in the text.
     */
    auto search(std::string_view text) const -> std::vector<int>;

    /**
     * @brief Sets a new pattern for searching.
     *
     * @param pattern The new pattern to search for.
     */
    void setPattern(std::string_view pattern);

private:
    /**
     * @brief Computes the bad character shift table for the current pattern.
     *
     * This table determines how far to shift the pattern relative to the text
     * based on the last occurrence of a mismatched character.
     */
    void computeBadCharacterShift();

    /**
     * @brief Computes the good suffix shift table for the current pattern.
     *
     * This table helps determine how far to shift the pattern when a mismatch
     * occurs based on the occurrence of a partial match (suffix of the
     * pattern).
     */
    void computeGoodSuffixShift();

    std::string pattern_;  ///< The pattern to search for.
    std::unordered_map<char, int>
        bad_char_shift_;                  ///< Bad character shift table.
    std::vector<int> good_suffix_shift_;  ///< Good suffix shift table.

    mutable std::mutex mutex_;  ///< Mutex for thread-safe operations
};

template <std::size_t N>
BloomFilter<N>::BloomFilter(std::size_t num_hash_functions)
    : m_num_hash_functions_(num_hash_functions) {}

template <std::size_t N>
void BloomFilter<N>::insert(std::string_view element) {
    try {
        for (std::size_t i = 0; i < m_num_hash_functions_; ++i) {
            std::size_t hashValue = hash(element, i);
            m_bits_.set(hashValue % N);
        }
    } catch (const std::exception& e) {
        throw;
    }
}

template <std::size_t N>
auto BloomFilter<N>::contains(std::string_view element) const -> bool {
    try {
        for (std::size_t i = 0; i < m_num_hash_functions_; ++i) {
            std::size_t hashValue = hash(element, i);
            if (!m_bits_.test(hashValue % N)) {
                return false;
            }
        }
        return true;
    } catch (const std::exception& e) {
        throw;
    }
}

template <std::size_t N>
auto BloomFilter<N>::hash(std::string_view element,
                          std::size_t seed) const -> std::size_t {
    std::size_t hashValue = seed;
    for (char c : element) {
        hashValue = hashValue * 31 + static_cast<std::size_t>(c);
    }
    return hashValue;
}

}  // namespace atom::algorithm

#endif