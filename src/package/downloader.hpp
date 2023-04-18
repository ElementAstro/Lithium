/*
 * downloader.hpp
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

Description: Downloader

**************************************************/

#pragma once

#include <string>
#include <vector>

struct DownloadTask
{
    std::string url;
    std::string filepath;
    bool completed = false;
};

class DownloadManager
{
public:
    DownloadManager(const std::string &task_file);
    ~DownloadManager();
    void add_task(const std::string &url, const std::string &filepath);
    bool remove_task(size_t index);
    void start();

private:
    std::string task_file_;
    std::vector<DownloadTask> tasks_;

    static void download_task(DownloadManager *manager, size_t index);
    void download_task(size_t index);
};
