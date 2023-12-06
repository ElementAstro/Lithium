/*
 * search.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-4

Description: A search engine

**************************************************/

#include "search.hpp"

std::vector<std::string> FuzzyMatch::match(const std::string &query, const std::unordered_map<size_t, std::vector<std::string>> &index, int threshold = 3)
{
    std::vector<std::string> results;
    for (const auto &[hashVal, strList] : index)
    {
        try
        {
            for (const std::string &str : strList)
            {
                if (editDistance(query, str) < threshold)
                {
                    results.push_back(str);
                }
            }
        }
        catch (const std::invalid_argument &)
        {
            // 忽略空字符串列表
        }
    }
    return results;
}

int FuzzyMatch::editDistance(const std::string &s1, const std::string &s2)
{
    int n1 = s1.size(), n2 = s2.size();
    std::vector<std::vector<int>> dp(n1 + 1, std::vector<int>(n2 + 1));
    for (int i = 0; i <= n1; ++i)
        dp[i][0] = i;
    for (int j = 0; j <= n2; ++j)
        dp[0][j] = j;
    for (int i = 1; i <= n1; ++i)
    {
        for (int j = 1; j <= n2; ++j)
        {
            if (s1[i - 1] == s2[j - 1])
            {
                dp[i][j] = dp[i - 1][j - 1];
            }
            else
            {
                dp[i][j] = std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]}) + 1;
            }
        }
    }
    return dp[n1][n2];
}

std::vector<std::string> RegexMatch::match(const std::string &query, const std::unordered_map<size_t, std::vector<std::string>> &index, int /*threshold*/ = 0)
{
    std::vector<std::string> results;
    try
    {
        std::regex regexPattern(query);
        for (const auto &[_, strList] : index)
        {
            for (const std::string &str : strList)
            {
                if (std::regex_search(str, regexPattern))
                {
                    results.push_back(str);
                }
            }
        }
    }
    catch (const std::regex_error &)
    {
        // 忽略无效的正则表达式
    }
    return results;
}

HammingMatch::HammingMatch(int maxDistance) : maxDistance_(maxDistance) {}

std::vector<std::string> HammingMatch::match(const std::string &query, const std::unordered_map<size_t, std::vector<std::string>> &index, int /*threshold*/ = 0)
{
    std::vector<std::string> results;
    for (const auto &[hashVal, strList] : index)
    {
        try
        {
            for (const std::string &str : strList)
            {
                if (hammingDistance(query, str) <= maxDistance_)
                {
                    results.push_back(str);
                }
            }
        }
        catch (const std::invalid_argument &)
        {
            // 忽略空字符串列表
        }
    }
    return results;
}

int HammingMatch::hammingDistance(const std::string &s1, const std::string &s2)
{
    if (s1.size() != s2.size())
    {
        throw std::invalid_argument("strings have different lengths");
    }
    int distance = 0;
    for (size_t i = 0; i < s1.size(); i++)
    {
        if (s1[i] != s2[i])
        {
            distance++;
        }
    }
    return distance;
}

TfIdfMatch::TfIdfMatch(const std::vector<std::string> &data)
{
    buildIndex(data);
    buildIdf();
}

std::vector<std::string> TfIdfMatch::match(const std::string &query, const std::unordered_map<size_t, std::vector<std::string>> &index, int /*threshold*/ = 0)
{
    std::vector<std::string> results;
    std::unordered_map<std::string, double> queryTf = calculateTf(query);
    std::unordered_map<std::string, double> queryTfidf = calculateTfidf(queryTf);
    for (const auto &[_, strList] : index)
    {
        std::unordered_map<std::string, double> strTfidf = calculateTfidf(strList);
        double similarity = cosineSimilarity(queryTfidf, strTfidf);
        if (similarity > 0.0)
        {
            results.push_back(strList[0]);
        }
    }
    return results;
}

void TfIdfMatch::buildIndex(const std::vector<std::string> &data)
{
    for (const auto &str : data)
    {
        std::unordered_map<std::string, double> tf = calculateTf(str);
        termFrequency_.push_back(tf);
    }
}

