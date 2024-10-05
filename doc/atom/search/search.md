# SearchEngine Class Documentation

## Overview

The `SearchEngine` class is part of the `atom::search` namespace and provides a versatile search functionality for documents. It supports various search methods including tag-based search, content-based search, boolean search, and autocomplete suggestions.

## Table of Contents

1. [Document Structure](#document-structure)
2. [SearchEngine Class](#searchengine-class)
3. [Adding Documents](#adding-documents)
4. [Search Methods](#search-methods)
   - [Tag-based Search](#tag-based-search)
   - [Content-based Search](#content-based-search)
   - [Boolean Search](#boolean-search)
   - [Autocomplete](#autocomplete)
5. [Usage Examples](#usage-examples)
6. [Best Practices](#best-practices)

## Document Structure

Before we dive into the `SearchEngine` class, let's look at the `Document` structure:

```cpp
struct Document {
    std::string id;
    std::string content;
    std::set<std::string> tags;
    int clickCount;

    explicit Document(std::string id, std::string content,
                      std::initializer_list<std::string> tags);
};
```

This structure represents a searchable document with an ID, content, tags, and a click count (used for adjusting search result weights).

## SearchEngine Class

The `SearchEngine` class provides the following public methods:

```cpp
class SearchEngine {
public:
    void addDocument(const Document& doc);
    std::vector<Document> searchByTag(const std::string& tag);
    std::vector<Document> fuzzySearchByTag(const std::string& tag, int tolerance);
    std::vector<Document> searchByTags(const std::vector<std::string>& tags);
    std::vector<Document> searchByContent(const std::string& query);
    std::vector<Document> booleanSearch(const std::string& query);
    std::vector<Document> autoComplete(const std::string& prefix);
};
```

## Adding Documents

To add documents to the search engine:

```cpp
void addDocument(const Document& doc);
```

This method adds a document to the search engine's index, making it searchable by tags and content.

## Search Methods

### Tag-based Search

1. Search by a single tag:

   ```cpp
   std::vector<Document> searchByTag(const std::string& tag);
   ```

2. Fuzzy search by tag (allows for misspellings):

   ```cpp
   std::vector<Document> fuzzySearchByTag(const std::string& tag, int tolerance);
   ```

3. Search by multiple tags:
   ```cpp
   std::vector<Document> searchByTags(const std::vector<std::string>& tags);
   ```

### Content-based Search

Search documents by their content:

```cpp
std::vector<Document> searchByContent(const std::string& query);
```

### Boolean Search

Perform a boolean search on document content:

```cpp
std::vector<Document> booleanSearch(const std::string& query);
```

### Autocomplete

Get autocomplete suggestions based on a prefix:

```cpp
std::vector<Document> autoComplete(const std::string& prefix);
```

## Usage Examples

Here are some examples of how to use the `SearchEngine` class:

```cpp
#include "search.hpp"
#include <iostream>

int main() {
    atom::search::SearchEngine engine;

    // Adding documents
    engine.addDocument(atom::search::Document("1", "C++ programming basics", {"programming", "cpp"}));
    engine.addDocument(atom::search::Document("2", "Python for data science", {"programming", "python", "data-science"}));
    engine.addDocument(atom::search::Document("3", "Introduction to algorithms", {"algorithms", "computer-science"}));

    // Searching by tag
    auto results = engine.searchByTag("programming");
    for (const auto& doc : results) {
        std::cout << "Found document: " << doc.id << " - " << doc.content << std::endl;
    }

    // Fuzzy search by tag
    results = engine.fuzzySearchByTag("programing", 1);  // Misspelled "programming"
    for (const auto& doc : results) {
        std::cout << "Found document (fuzzy): " << doc.id << " - " << doc.content << std::endl;
    }

    // Searching by content
    results = engine.searchByContent("data science");
    for (const auto& doc : results) {
        std::cout << "Found document by content: " << doc.id << " - " << doc.content << std::endl;
    }

    // Boolean search
    results = engine.booleanSearch("algorithms AND computer-science");
    for (const auto& doc : results) {
        std::cout << "Found document (boolean): " << doc.id << " - " << doc.content << std::endl;
    }

    // Autocomplete
    auto suggestions = engine.autoComplete("pro");
    for (const auto& suggestion : suggestions) {
        std::cout << "Autocomplete suggestion: " << suggestion << std::endl;
    }

    return 0;
}
```

## Best Practices

1. **Efficient Document Addition**: Add all documents to the search engine before performing any searches to ensure the index is fully built.

2. **Choosing Tags**: Use relevant and specific tags for documents to improve tag-based search accuracy.

3. **Fuzzy Search Tolerance**: When using fuzzy search, choose an appropriate tolerance level. A lower tolerance (e.g., 1 or 2) is usually sufficient for catching minor misspellings without introducing false positives.

4. **Boolean Search Queries**: For boolean searches, use clear AND, OR, NOT operators in your queries. For example: "python AND (data-science OR machine-learning)".

5. **Content Indexing**: Ensure that document content is properly tokenized and indexed for effective content-based searching.

6. **Autocomplete Performance**: The autocomplete feature can be resource-intensive for large datasets. Consider limiting the number of suggestions returned or implementing caching for frequent prefixes.

7. **Regular Index Updates**: If your document set changes frequently, make sure to update the search engine index regularly by adding new documents and removing obsolete ones.

8. **Click Count Utilization**: The `clickCount` field in the `Document` structure can be used to implement a relevance feedback mechanism. Consider updating this count when users interact with search results to improve future rankings.

9. **Error Handling**: Implement proper error handling, especially for methods that might throw exceptions (e.g., file I/O operations if you extend the class to support persistence).

10. **Performance Monitoring**: For large-scale applications, consider implementing performance monitoring to track search times, index sizes, and hit rates to optimize the search engine's performance.
