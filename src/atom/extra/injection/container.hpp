#pragma once

#include "common.hpp"
#include "binding.hpp"
#include <memory>

namespace atom::extra {

template <typename... SymbolTypes>
class Container {
public:
    using BindingMap = std::tuple<Binding<SymbolTypes, SymbolTypes...>...>;

    template <Symbolic T>
    BindingTo<typename T::value, SymbolTypes...>& bind() {
        static_assert((std::is_same_v<T, SymbolTypes> || ...),
                      "atom::extra::Container symbol not registered");
        return std::get<Binding<T, SymbolTypes...>>(bindings_);
    }

    template <Symbolic T>
    typename T::value get() {
        return get<T>(Tag(""));
    }

    template <Symbolic T>
    typename T::value get(const Tag& tag) {
        static_assert((std::is_same_v<T, SymbolTypes> || ...),
                      "atom::extra::Container symbol not registered");
        auto& binding = std::get<Binding<T, SymbolTypes...>>(bindings_);
        if (binding.matchesTag(tag)) {
            return binding.resolve(context_);
        }
        throw exceptions::ResolutionException("No matching binding found for the given tag.");
    }

    template <Symbolic T>
    typename T::value getNamed(const std::string& name) {
        static_assert((std::is_same_v<T, SymbolTypes> || ...),
                      "atom::extra::Container symbol not registered");
        auto& binding = std::get<Binding<T, SymbolTypes...>>(bindings_);
        if (binding.matchesTargetName(name)) {
            return binding.resolve(context_);
        }
        throw exceptions::ResolutionException("No matching binding found for the given name.");
    }

    template <Symbolic T>
    std::vector<typename T::value> getAll() {
        static_assert((std::is_same_v<T, SymbolTypes> || ...),
                      "atom::extra::Container symbol not registered");
        std::vector<typename T::value> result;
        auto& binding = std::get<Binding<T, SymbolTypes...>>(bindings_);
        result.push_back(binding.resolve(context_));
        return result;
    }

    template <Symbolic T>
    bool hasBinding() const {
        return std::get<Binding<T, SymbolTypes...>>(bindings_).resolver_ != nullptr;
    }

    template <Symbolic T>
    void unbind() {
        std::get<Binding<T, SymbolTypes...>>(bindings_).resolver_.reset();
    }

    std::unique_ptr<Container> createChildContainer() {
        auto child = std::make_unique<Container>();
        child->parent_ = this;
        return child;
    }

private:
    BindingMap bindings_;
    Context<SymbolTypes...> context_{*this};
    Container* parent_ = nullptr;
};

} // namespace atom::extra