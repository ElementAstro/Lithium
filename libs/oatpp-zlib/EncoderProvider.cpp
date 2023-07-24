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

#include "EncoderProvider.hpp"

#include "./Processor.hpp"

namespace oatpp { namespace zlib {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeflateEncoderProvider

oatpp::String DeflateEncoderProvider::getEncodingName() {
  return "deflate";
}

std::shared_ptr<data::buffer::Processor> DeflateEncoderProvider::getProcessor() {
  return std::make_shared<DeflateEncoder>(2048, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeflateDecoderProvider

oatpp::String DeflateDecoderProvider::getEncodingName() {
  return "deflate";
}

std::shared_ptr<data::buffer::Processor> DeflateDecoderProvider::getProcessor() {
  return std::make_shared<DeflateDecoder>(2048, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GzipEncoderProvider

oatpp::String GzipEncoderProvider::getEncodingName() {
  return "gzip";
}

std::shared_ptr<data::buffer::Processor> GzipEncoderProvider::getProcessor() {
  return std::make_shared<DeflateEncoder>(2048, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeflateDecoderProvider

oatpp::String GzipDecoderProvider::getEncodingName() {
  return "gzip";
}

std::shared_ptr<data::buffer::Processor> GzipDecoderProvider::getProcessor() {
  return std::make_shared<DeflateDecoder>(2048, true);
}

}}
