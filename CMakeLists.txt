cmake_minimum_required(VERSION 3.16)

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

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

# compiler options
include(cmake_modules/compiler_options.cmake)

#################################################################################
#
# general defines
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

add_subdirectory(modules)

include_directories(${CMAKE_SOURCE_DIR}/libs/crow)
include_directories(${CMAKE_SOURCE_DIR}/libs/)
include_directories(${CMAKE_SOURCE_DIR}/src/)

set(plugins_SRC
    ${openapt_src_dir}/src/plugins/thread.cpp
    ${openapt_src_dir}/src/plugins/thread.hpp

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

    ${openapt_src_dir}/src/plugins/crash.cpp
    ${openapt_src_dir}/src/plugins/crash.hpp
)

set(image_SRC
    ${openapt_src_dir}/src/image/image.cpp
)

set(api_SRC
    ${openapt_src_dir}/src/api/astap.cpp
    ${openapt_src_dir}/src/api/astap.hpp

    ${openapt_src_dir}/src/api/indiclient.cpp
    ${openapt_src_dir}/src/api/indiclient.hpp
)

set(openapt_SRC
    ${openapt_src_dir}/src/modloader.cpp
    ${openapt_src_dir}/src/modloader.hpp

    ${openapt_src_dir}/src/packageloader.cpp
    ${openapt_src_dir}/src/packageloader.hpp

    ${openapt_src_dir}/src/task/define.hpp
    ${openapt_src_dir}/src/task/runner.cpp
    ${openapt_src_dir}/src/task/runner.hpp

    ${openapt_src_dir}/src/device/basic_device.cpp
    ${openapt_src_dir}/src/device/basic_device.hpp

    ${openapt_src_dir}/src/device/manager.cpp
    ${openapt_src_dir}/src/device/manager.hpp

    ${openapt_src_dir}/src/config/configor.cpp
    ${openapt_src_dir}/src/config/configor.hpp

    ${openapt_src_dir}/src/openapt.cpp
    ${openapt_src_dir}/src/openapt.hpp
)

if(UNIX)
  # should be linux there
  add_executable(
   openapt
   ${plugins_SRC}
   ${image_SRC}
   ${api_SRC}
   ${openapt_SRC}
   )

  target_precompile_headers(openapt PUBLIC ${openapt_src_dir}/src/openapt.hpp)

  find_package(ZLIB REQUIRED)

  # 查找 Python 库
  find_package(Python REQUIRED COMPONENTS Development)

  find_package(CFITSIO REQUIRED)

  target_link_libraries(openapt ${CFITSIO_LIBRARIES})

  find_package(INDI 2.0.0 REQUIRED)

  include_directories(${INDI_INCLUDE_DIR})

  target_link_libraries(openapt ${INDI_CLIENT_LIBRARIES})

  # 判断 Python 是否存在
  if(Python_FOUND)
      message(STATUS "Python version: ${Python_VERSION}")
  else()
      message(SEND_ERROR "Python not found.")
  endif()

  # 添加 Python 头文件和库路径
  include_directories(${Python_INCLUDE_DIRS})
  link_directories(${Python_LIBRARY_DIRS})

  # 编译器标志
  add_definitions(${Python_DEFINITIONS})

  # 链接 Python 库
  target_link_libraries(openapt ${Python_LIBRARIES})

  find_package(OpenSSL REQUIRED)
  if(OpenSSL_FOUND)
    target_link_libraries(openapt ${OPENSSL_LIBRARIES})
    message("-- Using OpenSSL ${OPENSSL_VERSION}")
  else()
    message("-- OpenSSL Not Found")
  endif()

  find_package(spdlog REQUIRED)
  target_link_libraries(openapt spdlog::spdlog_header_only)

  target_link_libraries(openapt ${ZLIB_LIBRARIES})

  target_link_libraries(openapt pugixml-static)

  set_target_properties(
    openapt
    PROPERTIES
      OUTPUT_NAME openapt
    )

else()
  message(FATAL_ERROR "Unsupported platform")
endif()
