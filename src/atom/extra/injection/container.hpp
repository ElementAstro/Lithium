#pragma once

#include <memory>
#include "binding.hpp"
#include "common.hpp"

namespace atom::extra {

/**
 * @class Container
 * @brief A dependency injection container for managing bindings and resolving
 * dependencies.
 * @tparam SymbolTypes The symbol types associated with the container.
 */
template <typename... SymbolTypes>
class Container {
public:
    using BindingMap =
        std::tuple<Binding<SymbolTypes, SymbolTypes...>...>;  ///< The map of
                                                              ///< bindings.

    /**
     * @brief Binds a symbol to a value or factory.
     * @tparam T The symbol type to bind.
     * @return A reference to the BindingTo object for further configuration.
     */
    template <Symbolic T>
    BindingTo<typename T::value, SymbolTypes...>& bind() {
        static_assert((std::is_same_v<T, SymbolTypes> || ...),
                      "atom::extra::Container symbol not registered");
        return std::get<Binding<T, SymbolTypes...>>(bindings_);
    }

    /**
     * @brief Resolves a value for a given symbol.
     * @tparam T The symbol type to resolve.
     * @return The resolved value.
     */
    template <Symbolic T>
    typename T::value get() {
        return get<T>(Tag(""));
    }

    /**
     * @brief Resolves a value for a given symbol and tag.
     * @tparam T The symbol type to resolve.
     * @param tag The tag to match.
     * @return The resolved value.
     * @throws exceptions::ResolutionException if no matching binding is found.
     */
    template <Symbolic T>
    typename T::value get(const Tag& tag) {
        static_assert((std::is_same_v<T, SymbolTypes> || ...),
                      "atom::extra::Container symbol not registered");
        auto& binding = std::get<Binding<T, SymbolTypes...>>(bindings_);
        if (binding.matchesTag(tag)) {
            return binding.resolve(context_);
        }
        throw exceptions::ResolutionException(
            "No matching binding found for the given tag.");
    }

    /**
     * @brief Resolves a value for a given symbol and name.
     * @tparam T The symbol type to resolve.
     * @param name The name to match.
     * @return The resolved value.
     * @throws exceptions::ResolutionException if no matching binding is found.
     */
    template <Symbolic T>
    typename T::value getNamed(const std::string& name) {
        static_assert((std::is_same_v<T, SymbolTypes> || ...),
                      "atom::extra::Container symbol not registered");
        auto& binding = std::get<Binding<T, SymbolTypes...>>(bindings_);
        if (binding.matchesTargetName(name)) {
            return binding.resolve(context_);
        }
        throw exceptions::ResolutionException(
            "No matching binding found for the given name.");
    }

    /**
     * @brief Resolves all values for a given symbol.
     * @tparam T The symbol type to resolve.
     * @return A vector of resolved values.
     */
    template <Symbolic T>
    std::vector<typename T::value> getAll() {
        static_assert((std::is_same_v<T, SymbolTypes> || ...),
                      "atom::extra::Container symbol not registered");
        std::vector<typename T::value> result;
        auto& binding = std::get<Binding<T, SymbolTypes...>>(bindings_);
        result.push_back(binding.resolve(context_));
        return result;
    }

    /**
     * @brief Checks if a binding exists for a given symbol.
     * @tparam T The symbol type to check.
     * @return True if a binding exists, false otherwise.
     */
    template <Symbolic T>
    bool hasBinding() const {
        return std::get<Binding<T, SymbolTypes...>>(bindings_).resolver_ !=
               nullptr;
    }

    /**
     * @brief Unbinds a symbol, removing its binding.
     * @tparam T The symbol type to unbind.
     */
    template <Symbolic T>
    void unbind() {
        std::get<Binding<T, SymbolTypes...>>(bindings_).resolver_.reset();
    }

    /**
     * @brief Creates a child container that inherits bindings from the parent.
     * @return A unique pointer to the child container.
     */
    std::unique_ptr<Container> createChildContainer() {
        auto child = std::make_unique<Container>();
        child->parent_ = this;
        return child;
    }

private:
    BindingMap bindings_;  ///< The map of bindings.
    Context<SymbolTypes...> context_{
        *this};                    ///< The context for resolving dependencies.
    Container* parent_ = nullptr;  ///< The parent container, if any.
};

}  // namespace atom::extra