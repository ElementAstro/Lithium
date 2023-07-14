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

#include "Groups.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace oatpp { namespace openssl { namespace configurer {

Groups::Groups(const oatpp::List<oatpp::String>& groups)
{
  if(groups->empty()) {
    return;
  }
  m_groupsColonSeparated = groups->front();
  std::for_each(std::next(groups->cbegin()), groups->cend(), [this](const std::string &group){
      m_groupsColonSeparated->append(":");
      m_groupsColonSeparated->append(group);
  });
}

void Groups::configure(SSL_CTX *ctx) {
  if (m_groupsColonSeparated->empty()) {
    return;
  }
  if (!SSL_CTX_set1_groups_list(ctx, m_groupsColonSeparated->c_str())) {
    throw std::runtime_error("[oatpp::openssl::configurer::Groups::configure()]: Error. SSL_CTX_set1_groups_list(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }
}

}}}
