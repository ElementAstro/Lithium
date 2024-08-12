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
#include <utility>
#include <vector>
#include "macro.hpp"

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
    explicit Quote(std::string text, std::string author)
        : text_(std::move(text)), author_(std::move(author)) {}

    /**
     * @brief Gets the text of the quote.
     *
     * @return The text of the quote.
     */
    ATOM_NODISCARD auto getText() const -> std::string { return text_; }

    /**
     * @brief Gets the author of the quote.
     *
     * @return The author of the quote.
     */
    ATOM_NODISCARD auto getAuthor() const -> std::string { return author_; }

private:
    std::string text_;
    std::string author_;
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

    void loadQuotesFromJson(const std::string &filename);

    void saveQuotesToJson(const std::string &filename) const;

    /**
     * @brief Searches for quotes containing a keyword.
     *
     * @param keyword The keyword to search for.
     * @return A vector of quotes containing the keyword.
     */
    ATOM_NODISCARD auto searchQuotes(const std::string &keyword) const
        -> std::vector<Quote>;

    /**
     * @brief Filters quotes by author.
     *
     * @param author The name of the author to filter by.
     * @return A vector of quotes by the specified author.
     */
    ATOM_NODISCARD auto filterQuotesByAuthor(const std::string &author) const
        -> std::vector<Quote>;

    /**
     * @brief Gets a random quote from the collection.
     *
     * @return A random quote.
     */
    ATOM_NODISCARD auto getRandomQuote() const -> std::string;

private:
    std::vector<Quote> quotes_;
};
}  // namespace atom::system

#endif
