/**
 * @file suggestion.hpp
 * @brief Command suggestion engine
 */

#ifndef LITHIUM_DEBUG_SUGGESTION_HPP
#define LITHIUM_DEBUG_SUGGESTION_HPP

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace lithium::debug {
/**
 * @class SuggestionEngine
 * @brief A class that generates suggestions based on a dataset using prefix or
 * substring matching.
 *
 * The SuggestionEngine class provides functionality to suggest relevant items
 * from a dataset based on the input string. It supports both prefix and
 * substring matching and can return a limited number of suggestions.
 */
class SuggestionEngine {
public:
    /**
     * @enum MatchType
     * @brief Specifies the type of matching to be used for suggestions.
     *
     * - Prefix: Match items that start with the input string.
     * - Substring: Match items that contain the input string anywhere within
     * them.
     */
    enum class MatchType {
        Prefix,   /**< Match items that start with the input string. */
        Substring /**< Match items that contain the input string anywhere within
                     them. */
    };

    /**
     * @brief Constructs a SuggestionEngine object.
     *
     * @param dataset The dataset to be used for generating suggestions.
     * @param maxSuggestions The maximum number of suggestions to return.
     * Default is 5.
     */
    explicit SuggestionEngine(const std::vector<std::string>& dataset,
                              int maxSuggestions = 5);

    /**
     * @brief Generates suggestions based on the input string and match type.
     *
     * @param input The input string to be matched against the dataset.
     * @param matchType The type of matching to be used. Default is
     * MatchType::Prefix.
     * @return A vector of suggestions matching the input string.
     */
    auto suggest(std::string_view input,
                 MatchType matchType = MatchType::Prefix)
        -> std::vector<std::string>;

    /**
     * @brief Updates the dataset with new items.
     *
     * @param newItems The new items to be added to the dataset.
     */
    void updateDataset(const std::vector<std::string>& newItems);

private:
    /**
     * @brief Builds an index for faster suggestion generation.
     *
     * This function is used internally for initializing or updating the index
     * based on the dataset.
     */
    void buildIndex();

    /**
     * @brief Checks if an item matches the input string based on the match
     * type.
     *
     * @param input The input string to be matched against the item.
     * @param item The item from the dataset to be checked.
     * @param matchType The type of matching to be used (Prefix or Substring).
     * @return True if the item matches the input string, otherwise false.
     */
    auto matches(const std::string& input, const std::string& item,
                 MatchType matchType) -> bool;

    /**
     * @brief Calculates a score for matching items based on the input string.
     *
     * The score is used to rank the suggestions; a higher score indicates a
     * better match.
     *
     * @param input The input string to be matched.
     * @param item The item from the dataset to be scored.
     * @return An integer representing the score of the match.
     */
    auto calculateScore(const std::string& input,
                        const std::string& item) -> int;

    std::unordered_map<std::string, std::string>
        index_; /**< An index for optimizing suggestions. */
    std::vector<std::string>
        dataset_;        /**< The dataset used for generating suggestions. */
    int maxSuggestions_; /**< The maximum number of suggestions to return. */
    std::mutex mutex_;   /**< Mutex for thread-safe operations. */
};
}  // namespace lithium::debug

#endif