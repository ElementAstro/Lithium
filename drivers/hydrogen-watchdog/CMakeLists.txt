# ########## Watch dog ###############
include_directories(${CMAKE_SOURCE_DIR}/modules/HydrogenClient)

SET(watchdog_SRC
    watchdog.cpp
    watchdogclient.cpp)

IF(UNITY_BUILD)
    ENABLE_UNITY_BUILD(watchdog watchdog_SRC 10 cpp)
ENDIF()

add_executable(hydrogen_watchdog ${watchdog_SRC})

target_link_libraries(hydrogen_watchdog hydrogendriverstatic hydrogenclient)
install(TARGETS hydrogen_watchdog RUNTIME DESTINATION bin)