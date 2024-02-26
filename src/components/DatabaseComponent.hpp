
#ifndef EXAMPLE_JWT_DATABASECOMPONENT_HPP
#define EXAMPLE_JWT_DATABASECOMPONENT_HPP

#include "database/UserDb.hpp"
#include "database/StoryDb.hpp"

class DatabaseComponent
{
public:
  /**
   * Create database connection provider component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::provider::Provider<oatpp::sqlite::Connection>>, connectionPool)([] {

    /* Create database-specific ConnectionProvider */
    auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>("./sql/db.sqlite");

    /* Create database-specific ConnectionPool */
    return oatpp::sqlite::ConnectionPool::createShared(connectionProvider,
                                                       10 /* max-connections */,
                                                       std::chrono::seconds(5) /* connection TTL */);

  }());

  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::sqlite::Executor>, databaseExecutor)
  ([]
   {
     /* get connection pool component */
     OATPP_COMPONENT(std::shared_ptr<oatpp::provider::Provider<oatpp::sqlite::Connection>>, connectionPool);

     /* Create database-specific Executor */
     return std::make_shared<oatpp::sqlite::Executor>(connectionPool);
   }());

  /**
   * Create database client
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<UserDb>, userDb)
  ([]
   {
     /* get DB executor component */
     OATPP_COMPONENT(std::shared_ptr<oatpp::sqlite::Executor>, databaseExecutor);

     /* Create MyClient database client */
     return std::make_shared<UserDb>(databaseExecutor);
   }());

  /**
   * Create database client
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<StoryDb>, storyDb)
  ([]
   {
     /* get DB executor component */
     OATPP_COMPONENT(std::shared_ptr<oatpp::sqlite::Executor>, databaseExecutor);

     /* Create MyClient database client */
     return std::make_shared<StoryDb>(databaseExecutor);
   }());
};

#endif // EXAMPLE_JWT_DATABASECOMPONENT_HPP
