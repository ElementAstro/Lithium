# CMakeLists.txt for OpenAPT
# This project is licensed under the terms of the GPL3 license.
#
# Project Name: OpenAPT
# Description: Automated image processing and analysis tool for astrophotography
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

include_directories(${CMAKE_SOURCE_DIR}/libs/crow)
include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${CMAKE_SOURCE_DIR}/src/)

set(plugins_SRC
		${openapt_src_dir}/src/plugins/file.cpp
		${openapt_src_dir}/src/plugins/file.hpp

		${openapt_src_dir}/src/plugins/process.cpp
		${openapt_src_dir}/src/plugins/process.hpp

		${openapt_src_dir}/src/plugins/compress.cpp
		${openapt_src_dir}/src/plugins/compress.hpp

		${openapt_src_dir}/src/plugins/time.cpp
		${openapt_src_dir}/src/plugins/time.hpp

		${openapt_src_dir}/src/plugins/io.cpp
		${openapt_src_dir}/src/plugins/io.hpp

		${openapt_src_dir}/src/plugins/xmlreader.cpp
		${openapt_src_dir}/src/plugins/xmlreader.hpp

		${openapt_src_dir}/src/plugins/terminal.cpp
		${openapt_src_dir}/src/plugins/terminal.hpp

		${openapt_src_dir}/src/plugins/sha256.hpp
		${openapt_src_dir}/src/plugins/glob.hpp
)

set(image_SRC
		${openapt_src_dir}/src/image/image.cpp
		${openapt_src_dir}/src/image/image.hpp

		${openapt_src_dir}/src/image/draw.cpp
)

set(asx_SRC
        ${openapt_src_dir}/src/asx/search.cpp
		${openapt_src_dir}/src/asx/search.hpp
)

set(api_SRC
		${openapt_src_dir}/src/api/astap.cpp
		${openapt_src_dir}/src/api/astap.hpp

		${openapt_src_dir}/src/api/astrometry.cpp
		${openapt_src_dir}/src/api/astrometry.hpp

		${openapt_src_dir}/src/api/indiclient.cpp
		${openapt_src_dir}/src/api/indiclient.hpp
)

set(webapi_SRC
		${openapt_src_dir}/src/webapi/http_api.cpp
		${openapt_src_dir}/src/webapi/http_api.hpp
)

set(indi_SRC
		${openapt_src_dir}/src/indi/indicamera.cpp
		${openapt_src_dir}/src/indi/indicamera.hpp

		${openapt_src_dir}/src/indi/indifocuser.cpp
		${openapt_src_dir}/src/indi/indifocuser.hpp

		${openapt_src_dir}/src/indi/inditelescope.cpp
		${openapt_src_dir}/src/indi/inditelescope.hpp

		${openapt_src_dir}/src/indi/indifilterwheel.cpp
		${openapt_src_dir}/src/indi/indifilterwheel.hpp
)

set(indiserver_SRC
        ${openapt_src_dir}/src/indiserver/indidvrinfo.cpp
        ${openapt_src_dir}/src/indiserver/indidvrinfo.hpp

        ${openapt_src_dir}/src/indiserver/indimsg.cpp
        ${openapt_src_dir}/src/indiserver/indimsg.hpp

        ${openapt_src_dir}/src/indiserver/indiserver.cpp
        ${openapt_src_dir}/src/indiserver/indiserver.hpp

        ${openapt_src_dir}/src/indiserver/indismsg.cpp
        ${openapt_src_dir}/src/indiserver/indismsg.hpp
)

set(openapt_SRC
		${openapt_src_dir}/src/module/modloader.cpp
		${openapt_src_dir}/src/module/modloader.hpp

		${openapt_src_dir}/src/module/pythonloader.cpp
		${openapt_src_dir}/src/module/pythonloader.hpp

		${openapt_src_dir}/src/module/compiler.cpp
		${openapt_src_dir}/src/module/compiler.hpp

		${openapt_src_dir}/src/module/sheller.cpp
		${openapt_src_dir}/src/module/sheller.hpp

		${openapt_src_dir}/src/package/downloader.cpp
		${openapt_src_dir}/src/package/downloader.hpp

		${openapt_src_dir}/src/package/packageloader.cpp
		${openapt_src_dir}/src/package/packageloader.hpp

		${openapt_src_dir}/src/task/camera_task.cpp
		${openapt_src_dir}/src/task/camera_task.hpp
		${openapt_src_dir}/src/task/runner.cpp
		${openapt_src_dir}/src/task/runner.hpp

		${openapt_src_dir}/src/device/basic_device.cpp
		${openapt_src_dir}/src/device/basic_device.hpp

		${openapt_src_dir}/src/device/manager.cpp
		${openapt_src_dir}/src/device/manager.hpp

		${openapt_src_dir}/src/config/logger.cpp
		${openapt_src_dir}/src/config/logger.hpp
		${openapt_src_dir}/src/config/achievement.cpp
		${openapt_src_dir}/src/config/achievement.hpp
		${openapt_src_dir}/src/config/achievement_list.cpp
		${openapt_src_dir}/src/config/achievement_list.hpp

		${openapt_src_dir}/src/openapt.cpp
		${openapt_src_dir}/src/openapt.hpp
)

if(WIN32)
    # Linux specific options
    add_executable(openapt ${plugins_SRC} ${image_SRC} ${api_SRC} ${webapi_SRC} ${indi_SRC} ${asx_SRC} ${openapt_SRC})

    # Use consistent formatting for target_precompile_headers
    target_precompile_headers(openapt PUBLIC ${openapt_src_dir}/src/openapt.hpp)

    # Use consistent formatting for find_package
    find_package(ZLIB REQUIRED)

    # Find and link CFITSIO libraries
    find_package(CFITSIO REQUIRED)
    target_link_libraries(openapt ${CFITSIO_LIBRARIES})

    find_package(dlfcn-win32 REQUIRED)
    target_link_libraries(openapt dlfcn-win32::dl)

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
        # Linux specific options
        add_executable(openapt ${plugins_SRC} ${image_SRC} ${api_SRC} ${webapi_SRC} ${indi_SRC} ${asx_SRC} ${openapt_SRC})

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
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()
endif()

