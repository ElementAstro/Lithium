#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <ctime>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <nlohmann/json.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

using json = nlohmann::json;

struct LogEntry {
    std::string timestamp;
    std::string level;
    std::string message;
};

class Launcher {
public:
    Launcher() : 
        logger(spdlog::stdout_color_mt("console")),
        launch_command(), 
        title(),
        window_width(800.0f),
        window_height(600.0f),
        font_size(16.0f),
        show_trace_logs(true),
        show_debug_logs(true),
        show_info_logs(true),
        show_warn_logs(true),
        show_error_logs(true),
        log_level_filter("INFO"),
        log_file_path(),
        log_entries(),
        error_message(), 
        process_running(false), 
        process_thread() {}

    ~Launcher() {
        if (process_running) {
            logger->info("Stopping process...");
            stop_process();
        }
    }

    void load_config(const std::string& config_file_path) {
        std::ifstream ifs(config_file_path);
        if (!ifs.good()) {
            logger->error("Failed to open config file: {}", config_file_path);
            return;
        }

        try {
            json config_json;
            ifs >> config_json;
            launch_command = config_json["launch_command"].get<std::string>();
            title = config_json["title"].get<std::string>();
            window_width = config_json["window_width"].get<float>();
            window_height = config_json["window_height"].get<float>();
            font_size = config_json["font_size"].get<float>();
            log_file_path = config_json["log_file_path"].get<std::string>();
            logger->info("Loaded launch command: {}", launch_command);
        } catch (const std::exception& e) {
            logger->error("Failed to parse config file: {}", e.what());
        }
    }

    void show_gui() {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height));
        ImGui::Begin(title.c_str(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Start")) {
            start_process();
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            stop_process();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Log")) {
            clear_log();
        }

        ImGui::Text("Launch Command:");
        char buf[256];
        strcpy(buf, launch_command.c_str());
        ImGui::InputText("##command_input", buf, sizeof(buf));
        launch_command = buf; // 将更改后的字符串赋值回原始的 std::string

        ImGui::Text("Log Level Filter:");
        if (ImGui::BeginCombo("##level_filter_combo", log_level_filter.c_str())) {
            for (const auto& level : {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"}) {
                bool is_selected = (log_level_filter == level);
                if (ImGui::Selectable(level, is_selected)) {
                    log_level_filter = level;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, -(font_size / 2)));

        for (const auto& entry : log_entries) {
            if (!should_log(entry.level)) {
                continue;
            }
            ImGui::TextColored(
                get_log_level_color(entry.level), 
                "[%s] %s - %s", 
                entry.timestamp.c_str(), 
                entry.level.c_str(), 
                entry.message.c_str()
            );
        }

        ImGui::PopStyleVar();

        if (process_running) {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Process is running...");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Process is not running.");
        }

        ImGui::End();

        if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", error_message.c_str());
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void start_process() {
        if (process_running) {
            logger->warn("Cannot start process, it is already running.");
            return;
        }

        logger->info("Starting process...");
        process_running = true;

        process_thread = std::thread([this]() {
            std::array<char, 256> buffer{};
            std::string current_output;
            FILE* pipe = nullptr;
            while (process_running) {
                pipe = popen(launch_command.c_str(), "r");
                if (!pipe) {
                    logger->error("Failed to open pipe to launch command: {}", launch_command);
                    error_message = "Failed to start process.";
                    process_running = false;
                    break;
                }

                while (fgets(buffer.data(), buffer.size(), pipe)) {
                    current_output += buffer.data();
                    if (current_output.back() == '\n') {
                        std::string level = extract_log_level(current_output);
                        if (should_log(level)) {
                            log_entries.emplace_back(LogEntry{get_current_timestamp(), level, current_output});
                        }
                        current_output.clear();
                    }
                }

                pclose(pipe);
                pipe = nullptr;

                if (process_running) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }

            if (pipe != nullptr) {
                pclose(pipe);
            }
        });
    }

    void stop_process() {
        if (!process_running) {
            logger->warn("Cannot stop process, it is not running.");
            return;
        }

        logger->info("Stopping process...");
        process_running = false;

        if (process_thread.joinable()) {
            process_thread.join();
        }

        // 如果进程还在运行，则终止它
        system(fmt::format("pkill -f {}", launch_command).c_str());
    }


    void clear_log() {
        log_entries.clear();
    }

    ImVec4 get_log_level_color(const std::string& level) {
        if (level == "TRACE") {
            return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        } else if (level == "DEBUG") {
            return ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
        } else if (level == "INFO") {
            return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        } else if (level == "WARN") {
            return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        } else if (level == "ERROR") {
            return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        } else {
            return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    std::string get_current_timestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t        (now);
        char buffer[20];
        std::strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        return buffer;
    }

    std::string extract_log_level(const std::string& log_line) {
        std::size_t level_end = log_line.find(']');
        if (level_end == std::string::npos) {
            return "";
        }
        std::size_t level_start = log_line.rfind('[', level_end - 1);
        if (level_start == std::string::npos) {
            return "";
        }
        return log_line.substr(level_start + 1, level_end - level_start - 1);
    }

    bool should_log(const std::string& level) {
        if (level == "TRACE") {
            return show_trace_logs;
        } else if (level == "DEBUG") {
            return show_debug_logs;
        } else if (level == "INFO") {
            return show_info_logs;
        } else if (level == "WARN") {
            return show_warn_logs;
        } else if (level == "ERROR") {
            return show_error_logs;
        } else {
            return true;
        }
    }

private:
    std::shared_ptr<spdlog::logger> logger;
    std::string launch_command;
    std::string title;
    float window_width;
    float window_height;
    float font_size;
    bool show_trace_logs;
    bool show_debug_logs;
    bool show_info_logs;
    bool show_warn_logs;
    bool show_error_logs;
    std::string log_level_filter;
    std::string log_file_path;
    std::vector<LogEntry> log_entries;
    std::string error_message;
    bool process_running;
    std::thread process_thread;
};

int main() {
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口并创建 OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 600, "Launcher", NULL, NULL);
    glfwMakeContextCurrent(window);

    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Launcher launcher;
    launcher.load_config("config.json");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        launcher.show_gui();

        ImGui::Render();
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 清理 ImGui 相关资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // 销毁窗口并终止 GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
