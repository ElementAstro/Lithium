#include "search.hpp"

#include <algorithm>
#include <fstream>
#include <queue>
#include <sstream>

#include "atom/log/loguru.hpp"

namespace atom::search {

Document::Document(std::string docId, std::string docContent,
                   std::initializer_list<std::string> docTags)
    : id(std::move(docId)),
      content(std::move(docContent)),
      tags(docTags),
      clickCount(0) {
    LOG_F(INFO, "Document created with id: {}", id);
}

void SearchEngine::addDocument(const Document& doc) {
    LOG_F(INFO, "Adding document with id: {}", doc.id);
    // Check if document already exists
    try {
        findDocumentById(doc.id);
        throw std::invalid_argument("Document with this ID already exists.");
    } catch (const DocumentNotFoundException&) {
        // Proceed to add
    }

    totalDocs_++;
    for (const auto& tag : doc.tags) {
        tagIndex_[tag].push_back(doc);
        docFrequency_[tag]++;
        LOG_F(INFO, "Tag '{}' added to index", tag);
    }
    addContentToIndex(doc);
}

void SearchEngine::removeDocument(const std::string& docId) {
    LOG_F(INFO, "Removing document with id: {}", docId);
    Document doc = findDocumentById(docId);
    // Remove from tagIndex_
    for (const auto& tag : doc.tags) {
        auto& docs = tagIndex_[tag];
        docs.erase(
            std::remove_if(docs.begin(), docs.end(),
                           [&](const Document& d) { return d.id == docId; }),
            docs.end());
        if (docs.empty()) {
            tagIndex_.erase(tag);
        }
        docFrequency_[tag]--;
        if (docFrequency_[tag] <= 0) {
            docFrequency_.erase(tag);
        }
    }
    // Remove from contentIndex_
    std::istringstream iss(doc.content);
    std::string word;
    while (iss >> word) {
        contentIndex_[word].erase(docId);
        if (contentIndex_[word].empty()) {
            contentIndex_.erase(word);
        }
    }
    totalDocs_--;
    LOG_F(INFO, "Document with id: {} removed", docId);
}

void SearchEngine::updateDocument(const Document& doc) {
    LOG_F(INFO, "Updating document with id: {}", doc.id);
    removeDocument(doc.id);
    addDocument(doc);
    LOG_F(INFO, "Document with id: {} updated", doc.id);
}

void SearchEngine::addContentToIndex(const Document& doc) {
    LOG_F(INFO, "Indexing content for document id: {}", doc.id);
    std::istringstream iss(doc.content);
    std::string word;
    while (iss >> word) {
        contentIndex_[word].insert(doc.id);
        docFrequency_[word]++;
        LOG_F(INFO, "Word '{}' indexed for document id: {}", word, doc.id);
    }
}

auto SearchEngine::searchByTag(const std::string& tag)
    -> std::vector<Document> {
    LOG_F(INFO, "Searching by tag: {}", tag);
    return tagIndex_.contains(tag) ? tagIndex_[tag] : std::vector<Document>{};
}

auto SearchEngine::fuzzySearchByTag(const std::string& tag,
                                    int tolerance) -> std::vector<Document> {
    LOG_F(INFO, "Fuzzy searching by tag: {} with tolerance: {}", tag,
          tolerance);
    std::vector<Document> results;
    for (const auto& [key, docs] : tagIndex_) {
        if (levenshteinDistance(tag, key) <= tolerance) {
            results.insert(results.end(), docs.begin(), docs.end());
            LOG_F(INFO, "Tag '{}' matched with '{}'", key, tag);
        }
    }
    return results;
}

auto SearchEngine::searchByTags(const std::vector<std::string>& tags)
    -> std::vector<Document> {
    LOG_F(INFO, "Searching by multiple tags");
    std::unordered_map<std::string, double> scores;
    for (const auto& tag : tags) {
        if (tagIndex_.contains(tag)) {
            for (const auto& doc : tagIndex_[tag]) {
                scores[doc.id] += tfIdf(doc, tag);
                LOG_F(INFO, "Tag '{}' found in document id: {}", tag, doc.id);
            }
        }
    }
    return getRankedResults(scores);
}

auto SearchEngine::searchByContent(const std::string& query)
    -> std::vector<Document> {
    LOG_F(INFO, "Searching by content: {}", query);
    std::istringstream iss(query);
    std::string word;
    std::unordered_map<std::string, double> scores;
    while (iss >> word) {
        if (contentIndex_.contains(word)) {
            for (const auto& docId : contentIndex_[word]) {
                Document doc = findDocumentById(docId);
                scores[doc.id] += tfIdf(doc, word);
                LOG_F(INFO, "Word '{}' found in document id: {}", word, doc.id);
            }
        }
    }
    return getRankedResults(scores);
}

auto SearchEngine::booleanSearch(const std::string& query)
    -> std::vector<Document> {
    LOG_F(INFO, "Performing boolean search: {}", query);
    std::istringstream iss(query);
    std::string word;
    std::unordered_map<std::string, double> scores;
    while (iss >> word) {
        bool isNot = false;
        if (word == "NOT") {
            if (!(iss >> word))
                break;
            isNot = true;
        }

        if (contentIndex_.contains(word)) {
            for (const auto& docId : contentIndex_[word]) {
                Document doc = findDocumentById(docId);
                if (isNot) {
                    scores[doc.id] -= tfIdf(doc, word);
                    LOG_F(INFO, "Word '{}' excluded from document id: {}", word,
                          doc.id);
                } else {
                    scores[doc.id] += tfIdf(doc, word);
                    LOG_F(INFO, "Word '{}' included in document id: {}", word,
                          doc.id);
                }
            }
        }
    }
    return getRankedResults(scores);
}

auto SearchEngine::autoComplete(const std::string& prefix)
    -> std::vector<std::string> {
    LOG_F(INFO, "Auto-completing for prefix: {}", prefix);
    std::vector<std::string> suggestions;
    for (const auto& [key, _] : tagIndex_) {
        if (key.find(prefix) == 0) {
            suggestions.push_back(key);
            LOG_F(INFO, "Suggestion: {}", key);
        }
    }
    return suggestions;
}

void SearchEngine::saveIndex(const std::string& filename) const {
    LOG_F(INFO, "Saving index to file: {}", filename);
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        throw std::ios_base::failure("Failed to open file for writing.");
    }
    // Simple serialization
    size_t tagSize = tagIndex_.size();
    ofs.write(reinterpret_cast<const char*>(&tagSize), sizeof(tagSize));
    for (const auto& [tag, docs] : tagIndex_) {
        size_t tagLength = tag.size();
        ofs.write(reinterpret_cast<const char*>(&tagLength), sizeof(tagLength));
        ofs.write(tag.c_str(), tagLength);
        size_t docsSize = docs.size();
        ofs.write(reinterpret_cast<const char*>(&docsSize), sizeof(docsSize));
        for (const auto& doc : docs) {
            size_t idLength = doc.id.size();
            ofs.write(reinterpret_cast<const char*>(&idLength),
                      sizeof(idLength));
            ofs.write(doc.id.c_str(), idLength);
            size_t contentLength = doc.content.size();
            ofs.write(reinterpret_cast<const char*>(&contentLength),
                      sizeof(contentLength));
            ofs.write(doc.content.c_str(), contentLength);
            size_t tagsCount = doc.tags.size();
            ofs.write(reinterpret_cast<const char*>(&tagsCount),
                      sizeof(tagsCount));
            for (const auto& t : doc.tags) {
                size_t tLength = t.size();
                ofs.write(reinterpret_cast<const char*>(&tLength),
                          sizeof(tLength));
                ofs.write(t.c_str(), tLength);
            }
            ofs.write(reinterpret_cast<const char*>(&doc.clickCount),
                      sizeof(doc.clickCount));
        }
    }
    LOG_F(INFO, "Index saved successfully");
}

