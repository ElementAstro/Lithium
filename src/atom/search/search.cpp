#include "search.hpp"
#include <algorithm>
#include <cmath>
#include <loguru.hpp>
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
      clickCount(0) {
    LOG_F(INFO, "Document created with id: %s", id.c_str());
}

void SearchEngine::addDocument(const Document& doc) {
    LOG_F(INFO, "Adding document with id: %s", doc.id.c_str());
    totalDocs_++;
    for (const auto& tag : doc.tags) {
        tagIndex_[tag].push_back(doc);
        docFrequency_[tag]++;
        LOG_F(INFO, "Tag '%s' added to index", tag.c_str());
    }
    addContentToIndex(doc);
}

void SearchEngine::addContentToIndex(const Document& doc) {
    LOG_F(INFO, "Indexing content for document id: %s", doc.id.c_str());
    std::istringstream iss(doc.content);
    std::string word;
    while (iss >> word) {
        contentIndex_[word].insert(doc.id);
        LOG_F(INFO, "Word '%s' indexed for document id: %s", word.c_str(),
              doc.id.c_str());
    }
}

auto SearchEngine::searchByTag(const std::string& tag)
    -> std::vector<Document> {
    LOG_F(INFO, "Searching by tag: %s", tag.c_str());
    return tagIndex_.contains(tag) ? tagIndex_[tag] : std::vector<Document>{};
}

auto SearchEngine::fuzzySearchByTag(const std::string& tag,
                                    int tolerance) -> std::vector<Document> {
    LOG_F(INFO, "Fuzzy searching by tag: %s with tolerance: %d", tag.c_str(),
          tolerance);
    std::vector<Document> results;
    for (const auto& [key, docs] : tagIndex_) {
        if (levenshteinDistance(tag, key) <= tolerance) {
            results.insert(results.end(), docs.begin(), docs.end());
            LOG_F(INFO, "Tag '%s' matched with '%s'", key.c_str(), tag.c_str());
        }
    }
    return results;
}

auto SearchEngine::searchByTags(const std::vector<std::string>& tags)
    -> std::vector<Document> {
    LOG_F(INFO, "Searching by tags");
    std::unordered_map<std::string, double> scores;
    for (const auto& tag : tags) {
        if (tagIndex_.contains(tag)) {
            for (const auto& doc : tagIndex_[tag]) {
                scores[doc.id] += tfIdf(doc, tag);
                LOG_F(INFO, "Tag '%s' found in document id: %s", tag.c_str(),
                      doc.id.c_str());
            }
        }
    }

    return getRankedResults(scores);
}

auto SearchEngine::searchByContent(const std::string& query)
    -> std::vector<Document> {
    LOG_F(INFO, "Searching by content: %s", query.c_str());
    std::istringstream iss(query);
    std::string word;
    std::unordered_map<std::string, double> scores;
    while (iss >> word) {
        if (contentIndex_.contains(word)) {
            for (const auto& docId : contentIndex_[word]) {
                Document doc = findDocumentById(docId);
                scores[doc.id] += tfIdf(doc, word);
                LOG_F(INFO, "Word '%s' found in document id: %s", word.c_str(),
                      doc.id.c_str());
            }
        }
    }

    return getRankedResults(scores);
}

auto SearchEngine::booleanSearch(const std::string& query)
    -> std::vector<Document> {
    LOG_F(INFO, "Performing boolean search: %s", query.c_str());
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
                    LOG_F(INFO, "Word '%s' excluded from document id: %s",
                          word.c_str(), doc.id.c_str());
                } else {
                    scores[doc.id] += tfIdf(doc, word);
                    LOG_F(INFO, "Word '%s' included in document id: %s",
                          word.c_str(), doc.id.c_str());
                }
            }
        }
    }

    return getRankedResults(scores);
}

auto SearchEngine::autoComplete(const std::string& prefix)
    -> std::vector<std::string> {
    LOG_F(INFO, "Auto-completing for prefix: %s", prefix.c_str());
    std::vector<std::string> suggestions;
    for (const auto& [key, _] : tagIndex_) {
        if (key.find(prefix) == 0) {
            suggestions.push_back(key);
            LOG_F(INFO, "Suggestion: %s", key.c_str());
        }
    }
    return suggestions;
}

auto SearchEngine::levenshteinDistance(const std::string& str1,
                                       const std::string& str2) -> int {
    LOG_F(INFO, "Calculating Levenshtein distance between '%s' and '%s'",
          str1.c_str(), str2.c_str());
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
    int distance = distanceMatrix[str1.size()][str2.size()];
    LOG_F(INFO, "Levenshtein distance: %d", distance);
    return distance;
}

auto SearchEngine::tfIdf(const Document& doc,
                         const std::string& term) -> double {
    LOG_F(INFO, "Calculating TF-IDF for term '%s' in document id: %s",
          term.c_str(), doc.id.c_str());
    int termCount = static_cast<int>(
        std::count(doc.content.begin(), doc.content.end(), term[0]));
    double termFrequency = static_cast<double>(termCount) /
                           static_cast<double>(doc.content.size());
    double inverseDocumentFrequency =
        log(static_cast<double>(totalDocs_) / (1 + docFrequency_[term]));
    double tfIdfValue = termFrequency * inverseDocumentFrequency;
    LOG_F(INFO, "TF-IDF value: %f", tfIdfValue);
    return tfIdfValue;
}

auto SearchEngine::findDocumentById(const std::string& docId) -> Document {
    LOG_F(INFO, "Finding document by id: %s", docId.c_str());
    for (const auto& [_, docs] : tagIndex_) {
        for (const auto& doc : docs) {
            if (doc.id == docId) {
                LOG_F(INFO, "Document found: %s", doc.id.c_str());
                return doc;
            }
        }
    }
    LOG_F(ERROR, "Document not found: %s", docId.c_str());
    throw std::runtime_error("Document not found");
}

auto SearchEngine::getRankedResults(
    const std::unordered_map<std::string, double>& scores)
    -> std::vector<Document> {
    LOG_F(INFO, "Getting ranked results");
    std::priority_queue<std::pair<double, Document>,
                        std::vector<std::pair<double, Document>>, Compare>
        priorityQueue;
    for (const auto& [docId, score] : scores) {
        Document doc = findDocumentById(docId);
        priorityQueue.emplace(score + doc.clickCount, doc);
        LOG_F(INFO, "Document id: %s, score: %f", doc.id.c_str(),
              score + doc.clickCount);
    }

    std::vector<Document> results;
    while (!priorityQueue.empty()) {
        results.push_back(priorityQueue.top().second);
        priorityQueue.pop();
    }

    LOG_F(INFO, "Ranked results obtained");
    return results;
}
}  // namespace atom::search