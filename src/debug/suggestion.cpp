/**
 * @file suggestion.cpp
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 * @date 2024-5-15
 * @brief Command suggestion engine
 */

#include "suggestion.hpp"

#include <algorithm>
#include <cctype>
#include <queue>

namespace lithium::debug {

SuggestionEngine::SuggestionEngine(const std::vector<std::string>& dataset,
                                   int maxSuggestions)
    : dataset_(dataset), maxSuggestions_(maxSuggestions) {
    buildIndex();
}

auto SuggestionEngine::suggest(std::string_view input, MatchType matchType)
    -> std::vector<std::string> {
    std::vector<std::string> suggestions;
    std::string inputLower(input.size(), '\0');
    std::transform(input.begin(), input.end(), inputLower.begin(), ::tolower);

    std::priority_queue<std::pair<int, std::string>> priorityQueue;
    for (const auto& [lowerItem, originalItem] : index_) {
        if (matches(inputLower, lowerItem, matchType)) {
            int score = calculateScore(inputLower, lowerItem);
            priorityQueue.emplace(score, originalItem);
            if (priorityQueue.size() > static_cast<size_t>(maxSuggestions_)) {
                priorityQueue.pop();
            }
        }
    }

    while (!priorityQueue.empty()) {
        suggestions.push_back(priorityQueue.top().second);
        priorityQueue.pop();
    }
    std::reverse(suggestions.begin(), suggestions.end());
    return suggestions;
}

void SuggestionEngine::buildIndex() {
    for (const auto& item : dataset_) {
        std::string itemLower(item.size(), '\0');
        std::transform(item.begin(), item.end(), itemLower.begin(), ::tolower);
        index_.emplace(itemLower, item);
    }
}

auto SuggestionEngine::matches(const std::string& input,
                               const std::string& item,
                               MatchType matchType) -> bool {
    switch (matchType) {
        case MatchType::Prefix:
            return item.starts_with(input);
        case MatchType::Substring:
            return item.find(input) != std::string::npos;
    }
    return false;
}

auto SuggestionEngine::calculateScore(const std::string& input,
                                      const std::string& item) -> int {
    int score = 0;
    size_t inputPos = 0;
    for (char character : item) {
        if (inputPos < input.size() && character == input[inputPos]) {
            score += 2;
            inputPos++;
        } else {
            score -= 1;
        }
    }
    return score;
}

}  // namespace lithium::debug