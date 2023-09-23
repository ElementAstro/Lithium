
add_library(oatpp-swagger
        AsyncController.hpp
        Controller.hpp
        ControllerPaths.hpp
        Generator.cpp
        Generator.hpp
        Model.hpp
        Resources.cpp
        Resources.hpp
        Types.cpp
        Types.hpp
        oas3/Model.hpp)

set_target_properties(oatpp-swagger PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(oatpp-swagger ${LIB_OATPP_EXTERNAL})
endif()

target_link_libraries(oatpp-swagger PUBLIC oatpp)

target_include_directories(oatpp-swagger
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
