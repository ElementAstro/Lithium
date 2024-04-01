# SqliteDB Class Documentation

## Overview

The `SqliteDB` class provides methods to interact with an SQLite database. It encapsulates functionality for executing queries, fetching data, updating records, and managing database transactions.

### Constructor

```cpp
SqliteDB(const char *dbPath);
```

#### Usage Example

```cpp
SqliteDB db("path/to/database.db");
```

---

## executeQuery Method

Executes a query on the database.

```cpp
bool executeQuery(const char *query);
```

#### Usage Example

```cpp
bool success = db.executeQuery("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)");
// Expected output: true if query execution is successful
```

---

## selectData Method

Executes a SELECT query and fetches the data.

```cpp
void selectData(const char *query);
```

#### Usage Example

```cpp
db.selectData("SELECT * FROM users");
// Expected output: Fetches and processes the data from the 'users' table
```

---

## getIntValue Method

Fetches an integer value from the result of a query.

```cpp
int getIntValue(const char *query);
```

#### Usage Example

```cpp
int userId = db.getIntValue("SELECT id FROM users WHERE name='John'");
// Expected output: Integer value representing the user ID
```

---

## getDoubleValue Method

Fetches a double value from the result of a query.

```cpp
double getDoubleValue(const char *query);
```

#### Usage Example

```cpp
double averageAge = db.getDoubleValue("SELECT AVG(age) FROM users");
// Expected output: Average age as a double value
```

---

## getTextValue Method

Fetches a text value from the result of a query.

```cpp
const unsigned char *getTextValue(const char *query);
```

#### Usage Example

```cpp
const unsigned char *userName = db.getTextValue("SELECT name FROM users WHERE id=1");
// Expected output: Text value representing the user's name
```

---

## searchData Method

Searches for a specified term in the query result.

```cpp
bool searchData(const char *query, const char *searchTerm);
```

#### Usage Example

```cpp
bool found = db.searchData("SELECT * FROM users", "John");
// Expected output: true if 'John' is found in the search results
```

---

## updateData Method

Updates data in the database.

```cpp
bool updateData(const char *query);
```

#### Usage Example

```cpp
bool success = db.updateData("UPDATE users SET age=30 WHERE name='John'");
// Expected output: true if the update operation is successful
```

---

## deleteData Method

Deletes data from the database.

```cpp
bool deleteData(const char *query);
```

#### Usage Example

```cpp
bool success = db.deleteData("DELETE FROM users WHERE age>60");
// Expected output: true if the delete operation is successful
```

---

## beginTransaction, commitTransaction, rollbackTransaction Methods

Methods for managing database transactions.

```cpp
bool beginTransaction();
bool commitTransaction();
bool rollbackTransaction();
```

#### Usage Example

```cpp
db.beginTransaction();
// Perform multiple database operations
db.commitTransaction();  // or db.rollbackTransaction() if needed
```

---

## handleSQLError Method

Handles SQLite errors.

```cpp
void handleSQLError();
```

#### Note

This method is called internally to handle SQLite errors within the class.

---

## validateData Method

Validates data based on a specific query condition.

```cpp
bool validateData(const char *query, const char *validationQuery);
```

#### Usage Example

```cpp
bool valid = db.validateData("SELECT * FROM users WHERE age > 18", "SELECT COUNT(*) FROM users");
// Expected output: true if the data meets the validation condition
```

---

## selectDataWithPagination Method

Performs pagination on the query result and fetches data.

```cpp
void selectDataWithPagination(const char *query, int limit, int offset);
```

#### Usage Example

```cpp
db.selectDataWithPagination("SELECT * FROM users", 10, 0);
// Expected output: Fetches the first 10 records from the 'users' table
```

---

## setErrorMessageCallback Method

Sets an error message callback function.

```cpp
void setErrorMessageCallback(const std::function<void(const char *)> &errorCallback);
```

#### Usage Example

```cpp
db.setErrorMessageCallback([](const char *errorMessage) {
    std::cerr << "SQLite Error: " << errorMessage << std::endl;
});
```

#### Note

This method allows setting a custom error message handler for SQLite errors.
