#include <iostream>
#include <vector>
#include <memory>
#include <spdlog/spdlog.h>
#include <imgui.h>
#include <csignal> // 包含信号处理相关的定义

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <Windows.h>
#include <TlHelp32.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cstdio>
#endif

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
#ifdef _WIN32
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        spdlog::error("Failed to CreateToolhelp32Snapshot: {}", GetLastError());
        return res;
    }
    PROCESSENTRY32 process_entry;
    ZeroMemory(&process_entry, sizeof(process_entry));
    process_entry.dwSize = sizeof(process_entry);

    if (!Process32First(snapshot, &process_entry))
    {
        spdlog::error("Failed to Process32First: {}", GetLastError());
        CloseHandle(snapshot);
        return res;
    }

    do
    {
        ProcessInfo info;
        info.name = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(process_entry.szExeFile);
        info.pid = process_entry.th32ProcessID;
        res.push_back(info);
    } while (Process32Next(snapshot, &process_entry));
    CloseHandle(snapshot);
#else
    DIR *dir = opendir("/proc");
    if (!dir)
    {
        spdlog::error("Failed to opendir /proc: {}", strerror(errno));
        return res;
    }
    struct dirent *entry;
    char path[256];
    while ((entry = readdir(dir)) != nullptr)
    {
        if (!isdigit(entry->d_name[0]))
            continue;
        int pid = atoi(entry->d_name);
        snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
        FILE *file = fopen(path, "r");
        if (!file)
            continue;
        char buf[kBufSize];
        if (fgets(buf, sizeof(buf), file) == NULL)
        {
        }
        fclose(file);
        std::string cmd(buf);
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
    closedir(dir);
#endif
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
#ifdef _WIN32
    HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (handle == nullptr)
    {
        spdlog::error("Failed to OpenProcess: {}", GetLastError());
        return false;
    }
    bool res = TerminateProcess(handle, 0);
    CloseHandle(handle);
    return res;
#else
    return kill(pid, SIGINT) == 0;
#endif
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

void RenderProcessControl(const std::vector<ProcessInfo> &processes, int selected_process_index)
{
    if (selected_process_index != -1)
    {
        const auto &process = processes[selected_process_index];
        std::string window_title = "Control Process " + std::to_string(process.pid);
        ImGui::Begin(window_title.c_str());

        // 显示进程信息和控制按钮
        ImGui::Text("ID: %d, Name: %s", process.pid, process.name.c_str());
        if (ImGui::Button("Kill"))
        {
            if (KillProcess(process.pid))
            {
                spdlog::info("Process {} terminated successfully.", process.pid);
            }
        }
        ImGui::End();
    }
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
    ImGui_ImplGlfw_InitForOpenGL(window, true);

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
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 显示进程列表
        auto processes = GetProcessList();
        RenderProcessList(processes, selected_process_index);

        // 显示进程控制面板
        RenderProcessControl(processes, selected_process_index);

        // 渲染窗口
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // 清理资源
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
