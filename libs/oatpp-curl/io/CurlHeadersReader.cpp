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

#include "CurlHeadersReader.hpp"

namespace oatpp { namespace curl { namespace io {
    
size_t CurlHeadersReader::headerCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  
  oatpp::String capturedText = oatpp::String(ptr, (v_buff_size)(size * nmemb));
  oatpp::parser::Caret caret(capturedText);
  
  CurlHeadersReader* instance = static_cast<CurlHeadersReader*>(userdata);
  
  if(instance->m_state == STATE_INITIALIZED) {
    instance->m_state = STATE_STARTED;
    oatpp::web::protocol::http::Status error;
    oatpp::web::protocol::http::Parser::parseResponseStartingLine(instance->m_startingLine, capturedText.getPtr(), caret, error);
  } else if(instance->m_state == STATE_STARTED) {
    if(caret.isAtRN()) {
      instance->m_state = STATE_FINISHED;
    }
    oatpp::web::protocol::http::Status error;
    oatpp::web::protocol::http::Parser::parseOneHeader(instance->m_headers, capturedText.getPtr(), caret, error);
  } else if(instance->m_state == STATE_FINISHED) {
    throw std::runtime_error("[oatpp::curl::CurlHeadersReader::headerCallback(...)]: Invalid state.");
  }
  
  return caret.getDataSize();
}

CurlHeadersReader::CurlHeadersReader(const std::shared_ptr<CurlHandles>& curlHandles)
  : m_handles(curlHandles)
  , m_position(0)
  , m_state(STATE_INITIALIZED)
{
  curl_easy_setopt(m_handles->getEasyHandle(), CURLOPT_HEADERFUNCTION, headerCallback);
  curl_easy_setopt(m_handles->getEasyHandle(), CURLOPT_HEADERDATA, this);
}

v_int32 CurlHeadersReader::getState() const {
  return m_state;
}

const oatpp::web::protocol::http::ResponseStartingLine& CurlHeadersReader::getStartingLine() const {
  return m_startingLine;
}

const oatpp::web::protocol::http::Headers& CurlHeadersReader::getHeaders() const {
  return m_headers;
}

}}}
