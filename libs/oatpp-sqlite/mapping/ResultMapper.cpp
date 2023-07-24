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

#include "ResultMapper.hpp"

namespace oatpp { namespace sqlite { namespace mapping {

ResultMapper::ResultData::ResultData(sqlite3_stmt* pStmt, const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver)
  : stmt(pStmt)
  , typeResolver(pTypeResolver)
{

  next();
  rowIndex = 0;

  {
    colCount = sqlite3_column_count(stmt);
    for (v_int32 i = 0; i < colCount; i++) {
      oatpp::String colName = (const char*) sqlite3_column_name(stmt, i);
      colNames.push_back(colName);
      colIndices.insert({colName, i});
    }
  }

}

void ResultMapper::ResultData::next() {

  auto res = sqlite3_step(stmt);

  switch(res) {

    case SQLITE_ROW: {
      hasMore = true;
      isSuccess = true;
      break;
    }

    case SQLITE_DONE: {
      hasMore = false;
      isSuccess = true;
      break;
    };

    default: {
      hasMore = false;
      isSuccess = false;
    }

  }

}

ResultMapper::ResultMapper() {

  {
    m_readOneRowMethods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

    setReadOneRowMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, &ResultMapper::readOneRowAsObject);

    setReadOneRowMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, &ResultMapper::readOneRowAsCollection);
    setReadOneRowMethod(data::mapping::type::__class::AbstractList::CLASS_ID, &ResultMapper::readOneRowAsCollection);
    setReadOneRowMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID,
                        &ResultMapper::readOneRowAsCollection);

    setReadOneRowMethod(data::mapping::type::__class::AbstractPairList::CLASS_ID, &ResultMapper::readOneRowAsMap);
    setReadOneRowMethod(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, &ResultMapper::readOneRowAsMap);
  }

  {
    m_readRowsMethods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

    setReadRowsMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, &ResultMapper::readRowsAsCollection);
    setReadRowsMethod(data::mapping::type::__class::AbstractList::CLASS_ID, &ResultMapper::readRowsAsCollection);
    setReadRowsMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, &ResultMapper::readRowsAsCollection);

  }

}

void ResultMapper::setReadOneRowMethod(const data::mapping::type::ClassId& classId, ReadOneRowMethod method) {
  const v_uint32 id = classId.id;
  if(id >= m_readOneRowMethods.size()) {
    m_readOneRowMethods.resize(id + 1, nullptr);
  }
  m_readOneRowMethods[id] = method;
}

void ResultMapper::setReadRowsMethod(const data::mapping::type::ClassId& classId, ReadRowsMethod method) {
  const v_uint32 id = classId.id;
  if(id >= m_readRowsMethods.size()) {
    m_readRowsMethods.resize(id + 1, nullptr);
  }
  m_readRowsMethods[id] = method;
}

oatpp::Void ResultMapper::readOneRowAsCollection(ResultMapper* _this, ResultData* dbData, const Type* type) {

  auto dispatcher = static_cast<const data::mapping::type::__class::Collection::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto collection = dispatcher->createObject();

  const Type* itemType = dispatcher->getItemType();

  for(v_int32 i = 0; i < dbData->colCount; i ++) {
    mapping::Deserializer::InData inData(dbData->stmt, i, dbData->typeResolver);
    dispatcher->addItem(collection, _this->m_deserializer.deserialize(inData, itemType));
  }

  return collection;

}

oatpp::Void ResultMapper::readOneRowAsMap(ResultMapper* _this, ResultData* dbData, const Type* type) {

  auto dispatcher = static_cast<const data::mapping::type::__class::Map::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto map = dispatcher->createObject();

  const Type* keyType = dispatcher->getKeyType();
  if(keyType->classId.id != oatpp::data::mapping::type::__class::String::CLASS_ID.id){
    throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::readOneRowAsMap()]: Invalid map key. Key should be String");
  }

  const Type* valueType = dispatcher->getValueType();
  for(v_int32 i = 0; i < dbData->colCount; i ++) {
    mapping::Deserializer::InData inData(dbData->stmt, i, dbData->typeResolver);
    dispatcher->addItem(map, dbData->colNames[i], _this->m_deserializer.deserialize(inData, valueType));
  }

  return map;

}

