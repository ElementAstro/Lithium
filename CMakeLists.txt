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

# #######################################  Paths  ###################################################
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/indi/")
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")

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

#################################################################################
#
# main project
# this should appear after setting the architecture
project(Lithium)

include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${lithium_src_dir})
include_directories(${lithium_src_dir}/modules)
include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-swagger)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-websocket)

# TODO : 更好的构建系统，不需要这样引用
include_directories(${lithium_src_dir}/)
include_directories(${lithium_src_dir}/core)
include_directories(${lithium_src_dir}/core/base)
include_directories(${lithium_src_dir}/core/dsp)
include_directories(${lithium_src_dir}/core/io)
include_directories(${lithium_src_dir}/core/property)
include_directories(${lithium_src_dir}/core/timer)
include_directories(${lithium_src_dir}/core/thread)

add_subdirectory(libs/)

add_subdirectory(tools)

include_directories(modules)

add_subdirectory(modules/HydrogenClient)
if(NOT WIN32)
add_subdirectory(modules/HydrogenServer)
endif()
add_subdirectory(modules/libridge)

add_subdirectory(drivers)

add_subdirectory(${lithium_src_dir}/core)
if(NOT WIN32)
add_subdirectory(${lithium_src_dir}/modules/deviceloader)
endif()

set(api_SRC
	${lithium_src_dir}/client/astap.cpp
	${lithium_src_dir}/client/astap.hpp

	${lithium_src_dir}/client/astrometry.cpp
	${lithium_src_dir}/client/astrometry.hpp

    ${lithium_src_dir}/client/phd2client.cpp
	${lithium_src_dir}/client/phd2client.hpp
)

set(config_SRC
    ${lithium_src_dir}/modules/config/configor.cpp
    ${lithium_src_dir}/modules/config/configor.hpp

    ${lithium_src_dir}/modules/config/achievement.cpp
    ${lithium_src_dir}/modules/config/achievement.hpp

    ${lithium_src_dir}/modules/config/achievement_list.cpp
    ${lithium_src_dir}/modules/config/achievement_list.hpp
)

set(device_SRC
    ${lithium_src_dir}/modules/device/device_manager.cpp
    ${lithium_src_dir}/modules/device/device_manager.hpp

    ${lithium_src_dir}/modules/device/indidevice_manager.cpp
    ${lithium_src_dir}/modules/device/indidevice_manager.hpp

    ${lithium_src_dir}/modules/device/device_utils.cpp
    ${lithium_src_dir}/modules/device/device_utils.hpp

    ${lithium_src_dir}/modules/device/indi_device.cpp
    ${lithium_src_dir}/modules/device/indi_device.hpp

    ${lithium_src_dir}/modules/device/hydrogen_device.cpp
    ${lithium_src_dir}/modules/device/hydrogen_device.hpp
)

set(driver_indi_SRC
    ${lithium_src_dir}/client/indiclient.cpp
    ${lithium_src_dir}/client/indiclient.hpp

    ${lithium_src_dir}/driver/indi/indi_exception.hpp

	${lithium_src_dir}/driver/indi/indicamera.cpp
	${lithium_src_dir}/driver/indi/indicamera.hpp

	${lithium_src_dir}/driver/indi/indifocuser.cpp
	${lithium_src_dir}/driver/indi/indifocuser.hpp

	${lithium_src_dir}/driver/indi/inditelescope.cpp
	${lithium_src_dir}/driver/indi/inditelescope.hpp

	${lithium_src_dir}/driver/indi/indifilterwheel.cpp
	${lithium_src_dir}/driver/indi/indifilterwheel.hpp
)

set(image_SRC
	${lithium_src_dir}/image/image.cpp
	${lithium_src_dir}/image/image.hpp

	${lithium_src_dir}/image/draw.cpp
)

set(io_SRC
    ${lithium_src_dir}/modules/io/compress.cpp
    ${lithium_src_dir}/modules/io/compress.hpp

    ${lithium_src_dir}/modules/io/file.cpp
    ${lithium_src_dir}/modules/io/file.hpp

    ${lithium_src_dir}/modules/io/glob.hpp

    ${lithium_src_dir}/modules/io/io.cpp
    ${lithium_src_dir}/modules/io/io.hpp
)

