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
template <typename... SymbolTypes>
class Container;

template <typename... SymbolTypes>
struct Context {
    Container<SymbolTypes...>& container;
};

// Concepts

/**
 * @brief Concept to check if a type is symbolic.
 * @tparam T The type to check.
 */
template <typename T>
concept Symbolic = requires { typename T::value; };

/**
 * @brief Concept to check if a type is injectable.
 * @tparam T The type to check.
 */
template <typename T>
concept Injectable = requires {
    {
        T::template resolve(std::declval<const Context<>&>())
    } -> std::convertible_to<std::tuple<>>;
};

// Symbol

/**
 * @brief A struct representing a symbol for an interface.
 * @tparam Interface The interface type.
 */
template <typename Interface>
struct Symbol {
    static_assert(!std::is_abstract_v<Interface>,
                  "atom::extra::Container cannot bind/get abstract class value "
                  "(use a smart pointer instead).");
    using value = Interface;
};

// Factory

/**
 * @brief A type alias for a factory function.
 * @tparam T The type to produce.
 * @tparam SymbolTypes The symbol types associated with the factory.
 */
template <typename T, typename... SymbolTypes>
using Factory = std::function<T(const Context<SymbolTypes...>&)>;

// Exceptions

namespace exceptions {

/**
 * @brief Exception thrown when resolution fails.
 */
struct ResolutionException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

}  // namespace exceptions

// Lifecycle

/**
 * @brief Enum representing the lifecycle of a binding.
 */
enum class Lifecycle {
    Transient,  ///< The binding is created anew each time.
    Singleton,  ///< The binding is created once and shared.
    Request     ///< The binding is created once per request.
};

// Tag

/**
 * @brief A struct representing a tag for a binding.
 */
struct Tag {
    std::string name;  ///< The name of the tag.
    explicit Tag(std::string tag_name) : name(std::move(tag_name)) {}
};

// Named

/**
 * @brief A struct representing a named binding.
 * @tparam T The type of the binding.
 */
template <typename T>
struct Named {
    std::string name;  ///< The name of the binding.
    using value = T;   ///< The type of the binding.
    explicit Named(std::string binding_name) : name(std::move(binding_name)) {}
};

// Multi

/**
 * @brief A struct representing a multi-binding.
 * @tparam T The type of the binding.
 */
template <typename T>
struct Multi {
    using value = std::vector<T>;  ///< The type of the multi-binding.
};

// Lazy

/**
 * @brief A class representing a lazy binding.
 * @tparam T The type of the binding.
 */
template <typename T>
class Lazy {
public:
    /**
     * @brief Constructs a Lazy binding with a factory function.
     * @param factory The factory function to produce the binding.
     */
    explicit Lazy(std::function<T()> factory) : factory_(std::move(factory)) {}

    /**
     * @brief Gets the value of the binding.
     * @return The value of the binding.
     */
    T get() const { return factory_(); }

private:
    std::function<T()>
        factory_;  ///< The factory function to produce the binding.
};

}  // namespace atom::extra