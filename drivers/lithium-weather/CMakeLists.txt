# ########## OpenWeatherMap Driver ###############
SET(OpenWeatherMap_SRC
    openweathermap.cpp)

find_package(CURL REQUIRED)

add_executable(lithium_openweathermap_weather ${OpenWeatherMap_SRC})
target_link_libraries(lithium_openweathermap_weather lithiumcore lithiumdevice dsp eventloop fpack lithiumdriver_OBJECT CURL::libcurl)
install(TARGETS lithium_openweathermap_weather RUNTIME DESTINATION bin)