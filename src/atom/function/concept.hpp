#ifndef ATOM_FUNCTION_CONCEPT_HPP
#define ATOM_FUNCTION_CONCEPT_HPP

#include <concepts>
#include <type_traits>

template <typename T>
concept Function = requires(T t) {
    { t() } -> std::same_as<void>;
};

#include <type_traits>

template <typename T>
concept Relocatable = requires(T t) {
    { std::is_nothrow_move_constructible_v<T> } -> true_type;
    { std::is_nothrow_move_assignable_v<T> } -> true_type;
};

#endif