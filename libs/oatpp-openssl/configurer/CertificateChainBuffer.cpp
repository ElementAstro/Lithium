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

#include "CertificateChainBuffer.hpp"

#include <openssl/x509_vfy.h>
#include <openssl/err.h>

namespace oatpp { namespace openssl { namespace configurer {

static void deleteStackOfX509Info(STACK_OF(X509_INFO) *p) {
  sk_X509_INFO_pop_free(p, X509_INFO_free);
}

CertificateChainBuffer::CertificateChainBuffer(const oatpp::String& certificateChainBuffer)
  : CertificateChainBuffer::CertificateChainBuffer(certificateChainBuffer->data(), certificateChainBuffer->size())
{}

CertificateChainBuffer::CertificateChainBuffer(const void *certificateChainBuffer, int certificateChainBufferLength)
{
  if (certificateChainBufferLength == 0) {
    return;
  }
  auto buffer = std::shared_ptr<BIO>(BIO_new_mem_buf(certificateChainBuffer, certificateChainBufferLength), BIO_free);
  if (buffer == nullptr) {
    throw std::runtime_error("[oatpp::openssl::configurer::CertificateChainBuffer::CertificateChainBuffer()]: Error. BIO_new_mem_buf(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }

  m_certificates = std::shared_ptr<STACK_OF(X509_INFO)>(PEM_X509_INFO_read_bio(buffer.get(), nullptr, nullptr, nullptr), deleteStackOfX509Info);
  if (m_certificates == nullptr)
  {
    throw std::runtime_error("[oatpp::openssl::configurer::CertificateChainBuffer::CertificateChainBuffer()]: Error. PEM_X509_INFO_read_bio(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }
}

void CertificateChainBuffer::configure(SSL_CTX *ctx) {
  if (m_certificates == nullptr) {
    return;
  }

  // The first certificate in the buffer is the certificate
  int i;
  int numberOfCertificates = sk_X509_INFO_num(m_certificates.get());
  for (i = 0; i < numberOfCertificates; i++) {
    auto certificate = sk_X509_INFO_value(m_certificates.get(), i)->x509;
    if (certificate != nullptr) {
      if(SSL_CTX_use_certificate(ctx, certificate)) {
        break;
      } else {
        throw std::runtime_error("[oatpp::openssl::configurer::CertificateChainBuffer::configure()]: Error. SSL_CTX_use_certificate(): "
                                 + std::string(ERR_error_string(ERR_get_error(), nullptr)));
      }
    }
  }

  // If we reached the end without setting a certificate it is an error
  if(i == numberOfCertificates) {
    throw std::runtime_error("[oatpp::openssl::configurer::CertificateChainBuffer::configure()]: Error. "
                             "No certificates in PEM buffer.");
  }

  // Cleanup the previous certificate chain
  if(!SSL_CTX_clear_chain_certs(ctx)) {
    throw std::runtime_error("[oatpp::openssl::configurer::CertificateChainBuffer::configure()]: Error. SSL_CTX_clear_chain_certs(): "
                             + std::string(ERR_error_string(ERR_get_error(), nullptr)));
  }

  // The next certificates in the chain is the intermediate certificates
  while (++i < numberOfCertificates) {
    auto certificate = sk_X509_INFO_value(m_certificates.get(), i)->x509;
    if(certificate != nullptr && !SSL_CTX_add1_chain_cert(ctx, certificate)) {
      throw std::runtime_error("[oatpp::openssl::configurer::CertificateChainBuffer::configure()]: Error. SSL_CTX_add1_chain_cert(): "
                               + std::string(ERR_error_string(ERR_get_error(), nullptr)));
    }
  }
}

}}}