void TfIdfMatch::buildIdf()
{
    size_t numDocs = termFrequency_.size();
    for (const auto &tf : termFrequency_)
    {
        for (const auto &[term, _] : tf)
        {
            inverseDocumentFrequency_[term] += 1.0;
        }
    }
    for (auto &[_, idf] : inverseDocumentFrequency_)
    {
        if (idf > 0.0)
        {
            idf = std::log(numDocs / idf);
        }
        else
        {
            idf = 0.0; // 或者设置为一个较小的非零值
        }
    }
}

std::unordered_map<std::string, double> TfIdfMatch::calculateTf(const std::string &str)
{
    std::unordered_map<std::string, double> tf;
    for (const char &c : str)
    {
        tf[std::string(1, c)]++;
    }
    return tf;
}

std::unordered_map<std::string, double> TfIdfMatch::calculateTf(const std::vector<std::string> &strList)
{
    std::unordered_map<std::string, double> tf;
    for (const auto &str : strList)
    {
        for (const char &c : str)
        {
            tf[std::string(1, c)]++;
        }
    }
    size_t numDocs = strList.size();
    for (auto &[_, val] : tf)
    {
        val /= numDocs;
    }
    return tf;
}

std::unordered_map<std::string, double> TfIdfMatch::calculateTfidf(const std::unordered_map<std::string, double> &tf)
{
    std::unordered_map<std::string, double> tfidf;
    for (const auto &[term, tfVal] : tf)
    {
        if (inverseDocumentFrequency_.count(term))
        {
            tfidf[term] = tfVal * inverseDocumentFrequency_[term];
        }
    }
    return tfidf;
}

std::unordered_map<std::string, double> TfIdfMatch::calculateTfidf(const std::vector<std::string> &strList)
{
    std::unordered_map<std::string, double> tf = calculateTf(strList);
    std::unordered_map<std::string, double> tfidf = calculateTfidf(tf);
    double sum = 0.0;
    for (const auto &[_, val] : tfidf)
    {
        sum += val * val;
    }
    double norm = std::sqrt(sum);
    for (auto &[term, val] : tfidf)
    {
        val /= norm;
    }
    return tfidf;
}

double TfIdfMatch::cosineSimilarity(const std::unordered_map<std::string, double> &tfidf1, const std::unordered_map<std::string, double> &tfidf2)
{
    std::vector<std::string> commonTerms;
    for (const auto &[term, _] : tfidf1)
    {
        if (tfidf2.count(term))
        {
            commonTerms.push_back(term);
        }
    }
    double dotProduct = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    for (const auto &term : commonTerms)
    {
        double val1 = tfidf1.at(term);
        double val2 = tfidf2.at(term);
        dotProduct += val1 * val2;
        norm1 += val1 * val1;
        norm2 += val2 * val2;
    }
    norm1 = std::sqrt(norm1);
    norm2 = std::sqrt(norm2);
    if (norm1 == 0.0 || norm2 == 0.0)
    {
        return 0.0;
    }
    else
    {
        return dotProduct / (norm1 * norm2);
    }
}

SearchEngine::SearchEngine(const std::vector<std::string> &data, MatchStrategy *strategy) : strategy_(strategy)
{
    buildIndex(data);
}

void SearchEngine::setMatchStrategy(MatchStrategy *strategy)
{
    strategy_ = strategy;
}

std::vector<std::string> SearchEngine::search(const std::string &query, int threshold = 3)
{
    return strategy_->match(query, index_, threshold);
}

void SearchEngine::buildIndex(const std::vector<std::string> &data)
{
    for (const auto &str : data)
    {
        size_t hashVal = std::hash<std::string>{}(str);
        index_[hashVal].push_back(str);
    }
}

void SearchEngine::addData(const std::string &str)
{
    size_t hashVal = std::hash<std::string>{}(str);
    index_[hashVal].push_back(str);
}

void SearchEngine::removeData(const std::string &str)
{
    size_t hashVal = std::hash<std::string>{}(str);
    auto it = index_.find(hashVal);
    if (it != index_.end())
    {
        auto &strList = it->second;
        strList.erase(std::remove(strList.begin(), strList.end(), str), strList.end());
        if (strList.empty())
        {
            index_.erase(it);
        }
    }
}
