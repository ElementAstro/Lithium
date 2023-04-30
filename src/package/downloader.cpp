/*
 * downloader.cpp
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

#include "downloader.hpp"

#include <iostream>
#include <fstream>
#include <thread>
#include <curl/curl.h>
#include "spdlog/spdlog.h"

DownloadManager::DownloadManager(const std::string& task_file)
    : task_file_(task_file)
{
    // 从文件中读取任务列表
    try {
        std::ifstream infile(task_file_);
        if (!infile) {
            spdlog::error("Failed to open task file {}.", task_file_);
            throw std::runtime_error("Failed to open task file.");
        }
        while (infile >> std::ws && !infile.eof()) {
            std::string url, filepath;
            infile >> url >> filepath;
            if (!url.empty() && !filepath.empty()) {
                tasks_.push_back({ url, filepath });
            }
        }
        infile.close();
    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
        throw;
    }
}

DownloadManager::~DownloadManager()
{
    save_task_list_to_file();
}

void DownloadManager::add_task(const std::string& url, const std::string& filepath)
{
    tasks_.push_back({ url, filepath });
    try {
        std::ofstream outfile(task_file_, std::ios_base::app);
        if (!outfile) {
            spdlog::error("Failed to open task file {}.", task_file_);
            throw std::runtime_error("Failed to open task file.");
        }
        outfile << url << " " << filepath << std::endl;
        outfile.close();
    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
        throw;
    }
}

bool DownloadManager::remove_task(size_t index)
{
    if (index >= tasks_.size()) {
        return false;
    }
    tasks_[index].completed = true;
    return true;
}

void DownloadManager::start()
{
    std::vector<std::thread> threads;
    for (size_t i = 0; i < tasks_.size(); ++i) {
        if (tasks_[i].completed) {
            continue;
        }
        threads.emplace_back([this, i]() {
            download_task(i);
        });
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void DownloadManager::download_task(size_t index)
{
    CURL *curl = curl_easy_init();
    if (!curl) {
        spdlog::error("Failed to initialize curl.");
        return;
    }

    auto& task = tasks_[index];
    CURLcode res;
    FILE *fp = fopen(task.filepath.c_str(), "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        spdlog::error("Failed to open file {}.", task.filepath);
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, task.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    // 使用 std::thread 实现多线程下载
    std::thread download_thread([&, curl, fp] {
        try {
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                spdlog::error("Failed to download {}.", task.url);
            }
            else {
                spdlog::info("Downloaded file {}.", task.filepath);
            }
            fclose(fp);
        }
        catch (const std::exception& e) {
            spdlog::error(e.what());
            fclose(fp);
            throw;
        }
        curl_easy_cleanup(curl);
    });

    download_thread.detach();
}

void DownloadManager::save_task_list_to_file()
{
    try {
        std::ofstream outfile(task_file_);
        if (!outfile) {
            spdlog::error("Failed to open task file {}.", task_file_);
            throw std::runtime_error("Failed to open task file.");
        }
        for (const auto& task : tasks_) {
            if (!task.completed) {
                outfile << task.url << " " << task.filepath << std::endl;
            }
        }
        outfile.close();
    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
    }
}