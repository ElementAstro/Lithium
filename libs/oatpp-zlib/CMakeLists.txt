add_library(oatpp-zlib
        Processor.cpp
        Processor.hpp
        EncoderProvider.cpp
        EncoderProvider.hpp
)

find_package(ZLIB)

set_target_properties(oatpp-zlib PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(oatpp-zlib ${LIB_OATPP_EXTERNAL})
endif()

target_link_libraries(oatpp-zlib PUBLIC oatpp)

target_include_directories(oatpp-zlib
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(oatpp-zlib
        PUBLIC ZLIB::ZLIB
)
