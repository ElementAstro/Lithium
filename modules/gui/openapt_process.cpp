/*
 * process_manager.hpp
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
 
Description: OpenAPT GUI Tool - Process
 
**************************************************/

#include <iostream>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <csignal> // 包含信号处理相关的定义

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cstdio>
#include <mntent.h>
#include <sys/statvfs.h>

const int kBufSize = 512;
const char *kLogFileName = "process_manager.log";

struct ProcessInfo
{
    int pid;
    std::string name;
};

std::vector<ProcessInfo> GetProcessListImpl()
{
    std::vector<ProcessInfo> res;

    DIR *dir = opendir("/proc");
    if (!dir)
    {
        spdlog::error("Failed to opendir /proc: {}", strerror(errno));
        return res;
    }

    struct dirent *entry;
    char path[256];
    std::string cmd;
    int pid = -1;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (!isdigit(entry->d_name[0]))
            continue;

        pid = atoi(entry->d_name);
        snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
        FILE *file = fopen(path, "r");
        if (!file)
            continue;

        char buf[kBufSize];
        size_t nread = fread(buf, 1, sizeof(buf), file);
        fclose(file);

        if (nread > 0)
        {
            cmd.assign(buf, nread);
            auto pos = cmd.find('\0');
            if (pos != std::string::npos)
            {
                cmd = cmd.substr(0, pos);
            }
            ProcessInfo info;
            info.name = cmd;
            info.pid = pid;
            res.push_back(info);
        }
    }
    closedir(dir);

    return res;
}

std::vector<ProcessInfo> GetProcessList()
{
    try
    {
        return GetProcessListImpl();
    }
    catch (const std::exception &e)
    {
        spdlog::error("GetProcessList failed: {}", e.what());
        return {};
    }
}

bool KillProcessImpl(int pid)
{
    return kill(pid, SIGINT) == 0;
}

bool KillProcess(int pid)
{
    try
    {
        return KillProcessImpl(pid);
    }
    catch (const std::exception &e)
    {
        spdlog::error("KillProcess failed: {}", e.what());
        return false;
    }
}

void RenderProcessList(const std::vector<ProcessInfo> &processes, int &selected_process_index)
{
    ImGui::Begin("Process List");

    for (int i = 0; i < processes.size(); i++)
    {
        const auto &process = processes[i];
        std::string label = "##" + std::to_string(i);
        bool is_selected = selected_process_index == i;

        // 如果用户单击了进程名称，则设置其为当前选定的进程
        if (ImGui::Selectable(label.c_str(), is_selected))
        {
            selected_process_index = is_selected ? -1 : i;
        }
        ImGui::SameLine();
        ImGui::Text("%d: %s", process.pid, process.name.c_str());
    }

    ImGui::End();
}

