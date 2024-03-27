/*
 * mysql.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-6

Description: Simple Mysql wrapper

**************************************************/

#ifndef ATOM_SEARCH_MYSQL_HPP
#define ATOM_SEARCH_MYSQL_HPP

#include <mariadb/mysql.h>
#include <functional>

/**
 * @class MysqlDB
 * @brief This class provides functionalities to interact with a MySQL database.
 *
 * It offers methods to execute queries, retrieve data, and manage transactions.
 */
class MysqlDB {
public:
    /**
     * @brief Constructor that initializes a connection to a MySQL database.
     * @param host The hostname or IP address of the MySQL server.
     * @param user The username for the MySQL database.
     * @param password The password for the MySQL database.
     * @param database The name of the database to connect to.
     */
    explicit MysqlDB(const char *host, const char *user, const char *password,
            const char *database);

    /**
     * @brief Destructor that closes the database connection.
     */
    ~MysqlDB();

    /**
     * @brief Executes a given SQL query.
     * @param query The SQL query string to be executed.
     * @return True if the query was successfully executed, false otherwise.
     */
    bool executeQuery(const char *query);

    /**
     * @brief Selects data from the database using a given query and prints the result.
     * @param query The SQL select query to be executed.
     */
    void selectData(const char *query);

    /**
     * @brief Retrieves an integer value from the first row in the query result.
     * @param query The SQL query to be executed.
     * @return The integer value from the query result.
     */
    int getIntValue(const char *query);

    /**
     * @brief Retrieves a double value from the first row in the query result.
     * @param query The SQL query to be executed.
     * @return The double value from the query result.
     */
    double getDoubleValue(const char *query);

    /**
     * @brief Retrieves a text value from the first row in the query result.
     * @param query The SQL query to be executed.
     * @return The text value from the query result. Note: the caller must ensure the returned pointer is used appropriately to avoid memory leaks or dangling pointers.
     */
    const char *getTextValue(const char *query);

    /**
     * @brief Searches for a term within the result of a given query.
     * @param query The SQL query to be executed.
     * @param searchTerm The term to search for in the query result.
     * @return True if the search term is found, false otherwise.
     */
    bool searchData(const char *query, const char *searchTerm);

    /**
     * @brief Updates data in the database using a given query.
     * @param query The SQL update query to be executed.
     * @return True if the data was successfully updated, false otherwise.
     */
    bool updateData(const char *query);

    /**
     * @brief Deletes data from the database using a given query.
     * @param query The SQL delete query to be executed.
     * @return True if the data was successfully deleted, false otherwise.
     */
    bool deleteData(const char *query);

    /**
     * @brief Begins a new transaction.
     * @return True if the transaction was successfully started, false otherwise.
     */
    bool beginTransaction();

    /**
     * @brief Commits the current transaction.
     * @return True if the transaction was successfully committed, false otherwise.
     */
    bool commitTransaction();

    /**
     * @brief Rolls back the current transaction.
     * @return True if the transaction was successfully rolled back, false otherwise.
     */
    bool rollbackTransaction();

    /**
     * @brief Handles any MySQL errors by printing the error message.
     */
    void handleMySQLError();

    /**
     * @brief Validates data using a provided query and a validation query.
     * @param query The SQL query to validate the data.
     * @param validationQuery The SQL query used for validation.
     * @return True if the data is valid according to the validation query, false otherwise.
     */
    bool validateData(const char *query, const char *validationQuery);

    /**
     * @brief Selects data with pagination support.
     * @param query The SQL select query to be executed.
     * @param limit The maximum number of rows to return.
     * @param offset The offset of the first row to return.
     */
    void selectDataWithPagination(const char *query, int limit, int offset);

    /**
     * @brief Sets a callback function for handling error messages.
     * @param errorCallback A function to be called when an error occurs, receiving the error message as a parameter.
     */
    void setErrorMessageCallback(
        const std::function<void(const char *)> &errorCallback);

private:
    MYSQL *db; ///< Pointer to the MySQL connection handle.

private:
    std::function<void(const char *)> errorCallback; ///< The callback function for error messages.
};

#endif
