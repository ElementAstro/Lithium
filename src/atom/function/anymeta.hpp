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
#include "macro.hpp"
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
    } ATOM_ALIGNAS(64);

private:
    std::unordered_map<std::string, MethodFunction> m_methods_;
    std::unordered_map<std::string, Property> m_properties_;
    std::vector<ConstructorFunction> m_constructors_;

public:
    void addMethod(const std::string& name, MethodFunction method) {
        m_methods_[name] = std::move(method);
    }

    void addProperty(const std::string& name, GetterFunction getter,
                     SetterFunction setter) {
        m_properties_[name] = {std::move(getter), std::move(setter)};
    }

    void addConstructor(ConstructorFunction constructor) {
        m_constructors_.push_back(std::move(constructor));
    }

    auto getMethod(const std::string& name) const
        -> std::optional<MethodFunction> {
        if (auto it = m_methods_.find(name); it != m_methods_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    auto getProperty(const std::string& name) const -> std::optional<Property> {
        if (auto it = m_properties_.find(name); it != m_properties_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    auto getConstructor(size_t index = 0) const
        -> std::optional<ConstructorFunction> {
        if (index < m_constructors_.size()) {
            return m_constructors_[index];
        }
        return std::nullopt;
    }
};

class TypeRegistry {
private:
    std::unordered_map<std::string, TypeMetadata> m_registry_;
    mutable std::shared_mutex m_mutex_;

public:
    static auto instance() -> TypeRegistry& {
        static TypeRegistry registry;
        return registry;
    }

    void registerType(const std::string& name, TypeMetadata metadata) {
        std::unique_lock lock(m_mutex_);
        m_registry_[name] = std::move(metadata);
    }

    auto getMetadata(const std::string& name) const
        -> std::optional<TypeMetadata> {
        std::shared_lock lock(m_mutex_);
        auto it = m_registry_.find(name);
        if (it != m_registry_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
};

// Helper function to call methods dynamically
inline auto callMethod(BoxedValue& obj, const std::string& method_name,
                       std::vector<BoxedValue> args) -> BoxedValue {
    auto metadata =
        TypeRegistry::instance().getMetadata(obj.getTypeInfo().name());
    if (metadata) {
        auto method = metadata->getMethod(method_name);
        if (method) {
            args.insert(args.begin(), obj);
            return (*method)(args);
        }
    }
    THROW_NOT_FOUND("Method not found");
}

// Helper function to get/set properties dynamically
inline auto getProperty(BoxedValue& obj,
                        const std::string& property_name) -> BoxedValue {
    auto metadata =
        TypeRegistry::instance().getMetadata(obj.getTypeInfo().name());
    if (metadata) {
        auto property = metadata->getProperty(property_name);
        if (property) {
            return property->getter(obj);
        }
    }
    THROW_NOT_FOUND("Property not found");
}

inline void setProperty(BoxedValue& obj, const std::string& property_name,
                        const BoxedValue& value) {
    auto metadata =
        TypeRegistry::instance().getMetadata(obj.getTypeInfo().name());
    if (metadata) {
        auto property = metadata->getProperty(property_name);
        if (property) {
            property->setter(obj, value);
            return;
        }
    }
    THROW_NOT_FOUND("Property not found");
}
}  // namespace atom::meta

#endif
