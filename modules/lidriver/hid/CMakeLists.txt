cmake_minimum_required(VERSION 3.13)
project(hid C)

add_library(${PROJECT_NAME} OBJECT "")

# Headers
list(APPEND ${PROJECT_NAME}_HEADERS
    hidapi.h
)

list(APPEND ${PROJECT_NAME}_PRIVATE_HEADERS
)

# Sources
if(APPLE)
    list(APPEND ${PROJECT_NAME}_SOURCES hid_mac.c)
elseif(WIN32)
    list(APPEND ${PROJECT_NAME}_SOURCES hid_win.c)
else()
    list(APPEND ${PROJECT_NAME}_SOURCES hid_libusb.c)
endif()

# Setup Target
target_sources(${PROJECT_NAME}
    PUBLIC
    ${${PROJECT_NAME}_HEADERS}
    PRIVATE
    ${${PROJECT_NAME}_SOURCES}
    ${${PROJECT_NAME}_PRIVATE_HEADERS}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC .
)

