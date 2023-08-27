# ########## IEQ Pro Legacy #############
add_executable(lithium_ieqlegacy_telescope
    ieqprolegacydriver.cpp
    ieqprolegacy.cpp)

target_link_libraries(lithium_ieqlegacy_telescope hydrogendriverstatic)

install(TARGETS lithium_ieqlegacy_telescope RUNTIME DESTINATION bin)

# ########## IEQ Pro #############
add_executable(lithium_ieq_telescope
    ieqdriverbase.cpp
    ieqpro.cpp)

target_link_libraries(lithium_ieq_telescope hydrogendriverstatic)

install(TARGETS lithium_ieq_telescope RUNTIME DESTINATION bin)

# ########## IOptronV3 / CEM120 #############
add_executable(lithium_ioptronv3_telescope
    ioptronv3driver.cpp
    ioptronv3.cpp)

target_link_libraries(lithium_ioptronv3_telescope hydrogendriverstatic)

install(TARGETS lithium_ioptronv3_telescope RUNTIME DESTINATION bin)