#ifndef ATOM_SEARCH_SEARCH_HPP
#define ATOM_SEARCH_SEARCH_HPP

#include <cmath>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace atom::search {

/**
 * @brief Represents a document with an ID, content, tags, and click count.
 */
struct Document {
    std::string id;              ///< The unique identifier of the document.
    std::string content;         ///< The content of the document.
    std::set<std::string> tags;  ///< The tags associated with the document.
    int clickCount;  ///< The click count used to adjust the document's weight.

    /**
     * @brief Constructs a Document object.
     * @param id The unique identifier of the document.
     * @param content The content of the document.
     * @param tags The tags associated with the document.
     */
    explicit Document(std::string id, std::string content,
                      std::initializer_list<std::string> tags);
};

/**
 * @brief A search engine for indexing and searching documents.
 */
class SearchEngine {
private:
    std::unordered_map<std::string, std::vector<Document>>
        tagIndex_;  ///< Index of documents by tags.
    std::unordered_map<std::string, std::unordered_set<std::string>>
        contentIndex_;  ///< Index of documents by content.
    std::unordered_map<std::string, int>
        docFrequency_;   ///< Document frequency for terms.
    int totalDocs_ = 0;  ///< Total number of documents in the search engine.

public:
    /**
     * @brief Adds a document to the search engine.
     * @param doc The document to add.
     */
    void addDocument(const Document& doc);

    /**
     * @brief Adds the content of a document to the content index.
     * @param doc The document whose content to index.
     */
    void addContentToIndex(const Document& doc);

    /**
     * @brief Searches for documents by a specific tag.
     * @param tag The tag to search for.
     * @return A vector of documents that match the tag.
     */
    auto searchByTag(const std::string& tag) -> std::vector<Document>;

    /**
     * @brief Performs a fuzzy search for documents by a tag with a specified
     * tolerance.
     * @param tag The tag to search for.
     * @param tolerance The tolerance for the fuzzy search.
     * @return A vector of documents that match the tag within the tolerance.
     */
    auto fuzzySearchByTag(const std::string& tag,
                          int tolerance) -> std::vector<Document>;

    /**
     * @brief Searches for documents by multiple tags.
     * @param tags The tags to search for.
     * @return A vector of documents that match all the tags.
     */
    auto searchByTags(const std::vector<std::string>& tags)
        -> std::vector<Document>;

    /**
     * @brief Searches for documents by content.
     * @param query The content query to search for.
     * @return A vector of documents that match the content query.
     */
    auto searchByContent(const std::string& query) -> std::vector<Document>;

    /**
     * @brief Performs a boolean search for documents by a query.
     * @param query The boolean query to search for.
     * @return A vector of documents that match the boolean query.
     */
    auto booleanSearch(const std::string& query) -> std::vector<Document>;

    /**
     * @brief Provides autocomplete suggestions for a given prefix.
     * @param prefix The prefix to autocomplete.
     * @return A vector of autocomplete suggestions.
     */
    auto autoComplete(const std::string& prefix) -> std::vector<std::string>;

private:
    /**
     * @brief Computes the Levenshtein distance between two strings.
     * @param s1 The first string.
     * @param s2 The second string.
     * @return The Levenshtein distance between the two strings.
     */
    auto levenshteinDistance(const std::string& s1,
                             const std::string& s2) -> int;

    /**
     * @brief Computes the TF-IDF score for a term in a document.
     * @param doc The document.
     * @param term The term.
     * @return The TF-IDF score for the term in the document.
     */
    auto tfIdf(const Document& doc, const std::string& term) -> double;

    /**
     * @brief Finds a document by its ID.
     * @param id The ID of the document.
     * @return The document with the specified ID.
     */
    auto findDocumentById(const std::string& id) -> Document;

    /**
     * @brief Comparator for ranking documents by their scores.
     */
    struct Compare {
        /**
         * @brief Compares two documents by their scores.
         * @param a The first document and its score.
         * @param b The second document and its score.
         * @return True if the first document's score is less than the second's,
         * false otherwise.
         */
        auto operator()(const std::pair<double, Document>& a,
                        const std::pair<double, Document>& b) const -> bool {
            return a.first < b.first;
        }
    };

    /**
     * @brief Gets the ranked results for a set of document scores.
     * @param scores The scores of the documents.
     * @return A vector of documents ranked by their scores.
     */
    auto getRankedResults(const std::unordered_map<std::string, double>& scores)
        -> std::vector<Document>;
};

}  // namespace atom::search

#endif  // ATOM_SEARCH_SEARCH_HPP
