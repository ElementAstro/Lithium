/*
 * sqlite.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_SEARCH_SQLITE_HPP
#define ATOM_SEARCH_SQLITE_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

/**
 * @class SqliteDB
 * @brief A class for managing SQLite database operations using the Pimpl design
 * pattern.
 */
class SqliteDB {
public:
    /**
     * @brief Constructor
     * @param dbPath Path to the database file
     */
    explicit SqliteDB(std::string_view dbPath);

    /**
     * @brief Destructor
     */
    ~SqliteDB();

    /**
     * @brief Execute a SQL query
     * @param query SQL query string
     * @return Whether the query was executed successfully
     */
    bool executeQuery(std::string_view query);

    /**
     * @brief Query and retrieve data
     * @param query SQL query string
     */
    void selectData(std::string_view query);

    /**
     * @brief Retrieve an integer value
     * @param query SQL query string
     * @return Optional integer value (empty if query fails)
     */
    std::optional<int> getIntValue(std::string_view query);

    /**
     * @brief Retrieve a floating-point value
     * @param query SQL query string
     * @return Optional double value (empty if query fails)
     */
    std::optional<double> getDoubleValue(std::string_view query);

    /**
     * @brief Retrieve a text value
     * @param query SQL query string
     * @return Optional text value (empty if query fails)
     */
    std::optional<std::string> getTextValue(std::string_view query);

    /**
     * @brief Search for a specific item in the query results
     * @param query SQL query string
     * @param searchTerm Term to search for
     * @return Whether a matching item was found
     */
    bool searchData(std::string_view query, std::string_view searchTerm);

    /**
     * @brief Update data in the database
     * @param query SQL update statement
     * @return Whether the update was successful
     */
    bool updateData(std::string_view query);

    /**
     * @brief Delete data from the database
     * @param query SQL delete statement
     * @return Whether the delete was successful
     */
    bool deleteData(std::string_view query);

    /**
     * @brief Begin a database transaction
     * @return Whether the transaction was started successfully
     */
    bool beginTransaction();

    /**
     * @brief Commit a database transaction
     * @return Whether the transaction was committed successfully
     */
    bool commitTransaction();

    /**
     * @brief Rollback a database transaction
     * @return Whether the transaction was rolled back successfully
     */
    bool rollbackTransaction();

    /**
     * @brief Validate data against a specified query condition
     * @param query SQL query string
     * @param validationQuery Validation condition query string
     * @return Validation result
     */
    bool validateData(std::string_view query, std::string_view validationQuery);

    /**
     * @brief Perform paginated data query and retrieval
     * @param query SQL query string
     * @param limit Number of records per page
     * @param offset Offset for pagination
     */
    void selectDataWithPagination(std::string_view query, int limit,
                                  int offset);

    /**
     * @brief Set an error message callback function
     * @param errorCallback Error message callback function
     */
    void setErrorMessageCallback(
        const std::function<void(std::string_view)> &errorCallback);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl; /**< Pointer to implementation */
    mutable std::mutex mtx;      /**< Mutex for thread safety */

#if defined (TEST_F)
// Allow Mock class to access private members for testing
    friend class SqliteDBTest;
#endif
};

#endif  // ATOM_SEARCH_SQLITE_HPP
