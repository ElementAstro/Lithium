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

#include <openssl/err.h>
#include "PrivateKeyBuffer.hpp"

namespace oatpp { namespace openssl { namespace configurer {

PrivateKeyBuffer::PrivateKeyBuffer(const oatpp::String& privateKeyBuffer)
  : PrivateKeyBuffer::PrivateKeyBuffer(privateKeyBuffer->data(), privateKeyBuffer->size())
{}

PrivateKeyBuffer::PrivateKeyBuffer(const void *privateKeyBuffer, int privateKeyBufferLength)
{
  if (privateKeyBufferLength == 0) {
    return;
  }
  auto buffer = std::shared_ptr<BIO>(BIO_new_mem_buf(privateKeyBuffer, privateKeyBufferLength), BIO_free);
  if (buffer == nullptr) {
    throw std::runtime_error("[oatpp::openssl::configurer::PrivateKeyBuffer::PrivateKeyBuffer()]: Error. BIO_new_mem_buf(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }

  m_privateKey = std::shared_ptr<EVP_PKEY>(PEM_read_bio_PrivateKey(buffer.get(), nullptr, nullptr, nullptr), EVP_PKEY_free);
  if (m_privateKey == nullptr) {
    throw std::runtime_error("[oatpp::openssl::configurer::PrivateKeyBuffer::PrivateKeyBuffer()]: Error. PEM_read_bio_PrivateKey(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }
}

void PrivateKeyBuffer::configure(SSL_CTX *ctx) {
  if (m_privateKey == nullptr) {
    return;
  }
  if (SSL_CTX_use_PrivateKey(ctx, m_privateKey.get()) <= 0) {
    throw std::runtime_error("[oatpp::openssl::configurer::PrivateKeyBuffer::configure()]: Error. SSL_CTX_use_PrivateKey(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }
}

}}}

