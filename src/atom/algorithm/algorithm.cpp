#include "algorithm.hpp"

#include <random>

namespace atom::algorithm {
KMP::KMP(std::string_view pattern) : pattern_(pattern) {
    failure_ = computeFailureFunction(pattern_);
}

std::vector<int> KMP::search(std::string_view text) {
    std::vector<int> occurrences;
    auto n = static_cast<int>(text.length());
    auto m = static_cast<int>(pattern_.length());
    int i = 0;
    int j = 0;
    while (i < n) {
        if (text[i] == pattern_[j]) {
            ++i;
            ++j;
            if (j == m) {
                occurrences.push_back(i - m);
                j = failure_[j - 1];
            }
        } else if (j > 0) {
            j = failure_[j - 1];
        } else {
            ++i;
        }
    }
    return occurrences;
}

void KMP::setPattern(std::string_view pattern) {
    pattern_ = pattern;
    failure_ = computeFailureFunction(pattern_);
}

auto KMP::computeFailureFunction(std::string_view pattern) -> std::vector<int> {
    auto m = static_cast<int>(pattern.length());
    std::vector<int> failure(m, 0);
    int i = 1;
    int j = 0;
    while (i < m) {
        if (pattern[i] == pattern[j]) {
            failure[i] = j + 1;
            ++i;
            ++j;
        } else if (j > 0) {
            j = failure[j - 1];
        } else {
            failure[i] = 0;
            ++i;
        }
    }
    return failure;
}

MinHash::MinHash(int num_hash_functions)
    : m_num_hash_functions_(num_hash_functions) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;

    for (int i = 0; i < num_hash_functions; ++i) {
        m_coefficients_a_.push_back(dis(gen));
        m_coefficients_b_.push_back(dis(gen));
    }
}

auto MinHash::computeSignature(const std::unordered_set<std::string>& set)
    -> std::vector<unsigned long long> {
    std::vector<unsigned long long> signature(
        m_num_hash_functions_, std::numeric_limits<unsigned long long>::max());

    for (const auto& element : set) {
        for (int i = 0; i < m_num_hash_functions_; ++i) {
            unsigned long long hashValue = hash(element, i);
            signature[i] = std::min(signature[i], hashValue);
        }
    }

    return signature;
}

auto MinHash::estimateSimilarity(
    const std::vector<unsigned long long>& signature1,
    const std::vector<unsigned long long>& signature2) const -> double {
    int numMatches = 0;
    for (int i = 0; i < m_num_hash_functions_; ++i) {
        if (signature1[i] == signature2[i]) {
            ++numMatches;
        }
    }
    return static_cast<double>(numMatches) / m_num_hash_functions_;
}

auto MinHash::hash(const std::string& element,
                   int index) -> unsigned long long {
    unsigned long long hashValue = 0;
    for (char c : element) {
        hashValue +=
            (m_coefficients_a_[index] * static_cast<unsigned long long>(c) +
             m_coefficients_b_[index]);
    }
    return hashValue;
}

BoyerMoore::BoyerMoore(std::string_view pattern) : pattern_(pattern) {
    computeBadCharacterShift();
    computeGoodSuffixShift();
}

auto BoyerMoore::search(std::string_view text) -> std::vector<int> {
    std::vector<int> occurrences;
    auto n = static_cast<int>(text.length());
    auto m = static_cast<int>(pattern_.length());
    int i = 0;
    while (i <= n - m) {
        int j = m - 1;
        while (j >= 0 && pattern_[j] == text[i + j]) {
            --j;
        }
        if (j < 0) {
            occurrences.push_back(i);
            i += good_suffix_shift_[0];
        } else {
            i += std::max(good_suffix_shift_[j + 1],
                          bad_char_shift_[text[i + j]] - m + 1 + j);
        }
    }
    return occurrences;
}

void BoyerMoore::setPattern(std::string_view pattern) {
    pattern_ = pattern;
    computeBadCharacterShift();
    computeGoodSuffixShift();
}

void BoyerMoore::computeBadCharacterShift() {
    bad_char_shift_.clear();
    for (int i = 0; i < static_cast<int>(pattern_.length()) - 1; ++i) {
        bad_char_shift_[pattern_[i]] =
            static_cast<int>(pattern_.length()) - 1 - i;
    }
}

void BoyerMoore::computeGoodSuffixShift() {
    auto m = static_cast<int>(pattern_.length());
    good_suffix_shift_.resize(m, m);
    std::vector<int> suffix(m, 0);
    int j = 0;
    for (int i = m - 1; i >= 0; --i) {
        if (pattern_.substr(i) == pattern_.substr(m - j - 1, j + 1)) {
            suffix[i] = j + 1;
        }
        if (i > 0) {
            good_suffix_shift_[m - suffix[i]] = m - i;
        }
    }
    for (int i = 0; i < m - 1; ++i) {
        good_suffix_shift_[m - suffix[i]] = m - i;
    }
}

}  // namespace atom::algorithm
