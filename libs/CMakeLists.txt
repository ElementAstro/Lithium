add_subdirectory(oatpp)

add_library(oatpp-swagger
    oatpp-swagger/AsyncController.hpp
    oatpp-swagger/Controller.hpp
    oatpp-swagger/ControllerPaths.hpp
    oatpp-swagger/Generator.cpp
    oatpp-swagger/Generator.hpp
    oatpp-swagger/Model.hpp
    oatpp-swagger/Resources.cpp
    oatpp-swagger/Resources.hpp
    oatpp-swagger/Types.cpp
    oatpp-swagger/Types.hpp
    oatpp-swagger/oas3/Model.hpp)

add_library(oatpp-websocket
    oatpp-websocket/AsyncConnectionHandler.cpp
    oatpp-websocket/AsyncConnectionHandler.hpp
    oatpp-websocket/AsyncWebSocket.cpp
    oatpp-websocket/AsyncWebSocket.hpp
    oatpp-websocket/Config.hpp
    oatpp-websocket/ConnectionHandler.cpp
    oatpp-websocket/ConnectionHandler.hpp
    oatpp-websocket/Connector.cpp
    oatpp-websocket/Connector.hpp
    oatpp-websocket/Frame.cpp
    oatpp-websocket/Frame.hpp
    oatpp-websocket/Handshaker.cpp
    oatpp-websocket/Handshaker.hpp
    oatpp-websocket/SHA1.cpp
    oatpp-websocket/SHA1.hpp
    oatpp-websocket/Utils.cpp
    oatpp-websocket/Utils.hpp
    oatpp-websocket/WebSocket.cpp
    oatpp-websocket/WebSocket.hpp
)

add_library(oatpp-openssl
    oatpp-openssl/Config.cpp
    oatpp-openssl/Config.hpp
    oatpp-openssl/Connection.cpp
    oatpp-openssl/Connection.hpp
    oatpp-openssl/ErrorStack.cpp
    oatpp-openssl/ErrorStack.hpp
    oatpp-openssl/client/ConnectionProvider.cpp
    oatpp-openssl/client/ConnectionProvider.hpp
    oatpp-openssl/server/ConnectionProvider.cpp
    oatpp-openssl/server/ConnectionProvider.hpp
    oatpp-openssl/configurer/CaCertificateBundleBuffer.cpp
    oatpp-openssl/configurer/CaCertificateBundleBuffer.hpp
    oatpp-openssl/configurer/CertificateChainBuffer.cpp
    oatpp-openssl/configurer/CertificateChainBuffer.hpp
    oatpp-openssl/configurer/CertificateChainFile.cpp
    oatpp-openssl/configurer/CertificateChainFile.hpp
    oatpp-openssl/configurer/CertificateBuffer.cpp
    oatpp-openssl/configurer/CertificateBuffer.hpp
    oatpp-openssl/configurer/CertificateFile.cpp
    oatpp-openssl/configurer/CertificateFile.hpp
    oatpp-openssl/configurer/ContextConfigurer.hpp
    oatpp-openssl/configurer/Groups.cpp
    oatpp-openssl/configurer/Groups.hpp
    oatpp-openssl/configurer/PeerCertificateVerification.cpp
    oatpp-openssl/configurer/PeerCertificateVerification.hpp
    oatpp-openssl/configurer/PrivateKeyBuffer.cpp
    oatpp-openssl/configurer/PrivateKeyBuffer.hpp
    oatpp-openssl/configurer/PrivateKeyFile.cpp
    oatpp-openssl/configurer/PrivateKeyFile.hpp
    oatpp-openssl/configurer/TemporaryDhParamsFile.cpp
    oatpp-openssl/configurer/TemporaryDhParamsFile.hpp
    oatpp-openssl/configurer/TrustStore.cpp
    oatpp-openssl/configurer/TrustStore.hpp
)

add_library(oatpp-zlib
    oatpp-zlib/Processor.cpp
    oatpp-zlib/Processor.hpp
    oatpp-zlib/EncoderProvider.cpp
    oatpp-zlib/EncoderProvider.hpp
)

add_library(loguru loguru/loguru.cpp)
if(LINUX)
    find_package(Threads REQUIRED)
    target_link_libraries(loguru Threads::Threads dl)
    set_target_properties(loguru PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )
endif()

add_library(cpp_httplib cpp_httplib/httplib.cpp)
target_link_libraries(cpp_httplib ${OPENSSL_LIBRARIES})
if(WIN32)
target_link_libraries(cpp_httplib crypt32)
endif()
if(LINUX)
    set_target_properties(cpp_httplib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    )
endif()

add_library(libzippp zippp/libzippp.cpp)

add_subdirectory(backward)