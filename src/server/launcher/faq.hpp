/*
 * faq.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: F&Q Manager for HEAL

**************************************************/

#include <mutex>
#include <string>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/type/json.hpp"
using json = nlohmann::json;

struct FAQ {
    std::string question;
    std::string description;
    std::string category;
    std::vector<std::string> solutions;
    int difficulty;
    std::vector<std::string> links;
};

class FAQManager {
public:
    void addFAQ(const FAQ &faq);

    void deleteFAQ(const std::string &question);

    std::vector<FAQ> searchFAQs(const std::string &keyword);

    std::vector<FAQ> getFAQs() const;

    std::vector<FAQ> getCategorizedFAQs(const std::string &category);

    void saveToFile(const std::string &filename);

    void loadFromFile(const std::string &filename);

    void printFAQs() const;

private:
    std::vector<FAQ> faqs;
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::vector<FAQ>> cache;
    emhash8::HashMap<std::string, std::vector<FAQ>> categoryCache;
#else
    std::unordered_map<std::string, std::vector<FAQ>> cache;
    std::unordered_map<std::string, std::vector<FAQ>> categoryCache;
#endif
    mutable std::mutex mutex;
};