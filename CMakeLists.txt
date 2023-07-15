# CMakeLists.txt for Lithium
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: Lithium
# Description: Open Astrophotography Terminal
# Author: Max Qian
# License: GPL3

# Use consistent formatting and naming conventions throughout the code
cmake_minimum_required(VERSION 3.16)

# Add comments to explain what different sections of code do
enable_language(C CXX)

if(COMMAND cmake_policy)
		cmake_policy(SET CMP0003 NEW)
		if(POLICY CMP0043)
			cmake_policy(SET CMP0043 NEW)
		endif()
endif()

# root directory of the project
set(Lithium_PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(lithium_src_dir ${Lithium_PROJECT_ROOT_DIR})

add_custom_target(CmakeAdditionalFiles
	SOURCES
	${lithium_src_dir}/cmake_modules/compiler_options.cmake)

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

#################################################################################
#
# main project
# this should appear after setting the architecture
project(Lithium)

include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-swagger)
include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp-websocket)

add_subdirectory(libs/)
add_subdirectory(libs/pugixml)

#add_subdirectory(src/indiserver)

set(api_SRC
	${lithium_src_dir}/src/api/astap.cpp
	${lithium_src_dir}/src/api/astap.hpp

	${lithium_src_dir}/src/api/astrometry.cpp
	${lithium_src_dir}/src/api/astrometry.hpp

    ${lithium_src_dir}/src/api/phd2client.cpp
	${lithium_src_dir}/src/api/phd2client.hpp
)

set(config_SRC
    ${lithium_src_dir}/src/config/configor.cpp
    ${lithium_src_dir}/src/config/configor.hpp

    ${lithium_src_dir}/src/config/achievement.cpp
    ${lithium_src_dir}/src/config/achievement.hpp

    ${lithium_src_dir}/src/config/achievement_list.cpp
    ${lithium_src_dir}/src/config/achievement_list.hpp
)

set(database_SRC
    ${lithium_src_dir}/src/database/database.cpp
    ${lithium_src_dir}/src/database/database.hpp
)

set(debug_SRC
    ${lithium_src_dir}/src/debug/terminal.cpp
    ${lithium_src_dir}/src/debug/terminal.hpp
)

set(device_SRC
    ${lithium_src_dir}/src/device/device.cpp
    ${lithium_src_dir}/src/device/device.hpp

    #${lithium_src_dir}/src/device/device_manager.cpp
    #${lithium_src_dir}/src/device/device_manager.hpp
)

set(driver_indi_SRC
    ${lithium_src_dir}/src/api/indiclient.cpp
    ${lithium_src_dir}/src/api/indiclient.hpp

    ${lithium_src_dir}/src/driver/indi/indi_exception.hpp

	${lithium_src_dir}/src/driver/indi/indicamera.cpp
	${lithium_src_dir}/src/driver/indi/indicamera.hpp

	${lithium_src_dir}/src/driver/indi/indifocuser.cpp
	${lithium_src_dir}/src/driver/indi/indifocuser.hpp

	${lithium_src_dir}/src/driver/indi/inditelescope.cpp
	${lithium_src_dir}/src/driver/indi/inditelescope.hpp

	${lithium_src_dir}/src/driver/indi/indifilterwheel.cpp
	${lithium_src_dir}/src/driver/indi/indifilterwheel.hpp
)

set(image_SRC
	${lithium_src_dir}/src/image/image.cpp
	${lithium_src_dir}/src/image/image.hpp

	${lithium_src_dir}/src/image/draw.cpp
)

set(io_SRC
    ${lithium_src_dir}/src/io/compress.cpp
    ${lithium_src_dir}/src/io/compress.hpp

    ${lithium_src_dir}/src/io/file.cpp
    ${lithium_src_dir}/src/io/file.hpp

    ${lithium_src_dir}/src/io/glob.hpp

    ${lithium_src_dir}/src/io/io.cpp
    ${lithium_src_dir}/src/io/io.hpp

    ${lithium_src_dir}/src/io/system.cpp
    ${lithium_src_dir}/src/io/system.hpp
)

