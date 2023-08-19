/*
 * elapsedtimer.hpp
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

#pragma once

#include <chrono>
#include <memory>

#include "util/macro.hpp"

namespace Lithium
{

    class ElapsedTimerPrivate
    {
    public:
        std::chrono::steady_clock::time_point start;
    };

    class ElapsedTimer
    {
    public:
        ElapsedTimer();
        explicit ElapsedTimer(ElapsedTimerPrivate &dd);
        ~ElapsedTimer();

        void start();
        int64_t restart();
        int64_t elapsed() const;
        int64_t nsecsElapsed() const;
        bool hasExpired(int64_t timeout) const;
        void nsecsRewind(int64_t nsecs);

    private:
        std::shared_ptr<ElapsedTimerPrivate> d_ptr;

        ElapsedTimerPrivate *d_func() { return d_ptr.get(); }
        const ElapsedTimerPrivate *d_func() const { return d_ptr.get(); }
    };

}
