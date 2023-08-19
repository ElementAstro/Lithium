/*
 * elapsedtimer.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-4-9

Description: Timer (indielapsedtimer)

**************************************************/

#include "elapsedtimer.hpp"

namespace Lithium
{
    ElapsedTimer::ElapsedTimer()
        : d_ptr(std::make_shared<ElapsedTimerPrivate>())
    {
        start();
    }

    ElapsedTimer::ElapsedTimer(ElapsedTimerPrivate &dd)
        : d_ptr(std::make_shared<ElapsedTimerPrivate>(dd))
    {
        start();
    }

    ElapsedTimer::~ElapsedTimer() = default;

    void ElapsedTimer::start()
    {
        D_PTR(ElapsedTimer);
        d->start = std::chrono::steady_clock::now();
    }

    int64_t ElapsedTimer::restart()
    {
        D_PTR(ElapsedTimer);
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        int64_t result = std::chrono::duration_cast<std::chrono::milliseconds>(now - d->start).count();
        d->start = now;
        return result;
    }

    int64_t ElapsedTimer::elapsed() const
    {
        D_PTR(const ElapsedTimer);
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - d->start).count();
    }

    int64_t ElapsedTimer::nsecsElapsed() const
    {
        D_PTR(const ElapsedTimer);
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now - d->start).count();
    }

    bool ElapsedTimer::hasExpired(int64_t timeout) const
    {
        return elapsed() > timeout;
    }

    void ElapsedTimer::nsecsRewind(int64_t nsecs)
    {
        D_PTR(ElapsedTimer);
        d->start += std::chrono::nanoseconds(nsecs);
    }

} // namespace Lithium
