# ########## IEQ Pro Legacy #############
add_executable(HYDROGEN_ieqlegacy_telescope
    ieqprolegacydriver.cpp
    ieqprolegacy.cpp)

target_link_libraries(HYDROGEN_ieqlegacy_telescope hydrogendriverstatic)

install(TARGETS HYDROGEN_ieqlegacy_telescope RUNTIME DESTINATION bin)

# ########## IEQ Pro #############
add_executable(HYDROGEN_ieq_telescope
    ieqdriverbase.cpp
    ieqpro.cpp)

target_link_libraries(HYDROGEN_ieq_telescope hydrogendriverstatic)

install(TARGETS HYDROGEN_ieq_telescope RUNTIME DESTINATION bin)

# ########## IOptronV3 / CEM120 #############
add_executable(HYDROGEN_ioptronv3_telescope
    ioptronv3driver.cpp
    ioptronv3.cpp)

target_link_libraries(HYDROGEN_ioptronv3_telescope hydrogendriverstatic)

install(TARGETS HYDROGEN_ioptronv3_telescope RUNTIME DESTINATION bin)