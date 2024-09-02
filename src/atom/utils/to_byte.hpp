#ifndef ATOM_UTILS_TO_BYTE_HPP
#define ATOM_UTILS_TO_BYTE_HPP

#include <cstdint>
#include <cstring>  // for memcpy
#include <fstream>
#include <list>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>  // for std::runtime_error
#include <string>
#include <type_traits>  // for type traits and SFINAE
#include <variant>
#include <vector>

namespace atom::utils {

/**
 * @brief Concept to determine if a type is serializable as plain bytes.
 *
 * This concept checks if a type is serializable by determining if it is
 * arithmetic, an enumeration, a `std::string`, or if it has a `serialize`
 * function available.
 */
template <typename T>
concept Serializable =
    std::is_arithmetic_v<T> || std::is_enum_v<T> ||
    std::same_as<T, std::string> || requires(T a) { serialize(a); };

/**
 * @brief Serializes a serializable type into a vector of bytes.
 *
 * This function converts the given data of a serializable type into a
 * vector of bytes. The size of the vector is equal to the size of the
 * type.
 *
 * @tparam T The type of the data to serialize, must satisfy the `Serializable`
 * concept.
 * @param data The data to serialize.
 * @return std::vector<uint8_t> A vector of bytes representing the serialized
 * data.
 */
template <Serializable T>
std::vector<uint8_t> serialize(const T& data) {
    std::vector<uint8_t> bytes(sizeof(T));
    std::memcpy(bytes.data(), &data, sizeof(T));
    return bytes;
}

/**
 * @brief Serializes a std::string into a vector of bytes.
 *
 * This function converts a `std::string` into a vector of bytes, including
 * the size of the string followed by the string's data.
 *
 * @param str The string to serialize.
 * @return std::vector<uint8_t> A vector of bytes representing the serialized
 * string.
 */
inline std::vector<uint8_t> serialize(const std::string& str) {
    std::vector<uint8_t> bytes;
    size_t size = str.size();
    bytes.resize(sizeof(size) + size);
    std::memcpy(bytes.data(), &size, sizeof(size));
    std::memcpy(bytes.data() + sizeof(size), str.data(), size);
    return bytes;
}

/**
 * @brief Serializes a std::vector into a vector of bytes.
 *
 * This function converts a `std::vector` into a vector of bytes, including
 * the size of the vector followed by the serialized elements.
 *
 * @tparam T The type of elements in the vector, must satisfy the `Serializable`
 * concept.
 * @param vec The vector to serialize.
 * @return std::vector<uint8_t> A vector of bytes representing the serialized
 * vector.
 */
template <typename T>
std::vector<uint8_t> serialize(const std::vector<T>& vec) {
    std::vector<uint8_t> bytes;
    size_t size = vec.size();
    auto size_bytes = serialize(size);
    bytes.insert(bytes.end(), size_bytes.begin(), size_bytes.end());

    for (const auto& item : vec) {
        auto item_bytes = serialize(item);
        bytes.insert(bytes.end(), item_bytes.begin(), item_bytes.end());
    }
    return bytes;
}

/**
 * @brief Serializes a std::list into a vector of bytes.
 *
 * This function converts a `std::list` into a vector of bytes, including
 * the size of the list followed by the serialized elements.
 *
 * @tparam T The type of elements in the list, must satisfy the `Serializable`
 * concept.
 * @param list The list to serialize.
 * @return std::vector<uint8_t> A vector of bytes representing the serialized
 * list.
 */
template <typename T>
std::vector<uint8_t> serialize(const std::list<T>& list) {
    std::vector<uint8_t> bytes;
    size_t size = list.size();
    auto size_bytes = serialize(size);
    bytes.insert(bytes.end(), size_bytes.begin(), size_bytes.end());

    for (const auto& item : list) {
        auto item_bytes = serialize(item);
        bytes.insert(bytes.end(), item_bytes.begin(), item_bytes.end());
    }
    return bytes;
}

/**
 * @brief Serializes a std::map into a vector of bytes.
 *
 * This function converts a `std::map` into a vector of bytes, including
 * the size of the map followed by the serialized key-value pairs.
 *
 * @tparam Key The type of keys in the map, must satisfy the `Serializable`
 * concept.
 * @tparam Value The type of values in the map, must satisfy the `Serializable`
 * concept.
 * @param map The map to serialize.
 * @return std::vector<uint8_t> A vector of bytes representing the serialized
 * map.
 */
template <typename Key, typename Value>
std::vector<uint8_t> serialize(const std::map<Key, Value>& map) {
    std::vector<uint8_t> bytes;
    size_t size = map.size();
    auto size_bytes = serialize(size);
    bytes.insert(bytes.end(), size_bytes.begin(), size_bytes.end());

    for (const auto& [key, value] : map) {
        auto key_bytes = serialize(key);
        auto value_bytes = serialize(value);
        bytes.insert(bytes.end(), key_bytes.begin(), key_bytes.end());
        bytes.insert(bytes.end(), value_bytes.begin(), value_bytes.end());
    }
    return bytes;
}

/**
 * @brief Serializes a std::optional into a vector of bytes.
 *
 * This function converts a `std::optional` into a vector of bytes, including
 * a boolean indicating whether the optional has a value, followed by the
 * serialized value if it exists.
 *
 * @tparam T The type of the value contained in the optional, must satisfy the
 * `Serializable` concept.
 * @param opt The optional to serialize.
 * @return std::vector<uint8_t> A vector of bytes representing the serialized
 * optional.
 */
template <typename T>
std::vector<uint8_t> serialize(const std::optional<T>& opt) {
    std::vector<uint8_t> bytes;
    bool has_value = opt.has_value();
    auto has_value_bytes = serialize(has_value);
    bytes.insert(bytes.end(), has_value_bytes.begin(), has_value_bytes.end());

    if (has_value) {
        auto value_bytes = serialize(opt.value());
        bytes.insert(bytes.end(), value_bytes.begin(), value_bytes.end());
    }
    return bytes;
}

/**
 * @brief Serializes a std::variant into a vector of bytes.
 *
 * This function converts a `std::variant` into a vector of bytes, including
 * the index of the active alternative, followed by the serialized value.
 *
 * @tparam Ts The types of the alternatives in the variant, must satisfy the
 * `Serializable` concept.
 * @param var The variant to serialize.
 * @return std::vector<uint8_t> A vector of bytes representing the serialized
 * variant.
 */
template <typename... Ts>
std::vector<uint8_t> serialize(const std::variant<Ts...>& var) {
    std::vector<uint8_t> bytes;
    size_t index = var.index();
    auto index_bytes = serialize(index);
    bytes.insert(bytes.end(), index_bytes.begin(), index_bytes.end());

    std::visit(
        [&bytes](const auto& value) {
            auto value_bytes = serialize(value);
            bytes.insert(bytes.end(), value_bytes.begin(), value_bytes.end());
        },
        var);
    return bytes;
}

/**
 * @brief Deserializes a type from a span of bytes.
 *
 * This function extracts data of a serializable type from the given span of
 * bytes, starting from the specified offset, and advances the offset.
 *
 * @tparam T The type of data to deserialize, must satisfy the `Serializable`
 * concept.
 * @param bytes The span of bytes containing the serialized data.
 * @param offset The offset in the span from where to start deserializing.
 * @return T The deserialized data.
 * @throws std::runtime_error if the data is too short to contain the expected
 * type.
 */
template <Serializable T>
T deserialize(const std::span<const uint8_t>& bytes, size_t& offset) {
    if (bytes.size() < offset + sizeof(T)) {
        throw std::runtime_error(
            "Invalid data: too short to contain the expected type.");
    }
    T data;
    std::memcpy(&data, bytes.data() + offset, sizeof(T));
    offset += sizeof(T);
    return data;
}

/**
 * @brief Deserializes a std::string from a span of bytes.
 *
 * This function extracts a `std::string` from the given span of bytes,
 * starting from the specified offset. The string is preceded by its size.
 *
 * @param bytes The span of bytes containing the serialized data.
 * @param offset The offset in the span from where to start deserializing.
 * @return std::string The deserialized string.
 * @throws std::runtime_error if the size of the string or the data is invalid.
 */
std::string deserializeString(const std::span<const uint8_t>& bytes,
                              size_t& offset) {
    size_t size = deserialize<size_t>(bytes, offset);
    if (bytes.size() < offset + size) {
        throw std::runtime_error("Invalid data: size mismatch.");
    }
    std::string str(reinterpret_cast<const char*>(bytes.data() + offset), size);
    offset += size;
    return str;
}

/**
 * @brief Deserializes a std::vector from a span of bytes.
 *
 * This function extracts a `std::vector` from the given span of bytes,
 * starting from the specified offset. The vector is preceded by its size.
 *
 * @tparam T The type of elements in the vector, must satisfy the `Serializable`
 * concept.
 * @param bytes The span of bytes containing the serialized data.
 * @param offset The offset in the span from where to start deserializing.
 * @return std::vector<T> The deserialized vector.
 */
template <typename T>
std::vector<T> deserializeVector(const std::span<const uint8_t>& bytes,
                                 size_t& offset) {
    size_t size = deserialize<size_t>(bytes, offset);
    std::vector<T> vec;
    vec.reserve(size);

    for (size_t i = 0; i < size; ++i) {
        T item = deserialize<T>(bytes, offset);
        vec.push_back(item);
    }
    return vec;
}

/**
 * @brief Deserializes a std::list from a span of bytes.
 *
 * This function extracts a `std::list` from the given span of bytes,
 * starting from the specified offset. The list is preceded by its size.
 *
 * @tparam T The type of elements in the list, must satisfy the `Serializable`
 * concept.
 * @param bytes The span of bytes containing the serialized data.
 * @param offset The offset in the span from where to start deserializing.
 * @return std::list<T> The deserialized list.
 */
template <typename T>
std::list<T> deserializeList(const std::span<const uint8_t>& bytes,
                             size_t& offset) {
    size_t size = deserialize<size_t>(bytes, offset);
    std::list<T> list;

    for (size_t i = 0; i < size; ++i) {
        T item = deserialize<T>(bytes, offset);
        list.push_back(item);
    }
    return list;
}

/**
 * @brief Deserializes a std::map from a span of bytes.
 *
 * This function extracts a `std::map` from the given span of bytes,
 * starting from the specified offset. The map is preceded by its size,
 * followed by serialized key-value pairs.
 *
 * @tparam Key The type of keys in the map, must satisfy the `Serializable`
 * concept.
 * @tparam Value The type of values in the map, must satisfy the `Serializable`
 * concept.
 * @param bytes The span of bytes containing the serialized data.
 * @param offset The offset in the span from where to start deserializing.
 * @return std::map<Key, Value> The deserialized map.
 */
template <typename Key, typename Value>
std::map<Key, Value> deserializeMap(const std::span<const uint8_t>& bytes,
                                    size_t& offset) {
    size_t size = deserialize<size_t>(bytes, offset);
    std::map<Key, Value> map;

    for (size_t i = 0; i < size; ++i) {
        Key key = (std::is_same_v<Key, std::string>)
                      ? deserializeString(bytes, offset)
                      : deserialize<Key>(bytes, offset);
        Value value = deserialize<Value>(bytes, offset);
        map.insert({key, value});
    }
    return map;
}

/**
 * @brief Deserializes a std::optional from a span of bytes.
 *
 * This function extracts a `std::optional` from the given span of bytes,
 * starting from the specified offset. It first reads a boolean indicating
 * whether the optional has a value, followed by the value if it exists.
 *
 * @tparam T The type of the value contained in the optional, must satisfy the
 * `Serializable` concept.
 * @param bytes The span of bytes containing the serialized data.
 * @param offset The offset in the span from where to start deserializing.
 * @return std::optional<T> The deserialized optional.
 */
template <typename T>
std::optional<T> deserializeOptional(const std::span<const uint8_t>& bytes,
                                     size_t& offset) {
    bool has_value = deserialize<bool>(bytes, offset);
    if (has_value) {
        return deserialize<T>(bytes, offset);
    }
    return std::nullopt;
}

/**
 * @brief Helper function to construct a std::variant from bytes.
 *
 * This function constructs a `std::variant` from the given bytes and index
 * of the active alternative. It uses the provided index sequence to deserialize
 * the value based on the alternative index.
 *
 * @tparam Variant The type of the variant to construct.
 * @tparam Is The index sequence for the variant alternatives.
 * @param bytes The span of bytes containing the serialized data.
 * @param offset The offset in the span from where to start deserializing.
 * @param index The index of the active alternative in the variant.
 * @param is The index sequence.
 * @return Variant The constructed variant.
 */
template <typename Variant, std::size_t... Is>
Variant constructVariant(const std::span<const uint8_t>& bytes, size_t& offset,
                         size_t index, std::index_sequence<Is...>) {
    Variant var;
    (
        [&](auto I) {
            if (index == I) {
                using T = std::variant_alternative_t<I, Variant>;
                var = deserialize<T>(bytes, offset);
            }
        }(std::integral_constant<std::size_t, Is>{}),
        ...);
    return var;
}

/**
 * @brief Deserializes a std::variant from a span of bytes.
 *
 * This function extracts a `std::variant` from the given span of bytes,
 * starting from the specified offset. It first reads the index of the active
 * alternative, then deserializes the value based on the alternative index.
 *
 * @tparam Ts The types of the alternatives in the variant, must satisfy the
 * `Serializable` concept.
 * @param bytes The span of bytes containing the serialized data.
 * @param offset The offset in the span from where to start deserializing.
 * @return std::variant<Ts...> The deserialized variant.
 * @throws std::runtime_error if the index of the variant is out of range.
 */
template <typename... Ts>
std::variant<Ts...> deserializeVariant(const std::span<const uint8_t>& bytes,
                                       size_t& offset) {
    size_t index = deserialize<size_t>(bytes, offset);
    if (index >= sizeof...(Ts)) {
        throw std::runtime_error("Invalid data: variant index out of range.");
    }
    return constructVariant<std::variant<Ts...>>(
        bytes, offset, index, std::index_sequence_for<Ts...>{});
}

/**
 * @brief Saves serialized data to a file.
 *
 * This function writes the given vector of bytes to a file. If the file cannot
 * be opened for writing, it throws a runtime error.
 *
 * @param data The vector of bytes to save.
 * @param filename The name of the file to write to.
 * @throws std::runtime_error if the file cannot be opened for writing.
 */
inline void saveToFile(const std::vector<uint8_t>& data,
                       const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file for writing: " +
                                 filename);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

/**
 * @brief Loads serialized data from a file.
 *
 * This function reads the contents of a file into a vector of bytes. If the
 * file cannot be opened for reading, it throws a runtime error.
 *
 * @param filename The name of the file to read from.
 * @return std::vector<uint8_t> A vector of bytes representing the loaded data.
 * @throws std::runtime_error if the file cannot be opened for reading.
 */
inline std::vector<uint8_t> loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file for reading: " +
                                 filename);
    }
    file.seekg(0, std::ios::end);
    std::vector<uint8_t> data(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), data.size());
    return data;
}

}  // namespace atom::utils

#endif  // ATOM_UTILS_TO_BYTE_HPP
