include_directories(${CMAKE_SOURCE_DIR}/modules/lidriver)
include_directories(${CMAKE_SOURCE_DIR}/modules/lidriver/core)
include_directories(${CMAKE_SOURCE_DIR}/modules/lidriver/core/base)
include_directories(${CMAKE_SOURCE_DIR}/modules/lidriver/core/io)
include_directories(${CMAKE_SOURCE_DIR}/modules/lidriver/core/property)
include_directories(${CMAKE_SOURCE_DIR}/modules/lidriver/core/timer)
include_directories(${CMAKE_SOURCE_DIR}/modules/lidriver/core/thread)

SET(astrometry_SRC
    lithium-astrometry/astrometrydriver.cpp)

add_executable(lithium_astrometry ${astrometry_SRC})
target_link_libraries(lithium_astrometry lithiumcore lithiumdevice dsp eventloop fpack lithiumdriver_OBJECT)
install(TARGETS lithium_astrometry RUNTIME DESTINATION bin)

add_subdirectory(lithium-simulator)

add_subdirectory(lithium-weather)

add_subdirectory(lithium-ioptron)

add_subdirectory(hydrogen-onstep)
