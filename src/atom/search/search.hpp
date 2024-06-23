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

class Document {
public:
    std::string id;
    std::string content;
    std::set<std::string> tags;
    int clickCount;  // 用于调整权重

    explicit Document(std::string id, std::string content,
             std::initializer_list<std::string> tags);
};

class SearchEngine {
private:
    std::unordered_map<std::string, std::vector<Document>> tagIndex;
    std::unordered_map<std::string, std::unordered_set<std::string>>
        contentIndex;
    std::unordered_map<std::string, int> docFrequency;
    int totalDocs = 0;

public:
    void addDocument(const Document& doc);

    void addContentToIndex(const Document& doc);

    std::vector<Document> searchByTag(const std::string& tag);

    std::vector<Document> fuzzySearchByTag(const std::string& tag,
                                           int tolerance);

    std::vector<Document> searchByTags(const std::vector<std::string>& tags);

    std::vector<Document> searchByContent(const std::string& query);

    std::vector<Document> booleanSearch(const std::string& query);

    std::vector<std::string> autoComplete(const std::string& prefix);

private:
    int levenshteinDistance(const std::string& s1, const std::string& s2);

    double tfIdf(const Document& doc, const std::string& term);

    Document findDocumentById(const std::string& id);

    struct Compare {
        bool operator()(const std::pair<double, Document>& a,
                        const std::pair<double, Document>& b) const {
            return a.first < b.first;
        }
    };

    std::vector<Document> getRankedResults(
        const std::unordered_map<std::string, double>& scores);
};
}  // namespace atom::search