set(launcher_SRC
    ${lithium_src_dir}/launcher/crash.cpp
    ${lithium_src_dir}/launcher/crash.hpp
)

set(logger_SRC
    ${lithium_src_dir}/logger/aptlogger.cpp
    ${lithium_src_dir}/logger/aptlogger.hpp

    ${lithium_src_dir}/logger/log_manager.cpp
    ${lithium_src_dir}/logger/log_manager.hpp
)

set(module_SRC
    ${lithium_src_dir}/modules/module/modloader.cpp
    ${lithium_src_dir}/modules/module/modloader.hpp

    ${lithium_src_dir}/modules/plugin/plugin.cpp
    ${lithium_src_dir}/modules/plugin/plugin.hpp

    ${lithium_src_dir}/modules/plugin/exe_plugin.cpp
    ${lithium_src_dir}/modules/plugin/exe_plugin.hpp

    ${lithium_src_dir}/modules/plugin/script_plugin.cpp
    ${lithium_src_dir}/modules/plugin/script_plugin.hpp

    ${lithium_src_dir}/modules/plugin/plugin_manager.cpp
    ${lithium_src_dir}/modules/plugin/plugin_manager.hpp

    ${lithium_src_dir}/modules/module/compiler.cpp
    ${lithium_src_dir}/modules/module/compiler.hpp
)

set(network_SRC
    ${lithium_src_dir}/modules/web/downloader.cpp
    ${lithium_src_dir}/modules/web/downloader.hpp

    ${lithium_src_dir}/modules/web/httpclient.cpp
    ${lithium_src_dir}/modules/web/httpclient.hpp

    ${lithium_src_dir}/modules/web/utils.cpp
    ${lithium_src_dir}/modules/web/utils.hpp

    # ${lithium_src_dir}/network/time.cpp
    # ${lithium_src_dir}/network/time.hpp
)

set(server_SRC
    ${lithium_src_dir}/modules/server/commander.cpp
    ${lithium_src_dir}/modules/server/commander.hpp

    ${lithium_src_dir}/websocket/WebSocketServer.cpp
    ${lithium_src_dir}/websocket/WebSocketServer.hpp
    ${lithium_src_dir}/websocket/WsDeviceComponent.cpp
    ${lithium_src_dir}/websocket/WsProcessComponent.cpp
    ${lithium_src_dir}/websocket/WsScriptComponent.cpp
    ${lithium_src_dir}/websocket/WsTaskComponent.cpp

    ${lithium_src_dir}/websocket/device/WsDeviceHub.cpp
    ${lithium_src_dir}/websocket/device/WsDeviceHub.hpp

    ${lithium_src_dir}/websocket/device/WsDeviceInstance.cpp
    ${lithium_src_dir}/websocket/device/WsDeviceInstance.hpp

    ${lithium_src_dir}/websocket/device/WsDeviceServer.cpp
    ${lithium_src_dir}/websocket/device/WsDeviceServer.hpp

    ${lithium_src_dir}/websocket/plugin/WsPluginHub.cpp
    ${lithium_src_dir}/websocket/plugin/WsPluginHub.hpp

    ${lithium_src_dir}/websocket/plugin/WsPluginInstance.cpp
    ${lithium_src_dir}/websocket/plugin/WsPluginInstance.hpp

    ${lithium_src_dir}/websocket/plugin/WsPluginServer.cpp
    ${lithium_src_dir}/websocket/plugin/WsPluginServer.hpp
)

set(script_SRC
    ${lithium_src_dir}/modules/script/script_manager.cpp
    ${lithium_src_dir}/modules/script/script_manager.hpp
)

set(task_SRC
    ${lithium_src_dir}/modules/task/task_manager.cpp
    ${lithium_src_dir}/modules/task/task_manager.hpp

    ${lithium_src_dir}/modules/task/task_stack.cpp
    ${lithium_src_dir}/modules/task/task_stack.hpp

    ${lithium_src_dir}/modules/task/task_generator.cpp
    ${lithium_src_dir}/modules/task/task_generator.hpp
)

set(thread_SRC
    ${lithium_src_dir}/modules/thread/thread.cpp
    ${lithium_src_dir}/modules/thread/thread.hpp
)

