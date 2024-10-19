/*
 * crash_quotes_.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-24

Description: Quote manager for crash report.

**************************************************/

#include "crash_quotes.hpp"

#include <algorithm>
#include <fstream>
#include <random>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/random.hpp"

using json = nlohmann::json;

namespace atom::system {
void QuoteManager::addQuote(const Quote &quote) {
    LOG_F(INFO, "Adding quote: {} - {}", quote.getText(), quote.getAuthor());
    quotes_.push_back(quote);
    LOG_F(INFO, "Quote added successfully");
}

void QuoteManager::removeQuote(const Quote &quote) {
    LOG_F(INFO, "Removing quote: {} - {}", quote.getText(), quote.getAuthor());
    auto it =
        std::find_if(quotes_.begin(), quotes_.end(), [&quote](const Quote &q) {
            return q.getText() == quote.getText() &&
                   q.getAuthor() == quote.getAuthor();
        });
    if (it != quotes_.end()) {
        quotes_.erase(it);
        LOG_F(INFO, "Quote removed successfully");
    } else {
        LOG_F(WARNING, "Quote not found: {} - {}", quote.getText(),
              quote.getAuthor());
    }
}

#ifdef DEBUG
void QuoteManager::displayQuotes() const {
    LOG_F(INFO, "Displaying all quotes");
    for (const auto &quote : quotes_) {
        std::cout << quote.getText() << " - " << quote.getAuthor() << std::endl;
    }
    LOG_F(INFO, "Displayed all quotes successfully");
}
#endif

void QuoteManager::shuffleQuotes() {
    LOG_F(INFO, "Shuffling quotes");
    atom::utils::Random<std::mt19937, std::uniform_int_distribution<>> random(
        std::random_device{}());
    std::shuffle(quotes_.begin(), quotes_.end(), random.engine());
    LOG_F(INFO, "Quotes shuffled successfully");
}

void QuoteManager::clearQuotes() {
    LOG_F(INFO, "Clearing all quotes");
    quotes_.clear();
    LOG_F(INFO, "All quotes cleared successfully");
}

void QuoteManager::loadQuotesFromJson(const std::string &filename) {
    LOG_F(INFO, "Loading quotes from JSON file: {}", filename);
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_F(ERROR, "Failed to open JSON file: {}", filename);
        return;
    }
    try {
        json data = json::parse(file);
        for (const auto &quote : data) {
            std::string quoteText = quote["text"];
            std::string quoteAuthor = quote["author"];
            if (!quoteText.empty() && !quoteAuthor.empty()) {
                addQuote(Quote(quoteText, quoteAuthor));
            }
        }
        LOG_F(INFO, "Quotes loaded successfully from JSON file: {}", filename);
    } catch (const nlohmann::json::parse_error &e) {
        LOG_F(ERROR, "Error parsing JSON file: {} - {}", filename, e.what());
        THROW_UNLAWFUL_OPERATION("Error parsing JSON file: " +
                                 std::string(e.what()));
    }
}

void QuoteManager::saveQuotesToJson(const std::string &filename) const {
    LOG_F(INFO, "Saving quotes to JSON file: {}", filename);
    std::ofstream file(filename);
    if (file.is_open()) {
        json data;
        for (const auto &quote : quotes_) {
            data.push_back(
                {{"text", quote.getText()}, {"author", quote.getAuthor()}});
        }
        file << data.dump(4);
        LOG_F(INFO, "Quotes saved successfully to JSON file: {}", filename);
    } else {
        LOG_F(ERROR, "Failed to open JSON file for writing: {}", filename);
    }
    file.close();
}

auto QuoteManager::searchQuotes(const std::string &keyword) const
    -> std::vector<Quote> {
    LOG_F(INFO, "Searching quotes with keyword: {}", keyword);
    std::vector<Quote> results;
    for (const auto &quote : quotes_) {
        if (quote.getText().find(keyword) != std::string::npos) {
            results.push_back(quote);
        }
    }
    LOG_F(INFO, "Found {} quotes with keyword: {}", results.size(), keyword);
    return results;
}

auto QuoteManager::filterQuotesByAuthor(const std::string &author) const
    -> std::vector<Quote> {
    LOG_F(INFO, "Filtering quotes by author: {}", author);
    std::vector<Quote> results;
    for (const auto &quote : quotes_) {
        if (quote.getAuthor() == author) {
            results.push_back(quote);
        }
    }
    LOG_F(INFO, "Found {} quotes by author: {}", results.size(), author);
    return results;
}

auto QuoteManager::getRandomQuote() const -> std::string {
    LOG_F(INFO, "Getting a random quote");
    if (quotes_.empty()) {
        LOG_F(WARNING, "No quotes available");
        return "";
    }
    int quoteId =
        utils::Random<std::mt19937, std::uniform_int_distribution<int>>(
            0, quotes_.size() - 1)();
    std::string randomQuote =
        quotes_[quoteId].getText() + " - " + quotes_[quoteId].getAuthor();
    LOG_F(INFO, "Random quote: {}", randomQuote);
    return randomQuote;
}
}  // namespace atom::system
