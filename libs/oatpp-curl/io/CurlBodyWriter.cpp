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

#include "CurlBodyWriter.hpp"

#include <thread>

namespace oatpp { namespace curl { namespace io {
  
size_t CurlBodyWriter::readCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
  CurlBodyWriter* instance = static_cast<CurlBodyWriter*>(userdata);
  
  if(instance->m_currentData != nullptr) {
    auto readSize = size * nitems;
    if(readSize > instance->m_currentDataSize) {
      readSize = instance->m_currentDataSize;
    }
    std::memcpy(buffer, instance->m_currentData, readSize);
    instance->m_currentData = nullptr;
    instance->m_currentDataSize = readSize;
    return readSize;
  }
  
  return 0;
}

CurlBodyWriter::CurlBodyWriter(const std::shared_ptr<CurlHandles>& curlHandles)
  : m_handles(curlHandles)
  , m_currentData(nullptr)
  , m_currentDataSize(0)
{
  curl_easy_setopt(m_handles->getEasyHandle(), CURLOPT_READFUNCTION, readCallback);
  curl_easy_setopt(m_handles->getEasyHandle(), CURLOPT_READDATA, this);
}

v_io_size CurlBodyWriter::write(const void *data, v_io_size count) {
  
  v_io_size writeCount;
  while ((writeCount = writeNonBlocking(data, count)) == oatpp::IOError::RETRY_WRITE) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return writeCount;
  
}
  
v_io_size CurlBodyWriter::writeNonBlocking(const void *data, v_io_size count) {
  
  m_currentData = data;
  m_currentDataSize = count;
  
  int still_running = 1;
  curl_multi_perform(m_handles->getMultiHandle(), &still_running);
  
  if(m_currentData == nullptr) {
    return m_currentDataSize;
  } else if(still_running) {
    return oatpp::IOError::RETRY_WRITE;
  }
  
  return oatpp::IOError::BROKEN_PIPE;
  
}
  
}}}
