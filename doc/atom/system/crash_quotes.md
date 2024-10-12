# Atom System Crash Quotes Library Documentation

This document provides a detailed explanation of the `Quote` and `QuoteManager` classes available in the `atom::system` namespace for managing quotes in crash reports.

## Table of Contents

1. [Quote Class](#quote-class)
2. [QuoteManager Class](#quotemanager-class)
   - [addQuote](#addquote)
   - [removeQuote](#removequote)
   - [displayQuotes](#displayquotes)
   - [shuffleQuotes](#shufflequotes)
   - [clearQuotes](#clearquotes)
   - [loadQuotesFromJson](#loadquotesfromjson)
   - [saveQuotesToJson](#savequotestojson)
   - [searchQuotes](#searchquotes)
   - [filterQuotesByAuthor](#filterquotesbyauthor)
   - [getRandomQuote](#getrandomquote)

## Quote Class

The `Quote` class represents a single quote with its text and author.

### Constructor

```cpp
explicit Quote(std::string text, std::string author);
```

Creates a new `Quote` object with the given text and author.

#### Parameters

- `text`: The text of the quote (string).
- `author`: The author of the quote (string).

### Methods

#### getText

```cpp
ATOM_NODISCARD auto getText() const -> std::string;
```

Returns the text of the quote.

#### getAuthor

```cpp
ATOM_NODISCARD auto getAuthor() const -> std::string;
```

Returns the author of the quote.

### Example Usage

```cpp
atom::system::Quote myQuote("To be or not to be, that is the question.", "William Shakespeare");
std::cout << "Quote: " << myQuote.getText() << std::endl;
std::cout << "Author: " << myQuote.getAuthor() << std::endl;
```

## QuoteManager Class

The `QuoteManager` class manages a collection of quotes and provides various operations on them.

### Methods

#### addQuote

```cpp
void addQuote(const Quote &quote);
```

Adds a quote to the collection.

##### Parameters

- `quote`: The quote to add (const reference to Quote object).

##### Example Usage

```cpp
atom::system::QuoteManager manager;
atom::system::Quote quote1("I think, therefore I am.", "René Descartes");
manager.addQuote(quote1);
```

#### removeQuote

```cpp
void removeQuote(const Quote &quote);
```

Removes a quote from the collection.

##### Parameters

- `quote`: The quote to remove (const reference to Quote object).

##### Example Usage

```cpp
atom::system::QuoteManager manager;
atom::system::Quote quote1("I think, therefore I am.", "René Descartes");
manager.addQuote(quote1);
manager.removeQuote(quote1);
```

#### displayQuotes

```cpp
void displayQuotes() const;
```

Displays all quotes in the collection. This method is only available in DEBUG mode.

##### Example Usage

```cpp
#ifdef DEBUG
atom::system::QuoteManager manager;
// Add some quotes...
manager.displayQuotes();
#endif
```

#### shuffleQuotes

```cpp
void shuffleQuotes();
```

Shuffles the quotes in the collection.

##### Example Usage

```cpp
atom::system::QuoteManager manager;
// Add some quotes...
manager.shuffleQuotes();
```

#### clearQuotes

```cpp
void clearQuotes();
```

Clears all quotes in the collection.

##### Example Usage

```cpp
atom::system::QuoteManager manager;
// Add some quotes...
manager.clearQuotes();
```

#### loadQuotesFromJson

```cpp
void loadQuotesFromJson(const std::string &filename);
```

Loads quotes from a JSON file.

##### Parameters

- `filename`: The name of the JSON file to load quotes from (string).

##### Example Usage

```cpp
atom::system::QuoteManager manager;
manager.loadQuotesFromJson("quotes.json");
```

#### saveQuotesToJson

```cpp
void saveQuotesToJson(const std::string &filename) const;
```

Saves quotes to a JSON file.

##### Parameters

- `filename`: The name of the JSON file to save quotes to (string).

##### Example Usage

```cpp
atom::system::QuoteManager manager;
// Add some quotes...
manager.saveQuotesToJson("quotes.json");
```

#### searchQuotes

```cpp
ATOM_NODISCARD auto searchQuotes(const std::string &keyword) const -> std::vector<Quote>;
```

Searches for quotes containing a keyword.

##### Parameters

- `keyword`: The keyword to search for (string).

##### Returns

A vector of `Quote` objects containing the keyword.

##### Example Usage

```cpp
atom::system::QuoteManager manager;
// Add some quotes...
std::vector<atom::system::Quote> results = manager.searchQuotes("life");
for (const auto& quote : results) {
    std::cout << quote.getText() << " - " << quote.getAuthor() << std::endl;
}
```

#### filterQuotesByAuthor

```cpp
ATOM_NODISCARD auto filterQuotesByAuthor(const std::string &author) const -> std::vector<Quote>;
```

Filters quotes by author.

##### Parameters

- `author`: The name of the author to filter by (string).

##### Returns

A vector of `Quote` objects by the specified author.

##### Example Usage

```cpp
atom::system::QuoteManager manager;
// Add some quotes...
std::vector<atom::system::Quote> shakespeareQuotes = manager.filterQuotesByAuthor("William Shakespeare");
for (const auto& quote : shakespeareQuotes) {
    std::cout << quote.getText() << std::endl;
}
```

#### getRandomQuote

```cpp
ATOM_NODISCARD auto getRandomQuote() const -> std::string;
```

Gets a random quote from the collection.

##### Returns

A random quote as a string.

##### Example Usage

```cpp
atom::system::QuoteManager manager;
// Add some quotes...
std::string randomQuote = manager.getRandomQuote();
std::cout << "Random quote: " << randomQuote << std::endl;
```

## Best Practices and Tips

1. **Error Handling**: Implement proper error handling for file operations in `loadQuotesFromJson` and `saveQuotesToJson` methods.

2. **Thread Safety**: If the `QuoteManager` is used in a multi-threaded environment, consider implementing thread-safe operations.

3. **Performance**: For large collections of quotes, consider using more efficient data structures or indexing for search and filter operations.

4. **Validation**: Implement input validation when adding quotes to ensure data integrity.

5. **Extensibility**: Consider adding support for additional metadata for quotes, such as categories or tags.

6. **Localization**: If your application supports multiple languages, consider implementing a localization system for quotes.

7. **Caching**: For frequently accessed quotes or operations, implement a caching mechanism to improve performance.
