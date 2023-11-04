# CMakeLists.txt for Lithium
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium
# Description: Lithium - Open Astrophotography Terminal
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.13)

enable_language(C CXX)

if(COMMAND cmake_policy)
		cmake_policy(SET CMP0003 NEW)
		if(POLICY CMP0043)
			cmake_policy(SET CMP0043 NEW)
		endif()
endif()

# root directory of the project
set(Lithium_PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(lithium_src_dir ${Lithium_PROJECT_ROOT_DIR}/src)
set(lithium_module_dir ${Lithium_PROJECT_ROOT_DIR}/src/modules)

add_custom_target(CmakeAdditionalFiles
	SOURCES
	${lithium_src_dir}/../cmake_modules/compiler_options.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

# compiler options
include(cmake_modules/compiler_options.cmake)

include(GNUInstallDirs)
include(FeatureSummary)
include(CheckSymbolExists)
include(CheckIncludeFileCXX)

# Clang Format support
if(UNIX OR APPLE)
    set(FORMAT_CODE OFF CACHE BOOL "Enable Clang Format")

    if(FORMAT_CODE MATCHES ON)
        FILE(GLOB_RECURSE ALL_SOURCE_FILES *.c *.cpp *.h)

        FOREACH(SOURCE_FILE ${ALL_SOURCE_FILES})
            STRING(FIND ${SOURCE_FILE} ${CMAKE_SOURCE_DIR} DIR_FOUND)

            if(NOT ${DIR_FOUND} EQUAL 0)
                LIST(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
            endif()
        ENDFOREACH()

        FIND_PROGRAM(CLANGFORMAT_EXE NAMES clang-format-5.0)

        if(CLANGFORMAT_EXE)
            ADD_CUSTOM_TARGET(clang-format COMMAND ${CLANGFORMAT_EXE} -style=file -i ${ALL_SOURCE_FILES})
        endif()
    endif()
endif()

# ####################################  Lithium version  ################################################
set(CMAKE_LITHIUM_VERSION_MAJOR 1)
set(CMAKE_LITHIUM_VERSION_MINOR 0)
set(CMAKE_LITHIUM_VERSION_RELEASE 0)

set(LITHIUM_SOVERSION ${CMAKE_LITHIUM_VERSION_MAJOR})
set(CMAKE_LITHIUM_VERSION_STRING "${CMAKE_LITHIUM_VERSION_MAJOR}.${CMAKE_LITHIUM_VERSION_MINOR}.${CMAKE_LITHIUM_VERSION_RELEASE}")
set(LITHIUM_VERSION ${CMAKE_LITHIUM_VERSION_MAJOR}.${CMAKE_LITHIUM_VERSION_MINOR}.${CMAKE_LITHIUM_VERSION_RELEASE})

if(APPLE)
    set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup")
endif(APPLE)

#################################################################################
#
# General defines for compatibility across different platforms
if(UNIX AND NOT APPLE)
	if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Lithium install path" FORCE)
	endif()
endif()

set(USE_FOLDERS TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(ENABLE_ASYNC "Enable Async Server Mode" ON)
if(ENABLE_ASYNC)
    set(ENABLE_ASYNC_FLAG "1")
endif()

option(ENABLE_NATIVE_SERVER "Enable to use INDI native server" OFF)
if(ENABLE_NATIVE_SERVER)
    set(ENABLE_NATIVE_SERVER_FLAG "1")
endif()

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#################################################################################

if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/Program Files/LithiumServer")
elseif(LINUX)
    set(CMAKE_INSTALL_PREFIX "/usr/lithium")
endif()

set(api_module
	${lithium_src_dir}/client/astap.cpp
	${lithium_src_dir}/client/astrometry.cpp
    ${lithium_src_dir}/client/phd2client.cpp
    ${lithium_module_dir}/client/indiclient.cpp
    ${lithium_src_dir}/client/hydrogen/hydrogencamera.cpp
)

set(config_module
    ${lithium_module_dir}/config/configor.cpp
)

set(device_module
    ${lithium_module_dir}/device/device_manager.cpp
    ${lithium_module_dir}/device/indidevice_manager.cpp
    ${lithium_module_dir}/device/device_utils.cpp
    ${lithium_module_dir}/device/indi_device.cpp
    ${lithium_module_dir}/device/hydrogen_device.cpp
)

set(image_module
	${lithium_src_dir}/image/image.cpp
	${lithium_src_dir}/image/draw.cpp
)

set(io_module
    ${lithium_module_dir}/io/compress.cpp
    ${lithium_module_dir}/io/file.cpp
    ${lithium_module_dir}/io/io.cpp
)

set(module_module
    ${lithium_module_dir}/module/modloader.cpp
    ${lithium_module_dir}/plugin/plugin.cpp
    ${lithium_module_dir}/plugin/exe_plugin.cpp
    ${lithium_module_dir}/plugin/script_plugin.cpp
    ${lithium_module_dir}/plugin/plugin_manager.cpp
    ${lithium_module_dir}/module/compiler.cpp
)

set(web_module
    ${lithium_module_dir}/web/downloader.cpp
    ${lithium_module_dir}/web/httpclient.cpp
    ${lithium_module_dir}/web/httplite.cpp
    ${lithium_module_dir}/web/utils.cpp
    ${lithium_module_dir}/web/time.cpp
)

set(server_module
    ${lithium_module_dir}/server/commander.hpp
    ${lithium_module_dir}/server/message_bus.hpp
    ${lithium_module_dir}/server/message_queue.hpp
    ${lithium_module_dir}/server/serialize.cpp

    ${lithium_src_dir}/websocket/WebSocketServer.cpp
    ${lithium_src_dir}/websocket/WsDeviceComponent.cpp
    ${lithium_src_dir}/websocket/WsProcessComponent.cpp
    ${lithium_src_dir}/websocket/WsScriptComponent.cpp
    ${lithium_src_dir}/websocket/WsTaskComponent.cpp

    ${lithium_src_dir}/websocket/device/WsDeviceHub.cpp
    ${lithium_src_dir}/websocket/device/WsDeviceInstance.cpp
    ${lithium_src_dir}/websocket/device/WsDeviceServer.cpp

    ${lithium_src_dir}/websocket/plugin/WsPluginHub.cpp
    ${lithium_src_dir}/websocket/plugin/WsPluginInstance.cpp
    ${lithium_src_dir}/websocket/plugin/WsPluginServer.cpp
)

set(script_module
    ${lithium_module_dir}/script/script_manager.cpp
)

set(task_module
    ${lithium_module_dir}/task/task_manager.cpp
    ${lithium_module_dir}/task/task_stack.cpp
    ${lithium_module_dir}/task/task_generator.cpp
)

set(thread_module
    ${lithium_module_dir}/thread/thread.cpp
    ${lithium_module_dir}/thread/thread_pool.hpp
)

set(system_module
    ${lithium_module_dir}/system/system.cpp
    ${lithium_module_dir}/system/crash.cpp
    ${lithium_module_dir}/system/process.cpp
    ${lithium_module_dir}/system/pid.cpp
    ${lithium_module_dir}/system/pidw.cpp
    # ${lithium_module_dir}/system/filew.cpp
    ${lithium_module_dir}/system/dirw.cpp
)

set(utils_module
    ${lithium_module_dir}/utils/time.cpp
    ${lithium_module_dir}/utils/string.cpp
    ${lithium_module_dir}/utils/switch.cpp
)

set(Lithium_module
    ${lithium_src_dir}/App.cpp
	${lithium_src_dir}/AppComponent.hpp
	${lithium_src_dir}/ErrorHandler.cpp
    ${lithium_src_dir}/LithiumApp.cpp
)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message("-- Found OpenSSL ${OPENSSL_VERSION}: ${OPENSSL_LIBRARIES}")
else()
    message("-- OpenSSL Not Found")
endif()

find_package(CFITSIO REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(LIBZIP REQUIRED)

#################################################################################
# i18n
include(cmake_modules/Gettext_helpers.cmake)
configure_gettext(
    DOMAIN "lithium"
    TARGET_NAME "lithium-gettext"
    SOURCES "${api_module}" "${config_module}" "${io_module}" "${module_module}" "${web_module}" "${device_module}" "${thread_module}" "${task_module}" "${server_module}" "${script_module}" "${system_module}" "${Lithium_module}"
    POTFILE_DESTINATION "locale"
    XGETTEXT_ARGS 
        "--keyword=_" "--keyword=N_" "--keyword=P_:1,2"
        "--package-name=${PROJECT_NAME}" "--package-version=${PROJECT_VERSION}"
        "--copyright-holder=Max Qian" "--msgid-bugs-address=astro_air@126.com"
    LANGUAGES "en_US.UTF-8")

find_package(Intl REQUIRED)

#################################################################################
# Main

project(Lithium)

include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${lithium_src_dir})
include_directories(${lithium_module_dir})
include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-swagger)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-websocket)

# TODO : 更好的构建系统，不需要这样引用
include_directories(${lithium_src_dir}/)
include_directories(${lithium_src_dir}/core)
include_directories(${lithium_src_dir}/core/base)
include_directories(${lithium_src_dir}/core/property)

add_subdirectory(libs/)

# 所有的工具组件
add_subdirectory(tools)

# 内置的模组
include_directories(modules)
add_subdirectory(modules/hydrogen_client)

# 构建Lithium内核
add_subdirectory(${lithium_src_dir}/core)
if(NOT WIN32)
add_subdirectory(${lithium_module_dir}/deviceloader)
endif()

add_executable(lithium_server ${api_module} ${config_module} ${io_module} ${module_module} ${web_module} ${device_module} ${thread_module} ${task_module} ${server_module} ${script_module} ${system_module} ${utils_module} ${Lithium_module})

target_link_directories(lithium_server PUBLIC ${CMAKE_BINARY_DIR}/libs)

target_link_libraries(lithium_server oatpp oatpp-websocket oatpp-swagger oatpp-openssl oatpp-zlib)
target_link_libraries(lithium_server loguru)
target_link_libraries(lithium_server libzippp)

target_link_libraries(lithium_server lithiumcorestatic)
target_link_libraries(lithium_server hydrogencore)
target_link_libraries(lithium_server hydrogenclientstatic)

CHECK_INCLUDE_FILE(format HAS_STD_FORMAT)
if(NOT HAS_STD_FORMAT)
    find_package(fmt REQUIRED)
    target_link_libraries(lithium_server fmt::fmt)
endif()

target_link_libraries(lithium_server ${CFITSIO_LIBRARIES})
target_link_libraries(lithium_server OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(lithium_server ${ZLIB_LIBRARIES})
target_link_libraries(lithium_server libzip::zip)
target_link_libraries(lithium_server pugixml-static)
target_link_libraries(lithium_server sqlite3)
target_link_libraries(lithium_server cpp_httplib)
target_link_libraries(lithium_server backward)

target_link_libraries(lithium_server asan)

target_link_libraries(lithium_server ${Libintl_LIBRARY})
target_include_directories(lithium_server PUBLIC ${Libintl_INCLUDE_DIRS})
add_dependencies(lithium_server lithium-gettext)

if(WIN32)
    target_link_libraries(lithium_server pdh iphlpapi winmm crypt32 wsock32 ws2_32)
    find_package(dlfcn-win32 REQUIRED)
    target_link_libraries(lithium_server dlfcn-win32::dl)
elseif(UNIX OR LINUX OR APPLE)
    target_link_libraries(lithium_server dl)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Set output name for Lithium executable
set_target_properties(
    lithium_server
    PROPERTIES
    OUTPUT_NAME lithium_server
)
