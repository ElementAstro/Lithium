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

#include "BodyOutputStream.hpp"

namespace oatpp { namespace curl { namespace io {

oatpp::data::stream::DefaultInitializedContext BodyOutputStream::DEFAULT_CONTEXT(oatpp::data::stream::StreamType::STREAM_INFINITE);
  
BodyOutputStream::BodyOutputStream(const std::shared_ptr<CurlBodyWriter> writer, oatpp::data::stream::IOMode ioMode)
  : m_writer(writer)
  , m_ioMode(ioMode)
{}

v_io_size BodyOutputStream::write(const void *data, v_buff_size count, async::Action& action) {
  if(m_ioMode == oatpp::data::stream::IOMode::ASYNCHRONOUS) {
    // No Action. Just return IOError::RETRY_WRITE in case no data is available.
    return m_writer->writeNonBlocking(data, count);
  } else {
    return m_writer->write(data, count);
  }
}

void BodyOutputStream::setOutputStreamIOMode(oatpp::data::stream::IOMode ioMode) {
  m_ioMode = ioMode;
}

oatpp::data::stream::IOMode BodyOutputStream::getOutputStreamIOMode() {
  return m_ioMode;
}

oatpp::data::stream::Context& BodyOutputStream::getOutputStreamContext() {
  return DEFAULT_CONTEXT;
}
  
}}}