set(launcher_SRC
    ${lithium_src_dir}/src/launcher/crash.cpp
    ${lithium_src_dir}/src/launcher/crash.hpp
)

set(logger_SRC
    ${lithium_src_dir}/src/logger/aptlogger.cpp
    ${lithium_src_dir}/src/logger/aptlogger.hpp

    ${lithium_src_dir}/src/logger/log_manager.cpp
    ${lithium_src_dir}/src/logger/log_manager.hpp
)

set(module_SRC
    ${lithium_src_dir}/src/module/modloader.cpp
    ${lithium_src_dir}/src/module/modloader.hpp

    #${lithium_src_dir}/src/module/compiler.cpp
    #${lithium_src_dir}/src/module/compiler.hpp
)

set(network_SRC
    ${lithium_src_dir}/src/network/downloader.cpp
    ${lithium_src_dir}/src/network/downloader.hpp

    ${lithium_src_dir}/src/network/httpclient.cpp
    ${lithium_src_dir}/src/network/httpclient.hpp

    ${lithium_src_dir}/src/network/socketclient.cpp
    ${lithium_src_dir}/src/network/socketclient.hpp

    ${lithium_src_dir}/src/network/time.cpp
    ${lithium_src_dir}/src/network/time.hpp
)

set(property_SRC
    ${lithium_src_dir}/src/property/base64.cpp
    ${lithium_src_dir}/src/property/base64.hpp

    ${lithium_src_dir}/src/property/imessage.cpp
    ${lithium_src_dir}/src/property/imessage.hpp

    ${lithium_src_dir}/src/property/sha256.hpp

    ${lithium_src_dir}/src/property/uuid.cpp
    ${lithium_src_dir}/src/property/uuid.hpp

    #${lithium_src_dir}/src/property/xml.cpp
    #${lithium_src_dir}/src/property/xml.hpp
)

set(server_SRC
    ${lithium_src_dir}/src/modules/server/commander.cpp
    ${lithium_src_dir}/src/modules/server/commander.hpp
)

set(task_SRC
    #${lithium_src_dir}/src/task/runner.cpp
	#${lithium_src_dir}/src/task/runner.hpp
)

set(thread_SRC
    ${lithium_src_dir}/src/thread/thread.cpp
    ${lithium_src_dir}/src/thread/thread.hpp

    ${lithium_src_dir}/src/thread/threadpool.cpp
    ${lithium_src_dir}/src/thread/threadpool.hpp
)

set(system_module
    ${lithium_src_dir}/src/modules/system/system.cpp
    ${lithium_src_dir}/src/modules/system/system.hpp
)

set(Lithium_SRC
${lithium_src_dir}/src/websocket/WebSocketServer.cpp

	${lithium_src_dir}/src/App.cpp
	${lithium_src_dir}/src/AppComponent.hpp


)

if(WIN32)
    #add_executable(lithium_server ${api_SRC} ${config_SRC} ${database_SRC} ${debug_SRC}
    #${device_SRC} ${image_SRC} ${io_SRC} ${logger_SRC} ${module_SRC} 
    #${network_SRC} ${property_SRC} ${server_SRC} ${task_SRC} ${thread_SRC} ${Lithium_SRC})

    add_executable(lithium_server ${server_SRC} ${system_module} ${Lithium_SRC})

    target_link_libraries(lithium_server pdh iphlpapi)

    target_link_libraries(lithium_server oatpp-websocket oatpp-swagger oatpp-openssl oatpp)

    find_package(CFITSIO REQUIRED)
    target_link_libraries(lithium_server ${CFITSIO_LIBRARIES})

    find_package(dlfcn-win32 REQUIRED)
    target_link_libraries(lithium_server dlfcn-win32::dl)

    find_package(OpenSSL REQUIRED)
    if(OpenSSL_FOUND)
        target_link_libraries(lithium_server ${OPENSSL_LIBRARIES})
        message("-- Using OpenSSL ${OPENSSL_VERSION}")
    else()
        message("-- OpenSSL Not Found")
    endif()

    find_package(ZLIB REQUIRED)
    target_link_libraries(lithium_server ${ZLIB_LIBRARIES})
    target_link_libraries(lithium_server pugixml-static)

    find_package(SQLite3 REQUIRED)
    target_link_libraries(lithium_server sqlite3)

    # Set output name for Lithium executable
    set_target_properties(
        lithium_server
        PROPERTIES
        OUTPUT_NAME lithium_server
    )
