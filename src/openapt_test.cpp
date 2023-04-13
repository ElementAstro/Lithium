/*
 * openapt_test.cpp
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

Date: 2023-3-27

Description: Main Test

**************************************************/

#include <catch2/catch.hpp>
#include "config/achievement_list.hpp"
#include "config/configor.hpp"
#include "device/manager.hpp"
#include "module/compiler.hpp"
#include "module/lualoader.hpp"
#include "module/modloader.hpp"
#include "task/manager.hpp"

using namespace OpenAPT;

TEST_CASE("Test addAchievement") {
    AchievementList al;
    auto achievement = std::make_shared<Achievement>("Test", "Description");
    al.addAchievement(achievement);
    REQUIRE(al.hasAchievement("Test"));
}

TEST_CASE("Test removeAchievementByName") {
    AchievementList al;
    auto achievement = std::make_shared<Achievement>("Test", "Description");
    al.addAchievement(achievement);
    REQUIRE(al.hasAchievement("Test"));
    al.removeAchievementByName("Test");
    REQUIRE_FALSE(al.hasAchievement("Test"));
}

TEST_CASE("Test modifyAchievementByName") {
    AchievementList al;
    auto achievement1 = std::make_shared<Achievement>("Test", "Description");
    auto achievement2 = std::make_shared<Achievement>("Test", "New Description");
    al.addAchievement(achievement1);
    REQUIRE(al.hasAchievement("Test"));
    al.modifyAchievementByName("Test", achievement2);
    REQUIRE(al.hasAchievement("Test"));
    REQUIRE(al.getAchievementByName("Test")->getDescription() == "New Description");
}

TEST_CASE("Test completeAchievementByName") {
    AchievementList al;
    auto achievement = std::make_shared<Achievement>("Test", "Description");
    al.addAchievement(achievement);
    REQUIRE_FALSE(al.getAchievementByName("Test")->isCompleted());
    al.completeAchievementByName("Test");
    REQUIRE(al.getAchievementByName("Test")->isCompleted());
}

TEST_CASE("Test printAchievements") {
    AchievementList al;
    al.addAstronomyPhotographyAchievements(); // 添加一些天文摄影成就到列表中
    al.printAchievements();
}

TEST_CASE("Test loadFromFile") {
    ConfigManager cm;
    cm.loadFromFile("testdata/config1.json");
    REQUIRE(cm.getValue("database/username") == "root");
    REQUIRE(cm.getValue("database/password") == "123456");
}

TEST_CASE("Test loadFromDir") {
    ConfigManager cm;
    cm.loadFromDir("testdata", false);
    REQUIRE(cm.getValue("database/username") == "root");
    REQUIRE(cm.getValue("database/password") == "123456");
    REQUIRE(cm.getValue("smtp/server") == "smtp.example.com");
    REQUIRE(cm.getValue("smtp/port") == 587);
}

TEST_CASE("Test setValue") {
    ConfigManager cm;
    cm.setValue("database/username", "testuser");
    REQUIRE(cm.getValue("database/username") == "testuser");

    nlohmann::json j = {
        {"smtp", {
            {"server", "smtp.example.com"},
            {"port", 587}
        }}
    };
    cm.setValue("email", j);
    REQUIRE(cm.getValue("email/smtp/server") == "smtp.example.com");
    REQUIRE(cm.getValue("email/smtp/port") == 587);
}

TEST_CASE("Test deleteValue") {
    ConfigManager cm;
    cm.loadFromFile("testdata/config1.json");
    REQUIRE(cm.getValue("database/username") == "root");
    cm.deleteValue("database/username");
    REQUIRE(cm.getValue("database/username") == nullptr);
}

TEST_CASE("Test saveToFile") {
    ConfigManager cm;
    cm.loadFromFile("testdata/config1.json");
    cm.setValue("database/username", "testuser");
    cm.saveToFile("output/config_output.json");
    ConfigManager cm2;
    cm2.loadFromFile("output/config_output.json");
    REQUIRE(cm2.getValue("database/username") == "testuser");
}

TEST_CASE("Test printAllValues") {
    ConfigManager cm;
    cm.loadFromDir("testdata", false);
    cm.printAllValues();
}

TEST_CASE("Test addDevice and getDevice") {
    DeviceManager dm;
    dm.addDevice(DeviceType::Camera, "Camera1");
    auto camera = dm.getCamera("Camera1");
    REQUIRE(camera != nullptr);
}

