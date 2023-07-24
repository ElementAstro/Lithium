# CMakeLists.txt for Lithium
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium
# Description: Open Astrophotography Terminal
# Author: Max Qian
# License: GPL3

cmake_minimum_required(VERSION 3.16)

enable_language(C CXX)

if(COMMAND cmake_policy)
		cmake_policy(SET CMP0003 NEW)
		if(POLICY CMP0043)
			cmake_policy(SET CMP0043 NEW)
		endif()
endif()

# root directory of the project
set(Lithium_PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(lithium_src_dir ${Lithium_PROJECT_ROOT_DIR}/src/)

add_custom_target(CmakeAdditionalFiles
	SOURCES
	${lithium_src_dir}/../cmake_modules/compiler_options.cmake)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

# compiler options
include(cmake_modules/compiler_options.cmake)

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

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

add_subdirectory(libs/)
add_subdirectory(libs/pugixml)

#add_subdirectory(src/indiserver)

set(api_SRC
	${lithium_src_dir}/api/astap.cpp
	${lithium_src_dir}/api/astap.hpp

	${lithium_src_dir}/api/astrometry.cpp
	${lithium_src_dir}/api/astrometry.hpp

    ${lithium_src_dir}/api/phd2client.cpp
	${lithium_src_dir}/api/phd2client.hpp
)

set(config_SRC
    ${lithium_src_dir}/modules/config/configor.cpp
    ${lithium_src_dir}/modules/config/configor.hpp

    ${lithium_src_dir}/modules/config/achievement.cpp
    ${lithium_src_dir}/modules/config/achievement.hpp

    ${lithium_src_dir}/modules/config/achievement_list.cpp
    ${lithium_src_dir}/modules/config/achievement_list.hpp
)

set(database_SRC
    ${lithium_src_dir}/database/database.cpp
    ${lithium_src_dir}/database/database.hpp
)

set(debug_SRC
    ${lithium_src_dir}/debug/terminal.cpp
    ${lithium_src_dir}/debug/terminal.hpp
)

set(device_SRC
    ${lithium_src_dir}/modules/device/device.cpp
    ${lithium_src_dir}/modules/device/device.hpp

    ${lithium_src_dir}/modules/device/device_manager.cpp
    ${lithium_src_dir}/modules/device/device_manager.hpp
)

set(driver_indi_SRC
    ${lithium_src_dir}/api/indiclient.cpp
    ${lithium_src_dir}/api/indiclient.hpp

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

    # ${lithium_src_dir}/modules/module/compiler.cpp
    # ${lithium_src_dir}/modules/module/compiler.hpp
)

set(network_SRC
    ${lithium_src_dir}/network/downloader.cpp
    ${lithium_src_dir}/network/downloader.hpp

    ${lithium_src_dir}/network/httpclient.cpp
    ${lithium_src_dir}/network/httpclient.hpp

    ${lithium_src_dir}/network/socketclient.cpp
    ${lithium_src_dir}/network/socketclient.hpp

    ${lithium_src_dir}/network/time.cpp
    ${lithium_src_dir}/network/time.hpp
)

set(property_SRC
    ${lithium_src_dir}/modules/property/base64.cpp
    ${lithium_src_dir}/modules/property/base64.hpp

    ${lithium_src_dir}/modules/property/imessage.cpp
    ${lithium_src_dir}/modules/property/imessage.hpp

    ${lithium_src_dir}/modules/property/sha256.hpp

    ${lithium_src_dir}/modules/property/uuid.cpp
    ${lithium_src_dir}/modules/property/uuid.hpp

    #${lithium_src_dir}/property/xml.cpp
    #${lithium_src_dir}/property/xml.hpp
)

set(server_SRC
    ${lithium_src_dir}/modules/server/commander.cpp
    ${lithium_src_dir}/modules/server/commander.hpp

    ${lithium_src_dir}/modules/server/login_manager.cpp
    ${lithium_src_dir}/modules/server/login_manager.hpp

    ${lithium_src_dir}/websocket/WebSocketServer.cpp
    ${lithium_src_dir}/websocket/WebSocketServer.hpp
    ${lithium_src_dir}/websocket/WsDeviceComponent.cpp
    ${lithium_src_dir}/websocket/WsProcessComponent.cpp
    ${lithium_src_dir}/websocket/WsTaskComponent.cpp
)

set(task_SRC
    ${lithium_src_dir}/modules/task/task_manager.cpp
    ${lithium_src_dir}/modules/task/task_manager.hpp

    ${lithium_src_dir}/modules/task/task.hpp
)

set(thread_SRC
    ${lithium_src_dir}/modules/thread/thread.cpp
    ${lithium_src_dir}/modules/thread/thread.hpp

    ${lithium_src_dir}/modules/thread/threadpool.cpp
    ${lithium_src_dir}/modules/thread/threadpool.hpp
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

if(WIN32)
    #add_executable(lithium_server ${api_SRC} ${config_SRC} ${database_SRC} ${debug_SRC}
     #${image_SRC} ${logger_SRC} ${module_SRC} 
    #${network_SRC}  ${server_SRC} ${task_SRC} ${thread_SRC} ${Lithium_SRC})

    add_executable(lithium_server ${config_SRC} ${io_SRC} ${module_SRC} ${property_SRC} ${device_SRC} ${thread_SRC} ${task_SRC} ${server_SRC} ${system_module} ${Lithium_SRC})

    target_link_libraries(lithium_server pdh iphlpapi winmm)

    target_link_libraries(lithium_server oatpp-websocket oatpp-swagger oatpp-openssl oatpp-sqlite oatpp-zlib oatpp)

    target_link_libraries(lithium_server loguru)

    find_package(dlfcn-win32 REQUIRED)
    target_link_libraries(lithium_server dlfcn-win32::dl)

    target_link_libraries(lithium_server ${CFITSIO_LIBRARIES})
    target_link_libraries(lithium_server ${OPENSSL_LIBRARIES})
    target_link_libraries(lithium_server ${ZLIB_LIBRARIES})
    target_link_libraries(lithium_server pugixml-static)
    target_link_libraries(lithium_server sqlite3)

    # Set output name for Lithium executable
    set_target_properties(
        lithium_server
        PROPERTIES
        OUTPUT_NAME lithium_server
    )
elseif(UNIX OR LINUX OR APPLE)
    
    add_executable(lithium_server ${config_SRC} ${io_SRC} ${property_SRC} ${device_SRC} ${thread_SRC} ${server_SRC} ${system_module} ${Lithium_SRC})

    target_link_libraries(lithium_server oatpp-websocket oatpp-swagger oatpp-openssl oatpp)

    target_link_libraries(lithium_server loguru)

    find_package(INDI 2.0.0 REQUIRED)
    include_directories(${INDI_INCLUDE_DIR})
    target_link_libraries(lithium_server ${INDI_CLIENT_LIBRARIES})

    target_link_libraries(lithium_server ${CFITSIO_LIBRARIES})
    target_link_libraries(lithium_server ${OPENSSL_LIBRARIES})
    target_link_libraries(lithium_server ${ZLIB_LIBRARIES})
    target_link_libraries(lithium_server pugixml-static)

    set_target_properties(
        lithium_server
        PROPERTIES
        OUTPUT_NAME lithium_server
    )
    
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

