#ifndef ATOM_EXPERIMENT_CALLABLE_HPP
#define ATOM_EXPERIMENT_CALLABLE_HPP

#include <memory>

template <typename Class, typename... Param>
struct Constructor {
    template <typename... Inner>
    std::shared_ptr<Class> operator()(Inner &&...inner) const {
        return std::make_shared<Class>(std::forward<Inner>(inner)...);
    }
};

template <typename Ret, typename Class, typename... Param>
struct Const_Caller {
    explicit Const_Caller(Ret (Class::*t_func)(Param...) const)
        : m_func(t_func) {}

    template <typename... Inner>
    Ret operator()(const Class &o, Inner &&...inner) const {
        return (o.*m_func)(std::forward<Inner>(inner)...);
    }

    Ret (Class::*m_func)(Param...) const;
};

template <typename Ret, typename... Param>
struct Fun_Caller {
    explicit Fun_Caller(Ret (*t_func)(Param...)) : m_func(t_func) {}

    template <typename... Inner>
    Ret operator()(Inner &&...inner) const {
        return (m_func)(std::forward<Inner>(inner)...);
    }

    Ret (*m_func)(Param...);
};

template <typename Ret, typename Class, typename... Param>
struct Caller {
    explicit Caller(Ret (Class::*t_func)(Param...)) : m_func(t_func) {}

    template <typename... Inner>
    Ret operator()(Class &o, Inner &&...inner) const {
        return (o.*m_func)(std::forward<Inner>(inner)...);
    }

    Ret (Class::*m_func)(Param...);
};

template <typename T>
struct Arity {};

template <typename Ret, typename... Params>
struct Arity<Ret(Params...)> {
    static const size_t arity = sizeof...(Params);
};

template <typename T>
struct Function_Signature {};

template <typename Ret, typename... Params>
struct Function_Signature<Ret(Params...)> {
    using Return_Type = Ret;
    using Signature = Ret (*)(Params...);  // 修改成这样
};

template <typename Ret, typename T, typename... Params>
struct Function_Signature<Ret (T::*)(Params...) const> {
    using Return_Type = Ret;
    using Signature = Ret (*)(Params...);  // 修改成这样
};

template <typename T>
struct Callable_Traits {
    using Signature =
        typename Function_Signature<decltype(&T::operator())>::Signature;
    using Return_Type =
        typename Function_Signature<decltype(&T::operator())>::Return_Type;
};

#endif
