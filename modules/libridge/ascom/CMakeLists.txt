add_library(
    ascom_remote SHARED
    ascom_camera.cpp
    ascom_camera.hpp

    ascom_covercalibrator.cpp
    ascom_covercalibrator.hpp

    ascom_device.cpp
    ascom_device.hpp

    ascom_filterwheel.cpp
    ascom_filterwheel.hpp

    ascom_focuser.cpp
    ascom_focuser.hpp

    ascom_telescope.cpp
    ascom_telescope.hpp
)

target_link_libraries(ascom_remote PRIVATE LithiumDriver LithiumProperty loguru cpp_httplib)

if(WIN32)
target_link_libraries(ascom_remote PRIVATE wsock32 ws2_32)
endif()

include_directories(${CMAKE_SOURCE_DIR}/modules/lidrvier)