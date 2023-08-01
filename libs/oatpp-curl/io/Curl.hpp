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

#ifndef oatpp_curl_Curl_hpp
#define oatpp_curl_Curl_hpp

#include "oatpp/core/Types.hpp"

#include <curl/curl.h>

namespace oatpp { namespace curl { namespace io {

/**
 * Wrapper over `curl_slist`.
 */
class CurlHeaders {
private:
  curl_slist* m_list;
public:

  /**
   * Constructor.
   */
  CurlHeaders();

  /**
   * Non-virtual destructor.
   */
  ~CurlHeaders();

  /**
   * Append Header to `curl_slist`.
   * @param key - header name. &id:oatpp::String;.
   * @param value - header value. &id:oatpp::String;.
   */
  void append(const oatpp::String& key, const oatpp::String& value);

  /**
   * Get underlying `curl_slist`.
   * @return - `curl_slist*`.
   */
  curl_slist* getCurlList() {
    return m_list;
  }
  
};

/**
 * Pair of `CURL` and `CURLM`.
 * Curl-multi is used by &id:oatpp::curl::RequestExecutor;, &id:oatpp::curl::io::CurlBodyReader;, &id:oatpp::curl::io::CurlBodyWriter;
 * just for non-blocking perform rather then for multi-handle-perform.
 */
class CurlHandles {
private:
  CURL* m_easyhandle;
  CURLM* m_multiHandle; // curl-multi is used for non-blocking perform
public:

  /**
   * Constructor.
   */
  CurlHandles();

  /**
   * Non-virtual destructor.
   */
  ~CurlHandles();

  /**
   * Get curl easy handle.
   * @return - `CURL*`.
   */
  CURL* getEasyHandle();

  /**
   * Get curl multi handle.
   * @return - `CURLM*`.
   */
  CURLM* getMultiHandle();
  
};
  
}}}

#endif /* oatpp_curl_Curl_hpp */
