#include "search.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace atom::search {
Document::Document(std::string id, std::string content,
                   std::initializer_list<std::string> tags)
    : id(std::move(id)),
      content(std::move(content)),
      tags(tags),
      clickCount(0) {}

void SearchEngine::addDocument(const Document& doc) {
    totalDocs++;
    for (const auto& tag : doc.tags) {
        tagIndex[tag].push_back(doc);
        docFrequency[tag]++;
    }
    addContentToIndex(doc);
}

void SearchEngine::addContentToIndex(const Document& doc) {
    std::istringstream iss(doc.content);
    std::string word;
    while (iss >> word) {
        contentIndex[word].insert(doc.id);
    }
}

std::vector<Document> SearchEngine::searchByTag(const std::string& tag) {
    return tagIndex.count(tag) ? tagIndex[tag] : std::vector<Document>{};
}

std::vector<Document> SearchEngine::fuzzySearchByTag(const std::string& tag,
                                                     int tolerance) {
    std::vector<Document> results;
    for (const auto& [key, docs] : tagIndex) {
        if (levenshteinDistance(tag, key) <= tolerance) {
            results.insert(results.end(), docs.begin(), docs.end());
        }
    }
    return results;
}

std::vector<Document> SearchEngine::searchByTags(
    const std::vector<std::string>& tags) {
    std::unordered_map<std::string, double> scores;
    for (const auto& tag : tags) {
        if (tagIndex.count(tag)) {
            for (const auto& doc : tagIndex[tag]) {
                scores[doc.id] += tfIdf(doc, tag);
            }
        }
    }

    return getRankedResults(scores);
}

std::vector<Document> SearchEngine::searchByContent(const std::string& query) {
    std::istringstream iss(query);
    std::string word;
    std::unordered_map<std::string, double> scores;
    while (iss >> word) {
        if (contentIndex.count(word)) {
            for (const auto& docId : contentIndex[word]) {
                Document doc = findDocumentById(docId);
                scores[doc.id] += tfIdf(doc, word);
            }
        }
    }

    return getRankedResults(scores);
}

std::vector<Document> SearchEngine::booleanSearch(const std::string& query) {
    // This is a simplified implementation. A full implementation would need
    // a proper parser.
    std::istringstream iss(query);
    std::string word;
    std::unordered_map<std::string, double> scores;
    while (iss >> word) {
        bool isNot = false;
        if (word == "NOT") {
            iss >> word;
            isNot = true;
        }

        if (contentIndex.count(word)) {
            for (const auto& docId : contentIndex[word]) {
                Document doc = findDocumentById(docId);
                if (isNot) {
                    scores[doc.id] -= tfIdf(doc, word);
                } else {
                    scores[doc.id] += tfIdf(doc, word);
                }
            }
        }
    }

    return getRankedResults(scores);
}

std::vector<std::string> SearchEngine::autoComplete(const std::string& prefix) {
    std::vector<std::string> suggestions;
    for (const auto& [key, _] : tagIndex) {
        if (key.find(prefix) == 0) {
            suggestions.push_back(key);
        }
    }
    return suggestions;
}

int SearchEngine::levenshteinDistance(const std::string& s1,
                                      const std::string& s2) {
    std::vector<std::vector<int>> dp(s1.size() + 1,
                                     std::vector<int>(s2.size() + 1));
    for (size_t i = 0; i <= s1.size(); i++)
        dp[i][0] = i;
    for (size_t j = 0; j <= s2.size(); j++)
        dp[0][j] = j;

    for (size_t i = 1; i <= s1.size(); i++) {
        for (size_t j = 1; j <= s2.size(); j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min(
                {dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
        }
    }
    return dp[s1.size()][s2.size()];
}

double SearchEngine::tfIdf(const Document& doc, const std::string& term) {
    int termCount = std::count(doc.content.begin(), doc.content.end(),
                               term[0]);  // Simplified term frequency count
    double tf = static_cast<double>(termCount) / doc.content.size();
    double idf = log(static_cast<double>(totalDocs) / (1 + docFrequency[term]));
    return tf * idf;
}

Document SearchEngine::findDocumentById(const std::string& id) {
    for (const auto& [_, docs] : tagIndex) {
        for (const auto& doc : docs) {
            if (doc.id == id) {
                return doc;
            }
        }
    }
    throw std::runtime_error("Document not found");
}

std::vector<Document> SearchEngine::getRankedResults(
    const std::unordered_map<std::string, double>& scores) {
    std::priority_queue<std::pair<double, Document>,
                        std::vector<std::pair<double, Document>>, Compare>
        pq;
    for (const auto& [id, score] : scores) {
        Document doc = findDocumentById(id);
        pq.push({score + doc.clickCount, doc});
    }

    std::vector<Document> results;
    while (!pq.empty()) {
        results.push_back(pq.top().second);
        pq.pop();
    }

    return results;
}
}  // namespace atom::search
