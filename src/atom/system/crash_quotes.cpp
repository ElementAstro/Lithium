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

namespace Atom::System {

Quote::Quote(const std::string &text, const std::string &author)
    : text(text), author(author) {}

const std::string &Quote::getText() const { return text; }

const std::string &Quote::getAuthor() const { return author; }

void QuoteManager::addQuote(const Quote &quote) { quotes.push_back(quote); }

void QuoteManager::removeQuote(const Quote &quote) {
    if (quotes.empty()) {
        return;
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

void QuoteManager::loadQuotesFromFile(const std::string &filename) {
    std::ifstream file(filename);
    try {
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                auto delimiterPos = line.find(" - ");
                if (delimiterPos != std::string::npos) {
                    std::string quoteText = line.substr(0, delimiterPos);
                    std::string quoteAuthor = line.substr(delimiterPos + 3);
                    addQuote(Quote(quoteText, quoteAuthor));
                }
            }
            file.close();
        }
    } catch (const std::exception &e) {
        THROW_EXCEPTION("QuoteManager::loadQuotesFromFile", e.what());
    }
}

void QuoteManager::saveQuotesToFile(const std::string &filename) const {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto &quote : quotes) {
            file << quote.getText() << " " << quote.getAuthor() << std::endl;
        }
        file.close();
    }
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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, quotes.size() - 1);
    return quotes[dis(gen)].getText() + " - " + quotes[dis(gen)].getAuthor();
}
}  // namespace Atom::System

/*
int main()
{
    QuoteManager quoteManager;

    // 添加名言
    quoteManager.addQuote(Quote("Be yourself; everyone else is already taken.",
"Oscar Wilde")); quoteManager.addQuote(Quote("In the end, it's not the years in
your life that count. It's the life in your years.", "Abraham Lincoln"));
    quoteManager.addQuote(Quote("The only way to do great work is to love what
you do.", "Steve Jobs"));

    // 显示所有名言
    std::cout << "All Quotes:" << std::endl;
    quoteManager.displayQuotes();

    // 按关键词搜索名言
    std::cout << "\nQuotes containing 'love':" << std::endl;
    auto loveQuotes = quoteManager.searchQuotes("love");
    for (const auto &quote : loveQuotes)
    {
        std::cout << quote.getText() << " - " << quote.getAuthor() << std::endl;
    }

    // 按作者过滤名言
    std::cout << "\nQuotes by Steve Jobs:" << std::endl;
    auto jobsQuotes = quoteManager.filterQuotesByAuthor("Steve Jobs");
    for (const auto &quote : jobsQuotes)
    {
        std::cout << quote.getText() << " - " << quote.getAuthor() << std::endl;
    }

    std::cout << "\nRandom Quote:" << std::endl;
    std::cout << quoteManager.getRandomQuote() << std::endl;

    // 保存名言到文件
    quoteManager.saveQuotesToFile("updated_quotes.txt");

    return 0;
}

*/
