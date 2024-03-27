# Quote and QuoteManager Classes Documentation

## Quote Class

Represents a quote with its text and author.

### Constructor

```cpp
explicit Quote(const std::string &text, const std::string &author);
```

### `getText()`

Returns the text of the quote.

```cpp
const std::string &getText() const;
```

### `getAuthor()`

Returns the author of the quote.

```cpp
const std::string &getAuthor() const;
```

## QuoteManager Class

Manages a collection of quotes.

### `addQuote()`

Adds a quote to the collection.

```cpp
void addQuote(const Quote &quote);
```

### `removeQuote()`

Removes a quote from the collection.

```cpp
void removeQuote(const Quote &quote);
```

### `shuffleQuotes()`

Shuffles the quotes in the collection.

### `clearQuotes()`

Clears all quotes in the collection.

### `loadQuotesFromFile()`

Loads quotes from a file into the collection.

```cpp
void loadQuotesFromFile(const std::string &filename);
```

### `saveQuotesToFile()`

Saves quotes in the collection to a file.

```cpp
void saveQuotesToFile(const std::string &filename) const;
```

### `searchQuotes()`

Searches for quotes containing a keyword and returns a vector of matching quotes.

```cpp
std::vector<Quote> searchQuotes(const std::string &keyword) const;
```

### `filterQuotesByAuthor()`

Filters quotes by author and returns a vector of quotes by the specified author.

```cpp
std::vector<Quote> filterQuotesByAuthor(const std::string &author) const;
```

### `getRandomQuote()`

Gets a random quote from the collection.

```cpp
std::string getRandomQuote() const;
```

### Debug Method (Conditional Compilation)

```cpp
#ifdef DEBUG
void displayQuotes() const;
#endif
```

This method is only available when compiled in debug mode. It displays all quotes in the collection.

### Example Usage

```cpp
// Create a new quote
Quote quote("To be or not to be", "William Shakespeare");

// Add the quote to the QuoteManager
QuoteManager manager;
manager.addQuote(quote);

// Save quotes to a file
manager.saveQuotesToFile("quotes.txt");

// Load quotes from a file
manager.loadQuotesFromFile("quotes.txt");

// Search quotes by keyword
auto searchResults = manager.searchQuotes("be");

// Filter quotes by author
auto shakespeareQuotes = manager.filterQuotesByAuthor("William Shakespeare");

// Get a random quote
std::string randomQuote = manager.getRandomQuote();
```
