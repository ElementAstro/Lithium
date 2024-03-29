/*
 * search.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-4

Description: A search engine

**************************************************/

#ifndef ATOM_SEARCH_SEARCH_HPP
#define ATOM_SEARCH_SEARCH_HPP

#include <algorithm>
#include <cmath>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Abstract base class for matching strategies.
 */
class MatchStrategy {
public:
    /**
     * @brief Matches the given query against the index using a specific
     * strategy.
     * @param query The query string to match.
     * @param index The index containing the data to match against.
     * @param threshold The matching threshold (optional).
     * @return A vector of matched strings.
     */
    virtual std::vector<std::string> match(
        const std::string &query,
        const std::unordered_map<size_t, std::vector<std::string>> &index,
        int threshold = 3) = 0;
};

/**
 * @brief Fuzzy matching strategy based on edit distance.
 */
class FuzzyMatch : public MatchStrategy {
public:
    /**
     * @brief Matches the given query against the index using fuzzy matching.
     * @param query The query string to match.
     * @param index The index containing the data to match against.
     * @param threshold The matching threshold.
     * @return A vector of matched strings.
     */
    std::vector<std::string> match(
        const std::string &query,
        const std::unordered_map<size_t, std::vector<std::string>> &index,
        int threshold) override;

private:
    /**
     * @brief Calculates the edit distance between two strings.
     * @param s1 The first string.
     * @param s2 The second string.
     * @return The edit distance between the two strings.
     */
    int editDistance(const std::string &s1, const std::string &s2);
};

/**
 * @brief Regular expression matching strategy.
 */
class RegexMatch : public MatchStrategy {
public:
    /**
     * @brief Matches the given query against the index using regular
     * expressions.
     * @param query The query string to match.
     * @param index The index containing the data to match against.
     * @param threshold The matching threshold (not used in this strategy).
     * @return A vector of matched strings.
     */
    std::vector<std::string> match(
        const std::string &query,
        const std::unordered_map<size_t, std::vector<std::string>> &index,
        int /*threshold*/) override;
};

/**
 * @brief Hamming distance matching strategy.
 */
class HammingMatch : public MatchStrategy {
public:
    /**
     * @brief Constructs a new HammingMatch object with the specified maximum
     * distance.
     * @param maxDistance The maximum Hamming distance allowed for a match.
     */
    HammingMatch(int maxDistance);

    /**
     * @brief Matches the given query against the index using Hamming distance.
     * @param query The query string to match.
     * @param index The index containing the data to match against.
     * @param threshold The matching threshold (not used in this strategy).
     * @return A vector of matched strings.
     */
    std::vector<std::string> match(
        const std::string &query,
        const std::unordered_map<size_t, std::vector<std::string>> &index,
        int /*threshold*/) override;

private:
    int maxDistance_;  ///< The maximum Hamming distance allowed for a match.

    /**
     * @brief Calculates the Hamming distance between two strings.
     * @param s1 The first string.
     * @param s2 The second string.
     * @return The Hamming distance between the two strings.
     */
    int hammingDistance(const std::string &s1, const std::string &s2);
};

/**
 * @brief TF-IDF matching strategy.
 */
class TfIdfMatch : public MatchStrategy {
public:
    /**
     * @brief Constructs a new TfIdfMatch object with the given data.
     * @param data The vector of strings to build the index from.
     */
    TfIdfMatch(const std::vector<std::string> &data);

    /**
     * @brief Matches the given query against the index using TF-IDF.
     * @param query The query string to match.
     * @param index The index containing the data to match against.
     * @param threshold The matching threshold (not used in this strategy).
     * @return A vector of matched strings.
     */
    std::vector<std::string> match(
        const std::string &query,
        const std::unordered_map<size_t, std::vector<std::string>> &index,
        int /*threshold*/) override;

private:
    std::vector<std::unordered_map<std::string, double>>
        termFrequency_;  ///< The term frequency for each document.
    std::unordered_map<std::string, double>
        inverseDocumentFrequency_;  ///< The inverse document frequency.

    /**
     * @brief Builds the index from the given data.
     * @param data The vector of strings to build the index from.
     */
    void buildIndex(const std::vector<std::string> &data);

    /**
     * @brief Builds the inverse document frequency (IDF) map.
     */
    void buildIdf();

    /**
     * @brief Calculates the term frequency (TF) for a single string.
     * @param str The input string.
     * @return The term frequency map for the input string.
     */
    std::unordered_map<std::string, double> calculateTf(const std::string &str);

    /**
     * @brief Calculates the term frequency (TF) for a vector of strings.
     * @param strList The input vector of strings.
     * @return The term frequency map for the input vector of strings.
     */
    std::unordered_map<std::string, double> calculateTf(
        const std::vector<std::string> &strList);

    /**
     * @brief Calculates the TF-IDF score for a map of term frequency (TF).
     * @param tf The term frequency map.
     * @return The TF-IDF map.
     */
    std::unordered_map<std::string, double> calculateTfidf(
        const std::unordered_map<std::string, double> &tf);

    /**
     * @brief Calculates the TF-IDF score for a vector of strings.
     * @param strList The input vector of strings.
     * @return The TF-IDF map.
     */
    std::unordered_map<std::string, double> calculateTfidf(
        const std::vector<std::string> &strList);

    /**
     * @brief Calculates the cosine similarity between two TF-IDF maps.
     * @param tfidf1 The first TF-IDF map.
     * @param tfidf2 The second TF-IDF map.
     * @return The cosine similarity between the two maps.
     */
    double cosineSimilarity(
        const std::unordered_map<std::string, double> &tfidf1,
        const std::unordered_map<std::string, double> &tfidf2);
};

/**
 * @brief Search engine class that uses a specific matching strategy.
 */
class SearchEngine {
public:
    /**
     * @brief Constructs a new SearchEngine object with the given data and
     * matching strategy.
     * @param data The vector of strings to build the index from.
     * @param strategy The matching strategy to use.
     */
    SearchEngine(const std::vector<std::string> &data, MatchStrategy *strategy);

    /**
     * @brief Sets the matching strategy to use.
     * @param strategy The matching strategy to use.
     */
    void setMatchStrategy(MatchStrategy *strategy);

    /**
     * @brief Searches for matches to the given query using the current matching
     * strategy.
     * @param query The query string to search for.
     * @param threshold The matching threshold (optional).
     * @return A vector of matched strings.
     */
    std::vector<std::string> search(const std::string &query, int threshold);

    /**
     * @brief Adds a new string to the index.
     * @param str The string to add.
     */
    void addData(const std::string &str);

    /**
     * @brief Removes a string from the index.
     * @param str The string to remove.
     */
    void removeData(const std::string &str);

private:
    std::unordered_map<size_t, std::vector<std::string>>
        index_;                ///< The index containing the data.
    MatchStrategy *strategy_;  ///< The matching strategy to use.

    /**
     * @brief Builds the index from the given data.
     * @param data The vector of strings to build the index from.
     */
    void buildIndex(const std::vector<std::string> &data);
};

#endif
