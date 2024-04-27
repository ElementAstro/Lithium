#ifndef ATOM_COMPONENT_VAR_HPP
#define ATOM_COMPONENT_VAR_HPP

#include <any>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "atom/type/noncopyable.hpp"
#include "atom/type/trackable.hpp"
#include "atom/utils/cstring.hpp"

class VariableManager : public NonCopyable {
public:
    template <typename T>
    void addVariable(const std::string& name, T initialValue,
                     const std::string& description = "",
                     const std::string& alias = "",
                     const std::string& group = "") {
        auto variable = std::make_shared<Trackable<T>>(std::move(initialValue));
        variables_[name] = {std::move(variable), description, alias, group};
    }

    template <typename T>
    void setRange(const std::string& name, T min, T max) {
        if (auto variable = getVariable<T>(name)) {
            ranges_[name] = std::make_pair(std::move(min), std::move(max));
        }
    }

    void setStringOptions(const std::string& name,
                          std::vector<std::string> options) {
        if (auto variable = getVariable<std::string>(name)) {
            stringOptions_[name] = std::move(options);
        }
    }

    template <typename T>
    std::shared_ptr<Trackable<T>> getVariable(const std::string& name) {
        auto it = variables_.find(name);
        if (it != variables_.end()) {
            try {
                return std::any_cast<std::shared_ptr<Trackable<T>>>(
                    it->second.variable);
            } catch (const std::bad_any_cast& e) {
                THROW_EXCEPTION(concat("Type mismatch: ", name));
            }
        }
        return nullptr;
    }

    void setValue(const std::string& name, const char* newValue) {
        setValue(name, std::string(newValue));
    }

    template <typename T>
    void setValue(const std::string& name, T newValue) {
        if (auto variable = getVariable<T>(name)) {
            if constexpr (std::is_arithmetic_v<T>) {
                if (ranges_.count(name)) {
                    auto [min, max] =
                        std::any_cast<std::pair<T, T>>(ranges_[name]);
                    if (newValue < min || newValue > max) {
                        THROW_EXCEPTION("Value out of range");
                    }
                }
            } else if constexpr (std::is_same_v<T, std::string>) {
                if (stringOptions_.count(name)) {
                    auto& options = stringOptions_[name];
                    if (std::find(options.begin(), options.end(), newValue) ==
                        options.end()) {
                        THROW_EXCEPTION("Invalid string option");
                    }
                }
            }
            *variable = std::move(newValue);
        } else {
            THROW_EXCEPTION("Variable not found");
        }
    }

private:
    struct VariableInfo {
        std::any variable;
        std::string description;
        std::string alias;
        std::string group;
    };

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, VariableInfo> variables_;
    emhash8::HashMap<std::string, std::any> ranges_;
    emhash8::HashMap<std::string, std::vector<std::string>> stringOptions_;
#else
    std::unordered_map<std::string, VariableInfo> variables_;
    std::unordered_map<std::string, std::any> ranges_;
    std::unordered_map<std::string, std::vector<std::string>> stringOptions_;
#endif
};

#include "var.inl"

#endif