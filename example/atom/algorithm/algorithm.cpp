#include "atom/algorithm/algorithm.hpp"

#include <iostream>

int main() {
    // Example 1: Using the KMP algorithm
    std::string text = "ababcabcababcabc";
    std::string pattern = "abc";

    // Create a KMP object with the pattern
    atom::algorithm::KMP kmp(pattern);

    // Search for the pattern in the text
    std::vector<int> kmpResults = kmp.search(text);

    std::cout << "KMP search results for pattern \"" << pattern
              << "\" in text \"" << text << "\":" << std::endl;
    for (int position : kmpResults) {
        std::cout << "Pattern found at position: " << position << std::endl;
    }

    // Example 2: Using the Boyer-Moore algorithm
    std::string bmText = "HERE IS A SIMPLE EXAMPLE";
    std::string bmPattern = "EXAMPLE";

    // Create a BoyerMoore object with the pattern
    atom::algorithm::BoyerMoore boyerMoore(bmPattern);

    // Search for the pattern in the text
    std::vector<int> bmResults = boyerMoore.search(bmText);

    std::cout << "Boyer-Moore search results for pattern \"" << bmPattern
              << "\" in text \"" << bmText << "\":" << std::endl;
    for (int position : bmResults) {
        std::cout << "Pattern found at position: " << position << std::endl;
    }

    // Example 3: Using the Bloom Filter
    const std::size_t BLOOM_FILTER_SIZE = 100;
    const std::size_t NUM_HASH_FUNCTIONS = 3;

    // Create a BloomFilter object with specified size and number of hash
    // functions
    atom::algorithm::BloomFilter<BLOOM_FILTER_SIZE> bloomFilter(
        NUM_HASH_FUNCTIONS);

    // Insert elements into the Bloom filter
    bloomFilter.insert("apple");
    bloomFilter.insert("banana");
    bloomFilter.insert("cherry");

    // Check for the presence of elements
    std::string element1 = "apple";
    std::string element2 = "grape";

    std::cout << "Checking presence of \"" << element1
              << "\" in the Bloom filter: "
              << (bloomFilter.contains(element1) ? "Possibly present"
                                                 : "Definitely not present")
              << std::endl;

    std::cout << "Checking presence of \"" << element2
              << "\" in the Bloom filter: "
              << (bloomFilter.contains(element2) ? "Possibly present"
                                                 : "Definitely not present")
              << std::endl;

    return 0;
}
