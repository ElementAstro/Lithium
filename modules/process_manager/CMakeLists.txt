include_directories(${CMAKE_SOURCE_DIR}/libs/imgui)

add_executable(main process_manager.cpp 
    ${CMAKE_SOURCE_DIR}/libs/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_tables.cpp 
    ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.cpp 
    ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp)

target_link_libraries(main glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(main OpenGL::GL)

find_package(spdlog REQUIRED)
target_link_libraries(main spdlog::spdlog_header_only)