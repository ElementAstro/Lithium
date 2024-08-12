#ifndef ATOM_SEARCH_SEARCH_HPP
#define ATOM_SEARCH_SEARCH_HPP

#include <cmath>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace atom::search {
struct Document {
    std::string id;
    std::string content;
    std::set<std::string> tags;
    int clickCount;  // 用于调整权重

    explicit Document(std::string id, std::string content,
                      std::initializer_list<std::string> tags);
};

class SearchEngine {
private:
    std::unordered_map<std::string, std::vector<Document>> tagIndex_;
    std::unordered_map<std::string, std::unordered_set<std::string>>
        contentIndex_;
    std::unordered_map<std::string, int> docFrequency_;
    int totalDocs_ = 0;

public:
    void addDocument(const Document& doc);

    void addContentToIndex(const Document& doc);

    auto searchByTag(const std::string& tag) -> std::vector<Document>;

    auto fuzzySearchByTag(const std::string& tag,
                          int tolerance) -> std::vector<Document>;

    auto searchByTags(const std::vector<std::string>& tags)
        -> std::vector<Document>;

    auto searchByContent(const std::string& query) -> std::vector<Document>;

    auto booleanSearch(const std::string& query) -> std::vector<Document>;

    auto autoComplete(const std::string& prefix) -> std::vector<std::string>;

private:
    auto levenshteinDistance(const std::string& s1,
                             const std::string& s2) -> int;

    auto tfIdf(const Document& doc, const std::string& term) -> double;

    auto findDocumentById(const std::string& id) -> Document;

    struct Compare {
        auto operator()(const std::pair<double, Document>& a,
                        const std::pair<double, Document>& b) const -> bool {
            return a.first < b.first;
        }
    };

    auto getRankedResults(const std::unordered_map<std::string, double>& scores)
        -> std::vector<Document>;
};
}  // namespace atom::search

#endif
