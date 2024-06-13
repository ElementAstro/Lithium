/*
 * crash_quotes.cpp
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

Quote::Quote(const std::string &text, const std::string &author)
    : text(text), author(author) {}

const std::string &Quote::getText() const { return text; }

const std::string &Quote::getAuthor() const { return author; }

void QuoteManager::addQuote(const Quote &quote) { quotes.push_back(quote); }

void QuoteManager::removeQuote(const Quote &quote) {
    if (quotes.empty()) {
        return;
    }
    for (auto it = quotes.begin(); it != quotes.end(); ++it) {
        if (it->getText() == quote.getText() &&
            it->getAuthor() == quote.getAuthor()) {
            quotes.erase(it);
            break;
        }
    }
}

#ifdef DEBUG
void QuoteManager::displayQuotes() const {
    for (const auto &quote : quotes) {
        std::cout << quote.getText() << " - " << quote.getAuthor() << std::endl;
    }
}
#endif

void QuoteManager::shuffleQuotes() {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(quotes.begin(), quotes.end(), g);
}

void QuoteManager::clearQuotes() { quotes.clear(); }

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
        for (const auto &quote : quotes) {
            data.push_back(
                {{"text", quote.getText()}, {"author", quote.getAuthor()}});
        }
        file << data.dump(4);
    }
    file.close();
}

std::vector<Quote> QuoteManager::searchQuotes(
    const std::string &keyword) const {
    std::vector<Quote> results;
    for (const auto &quote : quotes) {
        if (quote.getText().find(keyword) != std::string::npos) {
            results.push_back(quote);
        }
    }
    return results;
}

std::vector<Quote> QuoteManager::filterQuotesByAuthor(
    const std::string &author) const {
    std::vector<Quote> results;
    for (const auto &quote : quotes) {
        if (quote.getAuthor() == author) {
            results.push_back(quote);
        }
    }
    return results;
}

std::string QuoteManager::getRandomQuote() const {
    if (quotes.empty()) {
        return "";
    }
    int id = utils::Random<std::mt19937, std::uniform_int_distribution<int>>(
        0, quotes.size() - 1)();
    return quotes[id].getText() + " - " + quotes[id].getAuthor();
}
}  // namespace atom::system
