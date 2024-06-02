/*!
 * \file anymeta.hpp
 * \brief Type Metadata for Any
 * \author Max Qian <lightapt.com>
 * \date 2023-12-28
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_ANYMETA_HPP
#define ATOM_META_ANYMETA_HPP

#include "any.hpp"
#include "atom/error/exception.hpp"
#include "type_info.hpp"

namespace atom::meta {
class TypeMetadata {
public:
    using MethodFunction = std::function<BoxedValue(std::vector<BoxedValue>)>;
    using GetterFunction = std::function<BoxedValue(const BoxedValue&)>;
    using SetterFunction = std::function<void(BoxedValue&, const BoxedValue&)>;
    using ConstructorFunction =
        std::function<BoxedValue(std::vector<BoxedValue>)>;

    struct Property {
        GetterFunction getter;
        SetterFunction setter;
    };

private:
    std::unordered_map<std::string, MethodFunction> m_methods;
    std::unordered_map<std::string, Property> m_properties;
    std::vector<ConstructorFunction> m_constructors;

public:
    void add_method(const std::string& name, MethodFunction method) {
        m_methods[name] = std::move(method);
    }

    void add_property(const std::string& name, GetterFunction getter,
                      SetterFunction setter) {
        m_properties[name] = {std::move(getter), std::move(setter)};
    }

    void add_constructor(ConstructorFunction constructor) {
        m_constructors.push_back(std::move(constructor));
    }

    std::optional<MethodFunction> get_method(const std::string& name) const {
        if (auto it = m_methods.find(name); it != m_methods.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<Property> get_property(const std::string& name) const {
        if (auto it = m_properties.find(name); it != m_properties.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<ConstructorFunction> get_constructor(size_t index = 0) const {
        if (index < m_constructors.size()) {
            return m_constructors[index];
        }
        return std::nullopt;
    }
};

class TypeRegistry {
private:
    std::unordered_map<std::string, TypeMetadata> m_registry;
    mutable std::shared_mutex m_mutex;

public:
    static TypeRegistry& instance() {
        static TypeRegistry registry;
        return registry;
    }

    void register_type(const std::string& name, TypeMetadata metadata) {
        std::unique_lock lock(m_mutex);
        m_registry[name] = std::move(metadata);
    }

    std::optional<TypeMetadata> get_metadata(const std::string& name) const {
        std::shared_lock lock(m_mutex);
        auto it = m_registry.find(name);
        if (it != m_registry.end()) {
            return it->second;
        }
        return std::nullopt;
    }
};

// Helper function to call methods dynamically
inline BoxedValue call_method(BoxedValue& obj, const std::string& method_name,
                              std::vector<BoxedValue> args) {
    auto metadata =
        TypeRegistry::instance().get_metadata(obj.get_type_info().name());
    if (metadata) {
        auto method = metadata->get_method(method_name);
        if (method) {
            args.insert(args.begin(), obj);
            return (*method)(args);
        }
    }
    THROW_NOT_FOUND("Method not found");
}

// Helper function to get/set properties dynamically
inline BoxedValue get_property(BoxedValue& obj,
                               const std::string& property_name) {
    auto metadata =
        TypeRegistry::instance().get_metadata(obj.get_type_info().name());
    if (metadata) {
        auto property = metadata->get_property(property_name);
        if (property) {
            return property->getter(obj);
        }
    }
    THROW_NOT_FOUND("Property not found");
}

inline void set_property(BoxedValue& obj, const std::string& property_name,
                         const BoxedValue& value) {
    auto metadata =
        TypeRegistry::instance().get_metadata(obj.get_type_info().name());
    if (metadata) {
        auto property = metadata->get_property(property_name);
        if (property) {
            property->setter(obj, value);
            return;
        }
    }
    THROW_NOT_FOUND("Property not found");
}
}  // namespace atom::meta

#endif
