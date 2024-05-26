/*!
 * \file conversion_stl.hpp
 * \brief C++ Type Conversion For STL Types
 * \author Max Qian <lightapt.com>
 * \date 2024-03-01
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_CONVERSION_STL_HPP
#define ATOM_META_CONVERSION_STL_HPP

#include "conversion.hpp"

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
};

#endif
