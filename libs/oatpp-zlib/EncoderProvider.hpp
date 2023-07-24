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

#ifndef oatpp_zlib_EncoderProvider_hpp
#define oatpp_zlib_EncoderProvider_hpp

#include "oatpp/web/protocol/http/encoding/EncoderProvider.hpp"

namespace oatpp { namespace zlib {

/**
 * EncoderProvider for "deflate" encoding.
 */
class DeflateEncoderProvider : public web::protocol::http::encoding::EncoderProvider {
public:

  /**
   * Get encoding name.
   * @return
   */
  oatpp::String getEncodingName() override;

  /**
   * Get &id:oatpp::data::buffer::Processor; for chunked encoding.
   * @return - &id:oatpp::data::buffer::Processor;
   */
  std::shared_ptr<data::buffer::Processor> getProcessor() override;

};

/**
 * EncoderProvider for "deflate" decoding.
 */
class DeflateDecoderProvider : public web::protocol::http::encoding::EncoderProvider {
public:

  /**
   * Get encoding name.
   * @return
   */
  oatpp::String getEncodingName() override;

  /**
   * Get &id:oatpp::data::buffer::Processor; for chunked decoding.
   * @return - &id:oatpp::data::buffer::Processor;
   */
  std::shared_ptr<data::buffer::Processor> getProcessor() override;

};

/**
 * EncoderProvider for "gzip" encoding.
 */
class GzipEncoderProvider : public web::protocol::http::encoding::EncoderProvider {
public:

  /**
   * Get encoding name.
   * @return
   */
  oatpp::String getEncodingName() override;

  /**
   * Get &id:oatpp::data::buffer::Processor; for chunked encoding.
   * @return - &id:oatpp::data::buffer::Processor;
   */
  std::shared_ptr<data::buffer::Processor> getProcessor() override;

};

/**
 * EncoderProvider for "gzip" decoding.
 */
class GzipDecoderProvider : public web::protocol::http::encoding::EncoderProvider {
public:

  /**
   * Get encoding name.
   * @return
   */
  oatpp::String getEncodingName() override;

  /**
   * Get &id:oatpp::data::buffer::Processor; for chunked decoding.
   * @return - &id:oatpp::data::buffer::Processor;
   */
  std::shared_ptr<data::buffer::Processor> getProcessor() override;

};

}}

#endif //oatpp_zlib_EncoderProvider_hpp
