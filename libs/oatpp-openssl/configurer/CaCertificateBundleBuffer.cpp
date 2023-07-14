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

#include "CaCertificateBundleBuffer.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace oatpp { namespace openssl { namespace configurer {

static void deleteStackOfX509Info(STACK_OF(X509_INFO) *p) {
  sk_X509_INFO_pop_free(p, X509_INFO_free);
}

CaCertificateBundleBuffer::CaCertificateBundleBuffer(const oatpp::String& caBuffer)
  : CaCertificateBundleBuffer::CaCertificateBundleBuffer(caBuffer->data(), caBuffer->size())
{}

CaCertificateBundleBuffer::CaCertificateBundleBuffer(const void *caBuffer, int caBufferLength)
{
  if (caBufferLength == 0) {
    return;
  }

  auto buffer = std::shared_ptr<BIO>(BIO_new_mem_buf(caBuffer, caBufferLength), BIO_free);
  if (buffer == nullptr) {
    throw std::runtime_error("[oatpp::openssl::configurer::CaCertificateBundleBuffer::CaCertificateBundleBuffer()]: Error. BIO_new_mem_buf(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }

  m_certificates = std::shared_ptr<STACK_OF(X509_INFO)>(PEM_X509_INFO_read_bio(buffer.get(), nullptr, nullptr, nullptr), deleteStackOfX509Info);
  if (m_certificates == nullptr)
  {
    throw std::runtime_error("[oatpp::openssl::configurer::CaCertificateBundleBuffer::CaCertificateBundleBuffer()]: Error. PEM_X509_INFO_read_bio(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }
}

void CaCertificateBundleBuffer::configure(SSL_CTX *ctx) {
  if (m_certificates == nullptr) {
    return;
  }

  X509_STORE* trustedCertificatesStore = SSL_CTX_get_cert_store(ctx);

  if (trustedCertificatesStore == nullptr)
  {
    throw std::runtime_error("[oatpp::openssl::configurer::CaCertificateBundleBuffer::configure()]: Error. SSL_CTX_get_cert_store(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }

  for (int i = 0; i < sk_X509_INFO_num(m_certificates.get()); i++) {
    auto certificate = sk_X509_INFO_value(m_certificates.get(), i)->x509;
    if (certificate != nullptr && !X509_STORE_add_cert(trustedCertificatesStore, certificate)) {
      throw std::runtime_error("[oatpp::openssl::configurer::CaCertificateBundleBuffer::configure()]: Error. X509_STORE_add_cert(): "
                               + std::string(ERR_error_string(ERR_get_error(), nullptr)));

    }
  }
}

}}}
