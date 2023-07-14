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

#include "PeerCertificateVerification.hpp"
#include <openssl/ssl.h>

namespace oatpp { namespace openssl { namespace configurer {

PeerCertificateVerification::PeerCertificateVerification(CertificateVerificationMode mode)
  : m_mode(mode)
{}

static int toSslVerify(CertificateVerificationMode mode) {
  switch (mode) {
    case CertificateVerificationMode::EnabledStrong:
      return SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
    case CertificateVerificationMode::EnabledWeak:
      return SSL_VERIFY_PEER;
    case CertificateVerificationMode::Disabled:
      return SSL_VERIFY_NONE;
  }
}

void PeerCertificateVerification::configure(SSL_CTX *ctx) {
  SSL_CTX_set_verify(ctx, toSslVerify(m_mode), nullptr);
}

}}}
