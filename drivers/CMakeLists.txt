include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/base)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/io)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/property)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/timer)
include_directories(${CMAKE_SOURCE_DIR}/modules/Hydrogen/core/thread)

add_subdirectory(hydrogen-simulator)

add_subdirectory(hydrogen-weather)

add_subdirectory(hydrogen-ioptron)

add_subdirectory(hydrogen-onstep)

add_subdirectory(hydrogen-celestron)

add_subdirectory(hydrogen-skywatcher)

add_subdirectory(hydrogen-manualfilter)

add_subdirectory(hydrogen-watchdog)

add_subdirectory(hydrogen-astap)

add_subdirectory(hydrogen-astrometry)
