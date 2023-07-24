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

#ifndef oatpp_zlib_Processor_hpp
#define oatpp_zlib_Processor_hpp

#include "oatpp/core/data/buffer/Processor.hpp"

#include "zlib.h"
#include <memory>

namespace oatpp { namespace zlib {

/**
 * Deflate encoder.
 */
class DeflateEncoder : public oatpp::data::buffer::Processor {
public:
  static constexpr v_int32 ERROR_UNKNOWN = 100;
private:
  std::unique_ptr<v_char8[]> m_buffer;
  v_buff_size m_bufferSize;
private:
  bool m_finished;
  z_stream m_zStream;
public:

  /**
   * Constructor.
   * @param bufferSize
   * @param compressionLevel
   * @param useGzip
   */
  DeflateEncoder(v_buff_size bufferSize = 1024, bool gzip = false, v_int32 compressionLevel = Z_DEFAULT_COMPRESSION);

  ~DeflateEncoder();

  /**
   * If the client is using the input stream to read data and push it to the processor,
   * the client MAY ask the processor for a suggested read size.
   * @return - suggested read size.
   */
  v_io_size suggestInputStreamReadSize() override;

  /**
   * Process data.
   * @param dataIn - data provided by client to processor. Input data. &id:data::buffer::InlineReadData;.
   * Set `dataIn` buffer pointer to `nullptr` to designate the end of input.
   * @param dataOut - data provided to client by processor. Output data. &id:data::buffer::InlineReadData;.
   * @return - &l:Processor::Error;.
   */
  v_int32 iterate(data::buffer::InlineReadData& dataIn, data::buffer::InlineReadData& dataOut) override;

};

/**
 * Deflate decoder.
 */
class DeflateDecoder : public oatpp::data::buffer::Processor {
public:
  static constexpr v_int32 ERROR_UNKNOWN = 100;
private:
  std::unique_ptr<v_char8[]> m_buffer;
  v_buff_size m_bufferSize;
private:
  bool m_finished;
  z_stream m_zStream;
public:

  /**
   * Constructor.
   * @param bufferSize
   * @param gzip
   */
  DeflateDecoder(v_buff_size bufferSize = 1024, bool gzip = false);

  ~DeflateDecoder();

  /**
   * If the client is using the input stream to read data and push it to the processor,
   * the client MAY ask the processor for a suggested read size.
   * @return - suggested read size.
   */
  v_io_size suggestInputStreamReadSize() override;

  /**
   * Process data.
   * @param dataIn - data provided by client to processor. Input data. &id:data::buffer::InlineReadData;.
   * Set `dataIn` buffer pointer to `nullptr` to designate the end of input.
   * @param dataOut - data provided to client by processor. Output data. &id:data::buffer::InlineReadData;.
   * @return - &l:Processor::Error;.
   */
  v_int32 iterate(data::buffer::InlineReadData& dataIn, data::buffer::InlineReadData& dataOut) override;

};

}}

#endif // oatpp_zlib_Processor_hpp
