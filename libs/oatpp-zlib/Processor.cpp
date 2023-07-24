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

#include "Processor.hpp"

namespace oatpp { namespace zlib {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeflateEncoder

DeflateEncoder::DeflateEncoder(v_buff_size bufferSize, bool gzip, v_int32 compressionLevel)
  : m_buffer(new v_char8[bufferSize])
  , m_bufferSize(bufferSize)
  , m_finished(false)
{

  m_zStream.zalloc = Z_NULL;
  m_zStream.zfree = Z_NULL;
  m_zStream.opaque = Z_NULL;

  m_zStream.next_in = nullptr;
  m_zStream.avail_in = 0;
  m_zStream.next_out = nullptr;
  m_zStream.avail_out = 0;

  v_int32 res;

  if(gzip) {
    res = deflateInit2(&m_zStream,
                       compressionLevel,
                       Z_DEFLATED,
                       15 | 16,
                       8 /* default memory */,
                       Z_DEFAULT_STRATEGY);
    if(res != Z_OK) {
      OATPP_LOGE("[oatpp::zlib::DeflateEncoder::DeflateEncoder()]", "Error. Failed call to 'deflateInit2()'. Result %d", res)
    }
  } else {
    res = deflateInit(&m_zStream, compressionLevel);
    if(res != Z_OK) {
      OATPP_LOGE("[oatpp::zlib::DeflateEncoder::DeflateEncoder()]", "Error. Failed call to 'deflateInit()'. Result %d", res)
    }
  }

  if(res != Z_OK) {
    throw std::runtime_error("[oatpp::zlib::DeflateEncoder::DeflateEncoder()]: Error. Can't init.");
  }

}

DeflateEncoder::~DeflateEncoder() {
  v_int32 res = deflateEnd(&m_zStream);
  if(res != Z_OK) {
    OATPP_LOGE("[oatpp::zlib::DeflateEncoder::~DeflateEncoder()]", "Error. Failed call to 'deflateEnd()'. Result %d", res)
  }
}

v_io_size DeflateEncoder::suggestInputStreamReadSize() {
  return m_bufferSize;
}

v_int32 DeflateEncoder::iterate(data::buffer::InlineReadData& dataIn, data::buffer::InlineReadData& dataOut) {

  if(dataOut.bytesLeft > 0) {
    return Error::FLUSH_DATA_OUT;
  }

  if(m_finished){
    dataOut.set(nullptr, 0);
    return Error::FINISHED;
  }

  if(dataIn.currBufferPtr != nullptr) {

    if(dataIn.bytesLeft == 0) {
      return Error::PROVIDE_DATA_IN;
    }

    if(m_zStream.avail_out == 0) {
      m_zStream.next_out = (Bytef *) m_buffer.get();
      m_zStream.avail_out = (uInt) m_bufferSize;
    }

    if(m_zStream.avail_in == 0) {
      m_zStream.next_in = (Bytef *) dataIn.currBufferPtr;
      m_zStream.avail_in = (uInt) dataIn.bytesLeft;
    }

    int res = Z_OK;
    while(res == Z_OK && m_zStream.avail_in > 0 && m_zStream.avail_out > 0) {
      res = deflate(&m_zStream, Z_NO_FLUSH);
    }

    if(m_zStream.avail_in < dataIn.bytesLeft) {
      dataIn.inc(dataIn.bytesLeft - m_zStream.avail_in);
    }

    if(res != Z_BUF_ERROR && res != Z_OK) {
      m_finished = true;
      dataOut.set(nullptr, 0);
      return ERROR_UNKNOWN;
    }

    if(m_zStream.avail_out == 0) {
      dataOut.set(m_buffer.get(), m_bufferSize);
      return Error::FLUSH_DATA_OUT;
    }

    if(dataIn.bytesLeft == 0) {
      return Error::PROVIDE_DATA_IN;
    }

    return ERROR_UNKNOWN;

  }

  m_zStream.next_in = nullptr;
  m_zStream.avail_in = 0;

  if(m_zStream.avail_out == 0) {
    m_zStream.next_out = (Bytef *) m_buffer.get();
    m_zStream.avail_out = (uInt) m_bufferSize;
  }

  int res = Z_OK;
  while(res == Z_OK && m_zStream.avail_out > 0) {
    res = deflate(&m_zStream, Z_FINISH);
  }

  if(res == Z_STREAM_END) {

    m_finished = true;

    if(m_zStream.avail_out < m_bufferSize) {
      dataOut.set(m_buffer.get(), m_bufferSize - m_zStream.avail_out);
      return Error::FLUSH_DATA_OUT;
    } else {
      dataOut.set(nullptr, 0);
      return Error::FINISHED;
    }

  } else if(res == Z_OK && m_zStream.avail_out == 0) {
    dataOut.set(m_buffer.get(), m_bufferSize);
    return Error::FLUSH_DATA_OUT;
  }

  return ERROR_UNKNOWN;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeflateDecoder

DeflateDecoder::DeflateDecoder(v_buff_size bufferSize, bool gzip)
  : m_buffer(new v_char8[bufferSize])
  , m_bufferSize(bufferSize)
  , m_finished(false)
{

  m_zStream.zalloc = Z_NULL;
  m_zStream.zfree = Z_NULL;
  m_zStream.opaque = Z_NULL;

  m_zStream.next_in = nullptr;
  m_zStream.avail_in = 0;
  m_zStream.next_out = nullptr;
  m_zStream.avail_out = 0;

  v_int32 res;

  if(gzip) {
    res = inflateInit2(&m_zStream, 15 | 16);
    if(res != Z_OK) {
      OATPP_LOGE("[oatpp::zlib::DeflateDecoder::DeflateDecoder()]", "Error. Failed call to 'inflateInit2()'. Result %d", res)
    }
  } else {
    res = inflateInit(&m_zStream);
    if(res != Z_OK) {
      OATPP_LOGE("[oatpp::zlib::DeflateDecoder::DeflateDecoder()]", "Error. Failed call to 'inflateInit()'. Result %d", res)
    }
  }

  if(res != Z_OK) {
    throw std::runtime_error("[oatpp::zlib::DeflateDecoder::DeflateDecoder()]: Error. Can't init.");
  }

}

DeflateDecoder::~DeflateDecoder() {
  v_int32 res = inflateEnd(&m_zStream);
  if(res != Z_OK) {
    OATPP_LOGE("[oatpp::zlib::DeflateDecoder::~DeflateDecoder()]", "Error. Failed call to 'inflateEnd()'. Result %d", res)
  }
}

v_io_size DeflateDecoder::suggestInputStreamReadSize() {
  return m_bufferSize;
}

v_int32 DeflateDecoder::iterate(data::buffer::InlineReadData& dataIn, data::buffer::InlineReadData& dataOut) {


  if(dataOut.bytesLeft > 0) {
    return Error::FLUSH_DATA_OUT;
  }

  if(m_finished){
    dataOut.set(nullptr, 0);
    return Error::FINISHED;
  }

  if(dataIn.currBufferPtr != nullptr) {

    if(dataIn.bytesLeft == 0) {
      return Error::PROVIDE_DATA_IN;
    }

    if(m_zStream.avail_out == 0) {
      m_zStream.next_out = (Bytef *) m_buffer.get();
      m_zStream.avail_out = (uInt) m_bufferSize;
    }

    if(m_zStream.avail_in == 0) {
      m_zStream.next_in = (Bytef *) dataIn.currBufferPtr;
      m_zStream.avail_in = (uInt) dataIn.bytesLeft;
    }

    int res = Z_OK;
    while(res == Z_OK && m_zStream.avail_in > 0 && m_zStream.avail_out > 0) {
      res = inflate(&m_zStream, Z_NO_FLUSH);
    }

    if(m_zStream.avail_in < dataIn.bytesLeft) {
      dataIn.inc(dataIn.bytesLeft - m_zStream.avail_in);
    }

    if(res != Z_BUF_ERROR && res != Z_OK && res != Z_STREAM_END) {
      m_finished = true;
      dataOut.set(nullptr, 0);
      return ERROR_UNKNOWN;
    }

    if(m_zStream.avail_out == 0) {
      dataOut.set(m_buffer.get(), m_bufferSize);
      return Error::FLUSH_DATA_OUT;
    }

    if(dataIn.bytesLeft == 0) {
      return Error::PROVIDE_DATA_IN;
    }

    return ERROR_UNKNOWN;

  }

  m_zStream.next_in = nullptr;
  m_zStream.avail_in = 0;

  if(m_zStream.avail_out == 0) {
    m_zStream.next_out = (Bytef *) m_buffer.get();
    m_zStream.avail_out = (uInt) m_bufferSize;
  }

  int res = Z_OK;
  while(res == Z_OK && m_zStream.avail_out > 0) {
    res = inflate(&m_zStream, Z_FINISH);
  }

  if(res == Z_STREAM_END) {

    m_finished = true;

    if(m_zStream.avail_out < m_bufferSize) {
      dataOut.set(m_buffer.get(), m_bufferSize - m_zStream.avail_out);
      return Error::FLUSH_DATA_OUT;
    } else {
      dataOut.set(nullptr, 0);
      return Error::FINISHED;
    }

  } else if(res == Z_OK && m_zStream.avail_out == 0) {
    dataOut.set(m_buffer.get(), m_bufferSize);
    return Error::FLUSH_DATA_OUT;
  }

  return ERROR_UNKNOWN;

}

}}
