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

#ifndef oatpp_sqlite_mapping_Deserializer_hpp
#define oatpp_sqlite_mapping_Deserializer_hpp

#include "oatpp/core/data/mapping/TypeResolver.hpp"
#include "oatpp/core/Types.hpp"

#include <sqlite3.h>

namespace oatpp { namespace sqlite { namespace mapping {

/**
 * Mapper from SQLite values to oatpp values.
 */
class Deserializer {
public:
  typedef oatpp::data::mapping::type::Type Type;
public:

  struct InData {

    InData(sqlite3_stmt* pStmt, int pCol, const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver);

    sqlite3_stmt* stmt;
    int col;

    std::shared_ptr<const data::mapping::TypeResolver> typeResolver;

    int oid;
    bool isNull;

  };

public:
  typedef oatpp::Void (*DeserializerMethod)(const Deserializer*, const InData&, const Type*);
private:
  static v_int64 deInt(const InData& data);
private:
  std::vector<DeserializerMethod> m_methods;
public:

  Deserializer();

  void setDeserializerMethod(const data::mapping::type::ClassId& classId, DeserializerMethod method);

  oatpp::Void deserialize(const InData& data, const Type* type) const;

private:

  static oatpp::Void deserializeString(const Deserializer* _this, const InData& data, const Type* type);

  static oatpp::Void deserializeBlob(const Deserializer* _this, const InData& data, const Type* type);

  template<class IntWrapper>
  static oatpp::Void deserializeInt(const Deserializer* _this, const InData& data, const Type* type) {
    (void) _this;
    (void) type;

    if(data.isNull) {
      return IntWrapper();
    }
    auto value = deInt(data);
    return IntWrapper((typename IntWrapper::UnderlyingType) value);
  }

  static oatpp::Void deserializeFloat32(const Deserializer* _this, const InData& data, const Type* type);

  static oatpp::Void deserializeFloat64(const Deserializer* _this, const InData& data, const Type* type);

  static oatpp::Void deserializeAny(const Deserializer* _this, const InData& data, const Type* type);

  static oatpp::Void deserializeEnum(const Deserializer* _this, const InData& data, const Type* type);

};

}}}

#endif // oatpp_sqlite_mapping_Deserializer_hpp
