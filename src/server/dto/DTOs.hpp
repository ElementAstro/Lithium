#ifndef DTOs_hpp
#define DTOs_hpp

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(MessageCodes, v_int32, VALUE(CODE_INFO, 0),
     VALUE(CODE_PEER_JOINED, 1), VALUE(CODE_PEER_LEFT, 2),
     VALUE(CODE_PEER_MESSAGE, 3), VALUE(CODE_PEER_MESSAGE_FILE, 4),
     VALUE(CODE_PEER_IS_TYPING, 5),
     VALUE(CODE_FILE_SHARE, 6), VALUE(CODE_FILE_REQUEST_CHUNK, 7),
     VALUE(CODE_FILE_CHUNK_DATA, 8),
     VALUE(CODE_API_ERROR, 9), VALUE(CODE_PEER_COMMAND, 10)
);

class PeerDto : public oatpp::DTO {
public:
    DTO_INIT(PeerDto, DTO)

    DTO_FIELD(Int64, peerId);
    DTO_FIELD(String, peerName);
};

class FileDto : public oatpp::DTO {
    DTO_INIT(FileDto, DTO)

    DTO_FIELD(Int64, clientFileId);
    DTO_FIELD(Int64, serverFileId);
    DTO_FIELD(String, name);
    DTO_FIELD(Int64, size);

    DTO_FIELD(Int64, chunkPosition);
    DTO_FIELD(Int64, chunkSize);
    DTO_FIELD(Int64, subscriberId);
    DTO_FIELD(String, data);  // base64 data
};

class MessageDto : public oatpp::DTO {
public:
    typedef List<Object<FileDto>> FilesList;

public:
    DTO_INIT(MessageDto, DTO)

    DTO_FIELD(Int64, peerId);
    DTO_FIELD(String, peerName);
    DTO_FIELD(Enum<MessageCodes>::AsNumber::NotNull, code);
    DTO_FIELD(String, message);
    DTO_FIELD(Int64, timestamp);

    DTO_FIELD(List<Object<PeerDto>>, peers);
    DTO_FIELD(List<Object<MessageDto>>, history);

    DTO_FIELD(List<Object<FileDto>>, files);
};

class StatPointDto : public oatpp::DTO {
    DTO_INIT(StatPointDto, DTO);

    DTO_FIELD(Int64, timestamp);

    DTO_FIELD(UInt64, evFrontpageLoaded, "ev_front_page_loaded");

    DTO_FIELD(UInt64, evPeerConnected, "ev_peer_connected");
    DTO_FIELD(UInt64, evPeerDisconnected, "ev_peer_disconnected");
    DTO_FIELD(UInt64, evPeerZombieDropped, "ev_peer_zombie_dropped");
    DTO_FIELD(UInt64, evPeerSendMessage, "ev_peer_send_message");
    DTO_FIELD(UInt64, evPeerShareFile, "ev_peer_share_file");

    DTO_FIELD(UInt64, evRoomCreated, "ev_room_created");
    DTO_FIELD(UInt64, evRoomDeleted, "ev_room_deleted");

    DTO_FIELD(UInt64, fileServedBytes, "file_served_bytes");
};

#include OATPP_CODEGEN_END(DTO)

#endif  // DTOs_hpp
