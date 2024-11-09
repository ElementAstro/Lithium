# FindGMock.cmake
# This module finds the Google Mock library

# Set the name of the required package
find_package(GTest REQUIRED)

# Try to locate the GMock library
find_path(GMOCK_INCLUDE_DIR NAMES gmock/gmock.h HINTS ${GTEST_INCLUDE_DIR} ${GTEST_ROOT} PATHS /usr/local/include /usr/include)

find_library(GMOCK_LIBRARY NAMES gmock HINTS ${GTEST_LIBRARY_DIR} ${GTEST_ROOT} PATHS /usr/local/lib /usr/lib)

find_library(GMOCK_MAIN_LIBRARY NAMES gmock_main HINTS ${GTEST_LIBRARY_DIR} ${GTEST_ROOT} PATHS /usr/local/lib /usr/lib)

# Check if found
if (GMOCK_INCLUDE_DIR AND GMOCK_LIBRARY)
  set(GMOCK_FOUND TRUE)
else()
  set(GMOCK_FOUND FALSE)
endif()

# Provide the results of the search, either found or not found
if (GMOCK_FOUND)
  message(STATUS "Found Google Mock: ${GMOCK_LIBRARY}")
  message(STATUS "Includes: ${GMOCK_INCLUDE_DIR}")

  # Set the variables necessary for using GMock
  set(GMOCK_INCLUDE_DIRS ${GMOCK_INCLUDE_DIR} PARENT_SCOPE)
  set(GMOCK_LIBRARIES ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY} PARENT_SCOPE)
else()
  message(WARNING "Google Mock not found, please install it or set the paths correctly.")
endif()

# Provide the version information if necessary
# set(GMOCK_VERSION "x.y.z") # Optionally set the version
