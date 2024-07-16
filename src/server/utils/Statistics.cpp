#include "Statistics.hpp"
#include <thread>

void Statistics::takeSample() {
    auto maxPeriodMicro = m_maxPeriod.count();
    auto pushIntervalMicro = m_pushInterval.count();

    std::lock_guard<std::mutex> guard(m_dataLock);

    auto nowMicro = oatpp::Environment::getMicroTickCount();

    oatpp::Object<StatPointDto> point;

    if (m_dataPoints->size() > 0) {
        const auto& p = m_dataPoints->back();
        if (nowMicro - *p->timestamp < pushIntervalMicro) {
            point = p;
        }
    }

    if (!point) {
        point = StatPointDto::createShared();
        point->timestamp = nowMicro;

        m_dataPoints->push_back(point);

        auto diffMicro = nowMicro - *m_dataPoints->front()->timestamp;
        while (diffMicro > maxPeriodMicro) {
            m_dataPoints->pop_front();
            diffMicro = nowMicro - *m_dataPoints->front()->timestamp;
        }
    }

    point->evFrontpageLoaded = EVENT_FRONT_PAGE_LOADED.load();

    point->evPeerConnected = EVENT_PEER_CONNECTED.load();
    point->evPeerDisconnected = EVENT_PEER_DISCONNECTED.load();
    point->evPeerZombieDropped = EVENT_PEER_ZOMBIE_DROPPED.load();
    point->evPeerSendMessage = EVENT_PEER_SEND_MESSAGE.load();
    point->evPeerShareFile = EVENT_PEER_SHARE_FILE.load();

    point->evRoomCreated = EVENT_ROOM_CREATED.load();
    point->evRoomDeleted = EVENT_ROOM_DELETED.load();

    point->fileServedBytes = FILE_SERVED_BYTES.load();
}

oatpp::String Statistics::getJsonData() {
    std::lock_guard<std::mutex> guard(m_dataLock);
    return m_objectMapper.writeToString(m_dataPoints);
}

void Statistics::runStatLoop() {
    while (true) {
        std::chrono::duration<v_int64, std::micro> elapsed =
            std::chrono::microseconds(0);
        auto startTime = std::chrono::system_clock::now();

        do {
            std::this_thread::sleep_for(m_updateInterval - elapsed);
            elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now() - startTime);
        } while (elapsed < m_updateInterval);

        takeSample();
    }
}