TEST_CASE("Test removeDevice") {
    DeviceManager dm;
    dm.addDevice(DeviceType::Camera, "Camera1");
    dm.removeDevice(DeviceType::Camera, "Camera1");
    auto camera = dm.getCamera("Camera1");
    REQUIRE(camera == nullptr);
}

TEST_CASE("Test removeDevicesByName") {
    DeviceManager dm;
    dm.addDevice(DeviceType::Camera, "Camera1");
    dm.addDevice(DeviceType::Camera, "Camera2");
    dm.removeDevicesByName("Camera1");
    auto camera1 = dm.getCamera("Camera1");
    auto camera2 = dm.getCamera("Camera2");
    REQUIRE(camera1 == nullptr);
    REQUIRE(camera2 != nullptr);
}

TEST_CASE("Test findDevice") {
    DeviceManager dm;
    dm.addDevice(DeviceType::Camera, "Camera1");
    dm.addDevice(DeviceType::Camera, "Camera2");
    auto index1 = dm.findDevice(DeviceType::Camera, "Camera1");
    auto index2 = dm.findDevice(DeviceType::Camera, "Camera2");
    auto index3 = dm.findDevice(DeviceType::Camera, "Camera3");
    REQUIRE(index1 == 0);
    REQUIRE(index2 == 1);
    REQUIRE(index3 == -1);
}

TEST_CASE("Test findDeviceByName") {
    DeviceManager dm;
    dm.addDevice(DeviceType::Camera, "Camera1");
    auto camera1 = dm.findDeviceByName("Camera1");
    auto camera2 = dm.findDeviceByName("Camera2");
    REQUIRE(camera1 != nullptr);
    REQUIRE(camera2 == nullptr);
}

TEST_CASE("Test getSimpleTask") {
    DeviceManager dm;
    dm.addDevice(DeviceType::Camera, "Camera1");
    auto simple_task = std::make_shared<SimpleTask>("Capture Image");
    dm.getSimpleTask(DeviceType::Camera, "Camera", "Camera1", "Capture Image")->setDescription("Task Description");
    REQUIRE(dm.getDevice(DeviceType::Camera, "Camera1")->getTaskList().size() == 1);
    REQUIRE(dm.getDevice(DeviceType::Camera, "Camera1")->getTaskList()[0]->getDescription() == "Task Description");
}

TEST_CASE("Test getConditionalTask") {
    DeviceManager dm;
    dm.addDevice(DeviceType::Camera, "Camera1");
    auto conditional_task = std::make_shared<ConditionalTask>("Check Exposure Condition");
    dm.getDevice(DeviceType::Camera, "Camera1")->addTask(conditional_task);
    REQUIRE(dm.getDevice(DeviceType::Camera, "Camera1")->getTaskList().size() == 1);
    REQUIRE(dm.getConditionalTask("Camera1", "Check Exposure Condition") == conditional_task);
}

TEST_CASE("Test getLoopTask") {
    DeviceManager dm;
    dm.addDevice(DeviceType::Camera, "Camera1");
    auto loop_task = std::make_shared<LoopTask>("Capture Images in Loop");
    dm.getDevice(DeviceType::Camera, "Camera1")->addTask(loop_task);
    REQUIRE(dm.getDevice(DeviceType::Camera, "Camera1")->getTaskList().size() == 1);
    REQUIRE(dm.getLoopTask("Camera1", "Capture Images in Loop") == loop_task);
}

TEST_CASE("Test CompileToSharedLibrary") {
    Compiler compiler;
    std::string code = R"(
        #include <iostream>

        extern "C" int Run(int a, int b) {
            std::cout << "a + b = " << a + b << std::endl;
            return a + b;
        }
    )";
    std::string moduleName = "MyModule";
    std::string functionName = "Run";
    bool result = compiler.CompileToSharedLibrary(code, moduleName, functionName);
    REQUIRE(result == true);
}

TEST_CASE("Test LoadScript") {
    LuaScriptLoader luaScriptLoader;
    std::string name = "test_script";
    std::string path = "./test_script.lua";
    bool result = luaScriptLoader.LoadScript(name, path);
    REQUIRE(result == true);
}

TEST_CASE("Test UnloadScript") {
    LuaScriptLoader luaScriptLoader;
    std::string name = "test_script";
    luaScriptLoader.UnloadScript(name);
}

