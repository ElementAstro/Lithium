#include "search.hpp"

#include <algorithm>
#include <cmath>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace atom::search {
Document::Document(std::string docId, std::string docContent,
                   std::initializer_list<std::string> docTags)
    : id(std::move(docId)),
      content(std::move(docContent)),
      tags(docTags),
      clickCount(0) {}

void SearchEngine::addDocument(const Document& doc) {
    totalDocs_++;
    for (const auto& tag : doc.tags) {
        tagIndex_[tag].push_back(doc);
        docFrequency_[tag]++;
    }
    addContentToIndex(doc);
}

void SearchEngine::addContentToIndex(const Document& doc) {
    std::istringstream iss(doc.content);
    std::string word;
    while (iss >> word) {
        contentIndex_[word].insert(doc.id);
    }
}

auto SearchEngine::searchByTag(const std::string& tag)
    -> std::vector<Document> {
    return tagIndex_.contains(tag) ? tagIndex_[tag] : std::vector<Document>{};
}

auto SearchEngine::fuzzySearchByTag(const std::string& tag,
                                    int tolerance) -> std::vector<Document> {
    std::vector<Document> results;
    for (const auto& [key, docs] : tagIndex_) {
        if (levenshteinDistance(tag, key) <= tolerance) {
            results.insert(results.end(), docs.begin(), docs.end());
        }
    }
    return results;
}

auto SearchEngine::searchByTags(const std::vector<std::string>& tags)
    -> std::vector<Document> {
    std::unordered_map<std::string, double> scores;
    for (const auto& tag : tags) {
        if (tagIndex_.contains(tag)) {
            for (const auto& doc : tagIndex_[tag]) {
                scores[doc.id] += tfIdf(doc, tag);
            }
        }
    }

    return getRankedResults(scores);
}

auto SearchEngine::searchByContent(const std::string& query)
    -> std::vector<Document> {
    std::istringstream iss(query);
    std::string word;
    std::unordered_map<std::string, double> scores;
    while (iss >> word) {
        if (contentIndex_.contains(word)) {
            for (const auto& docId : contentIndex_[word]) {
                Document doc = findDocumentById(docId);
                scores[doc.id] += tfIdf(doc, word);
            }
        }
    }

    return getRankedResults(scores);
}

auto SearchEngine::booleanSearch(const std::string& query)
    -> std::vector<Document> {
    std::istringstream iss(query);
    std::string word;
    std::unordered_map<std::string, double> scores;
    while (iss >> word) {
        bool isNot = false;
        if (word == "NOT") {
            iss >> word;
            isNot = true;
        }

        if (contentIndex_.contains(word)) {
            for (const auto& docId : contentIndex_[word]) {
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

auto SearchEngine::autoComplete(const std::string& prefix)
    -> std::vector<std::string> {
    std::vector<std::string> suggestions;
    for (const auto& [key, _] : tagIndex_) {
        if (key.find(prefix) == 0) {
            suggestions.push_back(key);
        }
    }
    return suggestions;
}

auto SearchEngine::levenshteinDistance(const std::string& str1,
                                       const std::string& str2) -> int {
    std::vector<std::vector<int>> distanceMatrix(
        str1.size() + 1, std::vector<int>(str2.size() + 1));
    for (size_t i = 0; i <= str1.size(); i++) {
        distanceMatrix[i][0] = static_cast<int>(i);
    }
    for (size_t j = 0; j <= str2.size(); j++) {
        distanceMatrix[0][j] = static_cast<int>(j);
    }

    for (size_t i = 1; i <= str1.size(); i++) {
        for (size_t j = 1; j <= str2.size(); j++) {
            int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
            distanceMatrix[i][j] = std::min(
                {distanceMatrix[i - 1][j] + 1, distanceMatrix[i][j - 1] + 1,
                 distanceMatrix[i - 1][j - 1] + cost});
        }
    }
    return distanceMatrix[str1.size()][str2.size()];
}

auto SearchEngine::tfIdf(const Document& doc,
                         const std::string& term) -> double {
    int termCount = static_cast<int>(
        std::count(doc.content.begin(), doc.content.end(), term[0]));
    double termFrequency = static_cast<double>(termCount) /
                           static_cast<double>(doc.content.size());
    double inverseDocumentFrequency =
        log(static_cast<double>(totalDocs_) / (1 + docFrequency_[term]));
    return termFrequency * inverseDocumentFrequency;
}

auto SearchEngine::findDocumentById(const std::string& docId) -> Document {
    for (const auto& [_, docs] : tagIndex_) {
        for (const auto& doc : docs) {
            if (doc.id == docId) {
                return doc;
            }
        }
    }
    throw std::runtime_error("Document not found");
}

auto SearchEngine::getRankedResults(
    const std::unordered_map<std::string, double>& scores)
    -> std::vector<Document> {
    std::priority_queue<std::pair<double, Document>,
                        std::vector<std::pair<double, Document>>, Compare>
        priorityQueue;
    for (const auto& [docId, score] : scores) {
        Document doc = findDocumentById(docId);
        priorityQueue.emplace(score + doc.clickCount, doc);
    }

    std::vector<Document> results;
    while (!priorityQueue.empty()) {
        results.push_back(priorityQueue.top().second);
        priorityQueue.pop();
    }

    return results;
}
}  // namespace atom::search
