# ########## Manual Filter ##############
SET(manualfilter_SRC
    manual_filter.cpp)

add_executable(hydrogen_manual_wheel ${manualfilter_SRC})
target_link_libraries(hydrogen_manual_wheel hydrogendriverstatic)
install(TARGETS hydrogen_manual_wheel RUNTIME DESTINATION bin)