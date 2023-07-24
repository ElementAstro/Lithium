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

#include "ConnectionProvider.hpp"

namespace oatpp { namespace sqlite {

void ConnectionProvider::ConnectionInvalidator::invalidate(const std::shared_ptr<Connection> &connection) {
  (void) connection;
  // DO nothing
}

ConnectionProvider::ConnectionProvider(const oatpp::String& connectionString)
  : m_invalidator(std::make_shared<ConnectionInvalidator>())
  , m_connectionString(connectionString)
{}

provider::ResourceHandle<Connection> ConnectionProvider::get() {

  sqlite3* handle;
  auto res = sqlite3_open(m_connectionString->c_str(), &handle);
  auto connection = std::make_shared<ConnectionImpl>(handle);

  if(res != SQLITE_OK) {
    std::string errMsg = sqlite3_errmsg(handle);
    throw std::runtime_error("[oatpp::sqlite::ConnectionProvider::get()]: "
                             "Error. Can't connect. " + errMsg);
  }

  return provider::ResourceHandle<Connection>(connection, m_invalidator);

}

async::CoroutineStarterForResult<const provider::ResourceHandle<Connection>&> ConnectionProvider::getAsync() {
  throw std::runtime_error("[oatpp::sqlite::ConnectionProvider::getAsync()]: Error. Not implemented!");
}

void ConnectionProvider::stop() {
  // DO nothing
}

}}
