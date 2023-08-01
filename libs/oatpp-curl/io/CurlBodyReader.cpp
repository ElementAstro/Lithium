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

#include "CurlBodyReader.hpp"

#include <thread>

namespace oatpp { namespace curl { namespace io {
  
/*
 * This callback may be called several times during one non-blocking perform.
 * So check for (instance->m_position != 0). m_position == 0 means it was called multiple times in a row
 * it can happen if response in chunked encoded
 */
size_t CurlBodyReader::writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  CurlBodyReader* instance = static_cast<CurlBodyReader*>(userdata);
  
  if(instance->m_position != 0) {
    if(instance->m_position != instance->m_buffer.getCurrentPosition()){
      throw std::runtime_error("[oatpp::curl::CurlBodyReader::writeCallback(...)]: Invalid state.");
    }
    instance->m_buffer.setCurrentPosition(0);
    instance->m_position = 0;
  }
  return instance->m_buffer.writeSimple(ptr, size * nmemb);
}

CurlBodyReader::CurlBodyReader(const std::shared_ptr<CurlHandles>& curlHandles)
  : m_handles(curlHandles)
  , m_position(0)
{
  curl_easy_setopt(m_handles->getEasyHandle(), CURLOPT_WRITEFUNCTION, writeCallback);
  curl_easy_setopt(m_handles->getEasyHandle(), CURLOPT_WRITEDATA, this);
}

v_io_size CurlBodyReader::read(void *data, v_io_size count) {
  v_io_size readCount;
  while ((readCount = readNonBlocking(data, count)) == oatpp::IOError::RETRY_READ) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return readCount;
}
  
v_io_size CurlBodyReader::readNonBlocking(void *data, v_io_size count) {
  
  v_io_size availableBytes = getAvailableBytesCount();
  
  if(availableBytes == 0) {
    
    int still_running = 1;
    curl_multi_perform(m_handles->getMultiHandle(), &still_running);
    
    availableBytes = getAvailableBytesCount();
    
    if(availableBytes == 0) {
      
      if(still_running) {
        return oatpp::IOError::RETRY_READ;
      } else {
        return oatpp::IOError::BROKEN_PIPE;
      }
      
    }
    
  }

  v_int64 readCount = count;
  if(count + m_position > m_buffer.getCurrentPosition()) {
    readCount = m_buffer.getCurrentPosition() - m_position;
  }
  std::memcpy(data, m_buffer.getData() + m_position, readCount);
  m_position += readCount;
  return readCount;
  
}
  
v_io_size CurlBodyReader::getAvailableBytesCount() {
  return m_buffer.getCurrentPosition() - m_position;
}
  
}}}
