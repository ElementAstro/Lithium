/*!
 * \file anymeta.hpp
 * \brief Enhanced Type Metadata with Dynamic Reflection, Method Overloads, and
 * Event System \author Max Qian <lightapt.com> \date 2023-12-28 \copyright
 * Copyright (C) 2023-2024 Max Qian
 */

#ifndef ATOM_META_ANYMETA_HPP
#define ATOM_META_ANYMETA_HPP

#include "any.hpp"
#include "type_info.hpp"

#include <functional>
#include <iostream>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/error/exception.hpp"

#include "atom/macro.hpp"

namespace atom::meta {
class TypeMetadata {
public:
    using MethodFunction = std::function<BoxedValue(std::vector<BoxedValue>)>;
    using GetterFunction = std::function<BoxedValue(const BoxedValue&)>;
    using SetterFunction = std::function<void(BoxedValue&, const BoxedValue&)>;
    using ConstructorFunction =
        std::function<BoxedValue(std::vector<BoxedValue>)>;
    using EventCallback =
        std::function<void(BoxedValue&, const std::vector<BoxedValue>&)>;

    struct ATOM_ALIGNAS(64) Property {
        GetterFunction getter;
        SetterFunction setter;
        BoxedValue default_value;
        std::string description;
    };

    struct ATOM_ALIGNAS(32) Event {
        std::vector<std::pair<int, EventCallback>>
            listeners;  // Pair of priority and callback
        std::string description;
    };

private:
    std::unordered_map<std::string, std::vector<MethodFunction>>
        m_methods_;  // Supports overloaded methods
    std::unordered_map<std::string, Property> m_properties_;
    std::unordered_map<std::string, std::vector<ConstructorFunction>>
        m_constructors_;
    std::unordered_map<std::string, Event> m_events_;

public:
    // Add overloaded method to type metadata
    void addMethod(const std::string& name, MethodFunction method) {
        m_methods_[name].push_back(std::move(method));
    }

    // Remove method by name
    void removeMethod(const std::string& name) { m_methods_.erase(name); }

    // Add property (getter and setter) to type metadata
    void addProperty(const std::string& name, GetterFunction getter,
                     SetterFunction setter, BoxedValue default_value = {},
                     const std::string& description = "") {
        m_properties_[name] = {std::move(getter), std::move(setter),
                               std::move(default_value), description};
    }

    // Remove property by name
    void removeProperty(const std::string& name) { m_properties_.erase(name); }

    // Add constructor to type metadata with an associated type name
    void addConstructor(const std::string& type_name,
                        ConstructorFunction constructor) {
        m_constructors_[type_name].push_back(std::move(constructor));
    }

    // Add event to type metadata
    void addEvent(const std::string& event_name,
                  const std::string& description = "") {
        m_events_[event_name].description =
            description;  // Creates an empty event with description
    }

    // Remove event by name
    void removeEvent(const std::string& event_name) {
        m_events_.erase(event_name);
    }

    // Add event listener to a specific event with priority
    void addEventListener(const std::string& event_name, EventCallback callback,
                          int priority = 0) {
        m_events_[event_name].listeners.emplace_back(priority,
                                                     std::move(callback));
        std::sort(m_events_[event_name].listeners.begin(),
                  m_events_[event_name].listeners.end(),
                  [](const auto& a, const auto& b) {
                      return a.first > b.first;  // Higher priority first
                  });
    }

    // Fire event and notify listeners
    void fireEvent(BoxedValue& obj, const std::string& event_name,
                   const std::vector<BoxedValue>& args) const {
        if (auto eventIter = m_events_.find(event_name);
            eventIter != m_events_.end()) {
            for (const auto& [priority, listener] :
                 eventIter->second.listeners) {
                listener(obj, args);
            }
        } else {
            std::cerr << "Event " << event_name << " not found." << std::endl;
        }
    }

    // Retrieve all overloaded methods by name
    [[nodiscard]] auto getMethods(const std::string& name) const
        -> std::optional<const std::vector<MethodFunction>*> {
        if (auto methodIter = m_methods_.find(name);
            methodIter != m_methods_.end()) {
            return &methodIter->second;
        }
        return std::nullopt;
    }

    // Retrieve property by name
    [[nodiscard]] auto getProperty(const std::string& name) const
        -> std::optional<Property> {
        if (auto propertyIter = m_properties_.find(name);
            propertyIter != m_properties_.end()) {
            return propertyIter->second;
        }
        return std::nullopt;
    }

    // Retrieve constructor by index (defaults to the first constructor)
    [[nodiscard]] auto getConstructor(const std::string& type_name,
                                      size_t index = 0) const
        -> std::optional<ConstructorFunction> {
        if (auto constructorIter = m_constructors_.find(type_name);
            constructorIter != m_constructors_.end()) {
            if (index < constructorIter->second.size()) {
                return constructorIter->second[index];
            }
        }
        return std::nullopt;
    }

