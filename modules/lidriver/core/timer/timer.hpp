/*
 * timer.hpp
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

Description: Timer (inditimer)

**************************************************/

#pragma once

#include <chrono>
#include <functional>
#include <memory>

namespace Lithium
{

    class Timer
    {
    public:
        Timer() = default;
        ~Timer() = default;

        void callOnTimeout(const std::function<void()> &callback);
        void start();
        void start(int msec);
        void stop();
        void setInterval(int msec);
        void setSingleShot(bool singleShot);
        bool isActive() const;
        bool isSingleShot() const;
        int remainingTime() const;
        int interval() const;

        static void singleShot(int msec, const std::function<void()> &callback);

    private:
        std::function<void()> callback;
        int interval{1000};
        bool singleShot{false};
        bool active{false};
        std::shared_ptr<std::atomic<bool>> stopFlag;

        void timeout();
    };
}