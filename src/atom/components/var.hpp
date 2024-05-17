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

class VariableManager {
public:
    template <typename T>
    void addVariable(const std::string& name, T initialValue,
                     const std::string& description = "",
                     const std::string& alias = "",
                     const std::string& group = "");

    template <typename T>
    void setRange(const std::string& name, T min, T max);

    void setStringOptions(const std::string& name,
                          std::vector<std::string> options);

    template <typename T>
    std::shared_ptr<Trackable<T>> getVariable(const std::string& name);

    void setValue(const std::string& name, const char* newValue);

    template <typename T>
    void setValue(const std::string& name, T newValue);

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
