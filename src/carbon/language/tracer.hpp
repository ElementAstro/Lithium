

#ifndef CARBON_TRACER_HPP
#define CARBON_TRACER_HPP

namespace Carbon::eval {
struct Noop_Tracer_Detail {
    template <typename T>
    constexpr void trace(const Carbon::detail::Dispatch_State &,
                         const AST_Node_Impl<T> *) noexcept {}
};

template <typename... T>
struct Tracer : T... {
    Tracer() = default;
    constexpr explicit Tracer(T... t) : T(std::move(t))... {}

    void do_trace(const Carbon::detail::Dispatch_State &ds,
                  const AST_Node_Impl<Tracer<T...>> *node) {
        (static_cast<T &>(*this).trace(ds, node), ...);
    }

    static void trace(const Carbon::detail::Dispatch_State &ds,
                      const AST_Node_Impl<Tracer<T...>> *node) {
        ds->get_parser().get_tracer<Tracer<T...>>().do_trace(ds, node);
    }
};

using Noop_Tracer = Tracer<Noop_Tracer_Detail>;

}  // namespace Carbon::eval

#endif