elseif(UNIX)
    if(APPLE)
        # macOS specific options
        add_executable(Lithium ${plugins_SRC} ${image_SRC} ${api_SRC} ${webapi_SRC} ${indi_SRC} ${asx_SRC} ${Lithium_SRC})
        target_link_libraries(Lithium "-framework Foundation" "-framework Cocoa")

        # Use consistent formatting for target_precompile_headers
        target_precompile_headers(Lithium PUBLIC ${lithium_src_dir}/src/Lithium.hpp)

        # Use consistent formatting for find_package
        find_package(ZLIB REQUIRED)

        # Find and link CFITSIO libraries
        find_package(CFITSIO REQUIRED)
        target_link_libraries(Lithium ${CFITSIO_LIBRARIES})

        # Find and link INDI libraries
        find_package(INDI 2.0.0 REQUIRED)
        include_directories(${INDI_INCLUDE_DIR})
        target_link_libraries(Lithium ${INDI_CLIENT_LIBRARIES})

        find_package(CURL REQUIRED)
        target_link_libraries(Lithium CURL::libcurl)

        # Find and link OpenSSL libraries
        find_package(OpenSSL REQUIRED)
        if(OpenSSL_FOUND)
            target_link_libraries(Lithium ${OPENSSL_LIBRARIES})
            message("-- Using OpenSSL ${OPENSSL_VERSION}")
        else()
            message("-- OpenSSL Not Found")
        endif()

        find_package(GSL REQUIRED)
        target_link_libraries(Lithium ${GSL_LIBRARIES})

        find_package(OpenCV REQUIRED)
        include_directories(${OpenCV_INCLUDE_DIRS})
        target_link_libraries(Lithium ${OpenCV_LIBS})

        # Link ZLIB and pugixml-static libraries
        target_link_libraries(Lithium ${ZLIB_LIBRARIES})
        target_link_libraries(Lithium pugixml-static)

        # Set output name for Lithium executable
        set_target_properties(
            Lithium
            PROPERTIES
            OUTPUT_NAME Lithium
        )
    elseif(UNIX AND NOT APPLE)
        add_executable(Lithium ${plugins_SRC} ${image_SRC} ${api_SRC} ${webapi_SRC} ${indi_SRC} ${asx_SRC} ${Lithium_SRC})

        target_precompile_headers(Lithium PUBLIC ${lithium_src_dir}/src/Lithium.hpp)

        find_package(CFITSIO REQUIRED)
        target_link_libraries(Lithium ${CFITSIO_LIBRARIES})

        find_package(INDI 2.0.0 REQUIRED)
        include_directories(${INDI_INCLUDE_DIR})
        target_link_libraries(Lithium ${INDI_CLIENT_LIBRARIES})

        find_package(OpenSSL REQUIRED)
        if(OpenSSL_FOUND)
            target_link_libraries(Lithium ${OPENSSL_LIBRARIES})
            message("-- Using OpenSSL ${OPENSSL_VERSION}")
        else()
            message("-- OpenSSL Not Found")
        endif()

        find_package(ZLIB REQUIRED)
        target_link_libraries(Lithium ${ZLIB_LIBRARIES})
        target_link_libraries(Lithium pugixml-static)

        # Set output name for Lithium executable
        set_target_properties(
            Lithium
            PROPERTIES
            OUTPUT_NAME Lithium
        )
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()
endif()

