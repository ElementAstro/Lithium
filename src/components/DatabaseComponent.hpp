
#ifndef DATABASECOMPONENT_HPP
#define DATABASECOMPONENT_HPP

#include "database/UserDb.hpp"
#include "database/StarDb.hpp"

class DatabaseComponent
{
public:
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::sqlite::ConnectionPool>, connectionPool)
    ([]
     {
    oatpp::String connStr = "database.sqlite";

    /* Create database-specific ConnectionProvider */
    auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(connStr);

    /* Create database-specific ConnectionPool */
    return oatpp::sqlite::ConnectionPool::createShared(connectionProvider,
                                                           10 /* max-connections */,
                                                           std::chrono::seconds(5) /* connection TTL */); }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::sqlite::Executor>, databaseExecutor)
    ([]
     {
     /* get connection pool component */
     OATPP_COMPONENT(std::shared_ptr<oatpp::sqlite::ConnectionPool>, connectionPool);

     /* Create database-specific Executor */
     return std::make_shared<oatpp::sqlite::Executor>(connectionPool); }());

    /**
     * Create database client
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<UserDb>, userDb)
    ([]
     {
     /* get DB executor component */
     OATPP_COMPONENT(std::shared_ptr<oatpp::sqlite::Executor>, databaseExecutor);

     /* Create MyClient database client */
     return std::make_shared<UserDb>(databaseExecutor); }());
};

class StarDatabaseComponent
{
public:
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::sqlite::ConnectionPool>, connectionPool)
    ([]
     {
    oatpp::String connStr = "stardata.db";

    /* Create database-specific ConnectionProvider */
    auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(connStr);

    /* Create database-specific ConnectionPool */
    return oatpp::sqlite::ConnectionPool::createShared(connectionProvider,
                                                           10 /* max-connections */,
                                                           std::chrono::seconds(5) /* connection TTL */); }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::sqlite::Executor>, databaseExecutor)
    ([]
     {
     /* get connection pool component */
     OATPP_COMPONENT(std::shared_ptr<oatpp::sqlite::ConnectionPool>, connectionPool);

     /* Create database-specific Executor */
     return std::make_shared<oatpp::sqlite::Executor>(connectionPool); }());

    /**
     * Create database client
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<UserDb>, userDb)
    ([]
     {
     /* get DB executor component */
     OATPP_COMPONENT(std::shared_ptr<oatpp::sqlite::Executor>, databaseExecutor);

     /* Create MyClient database client */
     return std::make_shared<UserDb>(databaseExecutor); }());
};

#endif // DATABASECOMPONENT_HPP
