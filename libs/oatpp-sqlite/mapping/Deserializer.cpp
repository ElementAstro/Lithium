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

#include "Deserializer.hpp"
#include "oatpp-sqlite/Types.hpp"

namespace oatpp { namespace sqlite { namespace mapping {

Deserializer::InData::InData(sqlite3_stmt* pStmt,
                             int pCol,
                             const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver)
{
  stmt = pStmt;
  col = pCol;
  typeResolver = pTypeResolver;
  oid = sqlite3_column_type(stmt, col);
  isNull = (oid == SQLITE_NULL);
}

Deserializer::Deserializer() {

  m_methods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

  setDeserializerMethod(data::mapping::type::__class::String::CLASS_ID, &Deserializer::deserializeString);
  setDeserializerMethod(data::mapping::type::__class::Any::CLASS_ID, &Deserializer::deserializeAny);

  setDeserializerMethod(data::mapping::type::__class::Int8::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int8>);
  setDeserializerMethod(data::mapping::type::__class::UInt8::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt8>);

  setDeserializerMethod(data::mapping::type::__class::Int16::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int16>);
  setDeserializerMethod(data::mapping::type::__class::UInt16::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt16>);

  setDeserializerMethod(data::mapping::type::__class::Int32::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int32>);
  setDeserializerMethod(data::mapping::type::__class::UInt32::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt32>);

  setDeserializerMethod(data::mapping::type::__class::Int64::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int64>);
  setDeserializerMethod(data::mapping::type::__class::UInt64::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt64>);

  setDeserializerMethod(data::mapping::type::__class::Float32::CLASS_ID, &Deserializer::deserializeFloat32);
  setDeserializerMethod(data::mapping::type::__class::Float64::CLASS_ID, &Deserializer::deserializeFloat64);
  setDeserializerMethod(data::mapping::type::__class::Boolean::CLASS_ID, &Deserializer::deserializeInt<oatpp::Boolean>);

  setDeserializerMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractEnum::CLASS_ID, &Deserializer::deserializeEnum);

  setDeserializerMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractList::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, nullptr);

  setDeserializerMethod(data::mapping::type::__class::AbstractPairList::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, nullptr);

  // sqlite

  setDeserializerMethod(mapping::type::__class::Blob::CLASS_ID, &Deserializer::deserializeBlob);

}

void Deserializer::setDeserializerMethod(const data::mapping::type::ClassId& classId, DeserializerMethod method) {
  const v_uint32 id = classId.id;
  if(id >= m_methods.size()) {
    m_methods.resize(id + 1, nullptr);
  }
  m_methods[id] = method;
}

oatpp::Void Deserializer::deserialize(const InData& data, const Type* type) const {

  auto id = type->classId.id;
  auto& method = m_methods[id];

  if(method) {
    return (*method)(this, data, type);
  }

  auto* interpretation = type->findInterpretation(data.typeResolver->getEnabledInterpretations());
  if(interpretation) {
    return interpretation->fromInterpretation(deserialize(data, interpretation->getInterpretationType()));
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserialize()]: "
                           "Error. No deserialize method for type '" + std::string(type->classId.name) + "'");

}

v_int64 Deserializer::deInt(const InData& data) {
  switch(data.oid) {
    case SQLITE_INTEGER: return sqlite3_column_int64(data.stmt, data.col);
  }
  throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deInt()]: Error. Unknown OID.");
}

oatpp::Void Deserializer::deserializeString(const Deserializer* _this, const InData& data, const Type* type)
{

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::String();
  }

  auto ptr = (const char*) sqlite3_column_text(data.stmt, data.col);
  auto size = sqlite3_column_bytes(data.stmt, data.col);
  return oatpp::String(ptr, size);

}

oatpp::Void Deserializer::deserializeBlob(const Deserializer* _this, const InData& data, const Type* type) {

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::String();
  }

  auto ptr = (const char*) sqlite3_column_blob(data.stmt, data.col);
  auto size = sqlite3_column_bytes(data.stmt, data.col);
  return sqlite::Blob(std::make_shared<std::string>(ptr, size));

}

oatpp::Void Deserializer::deserializeFloat32(const Deserializer* _this, const InData& data, const Type* type) {

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::Float32();
  }

  switch(data.oid) {
    case SQLITE_INTEGER:
    case SQLITE_FLOAT: return oatpp::Float32(sqlite3_column_double(data.stmt, data.col));
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserializeFloat32()]: Error. Unknown OID.");

}

oatpp::Void Deserializer::deserializeFloat64(const Deserializer* _this, const InData& data, const Type* type) {

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::Float64();
  }

  switch(data.oid) {
    case SQLITE_INTEGER:
    case SQLITE_FLOAT: return oatpp::Float64(sqlite3_column_double(data.stmt, data.col));
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserializeFloat64()]: Error. Unknown OID.");

}

oatpp::Void Deserializer::deserializeAny(const Deserializer* _this, const InData& data, const Type* type) {

  (void) type;

  if(data.isNull) {
    return oatpp::Void(Any::Class::getType());
  }

  const Type* valueType;

  switch(data.oid) {
    case SQLITE3_TEXT:
      valueType = oatpp::String::Class::getType();
      break;
    case SQLITE_BLOB:
      valueType = oatpp::sqlite::Blob::Class::getType();
      break;
    case SQLITE_INTEGER:
      valueType = oatpp::Int64::Class::getType();
      break;
    case SQLITE_FLOAT:
      valueType = oatpp::Float64::Class::getType();
      break;
    default:
      throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserializeAny()]: Error. Unknown OID.");
  }

  auto value = _this->deserialize(data, valueType);
  auto anyHandle = std::make_shared<data::mapping::type::AnyHandle>(value.getPtr(), value.getValueType());
  return oatpp::Void(anyHandle, Any::Class::getType());

}

oatpp::Void Deserializer::deserializeEnum(const Deserializer* _this, const InData& data, const Type* type) {

  auto polymorphicDispatcher = static_cast<const data::mapping::type::__class::AbstractEnum::PolymorphicDispatcher*>(
    type->polymorphicDispatcher
  );

  data::mapping::type::EnumInterpreterError e = data::mapping::type::EnumInterpreterError::OK;
  const auto& value = _this->deserialize(data, polymorphicDispatcher->getInterpretationType());

  const auto& result = polymorphicDispatcher->fromInterpretation(value, e);

  if(e == data::mapping::type::EnumInterpreterError::OK) {
    return result;
  }

  switch(e) {
    case data::mapping::type::EnumInterpreterError::CONSTRAINT_NOT_NULL:
      throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserializeEnum()]: Error. Enum constraint violated - 'NotNull'.");

    default:
      throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserializeEnum()]: Error. Can't deserialize Enum.");
  }

}

}}}
