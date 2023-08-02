#ifndef STARDB_HPP
#define STARDB_HPP

#include "data/StarDto.hpp"
#include "oatpp-sqlite/orm.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * StarDb client definitions.
 */
class StarDb : public oatpp::orm::DbClient
{
public:
    StarDb(const std::shared_ptr<oatpp::orm::Executor> &executor)
        : oatpp::orm::DbClient(executor)
    {

        oatpp::orm::SchemaMigration migration(executor);
        //migration.addFile(1 /* start from version 1 */, DATABASE_MIGRATIONS "/001_init.sql");
        // TODO - Add more migrations here.
        //migration.migrate(); // <-- run migrations. This guy will throw on error.

        auto version = executor->getSchemaVersion();
        OATPP_LOGD("StarDb", "Migration - OK. Version=%lld.", version);
    }

    QUERY(selectData,
      "SELECT %s "
      "FROM %s "
      "WHERE %s"
      "%s",
      PARAM(oatpp::Object<StarDto>, star))  


};

#include OATPP_CODEGEN_END(DbClient) //<- End Codegen

#endif // STARDB_HPP
