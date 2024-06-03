# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Check and set C++ compiler requirements
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++20 HAS_CXX20_FLAG)
check_cxx_compiler_flag(-std=c++23 HAS_CXX23_FLAG)

if(HAS_CXX23_FLAG)
    set(CMAKE_CXX_STANDARD 23)
elseif(HAS_CXX20_FLAG)
    set(CMAKE_CXX_STANDARD 20)
else()
    message(FATAL_ERROR "C++20 standard is required!")
endif()

# Check and set compiler version requirements
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpfullversion -std=c++${CMAKE_CXX_STANDARD}
        OUTPUT_VARIABLE GCC_VERSION
    )
    string(REGEX MATCH "[0-9]+\\.[0-9]+" GCC_VERSION ${GCC_VERSION})
    if(GCC_VERSION VERSION_LESS 10.0)
        message(FATAL_ERROR "Minimum required version of g++ is 10.0")
    endif()
    message(STATUS "Using g++ version ${GCC_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --version
        OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
    )
    string(REGEX MATCH "clang version ([0-9]+\\.[0-9]+)" CLANG_VERSION ${CLANG_VERSION_OUTPUT})
    string(REGEX REPLACE "clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION ${CLANG_VERSION})
    if(CLANG_VERSION VERSION_LESS 10.0)
        message(FATAL_ERROR "Minimum required version of clang is 10.0")
    endif()
    message(STATUS "Using clang version ${CLANG_VERSION}")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.28)
        message(FATAL_ERROR "Minimum required version of MSVC is 19.28 (Visual Studio 2019 version 16.10)")
    endif()
    message(STATUS "Using MSVC version ${CMAKE_CXX_COMPILER_VERSION}")
endif()

# Set C standard
set(CMAKE_C_STANDARD 17)

# Set compiler flags for Apple platforms
if(APPLE)
    check_cxx_compiler_flag(-stdlib=libc++ HAS_LIBCXX_FLAG)
    if(HAS_LIBCXX_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

# Set build architecture for non-Apple platforms
if(NOT APPLE)
    set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "Build architecture for non-Apple platforms" FORCE)
endif()

# Additional compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# Enable Address Sanitizer (ASan) for Debug builds
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    endif()
endif()

# Enable Link Time Optimization (LTO) for Release builds
if(CMAKE_BUILD_TYPE MATCHES "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
    endif()
endif()