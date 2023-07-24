/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "Executor.hpp"

#include "ql_template/Parser.hpp"
#include "ql_template/TemplateValueProvider.hpp"

#include "QueryResult.hpp"
#include "Types.hpp"

#include "oatpp/orm/Transaction.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include <vector>

namespace oatpp { namespace sqlite {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class VersionRow : public oatpp::DTO {

  DTO_INIT(VersionRow, DTO);

  DTO_FIELD(Int64, version);

};

#include OATPP_CODEGEN_END(DTO)

}

void Executor::ConnectionInvalidator::invalidate(const std::shared_ptr<orm::Connection>& connection) {
  auto c = std::static_pointer_cast<Connection>(connection);
  auto invalidator = c->getInvalidator();
  if(!invalidator) {
    throw std::runtime_error("[oatpp::sqlite::Executor::ConnectionInvalidator::invalidate()]: Error. "
                             "Connection invalidator was NOT set.");
  }
  invalidator->invalidate(c);
}

Executor::Executor(const std::shared_ptr<provider::Provider<Connection>>& connectionProvider)
  : m_connectionInvalidator(std::make_shared<ConnectionInvalidator>())
  , m_connectionProvider(connectionProvider)
  , m_resultMapper(std::make_shared<mapping::ResultMapper>())
{
  m_defaultTypeResolver->addKnownClasses({
    Blob::Class::CLASS_ID
  });
}

std::shared_ptr<data::mapping::TypeResolver> Executor::createTypeResolver() {
  auto typeResolver = std::make_shared<data::mapping::TypeResolver>();
  typeResolver->addKnownClasses({
    Blob::Class::CLASS_ID
  });
  return typeResolver;
}

data::share::StringTemplate Executor::parseQueryTemplate(const oatpp::String& name,
                                                         const oatpp::String& text,
                                                         const ParamsTypeMap& paramsTypeMap,
                                                         bool prepare)
{

  (void) paramsTypeMap;

  auto&& t = ql_template::Parser::parseTemplate(text);

  auto extra = std::make_shared<ql_template::Parser::TemplateExtra>();
  t.setExtraData(extra);

  extra->prepare = prepare;
  extra->templateName = name;
  ql_template::TemplateValueProvider valueProvider;
  extra->preparedTemplate = t.format(&valueProvider);

  return t;

}

provider::ResourceHandle<orm::Connection> Executor::getConnection() {
  auto connection = m_connectionProvider->get();
  if(connection) {
    /* set correct invalidator before cast */
    connection.object->setInvalidator(connection.invalidator);
    return provider::ResourceHandle<orm::Connection>(
      connection.object,
      m_connectionInvalidator
    );
  }
  throw std::runtime_error("[oatpp::sqlite::Executor::getConnection()]: Error. Can't connect.");
}

Executor::QueryParameter Executor::parseQueryParameter(const oatpp::String& paramName) {

  parser::Caret caret(paramName);
  auto nameLabel = caret.putLabel();
  if(caret.findChar('.') && caret.getPosition() < caret.getDataSize() - 1) {

    QueryParameter result;
    result.name = nameLabel.toString();

    do {

      caret.inc();
      auto label = caret.putLabel();
      caret.findChar('.');
      result.propertyPath.push_back(label.std_str());

    } while (caret.getPosition() < caret.getDataSize());

    return result;

  }

  return {nameLabel.toString(), {}};

}

void Executor::bindParams(sqlite3_stmt* stmt,
                          const StringTemplate& queryTemplate,
                          const std::unordered_map<oatpp::String, oatpp::Void>& params,
                          const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver)
{

  data::mapping::TypeResolver::Cache cache;

  auto extra = std::static_pointer_cast<ql_template::Parser::TemplateExtra>(queryTemplate.getExtraData());

  oatpp::String queryName = extra->templateName;
  if(!queryName) {
    queryName = "UnNamed";
  }

  auto count = queryTemplate.getTemplateVariables().size();

  for(v_uint32 i = 0; i < count; i ++) {
    const auto& var = queryTemplate.getTemplateVariables()[i];
    auto it = params.find(var.name);

    auto queryParameter = parseQueryParameter(var.name);
    if(queryParameter.name) {

      it = params.find(queryParameter.name);
      if(it != params.end()) {
        auto value = typeResolver->resolveObjectPropertyValue(it->second, queryParameter.propertyPath, cache);
        if(value.getValueType()->classId.id == oatpp::Void::Class::CLASS_ID.id) {
          throw std::runtime_error("[oatpp::sqlite::Executor::QueryParams::QueryParams()]: "
                                   "Error."
                                   " Query '" + *queryName +
                                   "', parameter '" + *var.name +
                                   "' - property not found or its type is unknown.");
        }
        m_serializer.serialize(stmt, i + 1, value);
        continue;
      }

    }

    throw std::runtime_error("[oatpp::sqlite::Executor::bindParams()]: "
                             "Error. Parameter not found " + *var.name);

  }

}

std::shared_ptr<orm::QueryResult> Executor::execute(const StringTemplate& queryTemplate,
                                                    const std::unordered_map<oatpp::String, oatpp::Void>& params,
                                                    const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver,
                                                    const provider::ResourceHandle<orm::Connection>& connection)
{

  auto conn = connection;
  if(!conn) {
    conn = getConnection();
  }

  std::shared_ptr<const data::mapping::TypeResolver> tr = typeResolver;
  if(!tr) {
    tr = m_defaultTypeResolver;
  }

  auto sqliteConn = std::static_pointer_cast<sqlite::Connection>(conn.object);

  auto extra = std::static_pointer_cast<ql_template::Parser::TemplateExtra>(queryTemplate.getExtraData());

  sqlite3_stmt* stmt = nullptr;
  auto res = sqlite3_prepare_v2(sqliteConn->getHandle(),
                                extra->preparedTemplate->c_str(),
                                extra->preparedTemplate->size(),
                                &stmt,
                                nullptr);

  (void) res; // TODO check for res

  bindParams(stmt, queryTemplate, params, tr);

  return std::make_shared<QueryResult>(stmt, conn, m_resultMapper, tr);

}

std::shared_ptr<orm::QueryResult> Executor::exec(const oatpp::String& statement,
                                                 const provider::ResourceHandle<orm::Connection>& connection)
{

  auto conn = connection;
  if(!conn) {
    conn = getConnection();
  }

  auto sqliteConn = std::static_pointer_cast<sqlite::Connection>(conn.object);
  sqlite3_stmt* stmt;
  auto res = sqlite3_prepare_v2(sqliteConn->getHandle(), statement->c_str(), -1, &stmt, nullptr);
  (void) res; // TODO - check for res
  return std::make_shared<QueryResult>(stmt, conn, m_resultMapper, m_defaultTypeResolver);

}

std::shared_ptr<orm::QueryResult> Executor::begin(const provider::ResourceHandle<orm::Connection>& connection) {
  return exec("BEGIN", connection);
}

std::shared_ptr<orm::QueryResult> Executor::commit(const provider::ResourceHandle<orm::Connection>& connection) {
  if(!connection) {
    throw std::runtime_error("[oatpp::sqlite::Executor::commit()]: "
                             "Error. Can't COMMIT - NULL connection.");
  }
  return exec("COMMIT", connection);
}

std::shared_ptr<orm::QueryResult> Executor::rollback(const provider::ResourceHandle<orm::Connection>& connection) {
  if(!connection) {
    throw std::runtime_error("[oatpp::sqlite::Executor::commit()]: "
                             "Error. Can't ROLLBACK - NULL connection.");
  }
  return exec("ROLLBACK", connection);
}

oatpp::String Executor::getSchemaVersionTableName(const oatpp::String& suffix) {
  data::stream::BufferOutputStream stream;
  stream << "oatpp_schema_version";
  if (suffix && suffix->size() > 0) {
    stream << "_" << suffix;
  }
  return stream.toString();
}

std::shared_ptr<orm::QueryResult> Executor::updateSchemaVersion(v_int64 newVersion,
                                                                const oatpp::String& suffix,
                                                                const provider::ResourceHandle<orm::Connection>& connection)
{
  data::stream::BufferOutputStream stream;
  stream
  << "UPDATE "
  << getSchemaVersionTableName(suffix) << " "
  << "SET version=" << newVersion << ";";
  return exec(stream.toString(), connection);
}

v_int64 Executor::getSchemaVersion(const oatpp::String& suffix,
                                   const provider::ResourceHandle<orm::Connection>& connection)
{

  std::shared_ptr<orm::QueryResult> result;

  {
    data::stream::BufferOutputStream stream;
    stream << "CREATE TABLE IF NOT EXISTS " << getSchemaVersionTableName(suffix) << " (version BIGINT)";
    result = exec(stream.toString(), connection);
    if(!result->isSuccess()) {
      throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                               "Error. Can't create schema version table. " + result->getErrorMessage());
    }
  }

