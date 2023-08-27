
# ########## Syncscan ###############
add_executable(hydrogen_synscan_telescope
    synscandriver.cpp
    synscanmount.cpp)

target_link_libraries(hydrogen_synscan_telescope hydrogendriver AlignmentDriver)

install(TARGETS hydrogen_synscan_telescope RUNTIME DESTINATION bin)

# ########## Syncscan Legacy ###############
add_executable(hydrogen_synscanlegacy_telescope
    synscandriverlegacy.cpp
    synscanmountlegacy.cpp)

target_link_libraries(hydrogen_synscanlegacy_telescope hydrogendriver AlignmentDriver)

install(TARGETS hydrogen_synscanlegacy_telescope RUNTIME DESTINATION bin)

# ########## Sky Commander ###############
add_executable(hydrogen_skycommander_telescope
    skycommander.cpp)

target_link_libraries(hydrogen_skycommander_telescope hydrogendriverstatic)

install(TARGETS hydrogen_skycommander_telescope RUNTIME DESTINATION bin)