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

#ifndef oatpp_openssl_configurer_Groups_hpp
#define oatpp_openssl_configurer_Groups_hpp

#include "ContextConfigurer.hpp"

#include "oatpp/core/Types.hpp"

namespace oatpp { namespace openssl { namespace configurer {

/**
 * Context configurer for limiting the groups used by the TLS connection
 * @extends &id:oatpp::openssl::configurer::ContextConfigurer;.
 */
class Groups : public ContextConfigurer {
private:
  oatpp::String m_groupsColonSeparated;
public:

  /**
   * Constructor.
   * @param groups DH or ECDH groups (others may be added via external providers through openssl)
   */
  Groups(const oatpp::List<oatpp::String>& groups);

  void configure(SSL_CTX* ctx) override;

};

}}}

#endif // oatpp_openssl_configurer_Groups_hpp
