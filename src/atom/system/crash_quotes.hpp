/*
 * crash_quotes.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-24

Description: Quote manager for crash report.

**************************************************/

#ifndef ATOM_SYSTEM_CRASH_QUOTES_HPP
#define ATOM_SYSTEM_CRASH_QUOTES_HPP

#include <string>
#include <vector>

namespace atom::system {
/**
 * @brief Represents a quote with its text and author.
 */
class Quote {
public:
    /**
     * @brief Constructs a new Quote object.
     *
     * @param text The text of the quote.
     * @param author The author of the quote.
     */
    explicit Quote(const std::string &text, const std::string &author);

    /**
     * @brief Gets the text of the quote.
     *
     * @return The text of the quote.
     */
    const std::string &getText() const;

    /**
     * @brief Gets the author of the quote.
     *
     * @return The author of the quote.
     */
    const std::string &getAuthor() const;

private:
    std::string text;
    std::string author;
};

/**
 * @brief Manages a collection of quotes.
 */
class QuoteManager {
public:
    /**
     * @brief Adds a quote to the collection.
     *
     * @param quote The quote to add.
     */
    void addQuote(const Quote &quote);

    /**
     * @brief Removes a quote from the collection.
     *
     * @param quote The quote to remove.
     */
    void removeQuote(const Quote &quote);

#ifdef DEBUG
    /**
     * @brief Displays all quotes in the collection.
     */
    void displayQuotes() const;
#endif

    /**
     * @brief Shuffles the quotes in the collection.
     */
    void shuffleQuotes();

    /**
     * @brief Clears all quotes in the collection.
     */
    void clearQuotes();

    /**
     * @brief Loads quotes from a file into the collection.
     *
     * @param filename The name of the file to load quotes from.
     */
    void loadQuotesFromFile(const std::string &filename);

    /**
     * @brief Saves quotes in the collection to a file.
     *
     * @param filename The name of the file to save quotes to.
     */
    void saveQuotesToFile(const std::string &filename) const;

    /**
     * @brief Searches for quotes containing a keyword.
     *
     * @param keyword The keyword to search for.
     * @return A vector of quotes containing the keyword.
     */
    std::vector<Quote> searchQuotes(const std::string &keyword) const;

    /**
     * @brief Filters quotes by author.
     *
     * @param author The name of the author to filter by.
     * @return A vector of quotes by the specified author.
     */
    std::vector<Quote> filterQuotesByAuthor(const std::string &author) const;

    /**
     * @brief Gets a random quote from the collection.
     *
     * @return A random quote.
     */
    std::string getRandomQuote() const;

private:
    std::vector<Quote> quotes;
};
}  // namespace atom::system

#endif
