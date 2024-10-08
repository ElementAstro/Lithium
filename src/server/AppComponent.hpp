#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "dto/Config.hpp"
#include "rooms/Lobby.hpp"
#include "utils/Statistics.hpp"

#include "oatpp-openssl/server/ConnectionProvider.hpp"

#include "oatpp-zlib/EncoderProvider.hpp"

#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/web/server/interceptor/RequestInterceptor.hpp"

#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

#include "oatpp/json/ObjectMapper.hpp"

#include "oatpp/base/CommandLineArguments.hpp"
#include "oatpp/macro/component.hpp"

#include "oatpp/utils/Conversion.hpp"
#include "web/protocol/http/incoming/SimpleBodyDecoder.hpp"

#include <cstdlib>

/**
 *  Class which creates and holds Application components and registers
 * components in oatpp::Environment Order of components initialization is from
 * top to bottom
 */
class AppComponent {
private:
    class RedirectInterceptor
        : public oatpp::web::server::interceptor::RequestInterceptor {
    private:
        OATPP_COMPONENT(oatpp::Object<ConfigDto>, appConfig);

    public:
        std::shared_ptr<OutgoingResponse> intercept(
            const std::shared_ptr<IncomingRequest>& request) override {
            auto host =
                request->getHeader(oatpp::web::protocol::http::Header::HOST);
            auto siteHost = appConfig->getHostString();
            if (!host || host != siteHost) {
                auto response = OutgoingResponse::createShared(
                    oatpp::web::protocol::http::Status::CODE_301, nullptr);
                response->putHeader(
                    "Location", appConfig->getCanonicalBaseUrl() +
                                    request->getStartingLine().path.toString());
                return response;
            }
            return nullptr;
        }
    };

private:
    oatpp::base::CommandLineArguments m_cmdArgs;

public:
    AppComponent(const oatpp::base::CommandLineArguments& cmdArgs)
        : m_cmdArgs(cmdArgs) {}

public:
    /**
     * Create config component
     */
    OATPP_CREATE_COMPONENT(oatpp::Object<ConfigDto>, appConfig)
    ([this] {
        auto config = ConfigDto::createShared();

        config->host = std::getenv("EXTERNAL_ADDRESS");
        if (!config->host) {
            config->host =
                m_cmdArgs.getNamedArgumentValue("--host", "localhost");
        }

        const char* portText = std::getenv("EXTERNAL_PORT");
        if (!portText) {
            portText = m_cmdArgs.getNamedArgumentValue("--port", "8443");
        }

        bool success;
        auto port = oatpp::utils::Conversion::strToUInt32(portText, success);
        if (!success || port > 65535) {
            throw std::runtime_error("Invalid port!");
        }
        config->port = (v_uint16)port;

        config->tlsPrivateKeyPath = std::getenv("TLS_FILE_PRIVATE_KEY");
        if (!config->tlsPrivateKeyPath) {
            config->tlsPrivateKeyPath =
                m_cmdArgs.getNamedArgumentValue("--tls-key", "" CERT_PEM_PATH);
        }

        config->tlsCertificateChainPath = std::getenv("TLS_FILE_CERT_CHAIN");
        if (!config->tlsCertificateChainPath) {
            config->tlsCertificateChainPath = m_cmdArgs.getNamedArgumentValue(
                "--tls-chain", "" CERT_CRT_PATH);
        }

        config->statisticsUrl = std::getenv("URL_STATS_PATH");
        if (!config->statisticsUrl) {
            config->statisticsUrl = m_cmdArgs.getNamedArgumentValue(
                "--url-stats", "admin/stats.json");
        }

        return config;
    }());

    /**
     * Create Async Executor
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)
    ([] { return std::make_shared<oatpp::async::Executor>(); }());

    /**
     *  Create ConnectionProvider component which listens on the port
     */
    OATPP_CREATE_COMPONENT(
        std::shared_ptr<oatpp::network::ServerConnectionProvider>,
        serverConnectionProvider)
    ([] {
        OATPP_COMPONENT(oatpp::Object<ConfigDto>, appConfig);

        std::shared_ptr<oatpp::network::ServerConnectionProvider> result;

        if (appConfig->useTLS) {
            OATPP_LOGd("oatpp::openssl::Config", "key_path='{}'",
                       appConfig->tlsPrivateKeyPath);
            OATPP_LOGd("oatpp::openssl::Config", "chn_path='{}'",
                       appConfig->tlsCertificateChainPath);

            auto config =
                oatpp::openssl::Config::createDefaultServerConfigShared(
                    appConfig->tlsCertificateChainPath->c_str(),
                    appConfig->tlsPrivateKeyPath->c_str());
            result = oatpp::openssl::server::ConnectionProvider::createShared(
                config,
                {"0.0.0.0", appConfig->port, oatpp::network::Address::IP_4});
        } else {
            result =
                oatpp::network::tcp::server::ConnectionProvider::createShared(
                    {"0.0.0.0", appConfig->port,
                     oatpp::network::Address::IP_4});
        }

        return result;
    }());

    /**
     *  Create Router component
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>,
                           httpRouter)
    ([] { return oatpp::web::server::HttpRouter::createShared(); }());

    /**
     *  Create ConnectionHandler component which uses Router component to route
     * requests
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                           serverConnectionHandler)
    ("http", [] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>,
                        router);  // get Router component
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>,
                        executor);  // get Async executor component
        auto components =
            std::make_shared<oatpp::web::server::HttpProcessor::Components>(
                router);

        /* Add content encoders */
        auto encoders = std::make_shared<
            oatpp::web::protocol::http::encoding::ProviderCollection>();

        encoders->add(std::make_shared<oatpp::zlib::DeflateEncoderProvider>());
        encoders->add(std::make_shared<oatpp::zlib::GzipEncoderProvider>());

        /* Set content encoders */
        components->contentEncodingProviders = encoders;

        auto decoders = std::make_shared<
            oatpp::web::protocol::http::encoding::ProviderCollection>();

        decoders->add(std::make_shared<oatpp::zlib::DeflateDecoderProvider>());
        decoders->add(std::make_shared<oatpp::zlib::GzipDecoderProvider>());

        /* Set Body Decoder */
        components->bodyDecoder = std::make_shared<
            oatpp::web::protocol::http::incoming::SimpleBodyDecoder>(decoders);
        auto handler =
            oatpp::web::server::AsyncHttpConnectionHandler::createShared(
                router, executor);
        handler->addRequestInterceptor(std::make_shared<RedirectInterceptor>());
        return handler;
    }());

    /**
     *  Create ObjectMapper component to serialize/deserialize DTOs in
     * Contoller's API
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                           apiObjectMapper)
    ([] {
        auto mapper = std::make_shared<oatpp::json::ObjectMapper>();
        mapper->serializerConfig().mapper.includeNullFields = false;
        return mapper;
    }());

    /**
     *  Create statistics object
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<Statistics>, statistics)
    ([] { return std::make_shared<Statistics>(); }());

    /**
     *  Create chat lobby component.
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<Lobby>, lobby)
    ([] { return std::make_shared<Lobby>(); }());

    /**
     *  Create websocket connection handler
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                           websocketConnectionHandler)
    ("websocket", [] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        OATPP_COMPONENT(std::shared_ptr<Lobby>, lobby);
        auto connectionHandler =
            oatpp::websocket::AsyncConnectionHandler::createShared(executor);
        connectionHandler->setSocketInstanceListener(lobby);
        return connectionHandler;
    }());
};

#endif /* AppComponent_hpp */