TEST_CASE("Test CallFunction") {
    LuaScriptLoader luaScriptLoader;
    std::string name = "add";
    std::string scriptName = "test_script";
    int result = 0;
    bool callResult = luaScriptLoader.CallFunction(name, scriptName, result, 1, 2);
    REQUIRE(callResult == true);
    REQUIRE(result == 3);
}

TEST_CASE("Test SetGlobal and GetGlobal") {
    LuaScriptLoader luaScriptLoader;
    std::string name = "global_var";
    std::string scriptName = "test_script";
    int value = 123;
    luaScriptLoader.SetGlobal(name, scriptName, value);
    int result = 0;
    bool getResult = luaScriptLoader.GetGlobal(name, scriptName, result);
    REQUIRE(getResult == true);
    REQUIRE(result == value);
}

TEST_CASE("Test InjectFunctions") {
    LuaScriptLoader luaScriptLoader;
    std::unordered_map<std::string, lua_CFunction> functions = {{"Greet", Greet}};
    luaScriptLoader.InjectFunctions(functions);
}

TEST_CASE("Test LoadFunctionsFromJsonFile") {
    LuaScriptLoader luaScriptLoader;
    std::string file_path = "./functions.json";
    luaScriptLoader.LoadFunctionsFromJsonFile(file_path);
}

TEST_CASE("ModuleLoader Test", "[ModuleLoader]") {
    ModuleLoader loader;

    SECTION("LoadModule Test") {
        bool result = loader.LoadModule("module_path", "module_name");
        REQUIRE(result == true);
    }

    SECTION("UnloadModule Test") {
        bool result = loader.UnloadModule("module_name");
        REQUIRE(result == true);
    }

    SECTION("LoadBinary Test") {
        bool result = loader.LoadBinary("dir_path", "out_path", "build_path", "lib_name");
        REQUIRE(result == true);
    }

    SECTION("GetFunction Test") {
        using FuncType = int(*)(int, int); // 自定义函数类型
        FuncType func = loader.GetFunction<FuncType>("module_name", "function_name");
        REQUIRE(func != nullptr);
    }

    SECTION("GetFunctionObject Test") {
        using FuncType = void(*)(const std::string&); // 自定义函数类型
        std::function<FuncType> func = loader.GetFunctionObject<FuncType>("module_name", "function_name");
        REQUIRE(func != nullptr);
    }

    SECTION("LoadAndRunFunction Class Test") {
        // TODO: 在模块中定义一个类并导出类成员函数作为测试用例
        class MyClass {
            public:
                int add(int a, int b) { return a + b; }
        };
        MyClass obj;
        bool result = loader.LoadAndRunFunction<int, MyClass>("module_name", "add", "thread_name", true, &obj, 2, 3);
        REQUIRE(result == true);
    }

    SECTION("LoadAndRunFunction Function Test") {
        // TODO: 在模块中定义一个函数作为测试用例
        int add(int a, int b) { return a + b; }
        int result = loader.LoadAndRunFunction<int>("module_name", "add", "thread_name", true, 2, 3);
        REQUIRE(result == 5);
    }

    SECTION("HasModule Test") {
        bool result = loader.HasModule("module_name");
        REQUIRE(result == true);
    }

    SECTION("getArgsDesc Test") {
        // TODO: 创建一个模块并定义一个函数，以测试getArgsDesc函数
        void* handle = nullptr;
        nlohmann::json result = loader.getArgsDesc(handle, "function_name");
        REQUIRE(result.is_object());
    }
}

TEST_CASE("DownloadManager Test", "[DownloadManager]") {
    // 创建DownloadManager对象
    DownloadManager manager("path_to_task_file");

    SECTION("add_task Test") {
        manager.add_task("https://example.com/file1.zip", "path/to/local/file1.zip");
        REQUIRE(manager.tasks_.size() == 1);

        manager.add_task("https://example.com/file2.zip", "path/to/local/file2.zip");
        REQUIRE(manager.tasks_.size() == 2);
    }

    SECTION("remove_task Test") {
        manager.add_task("https://example.com/file1.zip", "path/to/local/file1.zip");
        manager.add_task("https://example.com/file2.zip", "path/to/local/file2.zip");
        REQUIRE(manager.tasks_.size() == 2);

        bool result = manager.remove_task(1);
        REQUIRE(result == true);
        REQUIRE(manager.tasks_.size() == 1);

        result = manager.remove_task(1); // 越界删除
        REQUIRE(result == false);
        REQUIRE(manager.tasks_.size() == 1);
    }

    SECTION("download_task Test") {
        // TODO: 编写该函数的测试用例
    }

    SECTION("start Test") {
        // TODO: 编写该函数的测试用例
    }
}

