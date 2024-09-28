/*!
 * \file refl.hpp
 * \brief Static reflection, modified from USRefl
 * \author Max Qian <lightapt.com>
 * \date 2024-5-25
 * \copyright Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef ATOM_META_REFL_HPP
#define ATOM_META_REFL_HPP

#include <array>
#include <string_view>
#include <tuple>

#ifdef __clang__
#define TSTR(s)                                                               \
    ([] {                                                                     \
        struct tmp {                                                          \
            static constexpr auto get() { return std::basic_string_view{s}; } \
        };                                                                    \
        return detail::TSTRH(tmp{});                                          \
    }())
#else
#define TSTR(s)                                                               \
    ([] {                                                                     \
        constexpr std::basic_string_view str{s};                              \
        return detail::TStr<detail::fcstr<typename decltype(str)::value_type, \
                                          str.size()>{str}>{};                \
    }())
#endif

namespace detail {

#ifdef __clang__
template <typename C, C... chars>
struct TStr {
    using Char = C;
    template <typename T>
    static constexpr auto Is(T = {}) -> bool {
        return std::is_same_v<T, TStr>;
    }
    static constexpr auto Data() -> const Char* { return data.data(); }
    static constexpr auto Size() -> std::size_t { return sizeof...(chars); }
    static constexpr auto View() -> std::basic_string_view<Char> {
        return data.data();
    }

private:
    static constexpr std::array<Char, sizeof...(chars) + 1> data{chars...,
                                                                 Char(0)};
};

template <typename Char, typename T, std::size_t... Ns>
constexpr auto TSTRHI(std::index_sequence<Ns...>)
    -> TStr<Char, T::get()[Ns]...> {
    return {};
}

template <typename T>
constexpr auto TSTRH(T) -> decltype(auto) {
    return TSTRHI<typename decltype(T::get())::value_type, T>(
        std::make_index_sequence<T::get().size()>{});
}
#else
template <typename C, std::size_t N>
struct fcstr {
    using value_type = C;
    value_type data[N + 1]{};
    static constexpr std::size_t size = N;
    constexpr fcstr(std::basic_string_view<value_type> str) {
        for (std::size_t i{0}; i < size; ++i)
            data[i] = str[i];
    }
};

template <fcstr str>
struct TStr {
    using Char = typename decltype(str)::value_type;
    template <typename T>
    static constexpr auto Is(T = {}) -> bool {
        return std::is_same_v<T, TStr>;
    }
    static constexpr auto Data() -> const Char* { return str.data; }
    static constexpr auto Size() -> std::size_t { return str.size; }
    static constexpr auto View() -> std::basic_string_view<Char> {
        return str.data;
    }
};
#endif

template <class L, class F>
constexpr auto FindIf(const L&, F&&, std::index_sequence<>) -> std::size_t {
    return -1;
}

template <class L, class F, std::size_t N0, std::size_t... Ns>
constexpr auto FindIf(const L& list, F&& func,
                      std::index_sequence<N0, Ns...>) -> std::size_t {
    return func(list.template Get<N0>()) ? N0
                                         : FindIf(list, std::forward<F>(func),
                                                  std::index_sequence<Ns...>{});
}

template <class L, class F, class R>
constexpr auto Acc(const L&, F&&, R result, std::index_sequence<>) -> R {
    return result;
}

template <class L, class F, class R, std::size_t N0, std::size_t... Ns>
constexpr auto Acc(const L& list, F&& func, R result,
                   std::index_sequence<N0, Ns...>) -> R {
    return Acc(list, std::forward<F>(func),
               func(std::move(result), list.template Get<N0>()),
               std::index_sequence<Ns...>{});
}

template <std::size_t D, class T, class R, class F>
constexpr auto DFS_Acc(T type, F&& func, R result) -> R {
    return type.bases.Accumulate(
        std::move(result), [&](auto result, auto base) {
            if constexpr (base.is_virtual) {
                return DFS_Acc<D + 1>(base.info, std::forward<F>(func),
                                      std::move(result));
            } else {
                return DFS_Acc<D + 1>(
                    base.info, std::forward<F>(func),
                    std::forward<F>(func)(std::move(result), base.info, D + 1));
            }
        });
}

template <class TI, class U, class F>
constexpr void NV_Var(TI, U&& obj, F&& func) {
    TI::fields.ForEach([&](auto&& field) {
        using Field = std::decay_t<decltype(field)>;
        if constexpr (!Field::is_static && !Field::is_func) {
            std::forward<F>(func)(field, std::forward<U>(obj).*(field.value));
        }
    });
    TI::bases.ForEach([&](auto base) {
        if constexpr (!base.is_virtual) {
            NV_Var(base.info, base.info.Forward(std::forward<U>(obj)),
                   std::forward<F>(func));
        }
    });
}

}  // namespace detail

namespace atom::meta {

template <class Name>
struct NamedValueBase {
    using TName = Name;
    static constexpr std::string_view name = TName::View();
};

template <class Name, class T>
struct NamedValue : NamedValueBase<Name> {
    T value;
    static constexpr bool has_value = true;
    explicit constexpr NamedValue(T val) : value{val} {}
    template <class U>
    constexpr auto operator==(U val) const -> bool {
        if constexpr (std::is_same_v<T, U>) {
            return value == val;
        } else {
            return false;
        }
    }
};

template <class Name>
struct NamedValue<Name, void> : NamedValueBase<Name> {
    static constexpr bool has_value = false;
    template <class U>
    constexpr auto operator==(U) const -> bool {
        return false;
    }
};

template <typename... Es>
struct ElemList {
    std::tuple<Es...> elems;
    static constexpr std::size_t size = sizeof...(Es);
    explicit constexpr ElemList(Es... elements) : elems{elements...} {}
    template <class Init, class Func>
    constexpr auto Accumulate(Init init, Func&& func) const -> decltype(auto) {
        return detail::Acc(*this, std::forward<Func>(func), std::move(init),
                           std::make_index_sequence<size>{});
    }
    template <class Func>
    constexpr void ForEach(Func&& func) const {
        Accumulate(0, [&](auto, const auto& field) {
            std::forward<Func>(func)(field);
            return 0;
        });
    }
    template <class S>
    static constexpr auto Contains(S = {}) -> bool {
        return (Es::TName::template Is<S>() || ...);
    }
    template <class Func>
    constexpr auto FindIf(Func&& func) const -> std::size_t {
        return detail::FindIf(*this, std::forward<Func>(func),
                              std::make_index_sequence<size>{});
    }
    template <class S>
    constexpr auto Find(S = {}) const -> const auto& {
        constexpr std::size_t idx = []() {
            constexpr std::array names{Es::name...};
            for (std::size_t i = 0; i < size; i++) {
                if (S::View() == names[i]) {
                    return i;
                }
            }
            return static_cast<std::size_t>(-1);
        }();
        return Get<idx>();
    }
    template <class T>
    constexpr auto FindValue(const T& val) const -> std::size_t {
        return FindIf([&val](auto elem) { return elem == val; });
    }
    template <typename T, typename S>
    constexpr auto ValuePtrOfName(S name) const -> const T* {
        return Accumulate(nullptr, [name](auto result, const auto& elem) {
            if constexpr (std::is_same_v<decltype(elem.value), T>) {
                return elem.name == name ? &elem.value : result;
            } else {
                return result;
            }
        });
    }
    template <typename T, typename S>
    constexpr auto ValueOfName(S name) const -> const T& {
        return *ValuePtrOfName<T>(name);
    }
    template <class T, class C = char>
    constexpr auto NameOfValue(const T& val) const
        -> std::basic_string_view<C> {
        return Accumulate(std::basic_string_view<C>{},
                          [&val](auto result, auto&& elem) {
                              return elem == val ? elem.name : result;
                          });
    }
    template <class E>
    constexpr auto Push(E elem) const -> ElemList<Es..., E> {
        return std::apply(
            [elem](auto... elements) {
                return ElemList<Es..., E>{elements..., elem};
            },
            elems);
    }
    template <class E>
    constexpr auto Insert(E elem) const -> decltype(auto) {
        if constexpr ((std::is_same_v<Es, E> || ...)) {
            return *this;
        } else {
            return Push(elem);
        }
    }
    template <std::size_t N>
    [[nodiscard]] constexpr auto Get() const -> const auto& {
        return std::get<N>(elems);
    }
#define ATOM_META_ElemList_GetByValue(list, value) \
    list.Get<list.FindValue(value)>()
};

template <class Name, class T>
struct Attr : NamedValue<Name, T> {
    explicit constexpr Attr(Name, T val) : NamedValue<Name, T>{val} {}
};

template <class Name>
struct Attr<Name, void> : NamedValue<Name, void> {
    explicit constexpr Attr(Name) {}
};

template <typename... As>
struct AttrList : ElemList<As...> {
    explicit constexpr AttrList(As... attrs) : ElemList<As...>{attrs...} {}
};

template <bool s, bool f>
struct FTraitsB {
    static constexpr bool is_static = s, is_func = f;
};

template <class T>
struct FTraits : FTraitsB<true, false> {};  // default is enum

template <class U, class T>
struct FTraits<T U::*> : FTraitsB<false, std::is_function_v<T>> {};

template <class T>
struct FTraits<T*> : FTraitsB<true, std::is_function_v<T>> {};  // static member

template <class Name, class T, class AList>
struct Field : FTraits<T>, NamedValue<Name, T> {
    AList attrs;
    constexpr Field(Name, T val, AList attr_list = {})
        : NamedValue<Name, T>{val}, attrs{attr_list} {}
};

template <typename... Fs>
struct FieldList : ElemList<Fs...> {
    explicit constexpr FieldList(Fs... fields) : ElemList<Fs...>{fields...} {}
};

template <class T>
struct TypeInfo;  // TypeInfoBase, name, fields, attrs

template <class T, bool IsVirtual = false>
struct Base {
    static constexpr auto info = TypeInfo<T>{};
    static constexpr bool is_virtual = IsVirtual;
};

template <typename... Bs>
struct BaseList : ElemList<Bs...> {
    explicit constexpr BaseList(Bs... bases) : ElemList<Bs...>{bases...} {}
};

template <typename... Ts>
struct TypeInfoList : ElemList<Ts...> {
    explicit constexpr TypeInfoList(Ts... types) : ElemList<Ts...>{types...} {}
};

template <class T, typename... Bases>
struct TypeInfoBase {
    using Type = T;
    static constexpr BaseList bases{Bases{}...};
    template <class U>
    static constexpr auto Forward(U&& derived) -> decltype(auto) {
        if constexpr (std::is_same_v<std::decay_t<U>, U>) {
            return static_cast<Type&&>(derived);  // right
        } else if constexpr (std::is_same_v<std::decay_t<U>&, U>) {
            return static_cast<Type&>(derived);  // left
        } else {
            return static_cast<const std::decay_t<U>&>(
                derived);  // std::is_same_v<const std::decay_t<U>&, U>
        }
    }
    static constexpr auto VirtualBases() {
        return bases.Accumulate(ElemList<>{}, [](auto acc, auto base) {
            auto concated = base.info.VirtualBases().Accumulate(
                acc, [](auto acc, auto b) { return acc.Insert(b); });
            if constexpr (!base.is_virtual) {
                return concated;
            } else {
                return concated.Insert(base.info);
            }
        });
    }
    template <class R, class F>
    static constexpr auto DFS_Acc(R result, F&& func) -> decltype(auto) {
        return detail::DFS_Acc<0>(
            TypeInfo<Type>{}, std::forward<F>(func),
            VirtualBases().Accumulate(
                std::forward<F>(func)(std::move(result), TypeInfo<Type>{}, 0),
                [&](auto acc, auto vb) {
                    return std::forward<F>(func)(std::move(acc), vb, 1);
                }));
    }
    template <class F>
    static constexpr void DFS_ForEach(F&& func) {
        DFS_Acc(0, [&](auto, auto type, auto depth) {
            std::forward<F>(func)(type, depth);
            return 0;
        });
    }
    template <class U, class Func>
    static constexpr void ForEachVarOf(U&& obj, Func&& func) {
        VirtualBases().ForEach([&](auto vb) {
            vb.fields.ForEach([&](const auto& fld) {
                using Field = std::decay_t<decltype(fld)>;
                if constexpr (!Field::is_static && !Field::is_func) {
                    std::forward<Func>(func)(fld,
                                             std::forward<U>(obj).*(fld.value));
                }
            });
        });
        detail::NV_Var(TypeInfo<Type>{}, std::forward<U>(obj),
                       std::forward<Func>(func));
    }
};

template <class Name>
Attr(Name) -> Attr<Name, void>;

template <class Name, class T>
Field(Name, T) -> Field<Name, T, AttrList<>>;

}  // namespace atom::meta

#define ATOM_META_TYPEINFO(Type, ...)                          \
    namespace atom::meta {                                     \
    template <>                                                \
    struct TypeInfo<Type> : TypeInfoBase<Type> {               \
        static constexpr auto fields = FieldList(__VA_ARGS__); \
    };                                                         \
    }

#define ATOM_META_FIELD(Name, Member) atom::meta::Field(TSTR(Name), Member)

#endif
