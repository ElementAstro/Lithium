# ########## Celestron GPS ############
add_executable(hydrogen_celestron_gps
    celestrondriver.cpp
    celestrongps.cpp)

target_link_libraries(hydrogen_celestron_gps hydrogendriverstatic)

install(TARGETS hydrogen_celestron_gps RUNTIME DESTINATION bin)