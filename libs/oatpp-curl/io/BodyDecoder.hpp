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

#ifndef oatpp_curl_BodyDecoder_hpp
#define oatpp_curl_BodyDecoder_hpp

#include "oatpp/web/protocol/http/incoming/BodyDecoder.hpp"

namespace oatpp { namespace curl { namespace io {

/**
 * Custom BodyDecoder for &id:oatpp::curl::RequestExecutor;. Extends &id:oatpp::web::protocol::http::incoming::BodyDecoder;.
 */
class BodyDecoder : public oatpp::web::protocol::http::incoming::BodyDecoder {
public:

  /**
   * Typedef for headers map. Headers map key is case-insensitive.
   * For more info see &id:oatpp::data::share::LazyStringMultimap;.
   */
  typedef oatpp::data::share::LazyStringMultimap<oatpp::data::share::StringKeyLabelCI> Headers;
public:

  /**
   * Just transfer everything we have in bodyStream to toStream as-is
   * Curl already did all decoding.
   * @param headers - Headers map. &id:oatpp::web::protocol::http::Headers;.
   * @param bodyStream - pointer to &id:oatpp::data::stream::InputStream;.
   * @param writeCallback - &id:oatpp::data::stream::WriteCallback;.
   * @param connection
   */
  virtual void decode(const Headers& headers, data::stream::InputStream* bodyStream,
                      data::stream::WriteCallback* writeCallback,
                      data::stream::IOStream* connection) const override;

  /**
   * Just transfer everything we have in bodyStream to toStream as-is
   * Curl already did all decoding.
   * @param headers - Headers map. &id:oatpp::web::protocol::http::Headers;.
   * @param bodyStream - `std::shared_ptr` to &id:oatpp::data::stream::InputStream;.
   * @param writeCallback - `std::shared_ptr` to &id:oatpp::data::stream::WriteCallback;.
   * @param connection
   * @return - &id:oatpp::async::CoroutineStarter;.
   */
  virtual oatpp::async::CoroutineStarter decodeAsync(const Headers& headers,
                                                     const std::shared_ptr<data::stream::InputStream>& bodyStream,
                                                     const std::shared_ptr<data::stream::WriteCallback>& writeCallback,
                                                     const std::shared_ptr<data::stream::IOStream>& connection) const override;

};
  
}}}

#endif /* oatpp_curl_BodyDecoder_hpp */
