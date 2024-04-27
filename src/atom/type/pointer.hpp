/*
 * pointer.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-3

Description: Pointer Sentinel for Atom

**************************************************/

#ifndef ATOM_TYPE_POINTER_HPP
#define ATOM_TYPE_POINTER_HPP

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <variant>

/**
 * @brief Concept to check if a type is a pointer type, including raw pointers,
 * std::shared_ptr, and std::unique_ptr.
 *
 * @tparam T The type to check.
 */
template <typename T>
concept PointerType =
    std::is_pointer_v<T> ||
    std::is_same_v<T, std::shared_ptr<typename std::remove_pointer<T>::type>> ||
    std::is_same_v<T, std::unique_ptr<typename std::remove_pointer<T>::type>>;

/**
 * @brief A class template to hold different types of pointers using
 * std::variant.
 *
 * @tparam T The type of the pointed-to object.
 */
template <typename T>
class PointerSentinel {
public:
    // Using std::variant to store different types of pointers
    std::variant<std::shared_ptr<T>, std::unique_ptr<T>, T*> ptr;

    /**
     * @brief Construct a new Pointer Sentinel object from a shared pointer.
     *
     * @param p The shared pointer.
     */
    explicit PointerSentinel(std::shared_ptr<T> p) : ptr(p) {}

    /**
     * @brief Construct a new Pointer Sentinel object from a unique pointer.
     *
     * @param p The unique pointer.
     */
    explicit PointerSentinel(std::unique_ptr<T>&& p) : ptr(std::move(p)) {}

    /**
     * @brief Construct a new Pointer Sentinel object from a raw pointer.
     *
     * @param p The raw pointer.
     */
    explicit PointerSentinel(T* p) : ptr(p) {}

    /**
     * @brief Copy constructor.
     *
     * @param other The other Pointer Sentinel object to copy from.
     */
    PointerSentinel(const PointerSentinel& other)
        : ptr(std::visit(
              [](const auto& p)
                  -> std::variant<std::shared_ptr<T>, std::unique_ptr<T>, T*> {
                  if constexpr (std::is_same_v<std::decay_t<decltype(p)>,
                                               std::shared_ptr<T>>) {
                      return p;
                  } else if constexpr (std::is_same_v<std::decay_t<decltype(p)>,
                                                      std::unique_ptr<T>>) {
                      return std::make_unique<T>(*p);
                  } else {
                      return new T(*p);
                  }
              },
              other.ptr)) {}

    /**
     * @brief Get the raw pointer stored in the variant.
     *
     * @return T* The raw pointer.
     */
    [[nodiscard]] T* get() const {
        return std::visit(
            [](auto&& arg) -> T* {
                using U = std::decay_t<decltype(arg)>;
                if constexpr (std::is_pointer_v<U>) {
                    return arg;  // 原始指针
                } else {
                    return arg.get();  // 智能指针
                }
            },
            ptr);
    }

    /**
     * @brief Helper method to invoke member functions on the pointed-to object.
     *
     * @tparam Func The type of the member function pointer.
     * @tparam Args The types of the arguments to the member function.
     * @param func The member function pointer.
     * @param args The arguments to the member function.
     * @return auto The return type of the member function.
     */
    template <typename Func, typename... Args>
    [[nodiscard]] auto invoke(Func func, Args&&... args) {
        static_assert(std::is_member_function_pointer_v<Func>,
                      "Func must be a member function pointer");
        return std::visit(
            [func, &args...](auto&& arg) -> decltype(auto) {
                using U = std::decay_t<decltype(arg)>;
                if constexpr (std::is_pointer_v<U>) {
                    return ((*arg).*func)(std::forward<Args>(args)...);
                } else {
                    return ((*arg.get()).*func)(std::forward<Args>(args)...);
                }
            },
            ptr);
    }
};

#endif
