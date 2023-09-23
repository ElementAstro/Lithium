
add_library(oatpp-websocket
        AsyncConnectionHandler.cpp
        AsyncConnectionHandler.hpp
        AsyncWebSocket.cpp
        AsyncWebSocket.hpp
        Config.hpp
        ConnectionHandler.cpp
        ConnectionHandler.hpp
        Connector.cpp
        Connector.hpp
        Frame.cpp
        Frame.hpp
        Handshaker.cpp
        Handshaker.hpp
        SHA1.cpp
        SHA1.hpp
        Utils.cpp
        Utils.hpp
        WebSocket.cpp
        WebSocket.hpp
)

set_target_properties(oatpp-websocket PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(oatpp-websocket ${LIB_OATPP_EXTERNAL})
endif()

target_link_libraries(oatpp-websocket PUBLIC oatpp)

target_include_directories(oatpp-websocket
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
