#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>
#include <unordered_map>
#include "json.hpp"

using json = nlohmann::json;

struct FAQ
{
    std::string question;
    std::string description;
    std::string category;
    std::vector<std::string> solutions;
    int difficulty;
    std::vector<std::string> links;
};
public:
    void addFAQ(const FAQ &faq)
    {
        std::lock_guard<std::mutex> lock(mutex);
        faqs.push_back(faq);
    }

    void deleteFAQ(const std::string &question)
    {
        std::lock_guard<std::mutex> lock(mutex);
        faqs.erase(std::remove_if(faqs.begin(), faqs.end(),
                                  [&](const FAQ &faq)
                                  { return faq.question == question; }),
                   faqs.end());
    }

    std::vector<FAQ> searchFAQs(const std::string &keyword)
    {
        std::lock_guard<std::mutex> lock(mutex);

        // 检查缓存中是否存在搜索结果
        if (cache.find(keyword) != cache.end())
        {
            return cache[keyword];
        }

        std::vector<FAQ> results;
        for (const auto &faq : faqs)
        {
            if (faq.question.find(keyword) != std::string::npos ||
                faq.description.find(keyword) != std::string::npos ||
                faq.category.find(keyword) != std::string::npos)
            {
                results.push_back(faq);
            }
        }

        // 将搜索结果存入缓存
        cache[keyword] = results;

        return results;
    }

    std::vector<FAQ> getFAQs() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return faqs;
    }

    std::vector<FAQ> getCategorizedFAQs(const std::string &category)
    {
        std::lock_guard<std::mutex> lock(mutex);

        // 检查缓存中是否存在分类结果
        if (categoryCache.find(category) != categoryCache.end())
        {
            return categoryCache[category];
        }

        std::vector<FAQ> results;
        for (const auto &faq : faqs)
        {
            if (faq.category == category)
            {
                results.push_back(faq);
            }
        }

        // 将分类结果存入缓存
        categoryCache[category] = results;

        return results;
    }

    void saveToFile(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(mutex);
        json jsonData;

        for (const auto &faq : faqs)
        {
            json jsonFAQ;
            jsonFAQ["question"] = faq.question;
            jsonFAQ["description"] = faq.description;
            jsonFAQ["category"] = faq.category;
            jsonFAQ["solutions"] = faq.solutions;
            jsonFAQ["difficulty"] = faq.difficulty;
            jsonFAQ["links"] = faq.links;

            jsonData.push_back(jsonFAQ);
        }

        std::ofstream file(filename);
        file << jsonData.dump(4); // 4-space indentation for pretty printing
    }

    void loadFromFile(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::ifstream file(filename);
        json jsonData;
        file >> jsonData;

        faqs.clear();
        cache.clear();
        categoryCache.clear();

        for (const auto &jsonFAQ : jsonData)
        {
            FAQ faq;
            faq.question = jsonFAQ["question"];
            faq.description = jsonFAQ["description"];
            faq.category = jsonFAQ["category"];
            faq.solutions = jsonFAQ["solutions"];
            faq.difficulty = jsonFAQ["difficulty"];
            faq.links = jsonFAQ["links"];

            faqs.push_back(faq);
        }
    }

    void printFAQs() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        json jsonData;

        for (const auto &faq : faqs)
        {
            json jsonFAQ;
            jsonFAQ["question"] = faq.question;
            jsonFAQ["description"] = faq.description;
            jsonFAQ["category"] = faq.category;
            jsonFAQ["solutions"] = faq.solutions;
            jsonFAQ["difficulty"] = faq.difficulty;
            jsonFAQ["links"] = faq.links;

            jsonData.push_back(jsonFAQ);
        }

        std::cout << jsonData.dump(4) << std::endl;
    }