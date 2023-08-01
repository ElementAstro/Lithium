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

#ifndef oatpp_curl_CurlBodyReader_hpp
#define oatpp_curl_CurlBodyReader_hpp

#include "./Curl.hpp"

#include "oatpp/core/data/stream/BufferStream.hpp"

namespace oatpp { namespace curl { namespace io {

/**
 * This class is wrapper over &id:oatpp::curl::io::CurlHandles; to provide input-stream like interface
 */
class CurlBodyReader {
private:
  std::shared_ptr<CurlHandles> m_handles;
  oatpp::data::stream::BufferOutputStream m_buffer;
  v_io_size m_position;
private:
  static size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
public:

  /**
   * Constructor.
   * @param curlHandles - &id:oatpp::curl::io::CurlHandles;.
   */
  CurlBodyReader(const std::shared_ptr<CurlHandles>& curlHandles);

  /**
   * Read body data.
   * @param data - buffer to read data to.
   * @param count - buffer size.
   * @return - actual amount of bytes read. &id:oatpp::v_io_size;.
   */
  v_io_size read(void *data, v_io_size count);

  /**
   * Non blocking attempt to Read body data.
   * @param data - buffer to read data to.
   * @param count - buffer size.
   * @return - actual amount of bytes read. &id:oatpp::v_io_size;.
   */
  v_io_size readNonBlocking(void *data, v_io_size count);

  /**
   * Available amount of bytes currently buffered.
   * @return - &id:oatpp::v_io_size;.
   */
  v_io_size getAvailableBytesCount();
  
};
  
}}}

#endif /* oatpp_curl_CurlBodyReader_hpp */
