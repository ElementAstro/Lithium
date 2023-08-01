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

#ifndef oatpp_curl_CurlHeadersReader_hpp
#define oatpp_curl_CurlHeadersReader_hpp

#include "./Curl.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"

namespace oatpp { namespace curl { namespace io {

/**
 * Curl headers reader is responsible for reading response headers. <br>
 * It implements `CURLOPT_HEADERFUNCTION` and stores headers in &id:oatpp::web::protocol::http::Headers;.
 * It also captures response &id:oatpp::web::protocol::http::ResponseStartingLine;.
 */
class CurlHeadersReader {
public:
  /**
   * STATE_INITIALIZED state of CurlHeadersReader.
   */
  constexpr static v_int32 STATE_INITIALIZED = 0;

  /**
   * STATE_STARTED state of CurlHeadersReader.
   */
  constexpr static v_int32 STATE_STARTED = 1;

  /**
   * STATE_FINISHED state of CurlHeadersReader.
   */
  constexpr static v_int32 STATE_FINISHED = 2;
private:
  std::shared_ptr<CurlHandles> m_handles;
  v_io_size m_position;
  v_int32 m_state;
  oatpp::web::protocol::http::Headers m_headers;
  oatpp::web::protocol::http::ResponseStartingLine m_startingLine;
  oatpp::data::stream::BufferOutputStream m_buffer;
private:
  static size_t headerCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
public:

  /**
   * Constructor.
   * @param curlHandles - &id:oatpp::curl::io::CurlHandles;.
   */
  CurlHeadersReader(const std::shared_ptr<CurlHandles>& curlHandles);

  /**
   * State of CurlHeadersReader.
   * @return - one of:
   * <ul>
   *   <li>&l:CurlHeadersReader::STATE_INITIALIZED;</li>
   *   <li>&l:CurlHeadersReader::STATE_STARTED;</li>
   *   <li>&l:CurlHeadersReader::STATE_FINISHED;</li>
   * </ul>
   */
  v_int32 getState() const;

  /**
   * Get response starting line.
   * @return - &id:oatpp::web::protocol::http::ResponseStartingLine;.
   */
  const oatpp::web::protocol::http::ResponseStartingLine& getStartingLine() const;

  /**
   * Get headers map.
   * @return - &id:oatpp::web::protocol::http::Headers;.
   */
  const oatpp::web::protocol::http::Headers& getHeaders() const;
  
};
  
}}}

#endif /* oatpp_curl_CurlHeadersReader_hpp */
