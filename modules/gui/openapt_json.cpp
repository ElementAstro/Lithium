#include <iostream>
#include <fstream>
#include <string>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "nlohmann/json.hpp"

#include "spdlog/spdlog.h"

using json = nlohmann::json;

void glfwErrorCallback(int error, const char* description) {
    spdlog::error("GLFW Error: {0}", description);
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

GLFWwindow* InitializeGLFWWindow(const std::string& title, int width, int height) {
    // 初始化 GLFW
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        spdlog::error("Failed to initialize GLFW");
        return nullptr;
    }

    // 创建 GLFW 窗口并设置 OpenGL 版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (window == NULL) {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // 设置键盘回调
    glfwSetKeyCallback(window, glfwKeyCallback);

    return window;
}

void ShutdownGLFWWindow(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void InitializeImGui() {
    // 初始化 ImGui 上下文环境
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
}

void ShutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool InitializeImGuiGLFWRenderer(GLFWwindow* window) {
    // 初始化 ImGui GLFW 渲染器和 OpenGL 渲染器
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        spdlog::error("Failed to initialize ImGui GLFW Renderer");
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        spdlog::error("Failed to initialize ImGui OpenGL Renderer");
        return false;
    }
    return true;
}

json LoadJSONFile(const std::string& fileName) {
    // 加载文件并将其解析为 JSON 对象
    std::ifstream file(fileName);
    json data;
    if (file.is_open()) {
        file >> data;
        file.close();
    } else {
        spdlog::error("Failed to open file {0}", fileName);
    }
    return data;
}

void SaveJSONFile(const std::string& fileName, const json& data) {
    // 将 JSON 对象保存到文件中
    std::ofstream file(fileName);
    if (file.is_open()) {
        file << std::setw(4) << data << std::endl;
        file.close();
    } else {
        spdlog::error("Failed to open file {0}", fileName);
    }
}

void ShowJSONEditor(json& data) {
    // 在窗口中显示 JSON 数据
    ImGui::Begin("JSON Editor");

    // 添加 JSON 数据的功能
    static char buffer[1024];
    ImGui::InputText("New Key", buffer, sizeof(buffer));
    ImGui::SameLine();
    if (ImGui::Button("Add Key")) {
        data[buffer] = "";
    }

    // 显示 JSON 数据的所有 key 和 value
    for (json::iterator it = data.begin(); it != data.end(); ++it) {
        std::string key = it.key();
        json& value = it.value();

        if (value.is_object()) {
            if (ImGui::TreeNode(key.c_str())) {
                ShowJSONEditor(value);
                ImGui::TreePop();
            }
        } else if (value.is_array()) {
            if (ImGui::TreeNode(key.c_str())) {
                int index = 0;
                for (auto& element : value) {
                    ImGui::PushID(index++);
                    if (ImGui::TreeNode("Element")) {
                        ShowJSONEditor(element);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
        } else {
            // 将空字符串更改为可识别的字符串
            std::string valueStr = value.dump();
            std::string inputID = std::string("##") + key;
            ImGui::InputText(inputID.c_str(), &valueStr[0], valueStr.size());
            value = json::parse(valueStr);
        }
    }


    // 保存 JSON 数据的功能
    if (ImGui::Button("Save JSON")) {
        const std::string jsonFileName = "test.json";
        SaveJSONFile(jsonFileName, data);
    }

    ImGui::End();
}

int main() {
    // 初始化 GLFW 窗口和 OpenGL
    const std::string windowTitle = "JSON Editor";
    const int windowWidth = 800;
    const int windowHeight = 600;
    GLFWwindow* window = InitializeGLFWWindow(windowTitle, windowWidth, windowHeight);
    if (!window) {
        return -1;
    }

    // 初始化 ImGui
    InitializeImGui();

    // 初始化 ImGui 渲染器
    if (!InitializeImGuiGLFWRenderer(window)) {
        ShutdownImGui();
        ShutdownGLFWWindow(window);
        return -1;
    }

    // 加载 JSON 文件
    const std::string jsonFileName = "test.json";
    json jsonData = LoadJSONFile(jsonFileName);

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // 开始 ImGui 帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 显示 JSON 编辑器
        ShowJSONEditor(jsonData);

        // 渲染 ImGui
        ImGui::Render();

        // 渲染 OpenGL 图形
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 交换缓冲区
        glfwSwapBuffers(window);
    }

    // 清理资源
    ShutdownImGui();
    ShutdownGLFWWindow(window);

    return 0;
}
