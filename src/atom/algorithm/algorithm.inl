#ifndef ATOM_ALGORITHM_ALGORITHM_INL
#define ATOM_ALGORITHM_ALGORITHM_INL

#include "algorithm.hpp"

namespace atom::algorithm {
template <std::size_t N>
BloomFilter<N>::BloomFilter(std::size_t num_hash_functions)
    : m_num_hash_functions(num_hash_functions) {}

template <std::size_t N>
void BloomFilter<N>::insert(std::string_view element) {
    for (std::size_t i = 0; i < m_num_hash_functions; ++i) {
        std::size_t hash_value = hash(element, i);
        m_bits.set(hash_value % N);
    }
}

template <std::size_t N>
bool BloomFilter<N>::contains(std::string_view element) const {
    for (std::size_t i = 0; i < m_num_hash_functions; ++i) {
        std::size_t hash_value = hash(element, i);
        if (!m_bits.test(hash_value % N)) {
            return false;
        }
    }
    return true;
}

template <std::size_t N>
std::size_t BloomFilter<N>::hash(std::string_view element, std::size_t seed) const {
    std::size_t hash_value = seed;
    for (char c : element) {
        hash_value = hash_value * 31 + static_cast<std::size_t>(c);
    }
    return hash_value;
}
}  // namespace atom::algorithm

#endif