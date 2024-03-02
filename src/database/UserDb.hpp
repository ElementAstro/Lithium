
#ifndef EXAMPLE_JWT_USERDB_HPP
#define EXAMPLE_JWT_USERDB_HPP

#include "data/UserDto.hpp"
#include "model/UserModel.hpp"
#include "oatpp-sqlite/orm.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)  //<- Begin Codegen

/**
 * UserDb client definitions.
 */
class UserDb : public oatpp::orm::DbClient {
public:
    UserDb(const std::shared_ptr<oatpp::orm::Executor>& executor)
        : oatpp::orm::DbClient(executor) {
        oatpp::orm::SchemaMigration migration(executor);
        migration.addFile(1 /* start from version 1 */, "./sql/user.sql");
        // TODO - Add more migrations here.
        migration
            .migrate();  // <-- run migrations. This guy will throw on error.

        auto version = executor->getSchemaVersion();
        OATPP_LOGD("UserDb", "Migration - OK. Version=%lld.", version);
    }

    QUERY(createUser,
          "INSERT INTO AppUser"
          "(username, email, password, role) VALUES "
          "(:user.username, :user.email, :user.password, :user.role);",
          PARAM(oatpp::Object<UserDto>, user))

    QUERY(updateUser,
          "UPDATE AppUser "
          "SET "
          " username=:user.username, "
          " email=:user.email, "
          " password=:user.password, "
          " role=:user.role "
          "WHERE "
          " id=:user.id;",
          PARAM(oatpp::Object<UserDto>, user))

    QUERY(getUserById, "SELECT * FROM AppUser WHERE id=:id;",
          PARAM(oatpp::Int32, id))

    QUERY(getAllUsers, "SELECT * FROM AppUser LIMIT :limit OFFSET :offset;",
          PARAM(oatpp::UInt32, offset), PARAM(oatpp::UInt32, limit))

    QUERY(deleteUserById, "DELETE FROM AppUser WHERE id=:id;",
          PARAM(oatpp::Int32, id))

    QUERY(changeUserPassword,
          "UPDATE users "
          "SET "
          " pswhash=crypt(:newPassword, gen_salt('bf', 8)) "
          "WHERE "
          " id=:id AND pswhash=crypt(:oldPassword, pswhash);",
          PREPARE(true),  // prepared statement!
          PARAM(oatpp::String, userId, "id"), PARAM(oatpp::String, oldPassword),
          PARAM(oatpp::String, newPassword))

    QUERY(authenticateUser,
          "SELECT id FROM users WHERE username=:username AND "
          "pswhash=crypt(:password, pswhash);",
          PREPARE(true),  // prepared statement!
          PARAM(oatpp::String, username), PARAM(oatpp::String, password))
};

#include OATPP_CODEGEN_END(DbClient)  //<- End Codegen

#endif  // EXAMPLE_JWT_USERDB_HPP