  data::stream::BufferOutputStream stream;
  stream << "SELECT * FROM " << getSchemaVersionTableName(suffix);
  result = exec(stream.toString(), result->getConnection());
  if(!result->isSuccess()) {
    throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                             "Error. Can't get schema version. " + result->getErrorMessage());
  }

  auto rows = result->fetch<oatpp::Vector<oatpp::Object<VersionRow>>>();

  if(rows->size() == 0) {

    stream.setCurrentPosition(0);
    stream << "INSERT INTO " << getSchemaVersionTableName(suffix) << " (version) VALUES (0)";
    result = exec(stream.toString(), result->getConnection());

    if(result->isSuccess()) {
      return 0;
    }

    throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                             "Error. Can't init schema version. " + result->getErrorMessage());

  } else if(rows->size() == 1) {

    auto row = rows[0];
    if(!row->version) {
      throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                               "Error. The schema version table is corrupted - version is null.");
    }

    return row->version;

  }

  throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                           "Error. The schema version table is corrupted - multiple version rows.");

}

void Executor::migrateSchema(const oatpp::String& script,
                             v_int64 newVersion,
                             const oatpp::String& suffix,
                             const provider::ResourceHandle<orm::Connection>& connection)
{

  if(!script) {
    throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Script is null.");
  }

  if(!connection) {
    throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Connection is null.");
  }

  auto currVersion = getSchemaVersion(suffix, connection);
  if(newVersion <= currVersion) {
    return;
  }

  if(newVersion > currVersion + 1) {
    throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. +1 version increment is allowed only.");
  }

  if(script->size() == 0) {
    OATPP_LOGW("[oatpp::sqlite::Executor::migrateSchema()]", "Warning. Executing empty script for version %d", newVersion);
  }

  {

    auto nativeConnection = std::static_pointer_cast<sqlite::Connection>(connection.object);
    orm::Transaction transaction(this, connection);

    char* errmsg = nullptr;
    sqlite3_exec(nativeConnection->getHandle(), script->c_str(), nullptr, nullptr, &errmsg);

    if(errmsg) {
      OATPP_LOGE("[oatpp::sqlite::Executor::migrateSchema()]", "Error. Migration failed for version %d. %s", newVersion, errmsg);
      throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Migration failed. " + std::string((const char*) errmsg));
    }

    std::shared_ptr<orm::QueryResult> result;
    result = updateSchemaVersion(newVersion, suffix, connection);

    if(!result->isSuccess() || result->hasMoreToFetch() > 0) {
      throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Migration failed. Can't set new version.");
    }

    result = transaction.commit();
    if(!result->isSuccess()) {
      throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Migration failed. Can't commit.");
    }

  }

}

}}
