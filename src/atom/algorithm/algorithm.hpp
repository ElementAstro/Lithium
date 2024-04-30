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
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace atom::algorithm {
/**
 * @brief The KMP class implements the Knuth-Morris-Pratt string searching
 * algorithm.
 */
class KMP {
public:
    /**
     * @brief Constructs a new KMP object with the specified pattern.
     * @param pattern The pattern to search for.
     */
    explicit KMP(std::string_view pattern);

    /**
     * @brief Searches for occurrences of the pattern in the given text.
     * @param text The text to search in.
     * @return A vector containing the starting positions of all occurrences of
     * the pattern in the text.
     */
    std::vector<int> Search(std::string_view text);

    /**
     * @brief Sets a new pattern to search for.
     * @param pattern The new pattern to set.
     */
    void SetPattern(std::string_view pattern);

private:
    /**
     * @brief Computes the failure function for the specified pattern.
     * @param pattern The pattern for which to compute the failure function.
     * @return A vector containing the failure function values.
     */
    std::vector<int> ComputeFailureFunction(std::string_view pattern);

    std::string pattern_; /**< The pattern to search for. */
    std::vector<int>
        failure_; /**< The failure function for the current pattern. */
};

/**
 * @brief The MinHash class implements the MinHash algorithm for estimating the
 * Jaccard similarity between sets.
 */
class MinHash {
public:
    /**
     * @brief Constructs a new MinHash object with the specified number of hash
     * functions.
     * @param num_hash_functions The number of hash functions to use for
     * computing MinHash signatures.
     */
    explicit MinHash(int num_hash_functions);

    /**
     * @brief Computes the MinHash signature for the given set.
     * @param set The set for which to compute the MinHash signature.
     * @return A vector containing the MinHash signature of the set.
     */
    std::vector<unsigned long long> compute_signature(
        const std::unordered_set<std::string>& set);

    /**
     * @brief Estimates the Jaccard similarity between two sets using their
     * MinHash signatures.
     * @param signature1 The MinHash signature of the first set.
     * @param signature2 The MinHash signature of the second set.
     * @return The estimated Jaccard similarity between the two sets.
     */
    double estimate_similarity(
        const std::vector<unsigned long long>& signature1,
        const std::vector<unsigned long long>& signature2) const;

private:
    /**
     * @brief Computes the hash value of an element using a specific hash
     * function.
     * @param element The element to hash.
     * @param index The index of the hash function to use.
     * @return The hash value of the element.
     */
    unsigned long long hash(const std::string& element, int index);

    int m_num_hash_functions; /**< The number of hash functions used for
                                 MinHash. */
    std::vector<unsigned long long>
        m_coefficients_a; /**< Coefficients 'a' for hash functions. */
    std::vector<unsigned long long>
        m_coefficients_b; /**< Coefficients 'b' for hash functions. */
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
    bool contains(std::string_view element) const;

private:
    std::bitset<N> m_bits; /**< The bitset representing the Bloom filter. */
    std::size_t m_num_hash_functions; /**< The number of hash functions used. */

    /**
     * @brief Computes the hash value of an element using a specific seed.
     * @param element The element to hash.
     * @param seed The seed value for the hash function.
     * @return The hash value of the element.
     */
    std::size_t hash(std::string_view element, std::size_t seed) const;
};

/**
 * @brief The BoyerMoore class implements the Boyer-Moore algorithm for string
 * searching.
 */
class BoyerMoore {
public:
    /**
     * @brief Constructs a new BoyerMoore object with the specified pattern.
     * @param pattern The pattern to search for.
     */
    explicit BoyerMoore(std::string_view pattern);

    /**
     * @brief Searches for occurrences of the pattern in the given text.
     * @param text The text in which to search for the pattern.
     * @return A vector containing the starting positions of all occurrences of
     * the pattern in the text.
     */
    std::vector<int> Search(std::string_view text);

    /**
     * @brief Sets a new pattern for the BoyerMoore object.
     * @param pattern The new pattern to set.
     */
    void SetPattern(std::string_view pattern);

private:
    /**
     * @brief Computes the bad character shift table for the pattern.
     */
    void ComputeBadCharacterShift();

    /**
     * @brief Computes the good suffix shift table for the pattern.
     */
    void ComputeGoodSuffixShift();

    std::string pattern_; /**< The pattern to search for. */
    std::unordered_map<char, int>
        bad_char_shift_;                 /**< The bad character shift table. */
    std::vector<int> good_suffix_shift_; /**< The good suffix shift table. */
};
}  // namespace atom::algorithm

#endif
