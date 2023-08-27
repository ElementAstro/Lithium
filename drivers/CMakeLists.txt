include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen/core)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen/core/base)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen/core/io)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen/core/property)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen/core/timer)
include_directories(${CMAKE_SOURCE_DIR}/modules/hydrogen/core/thread)

SET(astrometry_SRC
    lithium-astrometry/astrometrydriver.cpp)

add_executable(lithium_astrometry ${astrometry_SRC})
target_link_libraries(lithium_astrometry hydrogendriverstatic)
install(TARGETS lithium_astrometry RUNTIME DESTINATION bin)

add_subdirectory(lithium-simulator)

add_subdirectory(lithium-weather)

add_subdirectory(lithium-ioptron)

add_subdirectory(hydrogen-onstep)
