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

#include "QueryResult.hpp"

namespace oatpp { namespace sqlite {

QueryResult::QueryResult(sqlite3_stmt* stmt,
                         const provider::ResourceHandle<orm::Connection>& connection,
                         const std::shared_ptr<mapping::ResultMapper>& resultMapper,
                         const std::shared_ptr<const data::mapping::TypeResolver>& typeResolver)
  : m_stmt(stmt)
  , m_connection(connection)
  , m_resultMapper(resultMapper)
  , m_resultData(stmt, typeResolver)
{
  auto sqliteConn = std::static_pointer_cast<Connection>(m_connection.object);
  m_errorMessage = sqlite3_errmsg(sqliteConn->getHandle());
}

QueryResult::~QueryResult() {
  sqlite3_finalize(m_stmt);
}

provider::ResourceHandle<orm::Connection> QueryResult::getConnection() const {
  return m_connection;
}

bool QueryResult::isSuccess() const {
  return m_resultData.isSuccess;
}

oatpp::String QueryResult::getErrorMessage() const {
  return m_errorMessage;
}

v_int64 QueryResult::getPosition() const {
  return m_resultData.rowIndex;
}

v_int64 QueryResult::getKnownCount() const {
  return -1;
}

bool QueryResult::hasMoreToFetch() const {
  return m_resultData.hasMore;
}

oatpp::Void QueryResult::fetch(const oatpp::Type* const type, v_int64 count) {
  return m_resultMapper->readRows(&m_resultData, type, count);
}

}}