void SearchEngine::loadIndex(const std::string& filename) {
    LOG_F(INFO, "Loading index from file: {}", filename);
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) {
        throw std::ios_base::failure("Failed to open file for reading.");
    }
    tagIndex_.clear();
    contentIndex_.clear();
    docFrequency_.clear();
    totalDocs_ = 0;

    size_t tagSize;
    ifs.read(reinterpret_cast<char*>(&tagSize), sizeof(tagSize));
    for (size_t i = 0; i < tagSize; ++i) {
        size_t tagLength;
        ifs.read(reinterpret_cast<char*>(&tagLength), sizeof(tagLength));
        std::string tag(tagLength, ' ');
        ifs.read(&tag[0], tagLength);
        size_t docsSize;
        ifs.read(reinterpret_cast<char*>(&docsSize), sizeof(docsSize));
        for (size_t j = 0; j < docsSize; ++j) {
            Document doc("", "", {});
            size_t idLength;
            ifs.read(reinterpret_cast<char*>(&idLength), sizeof(idLength));
            doc.id.resize(idLength);
            ifs.read(&doc.id[0], idLength);
            size_t contentLength;
            ifs.read(reinterpret_cast<char*>(&contentLength),
                     sizeof(contentLength));
            doc.content.resize(contentLength);
            ifs.read(&doc.content[0], contentLength);
            size_t tagsCount;
            ifs.read(reinterpret_cast<char*>(&tagsCount), sizeof(tagsCount));
            for (size_t k = 0; k < tagsCount; ++k) {
                size_t tLength;
                ifs.read(reinterpret_cast<char*>(&tLength), sizeof(tLength));
                std::string t(tLength, ' ');
                ifs.read(&t[0], tLength);
                doc.tags.insert(t);
            }
            ifs.read(reinterpret_cast<char*>(&doc.clickCount),
                     sizeof(doc.clickCount));
            tagIndex_[tag].push_back(doc);
            totalDocs_++;
            for (const auto& w : doc.content) {
                contentIndex_[std::string(1, w)].insert(doc.id);
            }
        }
    }
    LOG_F(INFO, "Index loaded successfully");
}

