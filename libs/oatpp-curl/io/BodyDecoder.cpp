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

#include "BodyDecoder.hpp"

namespace oatpp { namespace curl { namespace io {
  
void BodyDecoder::decode(const Headers& headers,
                         data::stream::InputStream* bodyStream,
                         data::stream::WriteCallback* writeCallback,
                         data::stream::IOStream* connection) const
{
  (void) connection;
  oatpp::data::buffer::IOBuffer buffer;
  oatpp::data::stream::transfer(bodyStream, writeCallback, 0, buffer.getData(), buffer.getSize());
}

oatpp::async::CoroutineStarter BodyDecoder::decodeAsync(const oatpp::web::protocol::http::Headers& headers,
                                                        const std::shared_ptr<oatpp::data::stream::InputStream>& bodyStream,
                                                        const std::shared_ptr<data::stream::WriteCallback>& writeCallback,
                                                        const std::shared_ptr<data::stream::IOStream>& connection) const
{
  (void) connection;
  auto buffer = oatpp::data::buffer::IOBuffer::createShared();
  return oatpp::data::stream::transferAsync(bodyStream, writeCallback, 0, buffer);
}
  
}}}
