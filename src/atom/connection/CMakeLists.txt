cmake_minimum_required(VERSION 3.10)
project(cpp-ipc)

option(LIBIPC_BUILD_SHARED_LIBS "Build shared libraries (DLLs)."                        OFF)
option(LIBIPC_USE_STATIC_CRT    "Set to ON to build with static CRT on Windows (/MT)."  OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    if (LIBIPC_USE_STATIC_CRT)
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
            string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach()
    else()
        foreach(CompilerFlag ${CompilerFlags})
            string(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
            string(REPLACE "/MTd" "/MDd" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach()
    endif()
endif()

set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/libs)
set(LIBIPC_PROJECT_DIR     ${PROJECT_SOURCE_DIR})

# Unicode Support
add_definitions(-DUNICODE -D_UNICODE)

aux_source_directory(${LIBIPC_PROJECT_DIR}/libipc SRC_FILES)
aux_source_directory(${LIBIPC_PROJECT_DIR}/libipc/sync SRC_FILES)
aux_source_directory(${LIBIPC_PROJECT_DIR}/libipc/platform SRC_FILES)

file(GLOB HEAD_FILES
  ${LIBIPC_PROJECT_DIR}/include/libipc/*.h
  ${LIBIPC_PROJECT_DIR}/libipc/*.h
  ${LIBIPC_PROJECT_DIR}/libipc/*.inc
  ${LIBIPC_PROJECT_DIR}/libipc/circ/*.h
  ${LIBIPC_PROJECT_DIR}/libipc/memory/*.h
  ${LIBIPC_PROJECT_DIR}/libipc/platform/*.h
  ${LIBIPC_PROJECT_DIR}/libipc/utility/*.h)

if (LIBIPC_BUILD_SHARED_LIBS)
  add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${HEAD_FILES})
  target_compile_definitions(${PROJECT_NAME}
    INTERFACE
      LIBIPC_LIBRARY_SHARED_USING__
    PRIVATE
      LIBIPC_LIBRARY_SHARED_BUILDING__)
else()
  add_library(${PROJECT_NAME} STATIC ${SRC_FILES} ${HEAD_FILES})
endif()

# set output directory
set_target_properties(${PROJECT_NAME}
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/libs"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/libs"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# set version
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    VERSION 1.2.0
    SOVERSION 3)

target_include_directories(${PROJECT_NAME}
  PUBLIC  ${LIBIPC_PROJECT_DIR}/include
  PRIVATE ${LIBIPC_PROJECT_DIR}/src
          $<$<BOOL:UNIX>:${LIBIPC_PROJECT_DIR}/libipc/platform/linux>)

if(NOT MSVC)
  target_link_libraries(${PROJECT_NAME} PUBLIC
    $<$<NOT:$<STREQUAL:${CMAKE_SYSTEM_NAME},QNX>>:pthread>
    $<$<NOT:$<OR:$<STREQUAL:${CMAKE_SYSTEM_NAME},Windows>,$<STREQUAL:${CMAKE_SYSTEM_NAME},QNX>>>:rt>)
endif()

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION libs
  ARCHIVE DESTINATION libs)
