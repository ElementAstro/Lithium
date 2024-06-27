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

#include <functional>
#include <memory>
#include <type_traits>
#include <variant>

/**
 * @brief Concept to check if a type is a pointer type, including raw pointers,
 * std::shared_ptr, std::unique_ptr, and std::weak_ptr.
 *
 * @tparam T The type to check.
 */
template <typename T>
concept PointerType =
    std::is_pointer_v<T> ||
    std::is_same_v<T, std::shared_ptr<typename std::remove_pointer<T>::type>> ||
    std::is_same_v<T, std::unique_ptr<typename std::remove_pointer<T>::type>> ||
    std::is_same_v<T, std::weak_ptr<typename std::remove_pointer<T>::type>>;

/**
 * @brief A class template to hold different types of pointers using
 * std::variant.
 *
 * @tparam T The type of the pointed-to object.
 */
template <typename T>
class PointerSentinel {
    // Using std::variant to store different types of pointers
    std::variant<std::shared_ptr<T>, std::unique_ptr<T>, std::weak_ptr<T>, T*>
        ptr_;

public:
    PointerSentinel() = default;
    /**
     * @brief Construct a new Pointer Sentinel object from a shared pointer.
     *
     * @param p The shared pointer.
     */
    explicit PointerSentinel(std::shared_ptr<T> p) : ptr_(std::move(p)) {}

    /**
     * @brief Construct a new Pointer Sentinel object from a unique pointer.
     *
     * @param p The unique pointer.
     */
    explicit PointerSentinel(std::unique_ptr<T>&& p) : ptr_(std::move(p)) {}

    /**
     * @brief Construct a new Pointer Sentinel object from a weak pointer.
     *
     * @param p The weak pointer.
     */
    explicit PointerSentinel(std::weak_ptr<T> p) : ptr_(std::move(p)) {}

    /**
     * @brief Construct a new Pointer Sentinel object from a raw pointer.
     *
     * @param p The raw pointer.
     */
    explicit PointerSentinel(T* p) : ptr_(p) {}

    /**
     * @brief Copy constructor.
     *
     * @param other The other Pointer Sentinel object to copy from.
     */
    PointerSentinel(const PointerSentinel& other)
        : ptr_(std::visit(
              [](const auto& p)
                  -> std::variant<std::shared_ptr<T>, std::unique_ptr<T>,
                                  std::weak_ptr<T>, T*> {
                  if constexpr (std::is_same_v<std::decay_t<decltype(p)>,
                                               std::shared_ptr<T>>) {
                      return p;
                  } else if constexpr (std::is_same_v<std::decay_t<decltype(p)>,
                                                      std::unique_ptr<T>>) {
                      return std::make_unique<T>(*p);
                  } else if constexpr (std::is_same_v<std::decay_t<decltype(p)>,
                                                      std::weak_ptr<T>>) {
                      return p;
                  } else {
                      return new T(*p);
                  }
              },
              other.ptr_)) {}

    /**
     * @brief Move constructor.
     *
     * @param other The other Pointer Sentinel object to move from.
     */
    PointerSentinel(PointerSentinel&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     *
     * @param other The other Pointer Sentinel object to copy from.
     * @return A reference to this Pointer Sentinel object.
     */
    auto operator=(const PointerSentinel& other) -> PointerSentinel& {
        if (this != &other) {
            ptr_ = std::visit(
                [](const auto& p)
                    -> std::variant<std::shared_ptr<T>, std::unique_ptr<T>,
                                    std::weak_ptr<T>, T*> {
                    if constexpr (std::is_same_v<std::decay_t<decltype(p)>,
                                                 std::shared_ptr<T>>) {
                        return p;
                    } else if constexpr (std::is_same_v<
                                             std::decay_t<decltype(p)>,
                                             std::unique_ptr<T>>) {
                        return std::make_unique<T>(*p);
                    } else if constexpr (std::is_same_v<
                                             std::decay_t<decltype(p)>,
                                             std::weak_ptr<T>>) {
                        return p;
                    } else {
                        return new T(*p);
                    }
                },
                other.ptr_);
        }
        return *this;
    }

    /**
     * @brief Move assignment operator.
     *
     * @param other The other Pointer Sentinel object to move from.
     * @return A reference to this Pointer Sentinel object.
     */
    auto operator=(PointerSentinel&& other) noexcept -> PointerSentinel& =
                                                            default;

    /**
     * @brief Get the raw pointer stored in the variant.
     *
     * @return T* The raw pointer.
     */
    [[nodiscard]] auto get() const -> T* {
        return std::visit(
            [](auto&& arg) -> T* {
                using U = std::decay_t<decltype(arg)>;
                if constexpr (std::is_pointer_v<U>) {
                    return arg;
                } else if constexpr (std::is_same_v<U, std::weak_ptr<T>>) {
                    auto spt = arg.lock();  // Try to lock the weak_ptr
                    return spt ? spt.get() : nullptr;
                } else {
                    return arg.get();
                }
            },
            ptr_);
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
                } else if constexpr (std::is_same_v<U, std::weak_ptr<T>>) {
                    auto spt = arg.lock();
                    if (spt) {
                        return ((*spt.get()).*
                                func)(std::forward<Args>(args)...);
                    }  // Handle the case where weak_ptr is expired
                    throw std::runtime_error("weak_ptr is expired");

                } else {
                    return ((*arg.get()).*func)(std::forward<Args>(args)...);
                }
            },
            ptr_);
    }

    /**
     * @brief Helper method to invoke a callable object on the pointed-to
     * object.
     *
     * @tparam Callable The type of the callable object.
     * @param callable The callable object.
     * @return auto The return type of the callable object.
     */
    template <typename Callable>
    [[nodiscard]] auto apply(Callable&& callable) {
        return std::visit(
            [&callable](auto&& arg) -> decltype(auto) {
                using U = std::decay_t<decltype(arg)>;
                if constexpr (std::is_pointer_v<U>) {
                    return std::invoke(std::forward<Callable>(callable), arg);
                } else if constexpr (std::is_same_v<U, std::weak_ptr<T>>) {
                    auto spt = arg.lock();
                    if (spt) {
                        return std::invoke(std::forward<Callable>(callable),
                                           spt.get());
                    }
                    throw std::runtime_error("weak_ptr is expired");

                } else {
                    return std::invoke(std::forward<Callable>(callable),
                                       arg.get());
                }
            },
            ptr_);
    }

    /**
     * @brief Helper function to apply a function to the pointed-to object,
     * with the function returning a void.
     *
     * @tparam Func The type of the function to apply.
     * @tparam Args The types of the arguments to the function.
     * @param func The function to apply.
     * @param args The arguments to the function.
     */
    template <typename Func, typename... Args>
    void applyVoid(Func func, Args&&... args) {
        std::visit(
            [&func, &args...](auto&& arg) {
                if constexpr (std::is_pointer_v<std::decay_t<decltype(arg)>>) {
                    func(*arg, std::forward<Args>(args)...);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>,
                                                    std::weak_ptr<T>>) {
                    auto spt = arg.lock();
                    if (spt) {
                        func(*spt.get(), std::forward<Args>(args)...);
                    } else {
                        // Handle the case where weak_ptr is expired
                        throw std::runtime_error("weak_ptr is expired");
                    }
                } else {
                    func(*arg.get(), std::forward<Args>(args)...);
                }
            },
            ptr_);
    }
};

#endif  // ATOM_TYPE_POINTER_HPP
