# Matching Strategies Library

## MatchStrategy Class

Abstract base class for matching strategies.

### Usage Example

```cpp
MatchStrategy *strategy = new FuzzyMatch();
std::vector<std::string> result = strategy->match("query", index);
// Expected output: Vector of matched strings
```

---

## FuzzyMatch Class

Fuzzy matching strategy based on edit distance.

### Usage Example

```cpp
FuzzyMatch fuzzy;
std::vector<std::string> result = fuzzy.match("query", index, 3);
// Expected output: Vector of matched strings
```

---

## RegexMatch Class

Regular expression matching strategy.

### Usage Example

```cpp
RegexMatch regex;
std::vector<std::string> result = regex.match("query", index, 0);
// Expected output: Vector of matched strings
```

Note: Matching threshold is not used in this strategy.

---

## HammingMatch Class

Hamming distance matching strategy.

### Usage Example

```cpp
HammingMatch hamming(2);
std::vector<std::string> result = hamming.match("query", index, 0);
// Expected output: Vector of matched strings
```

Note: Maximum Hamming distance allowed for a match is specified during object construction.

---

## TfIdfMatch Class

TF-IDF matching strategy.

### Usage Example

```cpp
std::vector<std::string> data = {"data1", "data2"};
TfIdfMatch tfidf(data);
std::vector<std::string> result = tfidf.match("query", index, 0);
// Expected output: Vector of matched strings
```

---

## SearchEngine Class

Search engine class that uses a specific matching strategy.

### Usage Example

```cpp
std::vector<std::string> data = {"data1", "data2"};
SearchEngine engine(data, new FuzzyMatch());
engine.setMatchStrategy(new RegexMatch());
engine.addData("newData");
std::vector<std::string> result = engine.search("query", 3);
// Expected output: Vector of matched strings
```
