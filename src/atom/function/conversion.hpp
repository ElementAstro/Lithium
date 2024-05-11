/*
 * conversion.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: C++ Type Conversion

**************************************************/

#ifndef ATOM_FUNCTION_CONVERSION_HPP
#define ATOM_FUNCTION_CONVERSION_HPP

#include <any>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "type_info.hpp"

#include "conversion_stl.hpp"

class Type_Conversion_Base {
public:
    virtual std::any convert(const std::any& from) const = 0;
    virtual ~Type_Conversion_Base() = default;

    Type_Info to_type;
    Type_Info from_type;

protected:
    Type_Conversion_Base(const Type_Info& to, const Type_Info& from)
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

class Type_Conversions {
public:
    Type_Conversions() = default;

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
                        std::cerr << "Bad any_cast: " << e.what() << "\n";
                        throw;
                    }
                }
            }
        }
        throw bad_conversion(from_type, to_type);
    }

    bool can_convert(const Type_Info& from, const Type_Info& to) const {
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

    // In Type_Conversions class
    template <template <typename...> class MapType, typename K1, typename V1,
              typename K2, typename V2>
    void add_map_conversion() {
        add_conversion(
            std::make_shared<Map_Conversion<MapType, K1, V1, K2, V2>>());
    }

    // In Type_Conversions class
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
    emhash8::HashMap<Type_Info,
                       std::vector<std::shared_ptr<Type_Conversion_Base>>,
                       std::hash<Type_Info>>
        conversions;
#else
    std::unordered_map<Type_Info,
                       std::vector<std::shared_ptr<Type_Conversion_Base>>,
                       std::hash<Type_Info>>
        conversions;
#endif
};

#endif