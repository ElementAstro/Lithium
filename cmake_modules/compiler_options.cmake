# should be done *before* declaring project.
if (APPLE)
  if (APPLE32)
    # set build architecture for OSX
    set(CMAKE_OSX_ARCHITECTURES i386 CACHE STRING "build architecture for OSX" FORCE)
  endif ()
  set(CMAKE_MACOSX_RPATH TRUE)
elseif (WIN32)
  # set Platform Toolset for Windows builds
  if (NOT CMAKE_GENERATOR_TOOLSET MATCHES "v120_xp")
    message(FATAL_ERROR "Visual Studio 2013 or later with v120_xp toolset is required.")
  endif ()
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

# set default build type to Release
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

# check and set C++ compiler flags
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++20 HAS_CXX20_FLAG)
check_cxx_compiler_flag(-std=c++17 HAS_CXX17_FLAG)
check_cxx_compiler_flag(-std=c++11 HAS_CXX11_FLAG)
check_cxx_compiler_flag(-std=c++0x HAS_CXX0X_FLAG)

if (HAS_CXX20_FLAG)
  set(CMAKE_CXX_STANDARD 20)
elseif (HAS_CXX17_FLAG)
  set(CMAKE_CXX_STANDARD 17)
elseif (HAS_CXX11_FLAG)
  set(CMAKE_CXX_STANDARD 11)
elseif (HAS_CXX0X_FLAG)
  set(CMAKE_CXX_STANDARD 0x)
endif ()

# disable deprecated declarations warning
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

if (APPLE)
  check_cxx_compiler_flag(-stdlib=libc++ HAS_LIBCXX11_FLAG)

  if (HAS_LIBCXX11_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif ()
endif ()

# set build architecture for non-Apple platforms
if (NOT APPLE)
  set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "build architecture for non-Apple platforms" FORCE)
endif ()
