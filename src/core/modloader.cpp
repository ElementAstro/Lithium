/*
 * modloader.cpp
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

Date: 2023-6-1

Description: Dynamic Lib Loader

*************************************************/

#include <dlfcn.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <spdlog/spdlog.h>

#include "modloader.hpp"

#include <openssl/md5.h>
#include <fstream>

std::string DynamicLibManager::calcMD5(const std::string &filename)
{
    // 打开文件
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file)
    {
        spdlog::error("failed to open file {}\n", filename);
        return "";
    }

    // 计算md5值
    unsigned char md5[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)))
    {
        MD5_Update(&ctx, buffer, sizeof(buffer));
    }
    MD5_Update(&ctx, buffer, file.gcount());

    MD5_Final(md5, &ctx);

    // 将md5值转换为字符串
    std::stringstream ss;
    for (int i = 0; i < sizeof(md5); i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)md5[i];
    }

    return ss.str();
}

// 加载动态库
bool DynamicLibManager::loadLibInternal(const std::string &libName, const std::string &filename)
{
    // 检查文件是否存在
    if (!fs::exists(filename))
    {
        spdlog::error("failed to load dynamic lib {}: file does not exist.\n", libName);
        return false;
    }

    // 检查是否已经加载相同的模块
    auto md5 = calcMD5(filename);
    if (isLibLoaded(md5))
    {
        spdlog::error("lib {} already loaded.\n", libName);
        return false;
    }

    // 加载动态库
    void *handle = dlopen(filename.c_str(), RTLD_LAZY);
    if (handle == nullptr)
    {
        spdlog::error("failed to load dynamic lib {}: {}\n", libName, dlerror());
        return false;
    }

    // 读取info.json文件中的信息
    nlohmann::json info;
    try
    {
        std::ifstream ifs(filename + "/info.json");
        if (ifs.is_open())
        {
            ifs >> info;
            ifs.close();

            // 将动态库信息加入表中
            libTable[md5] = {
                .md5 = md5,
                .handle = handle,
                .symTable = {}};

            // 将函数和变量放入符号表中
            for (const auto &item : info.items())
            {
                const auto &name = item.key();
                const auto &value = item.value();
                const auto &type = value["type"].get<std::string>();
                if (type == "function")
                {
                    auto sym = dlsym(handle, name.c_str());
                    if (sym != nullptr)
                    {
                        auto length = sizeof(sym) + sizeof(type);
                        libTable[md5].symTable[name] = std::span<void>(sym, length);
                    }
                    else
                    {
                        spdlog::error("failed to load function {} from dynamic lib {}: {}\n", name, libName, dlerror());
                    }
                }
                else if (type == "variable")
                {
                    auto offset = reinterpret_cast<void *>(value["offset"].get<int>());
                    auto length = value["length"].get<size_t>();
                    if (offset != nullptr && length > 0)
                    {
                        libTable[md5].symTable[name] = std::span<void>(offset, length);
                    }
                    else
                    {
                        spdlog::error("failed to load variable {} from dynamic lib {}: invalid offset or length.\n", name, libName);
                    }
                }
            }
            return true;
        }
        else
        {
            spdlog::error("failed to read info.json from dynamic lib {}.\n", libName);
            dlclose(handle);
            return false;
        }
    }
    catch (const std::exception &ex)
    {
        spdlog::error("failed to load dynamic lib {}: {}\n", libName, ex.what());
        dlclose(handle);
        return false;
    }
}

bool DynamicLibManager::loadLib(const std::string &libName)
{
    const std::string &filename = libName;
    bool success = loadLibInternal(libName, filename);
    if (success)
    {
        spdlog::info("loaded lib {}.", libName);
    }
    else
    {
        spdlog::error("failed to load lib {}.\n", libName);
    }
    return success;
}

// 卸载动态库
bool DynamicLibManager::unloadLib(const std::string &libName)
{
    auto md5 = calcMD5(libName);
    auto iter = libTable.find(md5);
    if (iter == libTable.end())
    {
        spdlog::error("lib {} is not loaded.\n", libName);
        return false;
    }

    auto info = iter->second;

    // 卸载动态库
    dlclose(info.handle);

    // 清除符号表和信息表的数据
    libTable.erase(iter);

    spdlog::info("unloaded lib {}.", libName);
    return true;
}

// 根据名字搜索
std::shared_ptr<Task> DynamicLibManager::search(const std::string &taskName)
{
    for (auto &[md5, info] : libTable)
    {
        auto iter = info.symTable.find(taskName);
        if (iter != info.symTable.end())
        {
            // 获取函数指针或变量指针
            auto span = iter->second;
            if (span.size() >= sizeof(void *))
            {
                auto ptr = *reinterpret_cast<void **>(span.data());
                if (ptr != nullptr)
                {
                    // 如果是函数指针，创建一个Task对象并返回
                    if (std::string(span.data() + sizeof(void *), span.size() - sizeof(void *)) == "function")
                    {
                        auto task = std::shared_ptr<Task>(new TaskFunc(ptr));
                        return task;
                    }
                }
            }
        }
    }

    return nullptr;
}

// 获取函数指针
void *DynamicLibManager::getFunctionPtr(const std::string &functionName)
{
    for (auto &[md5, info] : libTable)
    {
        auto iter = info.symTable.find(functionName);
        if (iter != info.symTable.end())
        {
            auto span = iter->second;
            if (span.size() >= sizeof(void *))
            {
                auto ptr = *reinterpret_cast<void **>(span.data());
                if (ptr != nullptr)
                {
                    auto type = std::string(span.data() + sizeof(void *), span.size() - sizeof(void *));
                    if (type == "function")
                    {
                        return ptr;
                    }
                }
            }
        }
    }

    return nullptr;
}

// 获取变量指针
void *DynamicLibManager::getVariablePtr(const std::string &variableName)
{
    for (auto &[md5, info] : libTable)
    {
        auto iter = info.symTable.find(variableName);
        if (iter != info.symTable.end())
        {
            auto span = iter->second;
            if (span.size() >= sizeof(void *))
            {
                auto ptr = reinterpret_cast<void *>(span.data());
                return ptr;
            }
        }
    }

    return nullptr;
}

// 检查动态库是否已经加载
bool DynamicLibManager::isLibLoaded(const std::string &md5)
{
    return libTable.find(md5) != libTable.end();
}