void RenderProcessControl(const std::vector<ProcessInfo>& processes, int selected_process_index)
{
    if (selected_process_index != -1)
    {
        const auto& process = processes[selected_process_index];
        std::string window_title = "Control Process " + std::to_string(process.pid);
        ImGui::Begin(window_title.c_str());

        // 显示进程信息和控制按钮
        ImGui::Text("ID: %d, Name: %s", process.pid, process.name.c_str());

        ImGui::Separator();

        ImGui::Spacing();

        // 控制按钮
        ImGui::Text("Control:");
        ImGui::Indent();
        if (ImGui::Button("Kill"))
        {
            if (KillProcess(process.pid))
            {
                spdlog::info("Process {} terminated successfully.", process.pid);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            selected_process_index = -1;
        }
        ImGui::Unindent();

        ImGui::End();
    }
}



void RenderSystemInfo()
{
    ImGui::Begin("System Info");

    // 显示 CPU 信息
    ImGui::Text("CPU:");
    ImGui::Indent();
    FILE *cpu_info_fp = fopen("/proc/cpuinfo", "r");
    if (cpu_info_fp != nullptr)
    {
        char processor[256] = {0};
        char vendor_id[256] = {0};
        char cpu_family[256] = {0};
        char model_name[256] = {0};
        char cpu_mhz[256] = {0};

        while (!feof(cpu_info_fp))
        {
            char line[1024] = {0};
            fgets(line, sizeof(line), cpu_info_fp);

            if (strstr(line, "processor"))
                sscanf(line, "%*s %*s %[^\n]", processor);
            else if (strstr(line, "vendor_id"))
                sscanf(line, "%*s %*s %[^\n]", vendor_id);
            else if (strstr(line, "cpu family"))
                sscanf(line, "%*s %[^\n]", cpu_family);
            else if (strstr(line, "model name"))
                sscanf(line, "%*s %[^\n]", model_name);
            else if (strstr(line, "cpu MHz"))
                sscanf(line, "%*s %[^\n]", cpu_mhz);
        }
        fclose(cpu_info_fp);

        ImGui::Text("%s %s %s\n%s @ %s", vendor_id, cpu_family, model_name, processor, cpu_mhz);
    }
    else
    {
        ImGui::Text("N/A");
    }
    ImGui::Unindent();
    ImGui::Spacing();

    // 显示 CPU 负载
    ImGui::Text("CPU Load:");
    ImGui::Indent();
    FILE *loadavg_fp = fopen("/proc/loadavg", "r");
    if (loadavg_fp != nullptr)
    {
        float load1, load5, load15;
        int ncores = std::thread::hardware_concurrency();
        fscanf(loadavg_fp, "%f %f %f", &load1, &load5, &load15);
        fclose(loadavg_fp);

        ImGui::Text(" %.2f (1 min) / %.2f (5 min) / %.2f (15 min)", load1 / ncores, load5 / ncores, load15 / ncores);

        float cpu_usage = load1 / ncores * 100; // 计算 CPU 使用率
        ImGui::ProgressBar(cpu_usage / 100.0f, ImVec2(-1.0f, 0.0f), "");
        ImGui::Text("%.2f%%", cpu_usage);
    }
    else
    {
        ImGui::Text("N/A");
    }

    ImGui::Unindent();
    ImGui::Spacing();

    // 显示内存信息
    ImGui::Text("Memory:");
    ImGui::Indent();
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp != nullptr)
    {
        char line[256];
        long total_mem = 0, free_mem = 0;
        while (fgets(line, sizeof(line), fp))
        {
            if (strncmp(line, "MemTotal:", 9) == 0)
            {
                strtok(line, " ");
                char *total = strtok(nullptr, " ");
                if (total != nullptr)
                    total_mem = strtol(total, nullptr, 10);
            }
            else if (strncmp(line, "MemFree:", 8) == 0)
            {
                strtok(line, " ");
                char *free = strtok(nullptr, " ");
                if (free != nullptr)
                    free_mem = strtol(free, nullptr, 10);
            }
        }
        fclose(fp);

        float memory_usage = 100.0f * (total_mem - free_mem) / total_mem;
        ImGui::Text(" %ld MB Total, %ld MB Free, %.2f%% Used", total_mem / 1024, free_mem / 1024, memory_usage);
        ImGui::ProgressBar(memory_usage / 100.0f);
    }
    else
    {
        ImGui::Text("Failed to open /proc/meminfo");
    }
    ImGui::Unindent();

    // 显示磁盘信息
    ImGui::Text("Disk:");
    ImGui::Indent();
    FILE *mounts = setmntent("/proc/mounts", "r");
    if (mounts != nullptr)
    {
        struct mntent *ent;
        while ((ent = getmntent(mounts)) != nullptr)
        {
            if (strcmp(ent->mnt_type, "ext4") != 0 && strcmp(ent->mnt_type, "xfs") != 0)
                continue;

            ImGui::Text(" %s (%s)", ent->mnt_dir, ent->mnt_fsname);

            struct statvfs vfs;
            if (statvfs(ent->mnt_dir, &vfs) == -1)
            {
                ImGui::Text(" Failed to get disk usage: %s", strerror(errno));
            }
            else
            {
                float used = (vfs.f_blocks - vfs.f_bfree) * (float)vfs.f_frsize / 1024 / 1024;
                float total = vfs.f_blocks * (float)vfs.f_frsize / 1024 / 1024;
                float usage = 100.0f * used / total;
                ImGui::Text(" %.1f GB Used / %.1f GB Total (%.2f%%)", used / 1024, total / 1024, usage);
                ImGui::ProgressBar(usage / 100.0f);
            }
        }
        endmntent(mounts);
    }
    else
    {
        ImGui::Text("Failed to open /proc/mounts");
    }
    ImGui::Unindent();

    ImGui::End();
}

int main()
{
    // 初始化日志模块
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    // 创建 GLFW 窗口和 OpenGL 上下文
    if (!glfwInit())
    {
        spdlog::error("Failed to glfwInit");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Process Manager", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        spdlog::error("Failed to glfwCreateWindow");
        return 1;
    }
    glfwMakeContextCurrent(window);

    // 初始化 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // 添加默认字体
    ImFontAtlas* atlas = io.Fonts;
    ImFontConfig config;
    config.PixelSnapH = true; 
    io.Fonts->AddFontDefault(&config);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 检查是否使用了 imgui-SFML.h 头文件
#if defined(IMGUISFML_VERSION)
    spdlog::warn("imgui-SFML.h is used.");
#endif

    // 循环处理事件和绘制界面
    int selected_process_index = -1;
    while (!glfwWindowShouldClose(window))
    {
        // 处理事件
        glfwPollEvents();

        // 开始新帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 显示进程列表
        auto processes = GetProcessList();
        RenderProcessList(processes, selected_process_index);

        // 显示进程控制面板
        RenderProcessControl(processes, selected_process_index);

        // 显示系统信息
        RenderSystemInfo();

        // 渲染窗口
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        // 等待一段时间，降低系统占用
        usleep(5000);
    }

    // 清理资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
