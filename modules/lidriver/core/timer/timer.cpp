/*
 * timer.cpp
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

#include "timer.hpp"

namespace Lithium
{

    void Timer::callOnTimeout(const std::function<void()> &callback)
    {
        this->callback = callback;
    }

    void Timer::start()
    {
        stop();
        stopFlag = std::make_shared<std::atomic<bool>>(false);
        active = true;

        if (singleShot)
        {
            std::thread([this, stopFlag = stopFlag]
                        {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if (!stopFlag->load())
                timeout(); })
                .detach();
        }
        else
        {
            std::thread([this, stopFlag = stopFlag]()
                        {
            while (!stopFlag->load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                if (!stopFlag->load())
                    timeout();
            } })
                .detach();
        }
    }

    void Timer::start(int msec)
    {
        setInterval(msec);
        start();
    }

    void Timer::stop()
    {
        if (active)
        {
            *stopFlag = true;
            active = false;
        }
    }

    void Timer::setInterval(int msec)
    {
        interval = msec;
    }

    void Timer::setSingleShot(bool singleShot)
    {
        this->singleShot = singleShot;
    }

    bool Timer::isActive() const
    {
        return active;
    }

    bool Timer::isSingleShot() const
    {
        return singleShot;
    }

    int Timer::remainingTime() const
    {
        return active ? interval : 0;
    }

    int Timer::interval() const
    {
        return interval;
    }

    void Timer::timeout()
    {
        if (callback)
        {
            callback();
            if (singleShot)
                active = false;
        }
    }

    void Timer::singleShot(int msec, const std::function<void()> &callback)
    {
        Timer timer;
        timer.setSingleShot(true);
        timer.setInterval(msec);
        timer.callOnTimeout([callback = std::move(callback), &timer]()
                            {
        callback();
        timer.stop(); });
        timer.start();
    }
}