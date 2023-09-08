add_library(LithiumProperty
    base64.cpp
    base64.hpp

    iproperty.cpp
    iproperty.hpp

    iphoto.cpp
    iphoto.hpp

    sha256.hpp

    uuid.cpp
    uuid.hpp

    task/conditional_task.cpp
    task/conditional_task.hpp

    task/daemon_task.cpp
    task/daemon_task.hpp

    task/loop_task.cpp
    task/loop_task.hpp

    task/task.cpp
    task/task.hpp
)

if(LINUX)
set_target_properties(LithiumProperty PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)
endif()