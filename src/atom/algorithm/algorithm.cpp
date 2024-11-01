#include "algorithm.hpp"
#ifdef USE_OPENMP
#include <omp.h>
#endif

namespace atom::algorithm {

KMP::KMP(std::string_view pattern) { setPattern(pattern); }

auto KMP::search(std::string_view text) const -> std::vector<int> {
    std::vector<int> occurrences;
    auto n = static_cast<int>(text.length());
    auto m = static_cast<int>(pattern_.length());
    if (m == 0) {
        return occurrences;
    }

#ifdef USE_SIMD
    int i = 0;
    int j = 0;
    while (i <= n - m) {
        __m256i text_chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&text[i]));
        __m256i pattern_chunk = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&pattern_[0]));
        __m256i result = _mm256_cmpeq_epi8(text_chunk, pattern_chunk);
        int mask = _mm256_movemask_epi8(result);
        if (mask == 0xFFFFFFFF) {
            occurrences.push_back(i);
            i += m;
        } else {
            ++i;
        }
    }
#elif defined(USE_OPENMP)
    std::vector<int> local_occurrences[omp_get_max_threads()];
#pragma omp parallel
    {
        int i = omp_get_thread_num();
        int j = 0;
        while (i < n) {
            if (text[i] == pattern_[j]) {
                ++i;
                ++j;
                if (j == m) {
                    local_occurrences[omp_get_thread_num()].push_back(i - m);
                    j = failure_[j - 1];
                }
            } else if (j > 0) {
                j = failure_[j - 1];
            } else {
                ++i;
            }
        }
    }
    for (int t = 0; t < omp_get_max_threads(); ++t) {
        occurrences.insert(occurrences.end(), local_occurrences[t].begin(),
                           local_occurrences[t].end());
    }
#else
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
#endif
    return occurrences;
}

void KMP::setPattern(std::string_view pattern) {
    pattern_ = pattern;
    failure_ = computeFailureFunction(pattern_);
}

auto KMP::computeFailureFunction(std::string_view pattern) -> std::vector<int> {
    auto m = static_cast<int>(pattern.length());
    std::vector<int> failure(m, 0);
    int j = 0;
    for (int i = 1; i < m; ++i) {
        if (pattern[i] == pattern[j]) {
            failure[i] = ++j;
        } else if (j > 0) {
            j = failure[j - 1];
            --i;  // stay in the same position
        }
    }
    return failure;
}

BoyerMoore::BoyerMoore(std::string_view pattern) { setPattern(pattern); }

auto BoyerMoore::search(std::string_view text) const -> std::vector<int> {
    std::vector<int> occurrences;
    auto n = static_cast<int>(text.length());
    auto m = static_cast<int>(pattern_.length());
    if (m == 0) {
        return occurrences;
    }

#ifdef USE_OPENMP
    std::vector<int> local_occurrences[omp_get_max_threads()];
#pragma omp parallel
    {
        int i = omp_get_thread_num();
        while (i <= n - m) {
            int j = m - 1;
            while (j >= 0 && pattern_[j] == text[i + j]) {
                --j;
            }
            if (j < 0) {
                local_occurrences[omp_get_thread_num()].push_back(i);
                i += good_suffix_shift_[0];
            } else {
                int badCharShift =
                    bad_char_shift_.find(text[i + j]) != bad_char_shift_.end()
                        ? bad_char_shift_.at(text[i + j])
                        : m;
                i += std::max(good_suffix_shift_[j + 1],
                              badCharShift - m + 1 + j);
            }
        }
    }
    for (int t = 0; t < omp_get_max_threads(); ++t) {
        occurrences.insert(occurrences.end(), local_occurrences[t].begin(),
                           local_occurrences[t].end());
    }
#else
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
            int badCharShift =
                bad_char_shift_.find(text[i + j]) != bad_char_shift_.end()
                    ? bad_char_shift_.at(text[i + j])
                    : m;
            i += std::max(good_suffix_shift_[j + 1], badCharShift - m + 1 + j);
        }
    }
#endif
    return occurrences;
}

void BoyerMoore::setPattern(std::string_view pattern) {
    pattern_ = std::string(pattern);
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
    good_suffix_shift_.resize(m + 1, m);
    std::vector<int> suffix(m + 1, 0);
    suffix[m] = m + 1;

    for (int i = m; i > 0; --i) {
        int j = i - 1;
        while (j >= 0 && pattern_[j] != pattern_[m - 1 - (i - 1 - j)]) {
            --j;
        }
        suffix[i - 1] = j + 1;
    }

    for (int i = 0; i <= m; ++i) {
        good_suffix_shift_[i] = m;
    }

    for (int i = m; i > 0; --i) {
        if (suffix[i - 1] == i) {
            for (int j = 0; j < m - i; ++j) {
                if (good_suffix_shift_[j] == m) {
                    good_suffix_shift_[j] = m - i;
                }
            }
        }
    }

    for (int i = 0; i < m - 1; ++i) {
        good_suffix_shift_[m - suffix[i]] = m - 1 - i;
    }
}
}  // namespace atom::algorithm
