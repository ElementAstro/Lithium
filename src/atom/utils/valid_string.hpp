#ifndef ATOM_UTILS_VALID_STRING_HPP
#define ATOM_UTILS_VALID_STRING_HPP

#include <string>
#include <vector>

#include "atom/macro.hpp"

namespace atom::utils {
struct BracketInfo {
    char character;
    int position;
} ATOM_ALIGNAS(8);

struct ValidationResult {
    bool isValid;
    std::vector<BracketInfo> invalidBrackets;
    std::vector<std::string> errorMessages;
} ATOM_ALIGNAS(64);

auto isValidBracket(const std::string& str) -> ValidationResult;

template <std::size_t N>
struct BracketValidator {
    struct ValidationResult {
    private:
        bool isValid_ = true;
        std::array<int, N> errorPositions_{};
        int errorCount_ = 0;

    public:
        constexpr void addError(int position) {
            if (errorCount_ < static_cast<int>(N)) {
                errorPositions_[errorCount_++] = position;
                isValid_ = false;
            }
        }

        [[nodiscard]] constexpr auto isValid() const -> bool {
            return isValid_;
        }

        [[nodiscard]] constexpr auto getErrorPositions() const -> const std::array<int, N>& {
            return errorPositions_;
        }

        [[nodiscard]] constexpr auto getErrorCount() const -> int {
            return errorCount_;
        }
    };

    constexpr static auto validate(const std::array<char, N>& str) -> ValidationResult {
        ValidationResult result;
        std::array<char, N> stack{};
        int stackSize = 0;
        bool singleQuoteOpen = false;
        bool doubleQuoteOpen = false;

        for (std::size_t i = 0; i < N; ++i) {
            char current = str[i];

            if (current == '\0') {
                break;
            }

            auto isEscaped = [&str, i]() constexpr -> bool {
                int backslashCount = 0;
                std::size_t idx = i;
                while (idx > 0 && str[--idx] == '\\') {
                    ++backslashCount;
                }
                return (backslashCount % 2) == 1;
            }();

            if (current == '\'' && !doubleQuoteOpen && !isEscaped) {
                singleQuoteOpen = !singleQuoteOpen;
                continue;
            }

            if (current == '\"' && !singleQuoteOpen && !isEscaped) {
                doubleQuoteOpen = !doubleQuoteOpen;
                continue;
            }

            if (singleQuoteOpen || doubleQuoteOpen) {
                continue;
            }

            if (current == '(' || current == '{' || current == '[' || current == '<') {
                stack[stackSize++] = current;
            } else if (current == ')' || current == '}' || current == ']' || current == '>') {
                if (stackSize == 0 || !isMatching(stack[stackSize - 1], current)) {
                    result.addError(static_cast<int>(i));
                } else {
                    --stackSize;
                }
            }
        }

        while (stackSize > 0) {
            result.addError(static_cast<int>(N - 1));
            --stackSize;
        }

        if (singleQuoteOpen) {
            result.addError(static_cast<int>(N - 1));
        }

        if (doubleQuoteOpen) {
            result.addError(static_cast<int>(N - 1));
        }

        return result;
    }

private:
    constexpr static auto isMatching(char open, char close) -> bool {
        return (open == '(' && close == ')') || (open == '{' && close == '}') ||
               (open == '[' && close == ']') || (open == '<' && close == '>');
    }
};

template <std::size_t N>
constexpr auto toArray(const char (&str)[N]) -> std::array<char, N> {
    std::array<char, N> arr{};
    for (std::size_t i = 0; i < N; ++i) {
        arr[i] = str[i];
    }
    return arr;
}

template <std::size_t N>
constexpr auto validateBrackets(const char (&str)[N]) -> typename BracketValidator<N>::ValidationResult {
    return BracketValidator<N>::validate(toArray(str));
}

}  // namespace atom::utils

#endif
