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
#include <type_traits>
#include <typeinfo>
#include <vector>
#include "macro.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "type_info.hpp"

namespace atom::meta {

class BadConversionException : public error::RuntimeError {
    using atom::error::RuntimeError::RuntimeError;
};

#define THROW_CONVERSION_ERROR(...)                              \
    throw BadConversionException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                 ATOM_FUNC_NAME, __VA_ARGS__)

class TypeConversionBase {
public:
    ATOM_NODISCARD virtual auto convert(const std::any& from) const
        -> std::any = 0;
    ATOM_NODISCARD virtual auto convertDown(const std::any& to) const
        -> std::any = 0;

    ATOM_NODISCARD auto to() const ATOM_NOEXCEPT -> const TypeInfo& {
        return toType;
    }
    ATOM_NODISCARD auto from() const ATOM_NOEXCEPT -> const TypeInfo& {
        return fromType;
    }

    ATOM_NODISCARD virtual auto bidir() const ATOM_NOEXCEPT -> bool {
        return true;
    }

    virtual ~TypeConversionBase() = default;

    TypeInfo toType;
    TypeInfo fromType;

protected:
    TypeConversionBase(const TypeInfo& to, const TypeInfo& from)
        : toType(to), fromType(from) {}
};

template <typename From, typename To>
class StaticConversion : public TypeConversionBase {
public:
    StaticConversion() : TypeConversionBase(userType<To>(), userType<From>()) {}

    ATOM_NODISCARD auto convert(const std::any& from) const
        -> std::any override {
        // Pointer types static conversion (upcasting)
        try {
            if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
                auto fromPtr = std::any_cast<From>(from);
                return std::any(static_cast<To>(fromPtr));
            }
            // Reference types static conversion (upcasting)
            else if constexpr (std::is_reference_v<From> &&
                               std::is_reference_v<To>) {
                auto& fromRef = std::any_cast<From&>(from);
                return std::any(static_cast<To&>(fromRef));

            } else {
                THROW_CONVERSION_ERROR("Failed to convert ", fromType.name(),
                                       " to ", toType.name());
            }
        } catch (const std::bad_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", fromType.name(),
                                   " to ", toType.name());
        }
    }

    ATOM_NODISCARD auto convertDown(const std::any& to) const
        -> std::any override {
        // Pointer types static conversion (downcasting)
        try {
            if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
                auto toPtr = std::any_cast<To>(to);
                return std::any(static_cast<From>(toPtr));
            }
            // Reference types static conversion (downcasting)
            else if constexpr (std::is_reference_v<From> &&
                               std::is_reference_v<To>) {
                auto& toRef = std::any_cast<To&>(to);
                return std::any(static_cast<From&>(toRef));

            } else {
                THROW_CONVERSION_ERROR("Failed to convert ", toType.name(),
                                       " to ", fromType.name());
            }
        } catch (const std::bad_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", toType.name(), " to ",
                                   fromType.name());
        }
    }
};

template <typename From, typename To>
class DynamicConversion : public TypeConversionBase {
public:
    DynamicConversion()
        : TypeConversionBase(userType<To>(), userType<From>()) {}

    ATOM_NODISCARD auto convert(const std::any& from) const
        -> std::any override {
        // Pointer types dynamic conversion
        if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
            auto fromPtr = std::any_cast<From>(from);
            auto convertedPtr = dynamic_cast<To>(fromPtr);
            if (!convertedPtr && fromPtr != nullptr) {
                throw std::bad_cast();
            }
            return std::any(convertedPtr);
        }
        // Reference types dynamic conversion
        else if constexpr (std::is_reference_v<From> &&
                           std::is_reference_v<To>) {
            try {
                auto& fromRef = std::any_cast<From&>(from);
                return std::any(dynamic_cast<To&>(fromRef));
            } catch (const std::bad_cast&) {
                THROW_CONVERSION_ERROR("Failed to convert ", fromType.name(),
                                       " to ", toType.name());
            }
        } else {
            THROW_CONVERSION_ERROR("Failed to convert ", fromType.name(),
                                   " to ", toType.name());
        }
    }

    ATOM_NODISCARD auto convertDown(const std::any& to) const
        -> std::any override {
        // Pointer types dynamic conversion
        if constexpr (std::is_pointer_v<From> && std::is_pointer_v<To>) {
            auto toPtr = std::any_cast<To>(to);
            auto convertedPtr = dynamic_cast<From>(toPtr);
            if (!convertedPtr && toPtr != nullptr) {
                throw std::bad_cast();
            }
            return std::any(convertedPtr);
        }
        // Reference types dynamic conversion
        else if constexpr (std::is_reference_v<From> &&
                           std::is_reference_v<To>) {
            try {
                auto& toRef = std::any_cast<To&>(to);
                return std::any(dynamic_cast<From&>(toRef));
            } catch (const std::bad_cast&) {
                THROW_CONVERSION_ERROR("Failed to convert ", toType.name(),
                                       " to ", fromType.name());
            }
        } else {
            THROW_CONVERSION_ERROR("Failed to convert ", toType.name(), " to ",
                                   fromType.name());
        }
    }
};

