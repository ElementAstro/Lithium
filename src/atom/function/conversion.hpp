/*!
 * \file conversion.hpp
 * \brief C++ Type Conversion
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_CONVERSION_HPP
#define ATOM_META_CONVERSION_HPP

#include <any>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "type_info.hpp"

namespace atom::meta {
class bad_conversion : public std::bad_cast {
public:
    bad_conversion(const TypeInfo& from_type, const TypeInfo& to_type)
        : message("Failed to convert from " + std::string(from_type.name()) +
                  " to " + std::string(to_type.name())) {}

    const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};

class ConversionError : atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_CONVERSION_ERROR(...) \
    throw ConversionError(__FILE__, __LINE__, __func__, __VA_ARGS__)

class Type_Conversion_Base {
public:
    virtual std::any convert(const std::any& from) const = 0;
    virtual std::any convert_down(const std::any& to) const = 0;

    const TypeInfo& to() const noexcept { return to_type; }
    const TypeInfo& from() const noexcept { return from_type; }

    virtual bool bidir() const noexcept { return true; }

    virtual ~Type_Conversion_Base() = default;

    TypeInfo to_type;
    TypeInfo from_type;

protected:
    Type_Conversion_Base(const TypeInfo& to, const TypeInfo& from)
        : to_type(to), from_type(from) {}
};

template <typename From, typename To>
class Static_Conversion : public Type_Conversion_Base {
public:
    Static_Conversion()
        : Type_Conversion_Base(user_type<To>(), user_type<From>()) {}

    std::any convert(const std::any& from) const override {
        // Pointer types static conversion (upcasting)
        if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
            auto fromPtr = std::any_cast<From>(from);
            return std::any(static_cast<To>(fromPtr));
        }
        // Reference types static conversion (upcasting)
        else if constexpr (std::is_reference_v<From> &&
                           std::is_reference_v<To>) {
            try {
                auto& fromRef = std::any_cast<From&>(from);
                return std::any(static_cast<To&>(fromRef));
            } catch (const std::bad_cast&) {
                throw bad_conversion(from_type, to_type);
            }
        } else {
            throw bad_conversion(from_type, to_type);
        }
    }

    std::any convert_down(const std::any& to) const override {
        // Pointer types static conversion (downcasting)
        if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
            auto toPtr = std::any_cast<To>(to);
            return std::any(static_cast<From>(toPtr));
        }
        // Reference types static conversion (downcasting)
        else if constexpr (std::is_reference_v<From> &&
                           std::is_reference_v<To>) {
            try {
                auto& toRef = std::any_cast<To&>(to);
                return std::any(static_cast<From&>(toRef));
            } catch (const std::bad_cast&) {
                throw bad_conversion(to_type, from_type);
            }
        } else {
            throw bad_conversion(to_type, from_type);
        }
    }
};

template <typename From, typename To>
class Dynamic_Conversion : public Type_Conversion_Base {
public:
    Dynamic_Conversion()
        : Type_Conversion_Base(user_type<To>(), user_type<From>()) {}

    std::any convert(const std::any& from) const override {
        // Pointer types dynamic conversion
        if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
            auto fromPtr = std::any_cast<From>(from);
            auto convertedPtr = dynamic_cast<To>(fromPtr);
            if (!convertedPtr && fromPtr != nullptr)
                throw std::bad_cast();
            return std::any(convertedPtr);
        }
        // Reference types dynamic conversion
        else if constexpr (std::is_reference_v<From> &&
                           std::is_reference_v<To>) {
            try {
                auto& fromRef = std::any_cast<From&>(from);
                return std::any(dynamic_cast<To&>(fromRef));
            } catch (const std::bad_cast&) {
                throw bad_conversion(from_type, to_type);
            }
        } else {
            throw bad_conversion(from_type, to_type);
        }
    }

    std::any convert_down(const std::any& to) const override {
        // Pointer types dynamic conversion
        if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
            auto toPtr = std::any_cast<To>(to);
            auto convertedPtr = dynamic_cast<From>(toPtr);
            if (!convertedPtr && toPtr != nullptr)
                throw std::bad_cast();
            return std::any(convertedPtr);
        }
        // Reference types dynamic conversion
        else if constexpr (std::is_reference_v<From> &&
                           std::is_reference_v<To>) {
            try {
                auto& toRef = std::any_cast<To&>(to);
                return std::any(dynamic_cast<From&>(toRef));
            } catch (const std::bad_cast&) {
                throw bad_conversion(to_type, from_type);
            }
        } else {
            throw bad_conversion(to_type, from_type);
        }
    }
};

template <typename Base, typename Derived>
std::shared_ptr<Type_Conversion_Base> base_class() {
    if constexpr (std::is_polymorphic<Base>::value &&
                  std::is_polymorphic<Derived>::value) {
        return std::make_shared<Dynamic_Conversion<Derived*, Base*>>();
    } else {
        return std::make_shared<Static_Conversion<Derived, Base>>();
    }
}

// Specialized conversion for std::vector
template <typename From, typename To>
class Vector_Conversion : public Type_Conversion_Base {
public:
    Vector_Conversion()
        : Type_Conversion_Base(user_type<std::vector<To>>(),
                               user_type<std::vector<From>>()) {}

    std::any convert(const std::any& from) const override {
        try {
            const auto& fromVec = std::any_cast<const std::vector<From>&>(from);
            std::vector<To> toVec;
            toVec.reserve(fromVec.size());

            for (const auto& elem : fromVec) {
                // Convert each element using dynamic cast
                auto convertedElem =
                    std::dynamic_pointer_cast<typename To::element_type>(elem);
                if (!convertedElem)
                    throw std::bad_cast();
                toVec.push_back(convertedElem);
            }

            return std::any(toVec);
        } catch (const std::bad_any_cast&) {
            throw bad_conversion(from_type, to_type);
        }
    }

    std::any convert_down(const std::any& to) const override {
        try {
            const auto& toVec = std::any_cast<const std::vector<To>&>(to);
            std::vector<From> fromVec;
            fromVec.reserve(toVec.size());

            for (const auto& elem : toVec) {
                // Convert each element using dynamic cast
                auto convertedElem =
                    std::dynamic_pointer_cast<typename From::element_type>(
                        elem);
                if (!convertedElem)
                    throw std::bad_cast();
                fromVec.push_back(convertedElem);
            }

            return std::any(fromVec);
        }
        catch (const std::bad_any_cast&) {
            throw bad_conversion(to_type, from_type);
        }
    }
};

template <template <typename...> class MapType, typename K1, typename V1,
          typename K2, typename V2>
class Map_Conversion : public Type_Conversion_Base {
public:
    Map_Conversion()
        : Type_Conversion_Base(user_type<MapType<K2, V2>>(),
                               user_type<MapType<K1, V1>>()) {}

    std::any convert(const std::any& from) const override {
        try {
            const auto& fromMap = std::any_cast<const MapType<K1, V1>&>(from);
            MapType<K2, V2> toMap;

            for (const auto& [key, value] : fromMap) {
                // Convert each key and value in the map
                K2 convertedKey = std::any_cast<K2>(std::any(key));
                V2 convertedValue = std::any_cast<V2>(std::any(value));
                toMap.emplace(convertedKey, convertedValue);
            }

            return std::any(toMap);
        } catch (const std::bad_any_cast&) {
            throw bad_conversion(from_type, to_type);
        }
    }

    std::any convert_down(const std::any& to) const override {
        try {
            const auto& toMap = std::any_cast<const MapType<K2, V2>&>(to);
            MapType<K1, V1> fromMap;

            for (const auto& [key, value] : toMap) {
                // Convert each key and value in the map
                K1 convertedKey = std::any_cast<K1>(std::any(key));
                V1 convertedValue = std::any_cast<V1>(std::any(value));
                fromMap.emplace(convertedKey, convertedValue);
            }

            return std::any(fromMap);
        } catch (const std::bad_any_cast&) {
            throw bad_conversion(to_type, from_type);
        }
    }
};

template <template <typename...> class SeqType, typename From, typename To>
class Sequence_Conversion : public Type_Conversion_Base {
public:
    Sequence_Conversion()
        : Type_Conversion_Base(user_type<SeqType<To>>(),
                               user_type<SeqType<From>>()) {}

    std::any convert(const std::any& from) const override {
        try {
            const auto& fromSeq = std::any_cast<const SeqType<From>&>(from);
            SeqType<To> toSeq;

            for (const auto& elem : fromSeq) {
                // Convert each element using dynamic cast
                auto convertedElem =
                    std::dynamic_pointer_cast<typename To::element_type>(elem);
                if (!convertedElem)
                    throw std::bad_cast();
                toSeq.push_back(convertedElem);
            }

            return std::any(toSeq);
        } catch (const std::bad_any_cast&) {
            throw bad_conversion(from_type, to_type);
        }
    }

    std::any convert_down(const std::any& to) const override {
        try {
            const auto& toSeq = std::any_cast<const SeqType<To>&>(to);
            SeqType<From> fromSeq;

            for (const auto& elem : toSeq) {
                auto convertedElem =
                    std::dynamic_pointer_cast<typename From::element_type>(
                        elem);
                if (!convertedElem)
                    throw std::bad_cast();
                fromSeq.push_back(convertedElem);
            }

            return std::any(fromSeq);
        } catch (const std::bad_any_cast&) {
            throw bad_conversion(to_type, from_type);
        }
    }
};

template <template <typename...> class SetType, typename From, typename To>
class Set_Conversion : public Type_Conversion_Base {
public:
    Set_Conversion()
        : Type_Conversion_Base(user_type<SetType<To>>(),
                               user_type<SetType<From>>()) {}

    std::any convert(const std::any& from) const override {
        try {
            const auto& fromSet = std::any_cast<const SetType<From>&>(from);
            SetType<To> toSet;

            for (const auto& elem : fromSet) {
                auto convertedElem =
                    std::dynamic_pointer_cast<typename To::element_type>(elem);
                if (!convertedElem)
                    throw std::bad_cast();
                toSet.insert(convertedElem);
            }

            return std::any(toSet);
        } catch (const std::bad_any_cast&) {
            throw bad_conversion(from_type, to_type);
        }
    }

    std::any convert_down(const std::any& to) const override {
        try {
            const auto& toSet = std::any_cast<const SetType<To>&>(to);
            SetType<From> fromSet;

            for (const auto& elem : toSet) {
                auto convertedElem =
                    std::dynamic_pointer_cast<typename From::element_type>(
                        elem);
                if (!convertedElem)
                    throw std::bad_cast();
                fromSet.insert(convertedElem);
            }

            return std::any(fromSet);
        } catch (const std::bad_any_cast&) {
            throw bad_conversion(to_type, from_type);
        }
    }
};

class TypeConversions {
public:
    TypeConversions() = default;

    static std::shared_ptr<TypeConversions> createShared() {
        return std::make_shared<TypeConversions>();
    }

    void add_conversion(
        const std::shared_ptr<Type_Conversion_Base>& conversion) {
        auto key = conversion->from_type;
        conversions[key].push_back(conversion);
    }

    template <typename To, typename From>
    std::any convert(const std::any& from) const {
        auto from_type = user_type<From>();
        auto to_type = user_type<To>();

        if (conversions.count(from_type)) {
            for (const auto& conv : conversions.at(from_type)) {
                if (conv->to_type == to_type) {
                    try {
                        return conv->convert(from);
                    } catch (const std::bad_any_cast& e) {
                        THROW_CONVERSION_ERROR(from_type.name(), to_type.name(),
                                               e.what());
                    }
                }
            }
        }
        throw bad_conversion(from_type, to_type);
    }

    bool can_convert(const TypeInfo& from, const TypeInfo& to) const {
        if (conversions.count(from)) {
            for (const auto& conv : conversions.at(from)) {
                if (conv->to_type == to) {
                    return true;
                }
            }
        }
        return false;
    }

    template <typename Base, typename Derived>
    void add_base_class() {
        add_conversion(std::make_shared<Dynamic_Conversion<Derived*, Base*>>());
        if constexpr (!std::is_same_v<Base, Derived>) {
            add_conversion(
                std::make_shared<Static_Conversion<Base, Derived>>());
        }
    }

    // In TypeConversions class
    template <template <typename...> class MapType, typename K1, typename V1,
              typename K2, typename V2>
    void add_map_conversion() {
        add_conversion(
            std::make_shared<Map_Conversion<MapType, K1, V1, K2, V2>>());
    }

    // In TypeConversions class
    template <typename From, typename To>
    void add_vector_conversion() {
        add_conversion(
            std::make_shared<Vector_Conversion<std::shared_ptr<From>,
                                               std::shared_ptr<To>>>());
    }

    template <template <typename...> class SeqType, typename From, typename To>
    void add_sequence_conversion() {
        add_conversion(
            std::make_shared<Sequence_Conversion<SeqType, std::shared_ptr<From>,
                                                 std::shared_ptr<To>>>());
    }

    template <template <typename...> class SetType, typename From, typename To>
    void add_set_conversion() {
        add_conversion(
            std::make_shared<Set_Conversion<SetType, std::shared_ptr<From>,
                                            std::shared_ptr<To>>>());
    }

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<TypeInfo,
                     std::vector<std::shared_ptr<Type_Conversion_Base>>,
                     std::hash<TypeInfo>>
        conversions;
#else
    std::unordered_map<TypeInfo,
                       std::vector<std::shared_ptr<Type_Conversion_Base>>,
                       std::hash<TypeInfo>>
        conversions;
#endif
};

}  // namespace atom::meta

#endif