TEST_CASE("TaskManager construction and modification", "[TaskManager]") {
    // Test constructing an empty task manager
    TaskManager tm;
    REQUIRE(tm.getCompletedTaskCount() == 0);
    REQUIRE(tm.m_taskList.empty());

    // Test constructing a task manager from a file
    TaskManager tm2("task_list.json");
    REQUIRE(tm2.m_taskList.size() > 0);

    // Test adding a task
    auto task = std::make_shared<BasicTask>("test", "description", time(nullptr));
    tm.addTask(task);
    REQUIRE(tm.m_taskList.size() == 1);
    REQUIRE(tm.m_taskList[0] == task);

    // Test inserting a task
    auto task2 = std::make_shared<BasicTask>("test2", "description", time(nullptr));
    tm.insertTask(0, task2);
    REQUIRE(tm.m_taskList.size() == 2);
    REQUIRE(tm.m_taskList[0] == task2);

    // Test deleting a task by index
    tm.deleteTask(1);
    REQUIRE(tm.m_taskList.size() == 1);

    // Test deleting a task by name
    tm.deleteTaskByName("test");
    REQUIRE(tm.m_taskList.empty());

    // Test modifying a task by index
    auto task3 = std::make_shared<BasicTask>("test3", "description", time(nullptr));
    tm.addTask(task3);
    tm.modifyTask(0, std::make_shared<BasicTask>("test4", "new description", time(nullptr)), true);
    auto modifiedTask = tm.m_taskList[0];
    REQUIRE(modifiedTask->getName() == "test4");
    REQUIRE(modifiedTask->getDescription() == "new description");
    REQUIRE(modifiedTask->getTime() != task3->getTime());

    // Test modifying a task by name
    tm.modifyTaskByName("test4", std::make_shared<BasicTask>("test5", "modified description", time(nullptr)), false);
    auto modifiedTask2 = tm.m_taskList[0];
    REQUIRE(modifiedTask2->getName() == "test5");
    REQUIRE(modifiedTask2->getDescription() == "modified description");
    REQUIRE(modifiedTask2->getTime() != task3->getTime());
}

TEST_CASE("TaskManager executing and querying tasks", "[TaskManager]") {
    TaskManager tm;
    auto task1 = std::make_shared<BasicTask>("task1", "description1", time(nullptr) + 3600);
    auto task2 = std::make_shared<BasicTask>("task2", "description2", time(nullptr) + 7200);
    auto task3 = std::make_shared<BasicTask>("task3", "description3", time(nullptr) + 10800);
    tm.addTask(task1);
    tm.addTask(task2);
    tm.addTask(task3);
    REQUIRE(tm.m_taskList.size() == 3);

    // Test executing all tasks
    tm.executeAllTasks();
    REQUIRE(tm.getCompletedTaskCount() == 0); // no tasks should be completed yet
    tm.cleanCompletedTasks();
    REQUIRE(tm.m_taskList.empty()); // all tasks should be completed and removed

    // Test querying a task by name
    auto task4 = std::make_shared<BasicTask>("task4", "description4", time(nullptr) + 14400);
    tm.addTask(task4);
    std::stringstream ss;
    ss << "Name: " << task4->getName() << "\n"
       << "Description: " << task4->getDescription() << "\n"
       << "Time: " << task4->getTime() << "\n";
    REQUIRE(ss.str() == "Name: task4\nDescription: description4\nTime: " + std::to_string(task4->getTime()) + "\n");
}

TEST_CASE("TaskManager file I/O", "[TaskManager]") {
    // Test reading tasks from a file
    TaskManager tm;
    std::vector<std::string> filenames = { "task_list.json" };
    tm.runFromJson(filenames, false);
    REQUIRE(tm.m_taskList.size() > 0);

    // Test writing tasks to a file
    std::string filename = "test_output.json";
    tm.saveTasksToJson(filename);
    TaskManager tm2(filename);
    REQUIRE(tm.m_taskList.size() == tm2.m_taskList.size());
    for (int i = 0; i < tm.m_taskList.size(); ++i) {
        REQUIRE(*tm.m_taskList[i] == *tm2.m_taskList[i]);
    }
}