
add_library(oatpp-openssl
        Config.cpp
        Config.hpp
        Connection.cpp
        Connection.hpp
        ErrorStack.cpp
        ErrorStack.hpp
        client/ConnectionProvider.cpp
        client/ConnectionProvider.hpp
        server/ConnectionProvider.cpp
        server/ConnectionProvider.hpp
        configurer/CaCertificateBundleBuffer.cpp
        configurer/CaCertificateBundleBuffer.hpp
        configurer/CertificateChainBuffer.cpp
        configurer/CertificateChainBuffer.hpp
        configurer/CertificateChainFile.cpp
        configurer/CertificateChainFile.hpp
        configurer/CertificateBuffer.cpp
        configurer/CertificateBuffer.hpp
        configurer/CertificateFile.cpp
        configurer/CertificateFile.hpp
        configurer/ContextConfigurer.hpp
        configurer/Groups.cpp
        configurer/Groups.hpp
        configurer/PeerCertificateVerification.cpp
        configurer/PeerCertificateVerification.hpp
        configurer/PrivateKeyBuffer.cpp
        configurer/PrivateKeyBuffer.hpp
        configurer/PrivateKeyFile.cpp
        configurer/PrivateKeyFile.hpp
        configurer/TemporaryDhParamsFile.cpp
        configurer/TemporaryDhParamsFile.hpp
        configurer/TrustStore.cpp
        configurer/TrustStore.hpp
)

set_target_properties(oatpp-openssl PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(oatpp-openssl ${LIB_OATPP_EXTERNAL})
endif()

target_link_libraries(oatpp-openssl PUBLIC oatpp)

target_include_directories(oatpp-openssl
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

find_package(OpenSSL REQUIRED)

target_link_libraries(oatpp-openssl
        PUBLIC OpenSSL::SSL
        PUBLIC OpenSSL::Crypto
)
