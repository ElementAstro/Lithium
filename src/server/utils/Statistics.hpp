#ifndef Statistics_hpp
#define Statistics_hpp

#include "dto/DTOs.hpp"
#include "oatpp/Types.hpp"
#include "oatpp/json/ObjectMapper.hpp"

#include <chrono>

class Statistics {
public:
    std::atomic<v_uint64> EVENT_FRONT_PAGE_LOADED{0};  // On Frontpage Loaded

    std::atomic<v_uint64> EVENT_PEER_CONNECTED{
        0};  // On Connected event counter
    std::atomic<v_uint64> EVENT_PEER_DISCONNECTED{
        0};  // On Disconnected event counter
    std::atomic<v_uint64> EVENT_PEER_ZOMBIE_DROPPED{
        0};  // On Disconnected due to failed ping counter
    std::atomic<v_uint64> EVENT_PEER_SEND_MESSAGE{0};  // Sent messages counter
    std::atomic<v_uint64> EVENT_PEER_SHARE_FILE{0};    // Shared files counter

    std::atomic<v_uint64> EVENT_ROOM_CREATED{0};  // On room created
    std::atomic<v_uint64> EVENT_ROOM_DELETED{0};  // On room deleted

    std::atomic<v_uint64> FILE_SERVED_BYTES{
        0};  // Overall shared files served bytes

private:
    oatpp::json::ObjectMapper m_objectMapper;

private:
    oatpp::List<oatpp::Object<StatPointDto>> m_dataPoints =
        oatpp::List<oatpp::Object<StatPointDto>>::createShared();
    std::mutex m_dataLock;

private:
    std::chrono::duration<v_int64, std::micro> m_maxPeriod;
    std::chrono::duration<v_int64, std::micro> m_pushInterval;
    std::chrono::duration<v_int64, std::micro> m_updateInterval;

public:
    Statistics(const std::chrono::duration<v_int64, std::micro>& maxPeriod =
                   std::chrono::hours(7 * 24),
               const std::chrono::duration<v_int64, std::micro>& pushInterval =
                   std::chrono::hours(1),
               const std::chrono::duration<v_int64, std::micro>&
                   updateInterval = std::chrono::seconds(1))
        : m_maxPeriod(maxPeriod),
          m_pushInterval(pushInterval),
          m_updateInterval(updateInterval) {}

    void takeSample();
    oatpp::String getJsonData();

    void runStatLoop();
};

#endif  // Statistics_hpp
