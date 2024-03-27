# MysqlDB Class Documentation

## Introduction

The `MysqlDB` class provides functionalities to interact with a MySQL database. It allows executing queries, retrieving data, and managing transactions.

### Example Usage

```cpp
// Instantiate MysqlDB object
MysqlDB db("localhost", "root", "password", "my_database");
```

## Constructor

### Method Signature

```cpp
explicit MysqlDB(const char *host, const char *user, const char *password, const char *database);
```

### Usage Example

```cpp
MysqlDB db("localhost", "root", "password", "my_database");
```

### Expected Output

No output if successful. May throw an exception if connection fails.

## Destructor

### Method Signature

```cpp
~MysqlDB();
```

### Usage Example

```cpp
// Destructor is called automatically when object goes out of scope
```

### Expected Output

Closes the database connection.

## executeQuery

### Method Signature

```cpp
bool executeQuery(const char *query);
```

### Usage Example

```cpp
bool success = db.executeQuery("INSERT INTO table_name (column1, column2) VALUES (value1, value2)");
```

### Expected Output

Returns true if the query was successfully executed, false otherwise.

## selectData

### Method Signature

```cpp
void selectData(const char *query);
```

### Usage Example

```cpp
db.selectData("SELECT * FROM table_name");
```

### Expected Output

Prints the result of the SELECT query.

## getIntValue

### Method Signature

```cpp
int getIntValue(const char *query);
```

### Usage Example

```cpp
int result = db.getIntValue("SELECT COUNT(*) FROM table_name");
```

### Expected Output

Returns the integer value from the query result.

## getDoubleValue

### Method Signature

```cpp
double getDoubleValue(const char *query);
```

### Usage Example

```cpp
double result = db.getDoubleValue("SELECT AVG(salary) FROM employee_table");
```

### Expected Output

Returns the double value from the query result.

## getTextValue

### Method Signature

```cpp
const char *getTextValue(const char *query);
```

### Usage Example

```cpp
const char *text = db.getTextValue("SELECT name FROM user_table WHERE id=1");
```

### Expected Output

Returns the text value from the query result. Note Caller must handle memory management appropriately.

## searchData

### Method Signature

```cpp
bool searchData(const char *query, const char *searchTerm);
```

### Usage Example

```cpp
bool found = db.searchData("SELECT title FROM article_table", "MySQL");
```

### Expected Output

Returns true if the search term is found in the query result, false otherwise.

## updateData

### Method Signature

```cpp
bool updateData(const char *query);
```

### Usage Example

```cpp
bool success = db.updateData("UPDATE employee_table SET salary=60000 WHERE id=123");
```

### Expected Output

Returns true if the data was successfully updated, false otherwise.

## deleteData

### Method Signature

```cpp
bool deleteData(const char *query);
```

### Usage Example

```cpp
bool success = db.deleteData("DELETE FROM student_table WHERE graduation_year<2020");
```

### Expected Output

Returns true if the data was successfully deleted, false otherwise.

## beginTransaction

### Method Signature

```cpp
bool beginTransaction();
```

### Usage Example

```cpp
bool success = db.beginTransaction();
```

### Expected Output

Returns true if the transaction was successfully started, false otherwise.

## commitTransaction

### Method Signature

```cpp
bool commitTransaction();
```

### Usage Example

```cpp
bool success = db.commitTransaction();
```

### Expected Output

Returns true if the transaction was successfully committed, false otherwise.

## rollbackTransaction

### Method Signature

```cpp
bool rollbackTransaction();
```

### Usage Example

```cpp
bool success = db.rollbackTransaction();
```

### Expected Output

Returns true if the transaction was successfully rolled back, false otherwise.

## handleMySQLError

### Method Signature

```cpp
void handleMySQLError();
```

### Usage Example

```cpp
// Typically called internally to handle errors
```

### Expected Output

Prints the error message if there is any MySQL error.

## validateData

### Method Signature

```cpp
bool validateData(const char *query, const char *validationQuery);
```

### Usage Example

```cpp
bool isValid = db.validateData("SELECT * FROM user_table WHERE id=1", "SELECT COUNT(*) FROM user_table");
```

### Expected Output

Returns true if the data is valid according to the validation query, false otherwise.

## selectDataWithPagination

### Method Signature

```cpp
void selectDataWithPagination(const char *query, int limit, int offset);
```

### Usage Example

```cpp
db.selectDataWithPagination("SELECT * FROM large_table", 10, 0);
```

### Expected Output

Prints the result of the SELECT query with pagination support.

## setErrorMessageCallback

### Method Signature

```cpp
void setErrorMessageCallback(const stdfunction<void(const char *)> &errorCallback);
```

### Usage Example

```cpp
db.setErrorMessageCallback([](const char *errorMessage) {
    stdcerr << "Error " << errorMessage << stdendl;
});
```

### Expected Output

Sets a callback function for handling error messages.

## Private Members

- `MYSQL *db` Pointer to the MySQL connection handle.
- `stdfunction<void(const char *)> errorCallback` The callback function for error messages.
