#ifndef STAR_SEARCH_SEARCH_HPP
#define STAR_SEARCH_SEARCH_HPP

#include <concepts>
#include <list>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/macro.hpp"

namespace lithium::target {

/**
 * @brief A Least Recently Used (LRU) cache implementation.
 *
 * This class provides a thread-safe LRU cache that stores key-value pairs.
 * When the cache reaches its capacity, the least recently used item is evicted.
 *
 * @tparam Key The type of keys used to access values in the cache.
 * @tparam Value The type of values stored in the cache.
 * @requires Key must be equality comparable.
 */
template <typename Key, typename Value>
    requires std::equality_comparable<Key>
class LRUCache {
private:
    int capacity_;  ///< The maximum number of elements the cache can hold.
    std::list<std::pair<Key, Value>>
        cacheList_;  ///< List to maintain the order of items.
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator>
        cacheMap_;  ///< Map to store iterators pointing to elements in the
                    ///< list.
    std::mutex cacheMutex_;  ///< Mutex for thread-safe access.

public:
    /**
     * @brief Constructs an LRUCache with the specified capacity.
     *
     * @param capacity The maximum number of elements the cache can hold.
     */
    explicit LRUCache(int capacity) : capacity_(capacity) {}

    /**
     * @brief Retrieves a value from the cache.
     *
     * If the key exists in the cache, the corresponding value is returned and
     * the key is moved to the front of the list to mark it as recently used.
     * If the key is not found, an empty optional is returned.
     *
     * @param key The key to search for.
     * @return std::optional<Value> The value associated with the key, or
     * std::nullopt if not found.
     */
    auto get(const Key& key) -> std::optional<Value> {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        if (auto iter = cacheMap_.find(key); iter != cacheMap_.end()) {
            cacheList_.splice(cacheList_.begin(), cacheList_, iter->second);
            return iter->second->second;
        }
        return std::nullopt;
    }

    /**
     * @brief Inserts a key-value pair into the cache.
     *
     * If the key already exists, its value is updated and the key is moved to
     * the front of the list. If the cache is full, the least recently used
     * item is removed before inserting the new key-value pair.
     *
     * @param key The key to insert.
     * @param value The value to insert.
     */
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

/**
 * @brief A Trie (prefix tree) for storing and searching strings.
 *
 * The Trie is used for efficient storage and retrieval of strings, particularly
 * useful for tasks like auto-completion.
 */
class Trie {
    struct alignas(128) TrieNode {
        std::unordered_map<char, TrieNode*> children;  ///< Children nodes.
        bool isEndOfWord = false;  ///< Flag indicating the end of a word.
    };

public:
    /**
     * @brief Constructs an empty Trie.
     */
    Trie();

    /**
     * @brief Destroys the Trie and frees allocated memory.
     */
    ~Trie();

    // Deleted copy constructor and copy assignment operator
    Trie(const Trie&) = delete;
    Trie& operator=(const Trie&) = delete;

    // Defaulted move constructor and move assignment operator
    Trie(Trie&&) noexcept = default;
    Trie& operator=(Trie&&) noexcept = default;

    /**
     * @brief Inserts a word into the Trie.
     *
     * @param word The word to insert.
     */
    void insert(const std::string& word);

    /**
     * @brief Provides auto-complete suggestions based on a given prefix.
     *
     * @param prefix The prefix to search for.
     * @return std::vector<std::string> A vector of auto-complete suggestions.
     */
    [[nodiscard]] auto autoComplete(const std::string& prefix) const
        -> std::vector<std::string>;

private:
    /**
     * @brief Depth-first search to collect all words in the Trie starting with
     * a given prefix.
     *
     * @param node The current TrieNode being visited.
     * @param prefix The current prefix being formed.
     * @param suggestions A vector to collect the suggestions.
     */
    void dfs(TrieNode* node, const std::string& prefix,
             std::vector<std::string>& suggestions) const;

    /**
     * @brief Recursively frees the memory allocated for Trie nodes.
     *
     * @param node The current TrieNode being freed.
     */
    void clear(TrieNode* node);

