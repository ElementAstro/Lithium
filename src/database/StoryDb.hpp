
#ifndef EXAMPLE_JWT_STORYDB_HPP
#define EXAMPLE_JWT_STORYDB_HPP

#include "model/StoryModel.hpp"
#include "oatpp-sqlite/orm.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)  //<- Begin Codegen

/**
 * StoryDb client definitions.
 */
class StoryDb : public oatpp::orm::DbClient {
public:
    StoryDb(const std::shared_ptr<oatpp::orm::Executor>& executor)
        : oatpp::orm::DbClient(executor) {
        oatpp::orm::SchemaMigration migration(executor);
        migration.addFile(1 /* start from version 1 */, "./sql/story.sql");
        // TODO - Add more migrations here.
        migration
            .migrate();  // <-- run migrations. This guy will throw on error.

        auto version = executor->getSchemaVersion();
        OATPP_LOGD("UserDb", "Migration - OK. Version=%lld.", version);
    }

    QUERY(createStory,
          "INSERT INTO Stories"
          "(id, userid, content) VALUES "
          "(uuid_generate_v4(), :story.userid, :story.content) "
          "RETURNING *;",
          PREPARE(true),  // prepared statement!
          PARAM(oatpp::Object<StoryModel>, story))

    QUERY(updateStory,
          "UPDATE Stories "
          "SET "
          " content=:story.content, "
          "WHERE "
          " id=:story.id AND userid=:story.userid "
          "RETURNING *;",
          PREPARE(true),  // prepared statement!
          PARAM(oatpp::Object<StoryModel>, story))

    QUERY(getStoryByUserIdAndId,
          "SELECT * FROM Stories WHERE id=:id AND userid=:userId;",
          PREPARE(true),  // prepared statement!
          PARAM(oatpp::String, userId), PARAM(oatpp::String, id))

    QUERY(getAllUserStories,
          "SELECT * FROM Stories WHERE userid=:userId LIMIT :limit OFFSET "
          ":offset;",
          PREPARE(true),  // prepared statement!
          PARAM(oatpp::String, userId), PARAM(oatpp::UInt32, offset),
          PARAM(oatpp::UInt32, limit))

    QUERY(deleteStoryByUserIdAndId,
          "DELETE FROM Stories WHERE id=:id AND userid=:userId;",
          PREPARE(true),  // prepared statement!
          PARAM(oatpp::String, userId), PARAM(oatpp::String, id))
};

#include OATPP_CODEGEN_END(DbClient)  //<- End Codegen

#endif  // EXAMPLE_JWT_STORYDB_HPP