    // Retrieve event by name
    [[nodiscard]] auto getEvent(const std::string& name) const
        -> std::optional<const Event*> {
        if (auto eventIter = m_events_.find(name);
            eventIter != m_events_.end()) {
            return &eventIter->second;
        }
        return std::nullopt;
    }
};

class TypeRegistry {
private:
    std::unordered_map<std::string, TypeMetadata> m_registry_;
    mutable std::shared_mutex m_mutex_;

public:
    // Singleton pattern to retrieve the global type registry
    static auto instance() -> TypeRegistry& {
        static TypeRegistry registry;
        return registry;
    }

    // Register a type and its metadata
    void registerType(const std::string& name, TypeMetadata metadata) {
        std::unique_lock lock(m_mutex_);
        m_registry_[name] = std::move(metadata);
    }

    // Retrieve metadata for a registered type
    [[nodiscard]] auto getMetadata(const std::string& name) const
        -> std::optional<TypeMetadata> {
        std::shared_lock lock(m_mutex_);
        if (auto registryIter = m_registry_.find(name);
            registryIter != m_registry_.end()) {
            return registryIter->second;
        }
        return std::nullopt;
    }
};

// Helper function to dynamically call overloaded methods on BoxedValue objects
inline auto callMethod(BoxedValue& obj, const std::string& method_name,
                       std::vector<BoxedValue> args) -> BoxedValue {
    if (auto metadata =
            TypeRegistry::instance().getMetadata(obj.getTypeInfo().name());
        metadata) {
        if (auto methods = metadata->getMethods(method_name); methods) {
            for (const auto& method : **methods) {
                // TODO: FIX ME - 参数类型匹配逻辑:
                // 确保传入的参数与方法期望的参数类型一致
                /*
                auto argTypesMatch = true;
                for (size_t i = 0; i < args.size(); ++i) {
                    if (args[i].getTypeInfo() != method.argument_type(i)) {
                        argTypesMatch = false;
                        break;
                    }
                }
                */
                // if (argTypesMatch) {
                return method(args);
                //}
            }
        }
    }
    THROW_NOT_FOUND("Method not found or no matching overload found");
}

// Helper function to dynamically get properties from BoxedValue objects
inline auto getProperty(const BoxedValue& obj,
                        const std::string& property_name) -> BoxedValue {
    if (auto metadata =
            TypeRegistry::instance().getMetadata(obj.getTypeInfo().name());
        metadata) {
        if (auto property = metadata->getProperty(property_name); property) {
            return (*property).getter(
                obj);  // 修复后的代码，正确调用 getter 函数
        }
    }
    THROW_NOT_FOUND("Property not found");
}

// Helper function to dynamically set properties on BoxedValue objects
inline void setProperty(BoxedValue& obj, const std::string& property_name,
                        const BoxedValue& value) {
    if (auto metadata =
            TypeRegistry::instance().getMetadata(obj.getTypeInfo().name());
        metadata) {
        if (auto property = metadata->getProperty(property_name); property) {
            property->setter(obj, value);
            return;
        }
    }
    THROW_NOT_FOUND("Property not found");
}

// Helper function to fire events on BoxedValue objects
inline void fireEvent(BoxedValue& obj, const std::string& event_name,
                      const std::vector<BoxedValue>& args) {
    if (auto metadata =
            TypeRegistry::instance().getMetadata(obj.getTypeInfo().name());
        metadata) {
        metadata->fireEvent(obj, event_name, args);
    } else {
        std::cerr << "Event not found." << std::endl;
    }
}

// Factory function to dynamically construct an object by type name
inline auto createInstance(const std::string& type_name,
                           std::vector<BoxedValue> args) -> BoxedValue {
    if (auto metadata = TypeRegistry::instance().getMetadata(type_name);
        metadata) {
        if (auto constructor = metadata->getConstructor(type_name);
            constructor) {
            return (*constructor)(std::move(args));
        }
    }
    THROW_NOT_FOUND("Constructor not found");
}

// Reflective registration of types, methods, properties, and events leveraging
// C++20 features
template <typename T>
class TypeRegistrar {
public:
    // Register a type with metadata
    static void registerType(const std::string& type_name) {
        TypeMetadata metadata;

        // Register default constructor
        metadata.addConstructor(
            type_name, [](std::vector<BoxedValue> args) -> BoxedValue {
                if (args.empty()) {
                    return BoxedValue(T{});  // Default constructor
                }
                return BoxedValue{};  // Placeholder for more complex
                                      // constructors
            });

        // Register events
        metadata.addEvent("onCreate", "Triggered when an object is created");
        metadata.addEvent("onDestroy", "Triggered when an object is destroyed");

        // Add methods, properties, events dynamically as needed
        metadata.addMethod(
            "print", [](std::vector<BoxedValue> args) -> BoxedValue {
                if (!args.empty()) {
                    std::cout << "Method print called with value: "
                              << args[0].debugString() << std::endl;
                    return BoxedValue{};
                }
                return BoxedValue{};
            });

        // Register type in the global registry
        TypeRegistry::instance().registerType(type_name, std::move(metadata));
    }
};

}  // namespace atom::meta

#endif  // ATOM_META_ANYMETA_HPP