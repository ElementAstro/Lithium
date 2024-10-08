#ifndef Config_hpp
#define Config_hpp

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include "oatpp/data/stream/BufferStream.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class ConfigDto : public oatpp::DTO {
public:
    DTO_INIT(ConfigDto, DTO)

    DTO_FIELD(String, statisticsUrl);

    DTO_FIELD(String, host);
    DTO_FIELD(UInt16, port);
    DTO_FIELD(Boolean, useTLS) = true;

    /**
     * Path to TLS private key file.
     */
    DTO_FIELD(String, tlsPrivateKeyPath);

    /**
     * Path to TLS certificate chain file.
     */
    DTO_FIELD(String, tlsCertificateChainPath);

    /**
     * Max size of the received bytes. (the whole MessageDto structure).
     * The actual payload is smaller.
     */
    DTO_FIELD(UInt64, maxMessageSizeBytes) = 8 * 1024;  // Default - 8Kb

    /**
     * Number of the most recent messages to keep in the room history.
     */
    DTO_FIELD(UInt32, maxRoomHistoryMessages) = 100;

public:
    oatpp::String getHostString() {
        oatpp::data::stream::BufferOutputStream stream(256);
        v_uint16 defPort;
        if (useTLS) {
            defPort = 443;
        } else {
            defPort = 80;
        }
        stream << host;
        if (!port || defPort != port) {
            stream << ":" << port;
        }
        return stream.toString();
    }

    oatpp::String getCanonicalBaseUrl() {
        oatpp::data::stream::BufferOutputStream stream(256);
        v_uint16 defPort;
        if (useTLS) {
            stream << "https://";
            defPort = 443;
        } else {
            stream << "http://";
            defPort = 80;
        }
        stream << host;
        if (!port || defPort != port) {
            stream << ":" << port;
        }
        return stream.toString();
    }

    oatpp::String getWebsocketBaseUrl() {
        oatpp::data::stream::BufferOutputStream stream(256);
        if (useTLS) {
            stream << "wss://";
        } else {
            stream << "ws://";
        }
        stream << host << ":" << port;
        return stream.toString();
    }

    oatpp::String getStatsUrl() {
        return getCanonicalBaseUrl() + "/" + statisticsUrl;
    }
};

#include OATPP_CODEGEN_END(DTO)

#endif  // Config_hpp
