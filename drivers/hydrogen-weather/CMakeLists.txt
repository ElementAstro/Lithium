# ########## OpenWeatherMap Driver ###############
SET(OpenWeatherMap_SRC
    openweathermap.cpp)

find_package(CURL REQUIRED)

add_executable(hydrogen_openweathermap_weather ${OpenWeatherMap_SRC})
target_link_libraries(hydrogen_openweathermap_weather hydrogendriverstatic CURL::libcurl)
install(TARGETS hydrogen_openweathermap_weather RUNTIME DESTINATION bin)

# ########## Weather Safety Proxy ###############
SET(weathersafetyproxy_SRC
    weather_safety_proxy.cpp)
add_executable(indi_weather_safety_proxy ${weathersafetyproxy_SRC})
target_link_libraries(indi_weather_safety_proxy hydrogendriverstatic CURL::libcurl)
install(TARGETS indi_weather_safety_proxy RUNTIME DESTINATION bin)