oatpp::Void ResultMapper::readOneRowAsObject(ResultMapper* _this, ResultData* dbData, const Type* type) {

  auto dispatcher = static_cast<const data::mapping::type::__class::AbstractObject::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto object = dispatcher->createObject();
  const auto& fieldsMap = dispatcher->getProperties()->getMap();

  std::vector<std::pair<oatpp::BaseObject::Property*, v_int32>> polymorphs;

  for(v_int32 i = 0; i < dbData->colCount; i ++) {

    auto it = fieldsMap.find(*dbData->colNames[i]);

    if(it != fieldsMap.end()) {
      auto field = it->second;
      if(field->info.typeSelector && field->type == oatpp::Any::Class::getType()) {
        polymorphs.push_back({field, i});
      } else {
        mapping::Deserializer::InData inData(dbData->stmt, i, dbData->typeResolver);
        field->set(static_cast<oatpp::BaseObject *>(object.get()),
                   _this->m_deserializer.deserialize(inData, field->type));
      }
    } else {
      OATPP_LOGE("[oatpp::sqlite::mapping::ResultMapper::readOneRowAsObject]",
                 "Error. The object of type '%s' has no field to map column '%s'.",
                 type->nameQualifier, dbData->colNames[i]->c_str());
      throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::readOneRowAsObject]: Error. "
                               "The object of type " + std::string(type->nameQualifier) +
                               " has no field to map column " + *dbData->colNames[i]  + ".");
    }

  }

  for(auto& p : polymorphs) {
    v_int32 index = p.second;
    mapping::Deserializer::InData inData(dbData->stmt, index, dbData->typeResolver);
    auto selectedType = p.first->info.typeSelector->selectType(static_cast<oatpp::BaseObject *>(object.get()));
    auto value = _this->m_deserializer.deserialize(inData, selectedType);
    oatpp::Any any(value);
    p.first->set(static_cast<oatpp::BaseObject *>(object.get()), oatpp::Void(any.getPtr(), p.first->type));
  }

  return object;

}

oatpp::Void ResultMapper::readRowsAsCollection(ResultMapper* _this, ResultData* dbData, const Type* type, v_int64 count) {

  auto dispatcher = static_cast<const data::mapping::type::__class::Collection::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto collection = dispatcher->createObject();

  if(count != 0) {

    const Type *itemType = *type->params.begin();

    v_int64 counter = 0;
    while (dbData->hasMore) {
      dispatcher->addItem(collection, _this->readOneRow(dbData, itemType));
      ++dbData->rowIndex;
      dbData->next();
      if (count > 0) {
        ++counter;
        if (counter == count) {
          break;
        }
      }
    }

  }

  return collection;

}

oatpp::Void ResultMapper::readOneRow(ResultData* dbData, const Type* type) {

  auto id = type->classId.id;
  auto& method = m_readOneRowMethods[id];

  if(method) {
    return (*method)(this, dbData, type);
  }

  auto* interpretation = type->findInterpretation(dbData->typeResolver->getEnabledInterpretations());
  if(interpretation) {
    return interpretation->fromInterpretation(readOneRow(dbData, interpretation->getInterpretationType()));
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::readOneRow()]: "
                           "Error. Invalid result container type. "
                           "Allowed types are "
                           "oatpp::Vector, "
                           "oatpp::List, "
                           "oatpp::UnorderedSet, "
                           "oatpp::Fields, "
                           "oatpp::UnorderedFields, "
                           "oatpp::Object");

}

oatpp::Void ResultMapper::readRows(ResultData* dbData, const Type* type, v_int64 count) {

  auto id = type->classId.id;
  auto& method = m_readRowsMethods[id];

  if(method) {
    return (*method)(this, dbData, type, count);
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::readRows()]: "
                           "Error. Invalid result container type. "
                           "Allowed types are oatpp::Vector, oatpp::List, oatpp::UnorderedSet");

}

}}}
