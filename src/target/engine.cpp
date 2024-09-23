#include "engine.hpp"
#include <algorithm>

namespace lithium::target {

constexpr int CACHE_CAPACITY = 100;  // 定义 CACHE_CAPACITY

Trie::Trie() : root_(new TrieNode()) {}

Trie::~Trie() { clear(root_); }

void Trie::insert(const std::string& word) {
    TrieNode* node = root_;
    for (char character : word) {
        if (!node->children.contains(character)) {
            node->children[character] = new TrieNode();
        }
        node = node->children[character];
    }
    node->isEndOfWord = true;
}

auto Trie::autoComplete(const std::string& prefix) const
    -> std::vector<std::string> {
    std::vector<std::string> suggestions;
    TrieNode* node = root_;
    for (char character : prefix) {
        if (!node->children.contains(character)) {
            return suggestions;  // 前缀不存在
        }
        node = node->children[character];
    }
    dfs(node, prefix, suggestions);
    return suggestions;
}

void Trie::dfs(TrieNode* node, const std::string& prefix,
               std::vector<std::string>& suggestions) const {
    if (node->isEndOfWord) {
        suggestions.push_back(prefix);
    }
    for (const auto& [character, childNode] : node->children) {
        dfs(childNode, prefix + character, suggestions);
    }
}

void Trie::clear(TrieNode* node) {
    for (auto& [_, child] : node->children) {
        clear(child);
    }
    delete node;
}

SearchEngine::SearchEngine() : queryCache_(CACHE_CAPACITY) {}

void SearchEngine::addStarObject(const StarObject& starObject) {
    std::unique_lock lock(indexMutex_);
    starObjectIndex_.emplace(starObject.getName(), starObject);

    trie_.insert(starObject.getName());
    for (const auto& alias : starObject.getAliases()) {
        trie_.insert(alias);
    }
}

auto SearchEngine::searchStarObject(const std::string& query) const
    -> std::vector<StarObject> {
    std::shared_lock lock(indexMutex_);
    if (auto cached = queryCache_.get(query)) {
        return *cached;
    }

    std::vector<StarObject> results;
    auto searchFn = [&results, &query](const auto& pair) {
        const auto& [name, starObject] = pair;
        if (name == query || std::ranges::any_of(starObject.getAliases(),
                                                 [&query](const auto& alias) {
                                                     return alias == query;
                                                 })) {
            results.push_back(starObject);
        }
    };

    std::ranges::for_each(starObjectIndex_, searchFn);

    queryCache_.put(query, results);
    return results;
}

auto SearchEngine::fuzzySearchStarObject(
    const std::string& query, int tolerance) const -> std::vector<StarObject> {
    std::shared_lock lock(indexMutex_);
    std::vector<StarObject> results;

    auto searchFn = [&](const auto& pair) {
        const auto& [name, starObject] = pair;
        if (levenshteinDistance(query, name) <= tolerance) {
            results.push_back(starObject);
        } else {
            for (const auto& alias : starObject.getAliases()) {
                if (levenshteinDistance(query, alias) <= tolerance) {
                    results.push_back(starObject);
                    break;
                }
            }
        }
    };

    std::ranges::for_each(starObjectIndex_, searchFn);

    return results;
}

auto SearchEngine::autoCompleteStarObject(const std::string& prefix) const
    -> std::vector<std::string> {
    auto suggestions = trie_.autoComplete(prefix);

    std::vector<std::string> filteredSuggestions;

    auto filterFn = [&](const auto& suggestion) {
        for (const auto& [name, starObject] : starObjectIndex_) {
            if (name == suggestion ||
                std::ranges::any_of(starObject.getAliases(),
                                    [&suggestion](const auto& alias) {
                                        return alias == suggestion;
                                    })) {
                filteredSuggestions.push_back(suggestion);
                break;
            }
        }
    };

    std::ranges::for_each(suggestions, filterFn);

    return filteredSuggestions;
}

auto SearchEngine::getRankedResults(std::vector<StarObject>& results)
    -> std::vector<StarObject> {
    std::ranges::sort(results, std::ranges::greater{},
                      &StarObject::getClickCount);
    return results;
}

auto levenshteinDistance(const std::string& str1,
                         const std::string& str2) -> int {
    const auto STR1_SIZE = str1.size();  // 将 size1 改为 str1Size
    const auto STR2_SIZE = str2.size();  // 将 size2 改为 str2Size
    std::vector<std::vector<int>> distanceMatrix(
        STR1_SIZE + 1, std::vector<int>(STR2_SIZE + 1));

    for (size_t i = 0; i <= STR1_SIZE; i++) {
        distanceMatrix[i][0] = static_cast<int>(i);
    }
    for (size_t j = 0; j <= STR2_SIZE; j++) {
        distanceMatrix[0][j] = static_cast<int>(j);
    }

    for (size_t i = 1; i <= STR1_SIZE; i++) {
        for (size_t j = 1; j <= STR2_SIZE; j++) {
            const int EDIT_COST =
                (str1[i - 1] == str2[j - 1]) ? 0 : 1;  // 将 cost 改为 editCost
            distanceMatrix[i][j] = std::min(
                {distanceMatrix[i - 1][j] + 1, distanceMatrix[i][j - 1] + 1,
                 distanceMatrix[i - 1][j - 1] + EDIT_COST});
        }
    }
    return distanceMatrix[STR1_SIZE][STR2_SIZE];
}

}  // namespace lithium::target