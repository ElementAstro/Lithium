add_library(LithiumDriver
    core/device.cpp
    core/device.hpp

    core/deviceio.cpp
    core/deviceio.hpp

    core/camera.cpp
    core/camera.hpp

    core/telescope.cpp
    core/telescope.hpp

    core/focuser.cpp
    core/focuser.hpp

    core/filterwheel.cpp
    core/filterwheel.hpp

    core/guider.cpp
    core/guider.hpp

    core/solver.cpp
    core/solver.hpp

    event/eventloop.cpp
    event/eventloop.hpp

    event/ioloop.cpp
    event/ioloop.hpp

    network/fifoclient.cpp
    network/fifoclient.hpp

    network/fifoserver.cpp
    network/fifoserver.hpp

    util/utils.cpp
    util/utils.hpp
)

include_directories(.)
include_directories(dsp)
include_directories(fpack)
include_directories(event)

add_subdirectory(core)
add_subdirectory(fpack)
add_subdirectory(event)
add_subdirectory(dsp)
add_subdirectory(hid)