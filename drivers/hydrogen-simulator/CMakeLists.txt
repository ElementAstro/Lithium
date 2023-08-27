# ########## CCD Simulator ##############
SET(ccdsimulator_SRC
ccd_simulator.cpp)

add_executable(hydrogen_simulator_ccd ${ccdsimulator_SRC})
target_link_libraries(hydrogen_simulator_ccd hydrogendriverstatic)
install(TARGETS hydrogen_simulator_ccd RUNTIME DESTINATION bin)

# ############### Focuser Simulator ################
SET(focussimulator_SRC
    focus_simulator.cpp)

add_executable(hydrogen_simulator_focus ${focussimulator_SRC})
target_link_libraries(hydrogen_simulator_focus hydrogendriverstatic)
install(TARGETS hydrogen_simulator_focus RUNTIME DESTINATION bin)

# ############### Receiver Simulator ################
SET(receiversimulator_SRC
    receiver_simulator.cpp)

add_executable(hydrogen_simulator_receiver ${receiversimulator_SRC})
target_link_libraries(hydrogen_simulator_receiver hydrogendriverstatic)
install(TARGETS hydrogen_simulator_receiver RUNTIME DESTINATION bin)

# ########## Telescope Simulator ##############
add_executable(hydrogen_simulator_telescope
    telescope_simulator.cpp
    scopesim_helper.cpp)

target_link_libraries(hydrogen_simulator_telescope hydrogendriverstatic)

install(TARGETS hydrogen_simulator_telescope RUNTIME DESTINATION bin)

# ########## Weather Simulator ###############

SET(weathersimulator_SRC
    weathersimulator.cpp)

add_executable(hydrogen_simulator_weather ${weathersimulator_SRC})
target_link_libraries(hydrogen_simulator_weather hydrogendriverstatic)
install(TARGETS hydrogen_simulator_weather RUNTIME DESTINATION bin)

# ############### Dome Simulator ################
SET(domesimulator_SRC
    dome_simulator.cpp)

add_executable(hydrogen_simulator_dome ${domesimulator_SRC})
target_link_libraries(hydrogen_simulator_dome hydrogendriverstatic)
install(TARGETS hydrogen_simulator_dome RUNTIME DESTINATION bin)

# ########## Filter Simulator ##############
SET(filtersimulator_SRC
    filter_simulator.cpp)

add_executable(hydrogen_simulator_wheel ${filtersimulator_SRC})
target_link_libraries(hydrogen_simulator_wheel hydrogendriverstatic)
install(TARGETS hydrogen_simulator_wheel RUNTIME DESTINATION bin)

# ############### Rotator Simulator ########
SET(rotatorsim_SRC
    rotator_simulator.cpp)
    
add_executable(hydrogen_simulator_rotator ${rotatorsim_SRC})
target_link_libraries(hydrogen_simulator_rotator hydrogendriverstatic)
install(TARGETS hydrogen_simulator_rotator RUNTIME DESTINATION bin)