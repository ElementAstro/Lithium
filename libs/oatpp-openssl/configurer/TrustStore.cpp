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

#include "oatpp/core/Types.hpp"
#include "TrustStore.hpp"
#include "openssl/ssl.h"
#include "oatpp-openssl/ErrorStack.hpp"

namespace oatpp { namespace openssl { namespace configurer {

TrustStore::TrustStore(const oatpp::String &file, const oatpp::String &dir)
  : m_cafile(file), m_cadir(dir)
{}

void TrustStore::configure(SSL_CTX *ctx) {
  if (SSL_CTX_load_verify_locations(ctx, m_cafile?m_cafile->c_str():nullptr, m_cadir?m_cadir->c_str():nullptr) <= 0) {
    ErrorStack::logErrors("[oatpp::openssl::configurer::TrustStore::configure()]");
    throw std::runtime_error("Call to 'SSL_CTX_load_verify_locations' failed.");
  }
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
}

}}}
