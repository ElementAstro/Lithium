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

#ifndef oatpp_sqlite_mapping_type_Blob_hpp
#define oatpp_sqlite_mapping_type_Blob_hpp

#include "oatpp/encoding/Base64.hpp"
#include "oatpp/core/Types.hpp"

namespace oatpp { namespace sqlite { namespace mapping { namespace type {

namespace __class {
  class Blob;
}

/**
 * Blob type. <br>
 */
typedef oatpp::data::mapping::type::ObjectWrapper<std::string, __class::Blob> Blob;

namespace __class {

class Blob {
public:

  class Inter : public oatpp::Type::Interpretation<type::Blob, oatpp::String>  {
  public:

    oatpp::String interpret(const type::Blob& value) const override {
      if(value) {
        return encoding::Base64::encode(value->data(), value->size());
      }
      return nullptr;
    }

    type::Blob reproduce(const oatpp::String& value) const override {
      if(value) {
        return encoding::Base64::decode(value).getPtr();
      }
      return nullptr;
    }

  };

private:
  static oatpp::Type* createType();
public:

  static const oatpp::ClassId CLASS_ID;
  static oatpp::Type* getType();

};

}

}}}}

#endif // oatpp_sqlite_mapping_type_Blob_hpp