auto SearchEngine::levenshteinDistance(const std::string& s1,
                                       const std::string& s2) -> int {
    LOG_F(INFO, "Calculating Levenshtein distance between '{}' and '{}'", s1,
          s2);
    std::vector<std::vector<int>> distanceMatrix(
        s1.size() + 1, std::vector<int>(s2.size() + 1));
    for (size_t i = 0; i <= s1.size(); i++) {
        distanceMatrix[i][0] = static_cast<int>(i);
    }
    for (size_t j = 0; j <= s2.size(); j++) {
        distanceMatrix[0][j] = static_cast<int>(j);
    }

    for (size_t i = 1; i <= s1.size(); i++) {
        for (size_t j = 1; j <= s2.size(); j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            distanceMatrix[i][j] = std::min(
                {distanceMatrix[i - 1][j] + 1, distanceMatrix[i][j - 1] + 1,
                 distanceMatrix[i - 1][j - 1] + cost});
        }
    }
    int distance = distanceMatrix[s1.size()][s2.size()];
    LOG_F(INFO, "Levenshtein distance: {}", distance);
    return distance;
}

auto SearchEngine::tfIdf(const Document& doc,
                         const std::string& term) -> double {
    LOG_F(INFO, "Calculating TF-IDF for term '{}' in document id: {}", term,
          doc.id);
    int termCount = static_cast<int>(std::count_if(
        doc.content.begin(), doc.content.end(),
        [&](char c) { return std::tolower(c) == std::tolower(term[0]); }));
    double termFrequency = static_cast<double>(termCount) /
                           static_cast<double>(doc.content.size());
    double inverseDocumentFrequency =
        log(static_cast<double>(totalDocs_) /
            (1 + docFrequency_.count(term) ? docFrequency_.at(term) : 1));
    double tfIdfValue = termFrequency * inverseDocumentFrequency;
    LOG_F(INFO, "TF-IDF value: %f", tfIdfValue);
    return tfIdfValue;
}

auto SearchEngine::findDocumentById(const std::string& docId) -> Document {
    LOG_F(INFO, "Finding document by id: {}", docId);
    for (const auto& [_, docs] : tagIndex_) {
        for (const auto& doc : docs) {
            if (doc.id == docId) {
                LOG_F(INFO, "Document found: {}", doc.id);
                return doc;
            }
        }
    }
    LOG_F(ERROR, "Document not found: {}", docId);
    throw DocumentNotFoundException(docId);
}

auto SearchEngine::getRankedResults(
    const std::unordered_map<std::string, double>& scores)
    -> std::vector<Document> {
    LOG_F(INFO, "Getting ranked results");
    std::priority_queue<std::pair<double, Document>,
                        std::vector<std::pair<double, Document>>, Compare>
        priorityQueue;
    for (const auto& [docId, score] : scores) {
        try {
            Document doc = findDocumentById(docId);
            priorityQueue.emplace(score + doc.clickCount, doc);
            LOG_F(INFO, "Document id: {}, score: %f", doc.id,
                  score + doc.clickCount);
        } catch (const DocumentNotFoundException& e) {
            LOG_F(WARNING, "{}", e.what());
        }
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
