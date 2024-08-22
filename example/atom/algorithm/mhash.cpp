#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "atom/algorithm/mhash.hpp"

int main() {
    // Create some example sets for which we want to compute MinHash signatures.
    std::set<std::string> set1 = {"apple", "banana", "cherry"};
    std::set<std::string> set2 = {"banana", "cherry", "date", "fig"};

    // Specify the number of hash functions to use
    size_t numHashes = 100;

    // Create MinHash instance
    atom::algorithm::MinHash minHash(numHashes);

    // Compute MinHash signatures for both sets
    auto signature1 = minHash.computeSignature(set1);
    auto signature2 = minHash.computeSignature(set2);

    // Output the MinHash signatures
    std::cout << "MinHash Signature for Set 1: ";
    for (const auto& hash : signature1) {
        std::cout << hash << " ";
    }
    std::cout << std::endl;

    std::cout << "MinHash Signature for Set 2: ";
    for (const auto& hash : signature2) {
        std::cout << hash << " ";
    }
    std::cout << std::endl;

    // Compute the Jaccard index between the two sets
    double jaccardIdx =
        atom::algorithm::MinHash::jaccardIndex(signature1, signature2);
    std::cout << "Estimated Jaccard Index between Set 1 and Set 2: "
              << jaccardIdx << std::endl;

    return 0;
}
