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

#ifndef oatpp_curl_BodyOutputStream_hpp
#define oatpp_curl_BodyOutputStream_hpp

#include "CurlBodyWriter.hpp"

namespace oatpp { namespace curl { namespace io {

/**
 * Wrapper over &id:oatpp::curl::io::CurlBodyWriter; providing &id:oatpp::data::stream::OutputStream; interface.
 */
class BodyOutputStream : public oatpp::data::stream::OutputStream {
private:
  static oatpp::data::stream::DefaultInitializedContext DEFAULT_CONTEXT;
private:
  std::shared_ptr<CurlBodyWriter> m_writer;
  oatpp::data::stream::IOMode m_ioMode;
public:

  /**
   * Constructor.
   * @param writer
   * @param ioMode
   */
  BodyOutputStream(const std::shared_ptr<CurlBodyWriter> writer, oatpp::data::stream::IOMode ioMode);

  /**
   * Write data to stream. Implementation of &id:oatpp::data::stream::OutputStream::write; method.
   * @param data - data to write.
   * @param count - data size.
   * @return - actual amount of bytes written. &id:oatpp::v_io_size;.
   */
  v_io_size write(const void *data, v_buff_size count, async::Action& action) override;

  /**
   * Set OutputStream I/O mode.
   * @param ioMode
   */
  void setOutputStreamIOMode(oatpp::data::stream::IOMode ioMode) override;

  /**
   * Set OutputStream I/O mode.
   * @return
   */
  oatpp::data::stream::IOMode getOutputStreamIOMode() override;

  /**
   * Get stream context.
   * @return - &id:oatpp::data::stream::Context;.
   */
  oatpp::data::stream::Context& getOutputStreamContext() override;
  
};
  
}}}

#endif /* oatpp_curl_BodyOutputStream_hpp */
