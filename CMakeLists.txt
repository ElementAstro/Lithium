# CMakeLists.txt for OpenAPT
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: OpenAPT
# Description: OpenAPT Astrophotography Terminal
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
set(OPENAPT_PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})
set(openapt_src_dir ${OPENAPT_PROJECT_ROOT_DIR})

add_custom_target(CmakeAdditionalFiles
	SOURCES
	${openapt_src_dir}/cmake_modules/compiler_options.cmake)

# Add a GPL3 declaration
# Set the license information in the project here

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

# compiler options
include(cmake_modules/compiler_options.cmake)

#################################################################################
#
# General defines for compatibility across different platforms
if(UNIX AND NOT APPLE)
	if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "openapt install path" FORCE)
	endif()
endif()

set(USE_FOLDERS TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#################################################################################
#
# main project
# this should appear after setting the architecture
project(openapt)

add_subdirectory(libs/)
add_subdirectory(libs/pugixml)

#add_subdirectory(src/indiserver)

include_directories(${CMAKE_SOURCE_DIR}/libs/oatpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${CMAKE_SOURCE_DIR}/src/)

set(api_SRC
	${openapt_src_dir}/src/api/astap.cpp
	${openapt_src_dir}/src/api/astap.hpp

	${openapt_src_dir}/src/api/astrometry.cpp
	${openapt_src_dir}/src/api/astrometry.hpp

    ${openapt_src_dir}/src/api/phd2client.cpp
	${openapt_src_dir}/src/api/phd2client.hpp
)

set(config_SRC
    ${openapt_src_dir}/src/config/configor.cpp
    ${openapt_src_dir}/src/config/configor.hpp

    ${openapt_src_dir}/src/config/achievement.cpp
    ${openapt_src_dir}/src/config/achievement.hpp

    ${openapt_src_dir}/src/config/achievement_list.cpp
    ${openapt_src_dir}/src/config/achievement_list.hpp
)

set(database_SRC
    ${openapt_src_dir}/src/database/database.cpp
    ${openapt_src_dir}/src/database/database.hpp
)

set(debug_SRC
    ${openapt_src_dir}/src/debug/terminal.cpp
    ${openapt_src_dir}/src/debug/terminal.hpp
)

set(device_SRC
    ${openapt_src_dir}/src/device/device.cpp
    ${openapt_src_dir}/src/device/device.hpp

    #${openapt_src_dir}/src/device/device_manager.cpp
    #${openapt_src_dir}/src/device/device_manager.hpp
)

set(driver_indi_SRC
    ${openapt_src_dir}/src/api/indiclient.cpp
    ${openapt_src_dir}/src/api/indiclient.hpp

    ${openapt_src_dir}/src/driver/indi/indi_exception.hpp

	${openapt_src_dir}/src/driver/indi/indicamera.cpp
	${openapt_src_dir}/src/driver/indi/indicamera.hpp

	${openapt_src_dir}/src/driver/indi/indifocuser.cpp
	${openapt_src_dir}/src/driver/indi/indifocuser.hpp

	${openapt_src_dir}/src/driver/indi/inditelescope.cpp
	${openapt_src_dir}/src/driver/indi/inditelescope.hpp

	${openapt_src_dir}/src/driver/indi/indifilterwheel.cpp
	${openapt_src_dir}/src/driver/indi/indifilterwheel.hpp
)

set(image_SRC
	${openapt_src_dir}/src/image/image.cpp
	${openapt_src_dir}/src/image/image.hpp

	${openapt_src_dir}/src/image/draw.cpp
)

set(io_SRC
    ${openapt_src_dir}/src/io/compress.cpp
    ${openapt_src_dir}/src/io/compress.hpp

    ${openapt_src_dir}/src/io/file.cpp
    ${openapt_src_dir}/src/io/file.hpp

    ${openapt_src_dir}/src/io/glob.hpp

    ${openapt_src_dir}/src/io/io.cpp
    ${openapt_src_dir}/src/io/io.hpp

    ${openapt_src_dir}/src/io/system.cpp
    ${openapt_src_dir}/src/io/system.hpp
)

set(launcher_SRC
    ${openapt_src_dir}/src/launcher/crash.cpp
    ${openapt_src_dir}/src/launcher/crash.hpp
)

set(logger_SRC
    ${openapt_src_dir}/src/logger/aptlogger.cpp
    ${openapt_src_dir}/src/logger/aptlogger.hpp

    ${openapt_src_dir}/src/logger/log_manager.cpp
    ${openapt_src_dir}/src/logger/log_manager.hpp
)

set(module_SRC
    ${openapt_src_dir}/src/module/modloader.cpp
    ${openapt_src_dir}/src/module/modloader.hpp

    #${openapt_src_dir}/src/module/compiler.cpp
    #${openapt_src_dir}/src/module/compiler.hpp
)

set(network_SRC
    ${openapt_src_dir}/src/network/downloader.cpp
    ${openapt_src_dir}/src/network/downloader.hpp

    ${openapt_src_dir}/src/network/httpclient.cpp
    ${openapt_src_dir}/src/network/httpclient.hpp

    ${openapt_src_dir}/src/network/socketclient.cpp
    ${openapt_src_dir}/src/network/socketclient.hpp

    ${openapt_src_dir}/src/network/time.cpp
    ${openapt_src_dir}/src/network/time.hpp
)

set(property_SRC
    ${openapt_src_dir}/src/property/base64.cpp
    ${openapt_src_dir}/src/property/base64.hpp

    ${openapt_src_dir}/src/property/imessage.cpp
    ${openapt_src_dir}/src/property/imessage.hpp

    ${openapt_src_dir}/src/property/sha256.hpp

    ${openapt_src_dir}/src/property/uuid.cpp
    ${openapt_src_dir}/src/property/uuid.hpp

    #${openapt_src_dir}/src/property/xml.cpp
    #${openapt_src_dir}/src/property/xml.hpp
)

set(server_SRC
    ${openapt_src_dir}/src/server/commander.cpp
    ${openapt_src_dir}/src/server/commander.hpp

    ${openapt_src_dir}/src/server/hserver.cpp
    ${openapt_src_dir}/src/server/hserver.hpp

    ${openapt_src_dir}/src/server/server.cpp
    ${openapt_src_dir}/src/server/server.hpp

    ${openapt_src_dir}/src/server/wsserver.cpp
    ${openapt_src_dir}/src/server/wsserver.hpp
)

set(task_SRC
    #${openapt_src_dir}/src/task/runner.cpp
	#${openapt_src_dir}/src/task/runner.hpp
)

set(thread_SRC
    ${openapt_src_dir}/src/thread/thread.cpp
    ${openapt_src_dir}/src/thread/thread.hpp

    ${openapt_src_dir}/src/thread/threadpool.cpp
    ${openapt_src_dir}/src/thread/threadpool.hpp
)

set(openapt_SRC
	${openapt_src_dir}/src/openapt_server.cpp
	${openapt_src_dir}/src/openapt_server.hpp
)

if(WIN32)
    #add_executable(openapt_server ${api_SRC} ${config_SRC} ${database_SRC} ${debug_SRC}
    #${device_SRC} ${image_SRC} ${io_SRC} ${logger_SRC} ${module_SRC} 
    #${network_SRC} ${property_SRC} ${server_SRC} ${task_SRC} ${thread_SRC} ${openapt_SRC})

    add_executable(openapt_server ${openapt_SRC})

    find_package(CFITSIO REQUIRED)
    target_link_libraries(openapt_server ${CFITSIO_LIBRARIES})

    find_package(dlfcn-win32 REQUIRED)
    target_link_libraries(openapt_server dlfcn-win32::dl)

    find_package(OpenSSL REQUIRED)
    if(OpenSSL_FOUND)
        target_link_libraries(openapt_server ${OPENSSL_LIBRARIES})
        message("-- Using OpenSSL ${OPENSSL_VERSION}")
    else()
        message("-- OpenSSL Not Found")
    endif()

    find_package(ZLIB REQUIRED)
    target_link_libraries(openapt_server ${ZLIB_LIBRARIES})
    target_link_libraries(openapt_server pugixml-static)

    find_package(SQLite3 REQUIRED)
    target_link_libraries(openapt_server sqlite3)

    # Set output name for openapt executable
    set_target_properties(
        openapt_server
        PROPERTIES
        OUTPUT_NAME openapt_server
    )
elseif(UNIX)
    if(MINGW)
        find_package(dlfcn-win32 REQUIRED)
        target_link_libraries(openapt dlfcn-win32::dl)
    endif()
    if(APPLE)
        # macOS specific options
        add_executable(openapt ${plugins_SRC} ${image_SRC} ${api_SRC} ${webapi_SRC} ${indi_SRC} ${asx_SRC} ${openapt_SRC})
        target_link_libraries(openapt "-framework Foundation" "-framework Cocoa")

        # Use consistent formatting for target_precompile_headers
        target_precompile_headers(openapt PUBLIC ${openapt_src_dir}/src/openapt.hpp)

        # Use consistent formatting for find_package
        find_package(ZLIB REQUIRED)

        # Find and link CFITSIO libraries
        find_package(CFITSIO REQUIRED)
        target_link_libraries(openapt ${CFITSIO_LIBRARIES})

        # Find and link INDI libraries
        find_package(INDI 2.0.0 REQUIRED)
        include_directories(${INDI_INCLUDE_DIR})
        target_link_libraries(openapt ${INDI_CLIENT_LIBRARIES})

        find_package(CURL REQUIRED)
        target_link_libraries(openapt CURL::libcurl)

        # Find and link OpenSSL libraries
        find_package(OpenSSL REQUIRED)
        if(OpenSSL_FOUND)
            target_link_libraries(openapt ${OPENSSL_LIBRARIES})
            message("-- Using OpenSSL ${OPENSSL_VERSION}")
        else()
            message("-- OpenSSL Not Found")
        endif()

        find_package(GSL REQUIRED)
        target_link_libraries(openapt ${GSL_LIBRARIES})

        find_package(OpenCV REQUIRED)
        include_directories(${OpenCV_INCLUDE_DIRS})
        target_link_libraries(openapt ${OpenCV_LIBS})

        # Link ZLIB and pugixml-static libraries
        target_link_libraries(openapt ${ZLIB_LIBRARIES})
        target_link_libraries(openapt pugixml-static)

        # Set output name for openapt executable
        set_target_properties(
            openapt
            PROPERTIES
            OUTPUT_NAME openapt
        )
    elseif(UNIX AND NOT APPLE)
        add_executable(openapt ${plugins_SRC} ${image_SRC} ${api_SRC} ${webapi_SRC} ${indi_SRC} ${asx_SRC} ${openapt_SRC})

        target_precompile_headers(openapt PUBLIC ${openapt_src_dir}/src/openapt.hpp)

        find_package(CFITSIO REQUIRED)
        target_link_libraries(openapt ${CFITSIO_LIBRARIES})

        find_package(INDI 2.0.0 REQUIRED)
        include_directories(${INDI_INCLUDE_DIR})
        target_link_libraries(openapt ${INDI_CLIENT_LIBRARIES})

        find_package(OpenSSL REQUIRED)
        if(OpenSSL_FOUND)
            target_link_libraries(openapt ${OPENSSL_LIBRARIES})
            message("-- Using OpenSSL ${OPENSSL_VERSION}")
        else()
            message("-- OpenSSL Not Found")
        endif()

        find_package(ZLIB REQUIRED)
        target_link_libraries(openapt ${ZLIB_LIBRARIES})
        target_link_libraries(openapt pugixml-static)

        # Set output name for openapt executable
        set_target_properties(
            openapt
            PROPERTIES
            OUTPUT_NAME openapt
        )
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()
endif()

