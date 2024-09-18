#ifndef STAR_SEARCH_SEARCH_HPP
#define STAR_SEARCH_SEARCH_HPP

#include <algorithm>
#include <concepts>
#include <list>
#include <mutex>
#include <optional>
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
        if (auto iter = cacheMap_.find(key); iter != cacheMap_.end()) {
            cacheList_.splice(cacheList_.begin(), cacheList_, iter->second);
            return iter->second->second;
        }
        return std::nullopt;
    }

    void put(const Key& key, const Value& value) {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        if (auto iter = cacheMap_.find(key); iter != cacheMap_.end()) {
            cacheList_.splice(cacheList_.begin(), cacheList_, iter->second);
            iter->second->second = value;
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
    struct alignas(128) TrieNode {
        std::unordered_map<char, TrieNode*> children;
        bool isEndOfWord = false;
    };
    TrieNode* root_;

public:
    Trie() : root_(new TrieNode()) {}

    ~Trie() { clear(root_); }

    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    Trie(Trie&&) noexcept = default;
    Trie& operator=(Trie&&) noexcept = default;

    void insert(const std::string& word) {
        TrieNode* node = root_;
        for (char ch : word) {
            if (!node->children.contains(ch)) {
                node->children[ch] = new TrieNode();
            }
            node = node->children[ch];
        }
        node->isEndOfWord = true;
    }

    [[nodiscard]] auto autoComplete(const std::string& prefix) const
        -> std::vector<std::string> {
        std::vector<std::string> suggestions;
        TrieNode* node = root_;
        for (char ch : prefix) {
            if (!node->children.contains(ch)) {
                return suggestions;  // 前缀不存在
            }
            node = node->children[ch];
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
        for (const auto& [ch, childNode] : node->children) {
            dfs(childNode, prefix + ch, suggestions);
        }
    }

    void clear(TrieNode* node) {
        for (auto& [_, child] : node->children) {
            clear(child);
        }
        delete node;
    }
};

struct alignas(64) StarObject {
private:
    std::string name_;
    std::vector<std::string> aliases_;
    int clickCount_;

public:
    StarObject(std::string name, std::initializer_list<std::string> aliases,
               int clickCount = 0)
        : name_(std::move(name)), aliases_(aliases), clickCount_(clickCount) {}

    [[nodiscard]] auto getName() const -> const std::string& { return name_; }
    [[nodiscard]] auto getAliases() const -> const std::vector<std::string>& {
        return aliases_;
    }
    [[nodiscard]] auto getClickCount() const -> int { return clickCount_; }

    void setName(const std::string& name) { name_ = name; }
    void setAliases(const std::vector<std::string>& aliases) {
        aliases_ = aliases;
    }
    void setClickCount(int clickCount) { clickCount_ = clickCount; }
};

class SearchEngine {
private:
    std::unordered_map<std::string, StarObject> starObjectIndex_;
    Trie trie_;
    mutable LRUCache<std::string, std::vector<StarObject>> queryCache_;
    mutable std::shared_mutex indexMutex_;
    static constexpr int CACHE_CAPACITY = 10;

public:
    SearchEngine() : queryCache_(CACHE_CAPACITY) {}

    void addStarObject(const StarObject& starObject) {
        std::unique_lock lock(indexMutex_);
        starObjectIndex_.emplace(starObject.getName(), starObject);

        trie_.insert(starObject.getName());
        for (const auto& alias : starObject.getAliases()) {
            trie_.insert(alias);
        }
    }

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
                    starObject.getAliases(),
                    [&query](const auto& alias) { return alias == query; })) {
                results.push_back(starObject);
            }
        };

        std::ranges::for_each(starObjectIndex_, searchFn);

        queryCache_.put(query, results);
        return results;
    }

    auto fuzzySearchStarObject(const std::string& query,
                               int tolerance) const -> std::vector<StarObject> {
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

    auto autoCompleteStarObject(const std::string& prefix) const
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

    static auto getRankedResults(std::vector<StarObject>& results)
        -> std::vector<StarObject> {
        std::ranges::sort(results, std::ranges::greater{},
                          &StarObject::getClickCount);
        return results;
    }

private:
    static auto levenshteinDistance(const std::string& str1,
                                    const std::string& str2) -> int {
        const auto size1 = str1.size();
        const auto size2 = str2.size();
        std::vector<std::vector<int>> distanceMatrix(
            size1 + 1, std::vector<int>(size2 + 1));

        for (size_t i = 0; i <= size1; i++) {
            distanceMatrix[i][0] = static_cast<int>(i);
        }
        for (size_t j = 0; j <= size2; j++) {
            distanceMatrix[0][j] = static_cast<int>(j);
        }

        for (size_t i = 1; i <= size1; i++) {
            for (size_t j = 1; j <= size2; j++) {
                const int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
                distanceMatrix[i][j] = std::min(
                    {distanceMatrix[i - 1][j] + 1, distanceMatrix[i][j - 1] + 1,
                     distanceMatrix[i - 1][j - 1] + cost});
            }
        }
        return distanceMatrix[size1][size2];
    }
};

}  // namespace lithium::target

#endif