#include "Lobby.hpp"

v_int64 Lobby::obtainNewPeerId() { return m_peerIdCounter++; }

std::shared_ptr<Room> Lobby::getOrCreateRoom(const oatpp::String& roomName) {
    std::lock_guard<std::mutex> lock(m_roomsMutex);
    std::shared_ptr<Room>& room = m_rooms[roomName];
    if (!room) {
        room = std::make_shared<Room>(roomName);
    }
    return room;
}

std::shared_ptr<Room> Lobby::getRoom(const oatpp::String& roomName) {
    std::lock_guard<std::mutex> lock(m_roomsMutex);
    auto it = m_rooms.find(roomName);
    if (it != m_rooms.end()) {
        return it->second;
    }
    return nullptr;
}

void Lobby::deleteRoom(const oatpp::String& roomName) {
    std::lock_guard<std::mutex> lock(m_roomsMutex);
    m_rooms.erase(roomName);
}

void Lobby::runPingLoop(
    const std::chrono::duration<v_int64, std::micro>& interval) {
    while (true) {
        std::chrono::duration<v_int64, std::micro> elapsed =
            std::chrono::microseconds(0);
        auto startTime = std::chrono::system_clock::now();

        do {
            std::this_thread::sleep_for(interval - elapsed);
            elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - startTime);
        } while (elapsed < interval);

        std::lock_guard<std::mutex> lock(m_roomsMutex);
        for (const auto& room : m_rooms) {
            room.second->pingAllPeers();
        }
    }
}

void Lobby::onAfterCreate_NonBlocking(
    const std::shared_ptr<AsyncWebSocket>& socket,
    const std::shared_ptr<const ParameterMap>& params) {
    ++m_statistics->EVENT_PEER_CONNECTED;

    auto roomName = params->find("roomName")->second;
    auto nickname = params->find("nickname")->second;
    auto room = getOrCreateRoom(roomName);

    auto peer =
        std::make_shared<Peer>(socket, room, nickname, obtainNewPeerId());
    socket->setListener(peer);

    room->welcomePeer(peer);
    room->addPeer(peer);
    room->onboardPeer(peer);
}

void Lobby::onBeforeDestroy_NonBlocking(
    const std::shared_ptr<AsyncWebSocket>& socket) {
    ++m_statistics->EVENT_PEER_DISCONNECTED;

    auto peer = std::static_pointer_cast<Peer>(socket->getListener());
    auto room = peer->getRoom();

    room->removePeerById(peer->getPeerId());
    room->goodbyePeer(peer);
    peer->invalidateSocket();

    if (room->isEmpty()) {
        deleteRoom(room->getName());
    }
}
