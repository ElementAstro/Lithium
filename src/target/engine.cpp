#ifndef STAR_SEARCH_SEARCH_HPP
#define STAR_SEARCH_SEARCH_HPP

#include <algorithm>
#include <cmath>
#include <concepts>
#include <list>
#include <mutex>
#include <optional>
#include <ranges>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace lithium::target {

// LRU缓存结构
template <typename Key, typename Value>
    requires std::equality_comparable<Key>
class LRUCache {
private:
    int capacity_;
    std::list<std::pair<Key, Value>> cacheList_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator>
        cacheMap_;
    std::mutex cacheMutex_;

public:
    explicit LRUCache(int capacity) : capacity_(capacity) {}

    auto get(const Key& key) -> std::optional<Value> {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        if (auto it = cacheMap_.find(key); it != cacheMap_.end()) {
            cacheList_.splice(cacheList_.begin(), cacheList_, it->second);
            return it->second->second;
        }
        return std::nullopt;
    }

    void put(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        if (auto it = cacheMap_.find(key); it != cacheMap_.end()) {
            cacheList_.splice(cacheList_.begin(), cacheList_, it->second);
            it->second->second = value;
            return;
        }

        if (static_cast<int>(cacheList_.size()) == capacity_) {
            cacheMap_.erase(cacheList_.back().first);
            cacheList_.pop_back();
        }

        cacheList_.emplace_front(key, value);
        cacheMap_[key] = cacheList_.begin();
    }
};

// Trie树用于自动补全
class Trie {
private:
    struct TrieNode {
        std::unordered_map<char, TrieNode*> children;
        bool isEndOfWord = false;
    };
    TrieNode* root_;

public:
    Trie() : root_(new TrieNode()) {}

    ~Trie() { clear(root_); }

    void insert(const std::string& word) {
        TrieNode* node = root_;
        for (char c : word) {
            if (!node->children.contains(c)) {
                node->children[c] = new TrieNode();
            }
            node = node->children[c];
        }
        node->isEndOfWord = true;
    }

    auto autoComplete(const std::string& prefix) const
        -> std::vector<std::string> {
        std::vector<std::string> suggestions;
        TrieNode* node = root_;
        for (char c : prefix) {
            if (!node->children.contains(c)) {
                return suggestions;  // 前缀不存在
            }
            node = node->children[c];
        }
        dfs(node, prefix, suggestions);
        return suggestions;
    }

private:
    void dfs(TrieNode* node, const std::string& prefix,
             std::vector<std::string>& suggestions) const {
        if (node->isEndOfWord) {
            suggestions.push_back(prefix);
        }
        for (const auto& [c, childNode] : node->children) {
            dfs(childNode, prefix + c, suggestions);
        }
    }

    void clear(TrieNode* node) {
        for (auto& [_, child] : node->children) {
            clear(child);
        }
        delete node;
    }
};

struct StarObject {
    std::string name;
    std::vector<std::string> aliases;
    int clickCount;  // 用于调整权重

    StarObject(std::string name, std::initializer_list<std::string> aliases,
               int clickCount = 0)
        : name(std::move(name)), aliases(aliases), clickCount(clickCount) {}
};

class SearchEngine {
private:
    std::unordered_map<std::string, StarObject> starObjectIndex_;
    Trie trie_;
    mutable LRUCache<std::string, std::vector<StarObject>> queryCache_;
    mutable std::shared_mutex indexMutex_;

public:
    SearchEngine() : queryCache_(10) {}

    // 添加星体对象
    void addStarObject(const StarObject& starObject) {
        std::unique_lock lock(indexMutex_);
        starObjectIndex_.emplace(starObject.name, starObject);

        // 将名称和别名添加到Trie树中，用于自动补全和快速搜索
        trie_.insert(starObject.name);
        for (const auto& alias : starObject.aliases) {
            trie_.insert(alias);
        }
    }

    // 按名称或别名搜索
    auto searchStarObject(const std::string& query) const
        -> std::vector<StarObject> {
        std::shared_lock lock(indexMutex_);
        if (auto cached = queryCache_.get(query)) {
            return *cached;
        }

        std::vector<StarObject> results;
        auto searchFn = [&results, &query](const auto& pair) {
            const auto& [name, starObject] = pair;
            if (name == query ||
                std::ranges::any_of(
                    starObject.aliases,
                    [&query](const auto& alias) { return alias == query; })) {
                results.push_back(starObject);
            }
        };

        std::ranges::for_each(starObjectIndex_, searchFn);

        // 缓存结果
        queryCache_.put(query, results);
        return results;
    }

    // 模糊搜索
    auto fuzzySearchStarObject(const std::string& query,
                               int tolerance) const -> std::vector<StarObject> {
        std::shared_lock lock(indexMutex_);
        std::vector<StarObject> results;

        auto searchFn = [&](const auto& pair) {
            const auto& [name, starObject] = pair;
            if (levenshteinDistance(query, name) <= tolerance) {
                results.push_back(starObject);
            } else {
                for (const auto& alias : starObject.aliases) {
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

    // 自动补全
    auto autoCompleteStarObject(const std::string& prefix) const
        -> std::vector<std::string> {
        auto suggestions = trie_.autoComplete(prefix);

        // 进一步过滤建议，只返回与实际名称或别名相关的内容
        std::vector<std::string> filteredSuggestions;

        auto filterFn = [&](const auto& suggestion) {
            for (const auto& [name, starObject] : starObjectIndex_) {
                if (name == suggestion ||
                    std::ranges::any_of(starObject.aliases,
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

    // 按点击量排序结果
    auto getRankedResults(std::vector<StarObject>& results) const
        -> std::vector<StarObject> {
        std::ranges::sort(results, std::ranges::greater{},
                          &StarObject::clickCount);
        return results;
    }

private:
    static auto levenshteinDistance(const std::string& s1,
                                    const std::string& s2) -> int {
        const auto size1 = s1.size();
        const auto size2 = s2.size();
        std::vector<std::vector<int>> dp(size1 + 1,
                                         std::vector<int>(size2 + 1));

        for (size_t i = 0; i <= size1; i++)
            dp[i][0] = static_cast<int>(i);
        for (size_t j = 0; j <= size2; j++)
            dp[0][j] = static_cast<int>(j);

        for (size_t i = 1; i <= size1; i++) {
            for (size_t j = 1; j <= size2; j++) {
                const int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
                dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1,
                                     dp[i - 1][j - 1] + cost});
            }
        }
        return dp[size1][size2];
    }
};

}  // namespace lithium::target

#endif