template <typename Base, typename Derived>
auto baseClass() -> std::shared_ptr<TypeConversionBase> {
    if constexpr (std::is_polymorphic_v<Base> &&
                  std::is_polymorphic_v<Derived>) {
        return std::make_shared<DynamicConversion<Derived*, Base*>>();
    } else {
        return std::make_shared<StaticConversion<Derived, Base>>();
    }
}

// Specialized conversion for std::vector
template <typename From, typename To>
class VectorConversion : public TypeConversionBase {
public:
    VectorConversion()
        : TypeConversionBase(userType<std::vector<To>>(),
                             userType<std::vector<From>>()) {}

    std::any convert(const std::any& from) const override {
        try {
            const auto& fromVec = std::any_cast<const std::vector<From>&>(from);
            std::vector<To> toVec;
            toVec.reserve(fromVec.size());

            for (const auto& elem : fromVec) {
                // Convert each element using dynamic cast
                auto convertedElem =
                    std::dynamic_pointer_cast<typename To::element_type>(elem);
                if (!convertedElem) {
                    throw std::bad_cast();
                }
                toVec.push_back(convertedElem);
            }

            return std::any(toVec);
        } catch (const std::bad_any_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", fromType.name(),
                                   " to ", toType.name());
        }
    }

    ATOM_NODISCARD auto convertDown(const std::any& to) const
        -> std::any override {
        try {
            const auto& toVec = std::any_cast<const std::vector<To>&>(to);
            std::vector<From> fromVec;
            fromVec.reserve(toVec.size());

            for (const auto& elem : toVec) {
                // Convert each element using dynamic cast
                auto convertedElem =
                    std::dynamic_pointer_cast<typename From::element_type>(
                        elem);
                if (!convertedElem) {
                    throw std::bad_cast();
                }
                fromVec.push_back(convertedElem);
            }

            return std::any(fromVec);
        } catch (const std::bad_any_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", toType.name(), " to ",
                                   fromType.name());
        }
    }
};

template <template <typename...> class MapType, typename K1, typename V1,
          typename K2, typename V2>
class MapConversion : public TypeConversionBase {
public:
    MapConversion()
        : TypeConversionBase(userType<MapType<K2, V2>>(),
                             userType<MapType<K1, V1>>()) {}

    ATOM_NODISCARD auto convert(const std::any& from) const
        -> std::any override {
        try {
            const auto& fromMap = std::any_cast<const MapType<K1, V1>&>(from);
            MapType<K2, V2> toMap;

            for (const auto& [key, value] : fromMap) {
                K2 convertedKey = static_cast<K2>(key);
                V2 convertedValue =
                    std::dynamic_pointer_cast<typename V2::element_type>(value);
                if (!convertedValue) {
                    THROW_CONVERSION_ERROR("Failed to convert value in map");
                }
                toMap.emplace(convertedKey, convertedValue);
            }

            return std::any(toMap);
        } catch (const std::bad_any_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", fromType.name(),
                                   " to ", toType.name());
        }
    }

    ATOM_NODISCARD auto convertDown(const std::any& to) const
        -> std::any override {
        try {
            const auto& toMap = std::any_cast<const MapType<K2, V2>&>(to);
            MapType<K1, V1> fromMap;

            for (const auto& [key, value] : toMap) {
                K1 convertedKey = static_cast<K1>(key);
                V1 convertedValue =
                    std::dynamic_pointer_cast<typename V1::element_type>(value);
                if (!convertedValue) {
                    THROW_CONVERSION_ERROR("Failed to convert value in map");
                }
                fromMap.emplace(convertedKey, convertedValue);
            }

            return std::any(fromMap);
        } catch (const std::bad_any_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", toType.name(), " to ",
                                   fromType.name());
        }
    }
};

template <template <typename...> class SeqType, typename From, typename To>
class SequenceConversion : public TypeConversionBase {
public:
    SequenceConversion()
        : TypeConversionBase(userType<SeqType<To>>(),
                             userType<SeqType<From>>()) {}

    ATOM_NODISCARD auto convert(const std::any& from) const
        -> std::any override {
        try {
            const auto& fromSeq = std::any_cast<const SeqType<From>&>(from);
            SeqType<To> toSeq;

            for (const auto& elem : fromSeq) {
                // Convert each element using dynamic cast
                auto convertedElem =
                    std::dynamic_pointer_cast<typename To::element_type>(elem);
                if (!convertedElem) {
                    throw std::bad_cast();
                }
                toSeq.push_back(convertedElem);
            }

            return std::any(toSeq);
        } catch (const std::bad_any_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", fromType.name(),
                                   " to ", toType.name());
        }
    }

    ATOM_NODISCARD auto convertDown(const std::any& to) const
        -> std::any override {
        try {
            const auto& toSeq = std::any_cast<const SeqType<To>&>(to);
            SeqType<From> fromSeq;

            for (const auto& elem : toSeq) {
                auto convertedElem =
                    std::dynamic_pointer_cast<typename From::element_type>(
                        elem);
                if (!convertedElem) {
                    throw std::bad_cast();
                }
                fromSeq.push_back(convertedElem);
            }

            return std::any(fromSeq);
        } catch (const std::bad_any_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", toType.name(), " to ",
                                   fromType.name());
        }
    }
};

