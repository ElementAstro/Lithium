/*
 * defer.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Defer Exec

*************************************************/

#pragma once

#include <utility>
#include <functional>

namespace Atom::Event
{
    template <class Callable>
    class DeferExec final
    {
    public:
        DeferExec(Callable c) : c_(std::move(c)), canceled_(false) {}
        DeferExec(DeferExec &&other) noexcept : c_(std::move(other.c_)), canceled_(other.canceled_)
        {
            other.canceled_ = true;
        }
        ~DeferExec()
        {
            if (!canceled_)
            {
                c_();
            }
        }

        DeferExec(const DeferExec &) = delete;
        DeferExec &operator=(const DeferExec &) = delete;

        void cancel()
        {
            canceled_ = true;
        }

    private:
        Callable c_;
        bool canceled_;
    };

    template <typename Callable>
    DeferExec<Callable> make_defer(Callable c)
    {
        return {std::move(c)};
    }
} // namespace Atom::Event

#define CONCAT_XY(x, y) x##y
#define MAKE_DEFER(r, l) auto CONCAT_XY(defer_exec_, l) = Atom::Event::make_defer([&]() { r; })
#define DEFER(r) MAKE_DEFER(r, __LINE__)
