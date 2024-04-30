/*
 * deserialize.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-11

Description: This file contains the declaration of the DeserializationEngine
class

**************************************************/

#ifndef ATOM_SERVER_DESERIALIZE_HPP
#define ATOM_SERVER_DESERIALIZE_HPP

#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

class DeserializeEngine {
public:
    virtual ~DeserializeEngine() = default;
    virtual std::optional<std::any> deserialize(
        const std::string &data) const = 0;
};

class JsonDeserializer : public DeserializeEngine {
public:
    std::optional<std::any> deserialize(const std::string &data) const override;
};

namespace atom::server {
/**
 * @class DeserializationEngine
 * @brief A class responsible for deserializing data using different
 * deserialization engines.
 */
class DeserializationEngine {
public:
    /**
     * @brief Default constructor.
     */
    DeserializationEngine() = default;

    /**
     * @brief Default destructor.
     */
    ~DeserializationEngine() = default;

    /**
     * @brief Adds a deserialization engine to the engine map.
     * @param name The name of the deserialization engine.
     * @param engine A shared pointer to the deserialization engine.
     */
    void addDeserializeEngine(const std::string &name,
                              const std::shared_ptr<DeserializeEngine> &engine);

    /**
     * @brief Sets the current deserialization engine.
     * @param name The name of the deserialization engine to set as current.
     * @return True if the deserialization engine was set successfully, false
     * otherwise.
     */
    bool setCurrentDeserializeEngine(const std::string &name);

    /**
     * @brief Deserializes the given data using the current deserialization
     * engine.
     * @tparam T The type to deserialize the data into.
     * @param data The serialized data to deserialize.
     * @return An optional containing the deserialized object if successful, or
     * an empty optional otherwise.
     */
    template <typename T>
    std::optional<T> deserialize(const std::string &data) const;

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::shared_ptr<DeserializeEngine>>
        deserializationEngines_; /**< Map of deserialization engines. */
#else
    std::unordered_map<std::string, std::shared_ptr<DeserializeEngine>>
        deserializationEngines_; /**< Map of deserialization engines. */
#endif
    std::string currentDeserializationEngine_; /**< Name of the current
                                                  deserialization engine. */
    mutable std::mutex mutex_; /**< Mutex to ensure thread safety. */
};

template <typename T>
std::optional<T> DeserializationEngine::deserialize(
    const std::string &data) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = deserializationEngines_.find(currentDeserializationEngine_);
    if (it != deserializationEngines_.end()) {
        try {
            auto result = it->second->deserialize(data);
            if (result.has_value()) {
                return std::any_cast<T>(result.value());
            }
        } catch (const std::bad_any_cast &) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}
}  // namespace atom::server

#endif