set(system_module
    ${lithium_src_dir}/modules/system/system.cpp
    ${lithium_src_dir}/modules/system/system.hpp

    ${lithium_src_dir}/modules/system/crash.cpp
    ${lithium_src_dir}/modules/system/crash.hpp

    ${lithium_src_dir}/modules/system/process.cpp
    ${lithium_src_dir}/modules/system/process.hpp
)

set(Lithium_SRC
	${lithium_src_dir}/ErrorHandler.cpp
    ${lithium_src_dir}/ErrorHandler.hpp

    ${lithium_src_dir}/App.cpp
	${lithium_src_dir}/AppComponent.hpp

    ${lithium_src_dir}/LithiumApp.cpp
	${lithium_src_dir}/LithiumApp.hpp
)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message("-- Using OpenSSL ${OPENSSL_VERSION}")
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
    SOURCES "${api_SRC}" "${config_SRC}" "${io_SRC}" "${module_SRC}" "${network_SRC}" "${device_SRC}" "${thread_SRC}" "${task_SRC}" "${server_SRC}" "${script_SRC}" "${system_module}" "${Lithium_SRC}"
    POTFILE_DESTINATION "locale"
    XGETTEXT_ARGS 
        "--keyword=_" "--keyword=N_" "--keyword=P_:1,2"
        "--package-name=${PROJECT_NAME}" "--package-version=${PROJECT_VERSION}"
        "--copyright-holder=Max Qian" "--msgid-bugs-address=astro_air@126.com"
    LANGUAGES "en_US.UTF-8" "fr_FR" "es")

find_package(Intl REQUIRED)


if(WIN32)
    #add_executable(lithium_server ${api_SRC} ${config_SRC} ${database_SRC} ${debug_SRC}
     #${image_SRC} ${logger_SRC} ${module_SRC} 
    #  ${server_SRC} ${task_SRC} ${thread_SRC} ${Lithium_SRC})

    add_executable(lithium_server ${api_SRC} ${config_SRC} ${io_SRC} ${module_SRC} ${network_SRC} ${device_SRC} ${thread_SRC} ${task_SRC} ${server_SRC} ${script_SRC} ${system_module} ${Lithium_SRC})

    target_link_directories(lithium_server PUBLIC ${CMAKE_BINARY_DIR}/libs)
    
    target_link_libraries(lithium_server pdh iphlpapi winmm crypt32 wsock32 ws2_32)

    target_link_libraries(lithium_server oatpp oatpp-websocket oatpp-swagger oatpp-openssl oatpp-zlib)

    target_link_libraries(lithium_server loguru)

    target_link_libraries(lithium_server libzippp)

    target_link_libraries(lithium_server LithiumDriver LithiumBridge)

    target_link_libraries(lithium_server hydrogencore)

    find_package(dlfcn-win32 REQUIRED)
    target_link_libraries(lithium_server dlfcn-win32::dl)

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

    find_package(libuv REQUIRED)

    target_link_libraries(lithium_server ${Libintl_LIBRARY})
    target_include_directories(lithium_server PUBLIC ${Libintl_INCLUDE_DIRS})
    add_dependencies(lithium_server lithium-gettext)

    # Set output name for Lithium executable
    set_target_properties(
        lithium_server
        PROPERTIES
        OUTPUT_NAME lithium_server
    )

    install(TARGETS lithium_server)
    install(TARGETS oatpp)
    install(TARGETS oatpp-zlib)
    install(TARGETS oatpp-openssl)
    install(TARGETS oatpp-websocket)
elseif(UNIX OR LINUX OR APPLE)
    
    add_executable(lithium_server ${config_SRC} ${io_SRC} ${module_SRC} ${network_SRC} ${device_SRC} ${thread_SRC} ${task_SRC} ${server_SRC} ${script_SRC} ${system_module} ${Lithium_SRC})

    target_link_libraries(lithium_server oatpp-websocket oatpp-swagger oatpp-openssl oatpp-zlib oatpp)

    target_link_libraries(lithium_server loguru)

    target_link_libraries(lithium_server libzippp)

    target_link_libraries(lithium_server LithiumDriver LithiumProperty LithiumBridge)

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

    target_link_libraries(lithium_server dl)

    set_target_properties(
        lithium_server
        PROPERTIES
        OUTPUT_NAME lithium_server
    )
    
else()
    message(FATAL_ERROR "Unsupported platform")
endif()
