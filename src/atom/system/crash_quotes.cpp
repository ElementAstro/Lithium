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
#include "atom/type/json.hpp"
#include "atom/utils/random.hpp"
using json = nlohmann::json;

namespace atom::system {
void QuoteManager::addQuote(const Quote &quote) { quotes_.push_back(quote); }

void QuoteManager::removeQuote(const Quote &quote) {
    auto it =
        std::find_if(quotes_.begin(), quotes_.end(), [&quote](const Quote &q) {
            return q.getText() == quote.getText() &&
                   q.getAuthor() == quote.getAuthor();
        });
    if (it != quotes_.end()) {
        quotes_.erase(it);
    }
}

#ifdef DEBUG
void QuoteManager::displayQuotes() const {
    for (const auto &quote : quotes_) {
        std::cout << quote.getText() << " - " << quote.getAuthor() << std::endl;
    }
}
#endif

void QuoteManager::shuffleQuotes() {
    atom::utils::Random<std::mt19937, std::uniform_int_distribution<>> random(
        std::random_device{}());
    std::shuffle(quotes_.begin(), quotes_.end(), random.engine());
}

void QuoteManager::clearQuotes() { quotes_.clear(); }

void QuoteManager::loadQuotesFromJson(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
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
    } catch (const nlohmann::json::parse_error &e) {
        THROW_UNLAWFUL_OPERATION("Error parsing JSON file: " +
                                 std::string(e.what()));
    }
}

void QuoteManager::saveQuotesToJson(const std::string &filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        json data;
        for (const auto &quote : quotes_) {
            data.push_back(
                {{"text", quote.getText()}, {"author", quote.getAuthor()}});
        }
        file << data.dump(4);
    }
    file.close();
}

auto QuoteManager::searchQuotes(const std::string &keyword) const
    -> std::vector<Quote> {
    std::vector<Quote> results;
    for (const auto &quote : quotes_) {
        if (quote.getText().find(keyword) != std::string::npos) {
            results.push_back(quote);
        }
    }
    return results;
}

auto QuoteManager::filterQuotesByAuthor(const std::string &author) const
    -> std::vector<Quote> {
    std::vector<Quote> results;
    for (const auto &quote : quotes_) {
        if (quote.getAuthor() == author) {
            results.push_back(quote);
        }
    }
    return results;
}

auto QuoteManager::getRandomQuote() const -> std::string {
    if (quotes_.empty()) {
        return "";
    }
    int quoteId =
        utils::Random<std::mt19937, std::uniform_int_distribution<int>>(
            0, quotes_.size() - 1)();
    return quotes_[quoteId].getText() + " - " + quotes_[quoteId].getAuthor();
}
}  // namespace atom::system
