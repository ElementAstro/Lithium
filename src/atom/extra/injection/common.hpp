#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

namespace atom::extra {

// Forward declarations
template<typename... SymbolTypes>
class Container;

template<typename... SymbolTypes>
struct Context {
    Container<SymbolTypes...>& container;
};

// Concepts
template<typename T>
concept Symbolic = requires {
    typename T::value;
};

template<typename T>
concept Injectable = requires {
    { T::template resolve(std::declval<const Context<>&>()) } -> std::convertible_to<std::tuple<>>;
};

// Symbol
template <typename Interface>
struct Symbol {
    static_assert(!std::is_abstract_v<Interface>,
                  "atom::extra::Container cannot bind/get abstract class value "
                  "(use a smart pointer instead).");
    using value = Interface;
};

// Factory
template <typename T, typename... SymbolTypes>
using Factory = std::function<T(const Context<SymbolTypes...>&)>;

// Exceptions
namespace exceptions {
    struct ResolutionException : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}

// Lifecycle
enum class Lifecycle {
    Transient,
    Singleton,
    Request
};

// Tag
struct Tag {
    std::string name;
    explicit Tag(std::string tag_name) : name(std::move(tag_name)) {}
};

// Named
template <typename T>
struct Named {
    std::string name;
    using value = T;
    explicit Named(std::string binding_name) : name(std::move(binding_name)) {}
};

// Multi
template <typename T>
struct Multi {
    using value = std::vector<T>;
};

// Lazy
template <typename T>
class Lazy {
public:
    explicit Lazy(std::function<T()> factory) : factory_(std::move(factory)) {}
    T get() const { return factory_(); }

private:
    std::function<T()> factory_;
};

} // namespace atom::extra