    TrieNode* root_;  ///< The root node of the Trie.
};

/**
 * @brief Represents a star object with a name, aliases, and a click count.
 *
 * This structure is used to store information about celestial objects,
 * including their name, possible aliases, and a click count which can be used
 * to adjust search result rankings.
 */
struct alignas(64) StarObject {
private:
    std::string name_;  ///< The name of the star object.
    std::vector<std::string>
        aliases_;     ///< A list of aliases for the star object.
    int clickCount_;  ///< The number of times this object has been clicked,
                      ///< used for ranking.

public:
    /**
     * @brief Constructs a StarObject with a name, aliases, and an optional
     * click count.
     *
     * @param name The name of the star object.
     * @param aliases A list of aliases for the star object.
     * @param clickCount The initial click count (default is 0).
     */
    StarObject(std::string name, std::initializer_list<std::string> aliases,
               int clickCount = 0)
        : name_(std::move(name)), aliases_(aliases), clickCount_(clickCount) {}

    // Accessor methods
    [[nodiscard]] auto getName() const -> const std::string& { return name_; }
    [[nodiscard]] auto getAliases() const -> const std::vector<std::string>& {
        return aliases_;
    }
    [[nodiscard]] auto getClickCount() const -> int { return clickCount_; }

    // Mutator methods
    void setName(const std::string& name) { name_ = name; }
    void setAliases(const std::vector<std::string>& aliases) {
        aliases_ = aliases;
    }
    void setClickCount(int clickCount) { clickCount_ = clickCount; }
};

/**
 * @brief A search engine for star objects.
 *
 * This class provides functionality to add star objects, search for them by
 * name or alias, perform fuzzy searches, provide auto-complete suggestions, and
 * rank search results by click count.
 */
class SearchEngine {
private:
    std::unordered_map<std::string, StarObject>
        starObjectIndex_;  ///< Index of star objects by name.
    Trie trie_;            ///< Trie used for auto-completion.
    mutable LRUCache<std::string, std::vector<StarObject>>
        queryCache_;  ///< LRU cache to store recent search results.
    mutable std::shared_mutex
        indexMutex_;  ///< Mutex to protect the star object index.

public:
    /**
     * @brief Constructs an empty SearchEngine.
     */
    SearchEngine();

    /**
     * @brief Adds a StarObject to the search engine's index.
     *
     * @param starObject The star object to add.
     */
    void addStarObject(const StarObject& starObject);

    /**
     * @brief Searches for star objects by name or alias.
     *
     * The search is case-sensitive and returns all star objects whose name or
     * aliases match the query.
     *
     * @param query The name or alias to search for.
     * @return std::vector<StarObject> A vector of matching star objects.
     */
    auto searchStarObject(const std::string& query) const
        -> std::vector<StarObject>;

    /**
     * @brief Performs a fuzzy search for star objects.
     *
     * The fuzzy search allows for a specified tolerance in the difference
     * between the query and star object names/aliases using the Levenshtein
     * distance.
     *
     * @param query The name or alias to search for.
     * @param tolerance The maximum allowed Levenshtein distance for a match.
     * @return std::vector<StarObject> A vector of matching star objects.
     */
    auto fuzzySearchStarObject(const std::string& query,
                               int tolerance) const -> std::vector<StarObject>;

    /**
     * @brief Provides auto-complete suggestions for star objects based on a
     * prefix.
     *
     * @param prefix The prefix to search for.
     * @return std::vector<std::string> A vector of auto-complete suggestions.
     */
    auto autoCompleteStarObject(const std::string& prefix) const
        -> std::vector<std::string>;

    /**
     * @brief Sorts star objects by click count in descending order.
     *
     * This method is used to rank search results based on their popularity.
     *
     * @param results The vector of star objects to rank.
     * @return std::vector<StarObject> A vector of ranked star objects.
     */
    static auto getRankedResults(std::vector<StarObject>& results)
        -> std::vector<StarObject>;

private:
    /**
     * @brief Calculates the Levenshtein distance between two strings.
     *
     * The Levenshtein distance is a measure of the similarity between two
     * strings, defined as the minimum number of single-character edits required
     * to change one word into the other.
     *
     * @param str1 The first string.
     * @param str2 The second string.
     * @return int The Levenshtein distance between the two strings.
     */
    static auto levenshteinDistance(const std::string& str1,
                                    const std::string& str2) -> int;
};

}  // namespace lithium::target

#endif
