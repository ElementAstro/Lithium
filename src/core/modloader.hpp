/*
 * modloader.hpp
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

#ifndef DYNAMIC_LIB_MANAGER_H
#define DYNAMIC_LIB_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <span>

#include "nlohmann/json.hpp"

#include <filesystem>
namespace fs = std::filesystem;

#include "basic/task.hpp"

class DynamicLibManager
{
public:
    // 加载动态库
    bool loadLib(const std::string &libName);

    // 卸载动态库
    bool unloadLib(const std::string &libName);

    // 根据名字搜索动态库
    std::shared_ptr<Task> search(const std::string &taskName);

    // 获取函数指针
    void *getFunctionPtr(const std::string &functionName);

    // 获取变量指针
    void *getVariablePtr(const std::string &variableName);

private:
    // 动态库信息结构体
    struct LibInfo
    {
        std::string md5;
        void *handle;
        std::unordered_map<std::string, std::span<void>> symTable;
    };

    // md5值计算函数
    std::string calcMD5(const std::string &filename);

    // 加载动态库
    bool loadLibInternal(const std::string &libname, const std::string &filename);

    // 检查动态库是否已经加载
    bool isLibLoaded(const std::string &md5);

    // 动态库信息表
    std::unordered_map<std::string, LibInfo> libTable;
};

#endif // DYNAMIC_LIB_MANAGER_H

/*
#include "DynamicLibManager.h"
#include <iostream>

void test() {
    DynamicLibManager manager;
    manager.loadLib("libtest.so");
    auto task = manager.search("testTask");
    if (task) {
        task->run();
    } else {
        std::cout << "task not found." << std::endl;
    }
    manager.unloadLib("libtest.so");
}

int main() {
    test();
    return 0;
}

#include <iostream>
#include "Task.h"

class TestTask : public Task {
public:
    virtual void run() override {
        std::cout << "This is a test task." << std::endl;
    }
};

extern "C" {
    Task* testTask() {
        return new TestTask();
    }
}

*/