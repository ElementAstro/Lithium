#ifndef ATOM_FUNCTION_PROXY_PARAMS_HPP
#define ATOM_FUNCTION_PROXY_PARAMS_HPP

#include <any>
#include <array>
#include <vector>

class FunctionParams {
public:
    constexpr FunctionParams(const std::any *const t_begin,
                              const std::any *const t_end)
        : m_begin(t_begin), m_end(t_end) {}

    explicit FunctionParams(const std::any &bv)
        : m_begin(&bv), m_end(m_begin + 1) {}

    explicit FunctionParams(const std::vector<std::any> &vec)
        : m_begin(vec.empty() ? nullptr : &vec.front()),
          m_end(vec.empty() ? nullptr : &vec.front() + vec.size()) {}

    template <size_t Size>
    constexpr explicit FunctionParams(const std::array<std::any, Size> &a)
        : m_begin(&a.front()), m_end(&a.front() + Size) {}

    [[nodiscard]] constexpr const std::any &operator[](
        const std::size_t t_i) const noexcept {
        return m_begin[t_i];
    }

    [[nodiscard]] constexpr const std::any *begin() const noexcept {
        return m_begin;
    }

    [[nodiscard]] constexpr const std::any &front() const noexcept {
        return *m_begin;
    }

    [[nodiscard]] constexpr const std::any *end() const noexcept {
        return m_end;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return std::size_t(m_end - m_begin);
    }

    [[nodiscard]] std::vector<std::any> to_vector() const {
        return std::vector<std::any>{m_begin, m_end};
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return m_begin == m_end;
    }

private:
    const std::any *m_begin = nullptr;
    const std::any *m_end = nullptr;
};

// Constructor specialization for array of size 0
template <>
constexpr FunctionParams::FunctionParams(
    const std::array<std::any, size_t{0}> & /* a */)
    : m_begin(nullptr), m_end(nullptr) {}

#endif
