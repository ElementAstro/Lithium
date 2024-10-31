#include "valid_string.hpp"

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace atom::utils {
auto isValidBracket(const std::string& str) -> ValidationResult {
    std::stack<BracketInfo> stack;
    std::unordered_map<char, char> brackets = {
        {')', '('}, {']', '['}, {'}', '{'}, {'>', '<'}};

    ValidationResult result;
    result.isValid = true;

    bool singleQuoteOpen = false;
    bool doubleQuoteOpen = false;

    for (std::string::size_type i = 0; i < str.size(); ++i) {
        char current = str[i];

        if (current == '\'' && !doubleQuoteOpen) {
            singleQuoteOpen = !singleQuoteOpen;
            continue;
        }

        if (current == '\"' && !singleQuoteOpen) {
            doubleQuoteOpen = !doubleQuoteOpen;
            continue;
        }

        if (singleQuoteOpen || doubleQuoteOpen) {
            continue;
        }

        if (brackets.find(current) == brackets.end()) {
            // Push opening brackets onto the stack
            stack.push({current, static_cast<int>(i)});
        } else {
            // Check for matching closing brackets
            if (stack.empty() || stack.top().character != brackets[current]) {
                result.invalidBrackets.push_back(
                    {current, static_cast<int>(i)});
                result.errorMessages.push_back(
                    "Error: Closing bracket '" + std::string(1, current) +
                    "' at position " + std::to_string(i) +
                    " has no matching opening bracket.");
                result.isValid = false;
            } else {
                stack.pop();
            }
        }
    }

    // Handle unmatched opening brackets
    while (!stack.empty()) {
        auto top = stack.top();
        result.invalidBrackets.push_back(top);
        result.errorMessages.push_back(
            "Error: Opening bracket '" + std::string(1, top.character) +
            "' at position " + std::to_string(top.position) +
            " needs a closing bracket.");
        stack.pop();
        result.isValid = false;
    }

    // Handle unmatched quotes
    if (singleQuoteOpen) {
        result.errorMessages.push_back("Error: Single quote is not closed.");
        result.isValid = false;
    }

    if (doubleQuoteOpen) {
        result.errorMessages.push_back("Error: Double quote is not closed.");
        result.isValid = false;
    }

    return result;
}

}  // namespace atom::utils