template <template <typename...> class SetType, typename From, typename To>
class SetConversion : public TypeConversionBase {
public:
    SetConversion()
        : TypeConversionBase(userType<SetType<To>>(),
                             userType<SetType<From>>()) {}

    ATOM_NODISCARD auto convert(const std::any& from) const
        -> std::any override {
        try {
            const auto& fromSet = std::any_cast<const SetType<From>&>(from);
            SetType<To> toSet;

            for (const auto& elem : fromSet) {
                auto convertedElem =
                    std::dynamic_pointer_cast<typename To::element_type>(elem);
                if (!convertedElem) {
                    throw std::bad_cast();
                }
                toSet.insert(convertedElem);
            }

            return std::any(toSet);
        } catch (const std::bad_any_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", fromType.name(),
                                   " to ", toType.name());
        }
    }

    ATOM_NODISCARD auto convertDown(const std::any& to) const
        -> std::any override {
        try {
            const auto& toSet = std::any_cast<const SetType<To>&>(to);
            SetType<From> fromSet;

            for (const auto& elem : toSet) {
                auto convertedElem =
                    std::dynamic_pointer_cast<typename From::element_type>(
                        elem);
                if (!convertedElem) {
                    throw std::bad_cast();
                }
                fromSet.insert(convertedElem);
            }

            return std::any(fromSet);
        } catch (const std::bad_any_cast&) {
            THROW_CONVERSION_ERROR("Failed to convert ", toType.name(), " to ",
                                   fromType.name());
        }
    }
};

class TypeConversions {
public:
    TypeConversions() = default;

    static auto createShared() -> std::shared_ptr<TypeConversions> {
        return std::make_shared<TypeConversions>();
    }

    void addConversion(const std::shared_ptr<TypeConversionBase>& conversion) {
        auto key = conversion->fromType;
        conversions_[key].push_back(conversion);
    }

    template <typename To, typename From>
    auto convert(const std::any& from) const -> std::any {
        auto fromType = userType<From>();
        auto toType = userType<To>();

        if (conversions_.count(fromType)) {
            for (const auto& conv : conversions_.at(fromType)) {
                if (conv->toType == toType) {
                    try {
                        return conv->convert(from);
                    } catch (const std::bad_any_cast& e) {
                        THROW_CONVERSION_ERROR(fromType.name(), toType.name(),
                                               e.what());
                    }
                }
            }
        }
        THROW_CONVERSION_ERROR(fromType.name(), toType.name());
    }

    auto canConvert(const TypeInfo& from, const TypeInfo& to) const -> bool {
        if (conversions_.contains(from)) {
            for (const auto& conv : conversions_.at(from)) {
                if (conv->toType == to) {
                    return true;
                }
            }
        }
        return false;
    }

    template <typename Base, typename Derived>
    void addBaseClass() {
        addConversion(std::make_shared<DynamicConversion<Derived*, Base*>>());

        if constexpr (!std::is_same_v<Base, Derived>) {
            addConversion(std::make_shared<StaticConversion<Derived, Base>>());
        }
    }

    template <template <typename...> class MapType, typename K1, typename V1,
              typename K2, typename V2>
    void addMapConversion() {
        addConversion(
            std::make_shared<MapConversion<MapType, K1, V1, K2, V2>>());
    }

    template <typename From, typename To>
    void addVectorConversion() {
        addConversion(
            std::make_shared<VectorConversion<std::shared_ptr<From>,
                                              std::shared_ptr<To>>>());
    }

    template <template <typename...> class SeqType, typename From, typename To>
    void addSequenceConversion() {
        addConversion(
            std::make_shared<SequenceConversion<SeqType, std::shared_ptr<From>,
                                                std::shared_ptr<To>>>());
    }

    template <template <typename...> class SetType, typename From, typename To>
    void addSetConversion() {
        addConversion(
            std::make_shared<SetConversion<SetType, std::shared_ptr<From>,
                                           std::shared_ptr<To>>>());
    }

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<TypeInfo, std::vector<std::shared_ptr<TypeConversionBase>>,
                     std::hash<TypeInfo>>
        conversions_;
#else
    std::unordered_map<TypeInfo,
                       std::vector<std::shared_ptr<TypeConversionBase>>,
                       std::hash<TypeInfo>>
        conversions_;
#endif
};

}  // namespace atom::meta

#endif  // ATOM_META_CONVERSION_HPP
