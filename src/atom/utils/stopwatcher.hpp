/*
 * stopwatcher.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-12-25

Description: Simple implementation of a stopwatch

**************************************************/

#pragma once

#include <chrono>
#include <array>
#include <utility>
#include <cstddef>

namespace Atom::Utils
{
    template <size_t CountN = 1, class ClockT = std::chrono::steady_clock>
    class stopwatch : public ClockT
    {
        using base_t = ClockT;

        static_assert(CountN > 0, "The count must be greater than 0");

    public:
        using rep = typename ClockT::rep;
        using period = typename ClockT::period;
        using duration = typename ClockT::duration;
        using time_point = typename ClockT::time_point;

    private:
        using pair_t = std::pair<time_point, time_point>;

        std::array<pair_t, CountN> points_;
        bool is_stopped_ = true;

    public:
        stopwatch(bool start_watch = false)
        {
            if (start_watch)
                start();
        }

    public:
        bool is_stopped() const noexcept
        {
            return is_stopped_;
        }

        template <size_t N = 0>
        bool is_paused() const noexcept
        {
            return (points_[N].second != points_[N].first);
        }

        template <size_t N = 0>
        duration elapsed()
        {
            if (is_stopped())
                return duration::zero();
            else if (is_paused<N>())
                return (points_[N].second - points_[N].first);
            else
                return ClockT::now() - points_[N].first;
        }

        template <typename ToDur, size_t N = 0>
        auto elapsed() -> decltype(std::declval<ToDur>().count())
        {
            return std::chrono::duration_cast<ToDur>(elapsed<N>()).count();
        }

        template <size_t N = 0>
        void pause() noexcept
        {
            points_[N].second = ClockT::now();
        }

        template <size_t N = 0>
        void restart() noexcept
        {
            points_[N].second = points_[N].first =
                ClockT::now() - (points_[N].second - points_[N].first);
        }

        void start() noexcept
        {
            time_point now = ClockT::now();
            for (auto &pt : points_)
            {
                pt.second = pt.first = now - (pt.second - pt.first);
            }
            is_stopped_ = false;
        }

        void stop() noexcept
        {
            for (auto &pt : points_)
            {
                pt.second = pt.first;
            }
            is_stopped_ = true;
        }
    };
}