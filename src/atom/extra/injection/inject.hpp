#pragma once

#include "common.hpp"

namespace atom::extra {

template <Symbolic... Dependencies>
struct Inject {
    template <typename... SymbolTypes>
    static auto resolve(const Context<SymbolTypes...>& context) {
        return std::make_tuple(context.container.template get<Dependencies>()...);
    }
};

template <typename Implementation, Injectable Inject = Inject<>>
struct InjectableA : Inject {};

} // namespace atom::extra