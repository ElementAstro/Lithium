# 编译 ImGui 静态库
add_library(imgui STATIC
    ${CMAKE_SOURCE_DIR}/libs/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_tables.cpp 
    ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.cpp 
    ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/libs/imgui)

# 寻找 OpenGL 和 spdlog 库
find_package(OpenGL REQUIRED)
find_package(spdlog REQUIRED)

# 定义添加应用程序的函数
function(add_app target_name source_file)
    add_executable(${target_name} ${source_file})
    set_target_properties(${target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    target_link_libraries(${target_name} imgui glfw OpenGL::GL spdlog::spdlog_header_only)
    target_include_directories(${target_name} PRIVATE ${CMAKE_SOURCE_DIR}/libs/)
endfunction()

# 添加应用程序
add_app(openapt_process openapt_process.cpp)
add_app(openapt_launcher openapt_launcher.cpp)
add_app(openapt_json openapt_json.cpp)

