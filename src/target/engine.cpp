#include "engine.hpp"

#include <algorithm>
#include <shared_mutex>

#include "atom/log/loguru.hpp"
#include "atom/search/lru.hpp"

namespace lithium::target {

constexpr int CACHE_CAPACITY = 100;

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

class SearchEngine::Impl {
public:
    Impl() : queryCache_(CACHE_CAPACITY) {
        LOG_F(INFO, "SearchEngine initialized with cache capacity {}",
              CACHE_CAPACITY);
    }

    ~Impl() { LOG_F(INFO, "SearchEngine destroyed."); }

    void addStarObject(const StarObject& starObject) {
        std::unique_lock lock(indexMutex_);
        try {
            starObjectIndex_.emplace(starObject.getName(), starObject);
            trie_.insert(starObject.getName());
            for (const auto& alias : starObject.getAliases()) {
                trie_.insert(alias);
            }
            LOG_F(INFO, "Added StarObject: {}", starObject.getName());
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in addStarObject: {}", e.what());
        }
    }

    std::vector<StarObject> searchStarObject(const std::string& query) const {
        std::shared_lock lock(indexMutex_);
        try {
            if (auto cached = queryCache_.get(query)) {
                LOG_F(INFO, "Cache hit for query: {}", query);
                return *cached;
            }

            std::vector<StarObject> results;
            for (const auto& [name, starObject] : starObjectIndex_) {
                if (name == query ||
                    std::any_of(starObject.getAliases().begin(),
                                starObject.getAliases().end(),
                                [&query](const std::string& alias) {
                                    return alias == query;
                                })) {
                    results.push_back(starObject);
                }
            }

            queryCache_.put(query, results);
            LOG_F(INFO, "Search completed for query: {}", query);
            return results;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in searchStarObject: {}", e.what());
            return {};
        }
    }

    std::vector<StarObject> fuzzySearchStarObject(const std::string& query,
                                                  int tolerance) const {
        std::shared_lock lock(indexMutex_);
        std::vector<StarObject> results;
        try {
            for (const auto& [name, starObject] : starObjectIndex_) {
                if (levenshteinDistance(query, name) <= tolerance ||
                    std::any_of(starObject.getAliases().begin(),
                                starObject.getAliases().end(),
                                [&query, tolerance](const std::string& alias) {
                                    return levenshteinDistance(query, alias) <=
                                           tolerance;
                                })) {
                    results.push_back(starObject);
                }
            }
            LOG_F(INFO,
                  "Fuzzy search completed for query: {} with tolerance: {}",
                  query, tolerance);
            return results;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in fuzzySearchStarObject: {}", e.what());
            return {};
        }
    }

    std::vector<std::string> autoCompleteStarObject(
        const std::string& prefix) const {
        try {
            auto suggestions = trie_.autoComplete(prefix);
            std::vector<std::string> filteredSuggestions;

            for (const auto& suggestion : suggestions) {
                if (starObjectIndex_.find(suggestion) !=
                    starObjectIndex_.end()) {
                    filteredSuggestions.push_back(suggestion);
                }
            }

            LOG_F(INFO, "Auto-complete completed for prefix: {}", prefix);
            return filteredSuggestions;
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in autoCompleteStarObject: {}", e.what());
            return {};
        }
    }

    static std::vector<StarObject> getRankedResultsStatic(
        std::vector<StarObject>& results) {
        std::sort(results.begin(), results.end(),
                  [](const StarObject& a, const StarObject& b) {
                      return a.getClickCount() > b.getClickCount();
                  });
        LOG_F(INFO, "Results ranked by click count.");
        return results;
    }

    static int levenshteinDistance(const std::string& str1,
                                   const std::string& str2) {
        const size_t len1 = str1.size();
        const size_t len2 = str2.size();
        std::vector<std::vector<int>> distanceMatrix(
            len1 + 1, std::vector<int>(len2 + 1));

        for (size_t i = 0; i <= len1; ++i) {
            distanceMatrix[i][0] = static_cast<int>(i);
        }
        for (size_t j = 0; j <= len2; ++j) {
            distanceMatrix[0][j] = static_cast<int>(j);
        }

        for (size_t i = 1; i <= len1; ++i) {
            for (size_t j = 1; j <= len2; ++j) {
                int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
                distanceMatrix[i][j] = std::min(
                    {distanceMatrix[i - 1][j] + 1, distanceMatrix[i][j - 1] + 1,
                     distanceMatrix[i - 1][j - 1] + cost});
            }
        }
        return distanceMatrix[len1][len2];
    }

private:
    std::unordered_map<std::string, StarObject> starObjectIndex_;
    Trie trie_;
    mutable atom::search::ThreadSafeLRUCache<std::string,
                                             std::vector<StarObject>>
        queryCache_;
    mutable std::shared_mutex indexMutex_;
};

SearchEngine::SearchEngine() : pImpl_(std::make_unique<Impl>()) {}

SearchEngine::~SearchEngine() = default;

void SearchEngine::addStarObject(const StarObject& starObject) {
    pImpl_->addStarObject(starObject);
}

std::vector<StarObject> SearchEngine::searchStarObject(
    const std::string& query) const {
    return pImpl_->searchStarObject(query);
}

std::vector<StarObject> SearchEngine::fuzzySearchStarObject(
    const std::string& query, int tolerance) const {
    return pImpl_->fuzzySearchStarObject(query, tolerance);
}

std::vector<std::string> SearchEngine::autoCompleteStarObject(
    const std::string& prefix) const {
    return pImpl_->autoCompleteStarObject(prefix);
}

std::vector<StarObject> SearchEngine::getRankedResults(
    std::vector<StarObject>& results) {
    return Impl::getRankedResultsStatic(results);
}

}  // namespace lithium::target