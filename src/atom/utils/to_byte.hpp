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
// Helper to determine if a type is serializable as plain bytes
template <typename T>
concept Serializable =
    std::is_arithmetic_v<T> || std::is_enum_v<T> ||
    std::same_as<T, std::string> || requires(T a) { serialize(a); };

// General serialize function for serializable types
template <Serializable T>
std::vector<uint8_t> serialize(const T& data) {
    std::vector<uint8_t> bytes(sizeof(T));
    std::memcpy(bytes.data(), &data, sizeof(T));
    return bytes;
}

// Serialize std::string
inline std::vector<uint8_t> serialize(const std::string& str) {
    std::vector<uint8_t> bytes;
    size_t size = str.size();
    bytes.resize(sizeof(size) + size);
    std::memcpy(bytes.data(), &size, sizeof(size));
    std::memcpy(bytes.data() + sizeof(size), str.data(), size);
    return bytes;
}

// Serialize std::vector
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

// Serialize std::list
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

// Serialize std::map
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

// Serialize std::optional
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

// Serialize std::variant
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

// Deserialize helper function
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

// Deserialize std::string
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

// Deserialize std::vector
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

// Deserialize std::list
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

// Deserialize std::map
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

// Deserialize std::optional
template <typename T>
std::optional<T> deserializeOptional(const std::span<const uint8_t>& bytes,
                                     size_t& offset) {
    bool has_value = deserialize<bool>(bytes, offset);
    if (has_value) {
        return deserialize<T>(bytes, offset);
    }
    return std::nullopt;
}

// Deserialize std::variant
// Helper function to construct a variant from index and bytes
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

// Deserialize std::variant
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

// Save serialized data to a file
inline void saveToFile(const std::vector<uint8_t>& data,
                       const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file for writing: " +
                                 filename);
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// Load serialized data from a file
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

#endif
