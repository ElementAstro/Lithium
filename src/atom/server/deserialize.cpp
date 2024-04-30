/*
 * deserialize.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-11

Description: This file contains the declaration of the DeserializationEngine
class

**************************************************/

#include "deserialize.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

std::optional<std::any> JsonDeserializer::deserialize(
    const std::string &data) const {
    DLOG_F(INFO, "JsonDeserializer::deserialize called with {}", data);
    try {
        json jsonData = json::parse(data);
        DLOG_F(INFO,
               "JsonDeserializer::deserialize: Successfully deserialized json "
               "data.");
        return jsonData.get<std::map<std::string, std::string>>();
    } catch (const json::parse_error &) {
        LOG_F(
            ERROR,
            "JsonDeserializer::deserialize: Failed to deserialize json data.");
        return std::nullopt;
    }
}

namespace atom::server {
void DeserializationEngine::addDeserializeEngine(
    const std::string &name,
    const std::shared_ptr<DeserializeEngine> &DeserializeEngine) {
    DLOG_F(INFO, "DeserializationEngine::addDeserializeEngine called with {}",
           name);
    if (deserializationEngines_.find(name) != deserializationEngines_.end()) {
        LOG_F(ERROR,
              "DeserializationEngine::addDeserializeEngine: Deserialize engine "
              "already exists: {}",
              name);
        return;
    }
    deserializationEngines_[name] = DeserializeEngine;
    LOG_F(INFO,
          "DeserializationEngine::addDeserializeEngine: Add deserialize "
          "engine: {}",
          name);
}

bool DeserializationEngine::setCurrentDeserializeEngine(
    const std::string &name) {
    DLOG_F(INFO,
           "DeserializationEngine::setCurrentDeserializeEngine called with {}",
           name);
    auto it = deserializationEngines_.find(name);
    if (it != deserializationEngines_.end()) {
        currentDeserializationEngine_ = name;
        LOG_F(INFO,
              "DeserializationEngine::setCurrentDeserializeEngine: Set current "
              "deserialize engine to: {}",
              name);
        return true;
    }
    LOG_F(ERROR,
          "DeserializationEngine::setCurrentDeserializeEngine: No such "
          "deserialize engine: {}",
          name);
    return false;
}
}  // namespace atom::server