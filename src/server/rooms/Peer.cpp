#include "Peer.hpp"

#include <utility>
#include "Room.hpp"

#include "base/Log.hpp"
#include "dto/DTOs.hpp"
#include "oatpp/encoding/Base64.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

void Peer::sendMessageAsync(const oatpp::Object<MessageDto>& message) {
    class SendMessageCoroutine
        : public oatpp::async::Coroutine<SendMessageCoroutine> {
    private:
        oatpp::async::Lock* m_lock_;
        std::shared_ptr<AsyncWebSocket> m_websocket_;
        oatpp::String m_message_;

    public:
        SendMessageCoroutine(oatpp::async::Lock* lock,
                             const std::shared_ptr<AsyncWebSocket>& websocket,
                             oatpp::String message)
            : m_lock_(lock),
              m_websocket_(websocket),
              m_message_(std::move(message)) {}

        auto act() -> Action override {
            return oatpp::async::synchronize(
                       m_lock_, m_websocket_->sendOneFrameTextAsync(m_message_))
                .next(finish());
        }
    };

    if (m_socket_) {
        m_asyncExecutor->execute<SendMessageCoroutine>(
            &m_writeLock_, m_socket_, m_objectMapper->writeToString(message));
    }
}

auto Peer::sendPingAsync() -> bool {
    class SendPingCoroutine
        : public oatpp::async::Coroutine<SendPingCoroutine> {
    private:
        oatpp::async::Lock* m_lock_;
        std::shared_ptr<AsyncWebSocket> m_websocket_;

    public:
        SendPingCoroutine(oatpp::async::Lock* lock,
                          const std::shared_ptr<AsyncWebSocket>& websocket)
            : m_lock_(lock), m_websocket_(websocket) {}

        auto act() -> Action override {
            return oatpp::async::synchronize(
                       m_lock_, m_websocket_->sendPingAsync(nullptr))
                .next(finish());
        }
    };

    /******************************************************
     *
     * Ping counter is increased on sending ping
     * and decreased on receiving pong from the client.
     *
     * If the server didn't receive pong from client
     * before the next ping,- then the client is
     * considered to be disconnected.
     *
     ******************************************************/

    ++m_pingPoingCounter_;

    if (m_socket_ && m_pingPoingCounter_ == 1) {
        m_asyncExecutor->execute<SendPingCoroutine>(&m_writeLock_, m_socket_);
        return true;
    }

    return false;
}

auto Peer::onApiError(const oatpp::String& errorMessage)
    -> oatpp::async::CoroutineStarter {
    class SendErrorCoroutine
        : public oatpp::async::Coroutine<SendErrorCoroutine> {
    private:
        oatpp::async::Lock* m_lock_;
        std::shared_ptr<AsyncWebSocket> m_websocket_;
        oatpp::String m_message_;

    public:
        SendErrorCoroutine(oatpp::async::Lock* lock,
                           const std::shared_ptr<AsyncWebSocket>& websocket,
                           oatpp::String message)
            : m_lock_(lock),
              m_websocket_(websocket),
              m_message_(std::move(message)) {}

        auto act() -> Action override {
            /* synchronized async pipeline */
            return oatpp::async::synchronize(
                       /* Async write-lock to prevent concurrent writes to
                          socket */
                       m_lock_,
                       /* send error message, then close-frame */
                       std::move(m_websocket_->sendOneFrameTextAsync(m_message_)
                                     .next(m_websocket_->sendCloseAsync())))
                .next(
                    /* async error after error message and close-frame are sent
                     */
                    new oatpp::async::Error("API Error"));
        }
    };

    auto message = MessageDto::createShared();
    message->code = MessageCodes::CODE_API_ERROR;
    message->message = errorMessage;

    return SendErrorCoroutine::start(&m_writeLock_, m_socket_,
                                     m_objectMapper->writeToString(message));
}

auto Peer::validateFilesList(const MessageDto::FilesList& filesList)
    -> oatpp::async::CoroutineStarter {
    if (filesList->size() == 0) {
        return onApiError("Files list is empty.");
    }

    for (auto& fileDto : *filesList) {
        if (!fileDto) {
            return onApiError("File structure is not provided.");
        }
        if (!fileDto->clientFileId) {
            return onApiError("File clientId is not provided.");
        }
        if (!fileDto->name) {
            return onApiError("File name is not provided.");
        }
        if (!fileDto->size) {
            return onApiError("File size is not provided.");
        }
    }

    return nullptr;
}

auto Peer::handleFilesMessage(const oatpp::Object<MessageDto>& message)
    -> oatpp::async::CoroutineStarter {
    auto files = message->files;
    validateFilesList(files);

    auto fileMessage = MessageDto::createShared();
    fileMessage->code = MessageCodes::CODE_PEER_MESSAGE_FILE;
    fileMessage->peerId = m_peerId_;
    fileMessage->peerName = m_nickname_;
    fileMessage->timestamp = oatpp::Environment::getMicroTickCount();
    fileMessage->files = MessageDto::FilesList::createShared();

    for (auto& currFile : *files) {
        auto file = m_room_->shareFile(m_peerId_, currFile->clientFileId,
                                       currFile->name, currFile->size);

        auto sharedFile = FileDto::createShared();
        sharedFile->serverFileId = file->getServerFileId();
        sharedFile->name = file->getFileName();
        sharedFile->size = file->getFileSize();

        fileMessage->files->push_back(sharedFile);
    }

    m_room_->addHistoryMessage(fileMessage);
    m_room_->sendMessageAsync(fileMessage);

    return nullptr;
}

auto Peer::handleFileChunkMessage(const oatpp::Object<MessageDto>& message)
    -> oatpp::async::CoroutineStarter {
    auto filesList = message->files;
    if (!filesList) {
        return onApiError("No file provided.");
    }

    if (filesList->size() > 1) {
        return onApiError("Invalid files count. Expected - 1.");
    }

    auto fileDto = filesList->front();
    if (!fileDto) {
        return onApiError("File structure is not provided.");
    }
    if (!fileDto->serverFileId) {
        return onApiError("File clientId is not provided.");
    }
    if (!fileDto->subscriberId) {
        return onApiError("File subscriberId is not provided.");
    }
    if (!fileDto->data) {
        return onApiError("File chunk data is not provided.");
    }

    auto file = m_room_->getFileById(fileDto->serverFileId);

    if (!file) {
        return nullptr;  // Ignore if file doesn't exist. File may be deleted
    }
    // already.

    if (file->getHost()->getPeerId() != getPeerId()) {
        return onApiError("Wrong file host.");
    }

    auto data = oatpp::encoding::Base64::decode(fileDto->data);
    file->provideFileChunk(fileDto->subscriberId, data);

    return nullptr;
}

auto Peer::handleQTextMessage(const std::string& message)
    -> oatpp::async::CoroutineStarter {
        std::vector<std::string> parts;
    std::stringstream ss(message);
    std::string part;
    while (std::getline(ss, part, ':')) {
        parts.push_back(part);
    }

    auto trim = [](std::string &s) -> std::string {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    };

    if (parts.size() == 2 && trim(parts[0]) == "ConfirmIndiDriver") {
        std::string driverName = trim(parts[1]);
        indi_Driver_Confirm(driverName);
    } else if (parts.size() == 2 && trim(parts[0]) == "ConfirmIndiDevice") {
        std::string deviceName = trim(parts[1]);
        indi_Device_Confirm(deviceName);
    } else if (parts.size() == 3 && trim(parts[0]) == "SelectIndiDriver") {
        std::string Group = trim(parts[1]);
        int ListNum = std::stoi(trim(parts[2]));
        printDevGroups2(drivers_list, ListNum, Group);
    } else if (parts.size() == 2 && trim(parts[0]) == "takeExposure") {
        int ExpTime = std::stoi(trim(parts[1]));
        std::cout << ExpTime << std::endl;
        INDI_Capture(ExpTime);
        glExpTime = ExpTime;
    } else if (parts.size() == 2 && trim(parts[0]) == "focusSpeed") {
        int Speed = std::stoi(trim(parts[1]));
        std::cout << Speed << std::endl;
        int Speed_ = FocuserControl_setSpeed(Speed);
        wsThread->sendMessageToClient("FocusChangeSpeedSuccess:" + std::to_string(Speed_));
    } else if (parts.size() == 3 && trim(parts[0]) == "focusMove") {
        std::string LR = trim(parts[1]);
        int Steps = std::stoi(trim(parts[2]));
        if (LR == "Left") {
            FocusMoveAndCalHFR(true, Steps);
        } else if (LR == "Right") {
            FocusMoveAndCalHFR(false, Steps);
        } else if (LR == "Target") {
            FocusGotoAndCalFWHM(Steps);
        }
    } else if (parts.size() == 5 && trim(parts[0]) == "RedBox") {
        int x = std::stoi(trim(parts[1]));
        int y = std::stoi(trim(parts[2]));
        int width = std::stoi(trim(parts[3]));
        int height = std::stoi(trim(parts[4]));
        glROI_x = x;
        glROI_y = y;
        CaptureViewWidth = width;
        CaptureViewHeight = height;
        std::cout << "RedBox:" << glROI_x << glROI_y << CaptureViewWidth << CaptureViewHeight << std::endl;
    } else if (parts.size() == 2 && trim(parts[0]) == "RedBoxSizeChange") {
        BoxSideLength = std::stoi(trim(parts[1]));
        std::cout << "BoxSideLength:" << BoxSideLength << std::endl;
        wsThread->sendMessageToClient("MainCameraSize:" + std::to_string(glMainCCDSizeX) + ":" + std::to_string(glMainCCDSizeY));
    } else if (message == "AutoFocus") {
        AutoFocus();
    } else if (message == "StopAutoFocus") {
        StopAutoFocus = true;
    } else if (message == "abortExposure") {
        INDI_AbortCapture();
    } else if (message == "connectAllDevice") {
        DeviceConnect();
    } else if (message == "CS") {
        // std::string Dev = connectIndiServer();
        // websocket->messageSend("AddDevice:" + Dev);
    } else if (message == "DS") {
        disconnectIndiServer();
    } else if (message == "MountMoveWest") {
        if (dpMount != NULL) {
            indi_Client->setTelescopeMoveWE(dpMount, "WEST");
        }
    } else if (message == "MountMoveEast") {
        if (dpMount != NULL) {
            indi_Client->setTelescopeMoveWE(dpMount, "EAST");
        }
    } else if (message == "MountMoveNorth") {
        if (dpMount != NULL) {
            indi_Client->setTelescopeMoveNS(dpMount, "NORTH");
        }
    } else if (message == "MountMoveSouth") {
        if (dpMount != NULL) {
            indi_Client->setTelescopeMoveNS(dpMount, "SOUTH");
        }
    } else if (message == "MountMoveAbort") {
        if (dpMount != NULL) {
            indi_Client->setTelescopeAbortMotion(dpMount);
        }
    } else if (message == "MountPark") {
        if (dpMount != NULL) {
            bool isPark = TelescopeControl_Park();
            if (isPark) {
                wsThread->sendMessageToClient("TelescopePark:ON");
            } else {
                wsThread->sendMessageToClient("TelescopePark:OFF");
            }
        }
    } else if (message == "MountTrack") {
        if (dpMount != NULL) {
            bool isTrack = TelescopeControl_Track();
            if (isTrack) {
                wsThread->sendMessageToClient("TelescopeTrack:ON");
            } else {
                wsThread->sendMessageToClient("TelescopeTrack:OFF");
            }
        }
    } else if (message == "MountHome") {
        if (dpMount != NULL) {
            indi_Client->setTelescopeHomeInit(dpMount, "SLEWHOME");
        }
    } else if (message == "MountSYNC") {
        if (dpMount != NULL) {
            indi_Client->setTelescopeHomeInit(dpMount, "SYNCHOME");
        }
    } else if (parts.size() == 2 && trim(parts[0]) == "MountSpeedSet") {
        int Speed = std::stoi(trim(parts[1]));
        std::cout << "MountSpeedSet:" << Speed << std::endl;
        if (dpMount != NULL) {
            indi_Client->setTelescopeSlewRate(dpMount, Speed - 1);
            int Speed_;
            indi_Client->getTelescopeSlewRate(dpMount, Speed_);
            wsThread->sendMessageToClient("MountSetSpeedSuccess:" + std::to_string(Speed_));
        }
    } else if (parts.size() == 2 && trim(parts[0]) == "ImageGainR") {
        ImageGainR = std::stod(trim(parts[1]));
        std::cout << "GainR is set to " << ImageGainR << std::endl;
    } else if (parts.size() == 2 && trim(parts[0]) == "ImageGainB") {
        ImageGainB = std::stod(trim(parts[1]));
        std::cout << "GainB is set to " << ImageGainB << std::endl;
    } else if (trim(parts[0]) == "ScheduleTabelData") {
        ScheduleTabelData(message);
    } else if (parts.size() == 4 && trim(parts[0]) == "MountGoto") {
        std::vector<std::string> RaDecList;
        std::stringstream ss2(message);
        std::string part2;
        while (std::getline(ss2, part2, ',')) {
            RaDecList.push_back(part2);
        }
        std::vector<std::string> RaList;
        std::stringstream ss3(RaDecList[0]);
        while (std::getline(ss3, part2, ':')) {
            RaList.push_back(part2);
        }
        std::vector<std::string> DecList;
        std::stringstream ss4(RaDecList[1]);
        while (std::getline(ss4, part2, ':')) {
            DecList.push_back(part2);
        }

        double Ra_Rad = std::stod(trim(RaList[2]));
        double Dec_Rad = std::stod(trim(DecList[1]));

        std::cout << "RaDec(Rad):" << Ra_Rad << "," << Dec_Rad << std::endl;

        double Ra_Hour = Tools::RadToHour(Ra_Rad);
        double Dec_Degree = Tools::RadToDegree(Dec_Rad);

        MountGoto(Ra_Hour, Dec_Degree);
    } else if (message == "StopSchedule") {
        StopSchedule = true;
    } else if (message == "CaptureImageSave") {
        CaptureImageSave();
    } else if (message == "getConnectedDevices") {
        getConnectedDevices();
    } else if (message == "getStagingImage") {
                getStagingImage();
    } else if (trim(parts[0]) == "StagingScheduleData") {
        isStagingScheduleData = true;
        StagingScheduleData = message;
    } else if (message == "getStagingScheduleData") {
        getStagingScheduleData();
    } else if (trim(parts[0]) == "ExpTimeList") {
        Tools::saveExpTimeList(message);
    } else if (message == "getExpTimeList") {
        std::string expTimeList = Tools::readExpTimeList();
        if (!expTimeList.empty()) {
            wsThread->sendMessageToClient(expTimeList);
        }
    } else if (message == "getCaptureStatus") {
        std::cout << "MainCameraStatu: " << glMainCameraStatu << std::endl;
        if (glMainCameraStatu == "Exposuring") {
            wsThread->sendMessageToClient("CameraInExposuring:True");
        }
    } else if (parts.size() == 2 && trim(parts[0]) == "SetCFWPosition") {
        int pos = std::stoi(trim(parts[1]));
        if (dpCFW != NULL) {
            indi_Client->setCFWPosition(dpCFW, pos);
            wsThread->sendMessageToClient("SetCFWPositionSuccess:" + std::to_string(pos));
            std::cout << "Set CFW Position to " << pos << " Success!!!" << std::endl;
        }
    } else if (parts.size() == 2 && trim(parts[0]) == "CFWList") {
        if (dpCFW != NULL) {
            Tools::saveCFWList(std::string(dpCFW->getDeviceName()), parts[1]);
        }
    } else if (message == "getCFWList") {
        if (dpCFW != NULL) {
            int min, max, pos;
            indi_Client->getCFWPosition(dpCFW, pos, min, max);
            wsThread->sendMessageToClient("CFWPositionMax:" + std::to_string(max));
            std::string cfwList = Tools::readCFWList(std::string(dpCFW->getDeviceName()));
            if (!cfwList.empty()) {
                wsThread->sendMessageToClient("getCFWList:" + cfwList);
            }
        }
    } else if (message == "ClearCalibrationData") {
        ClearCalibrationData = true;
        std::cout << "ClearCalibrationData: " << ClearCalibrationData << std::endl;
    } else if (message == "GuiderSwitch") {
        if (isGuiding) {
            isGuiding = false;
            call_phd_StopLooping();
            wsThread->sendMessageToClient("GuiderStatus:false");
        } else {
            isGuiding = true;
            if (ClearCalibrationData) {
                ClearCalibrationData = false;
                call_phd_ClearCalibration();
            }
            call_phd_StartLooping();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            call_phd_AutoFindStar();
            call_phd_StartGuiding();
            wsThread->sendMessageToClient("GuiderStatus:true");
        }
    } else if (parts.size() == 2 && trim(parts[0]) == "GuiderExpTimeSwitch") {
        call_phd_setExposureTime(std::stoi(trim(parts[1])));
    } else if (message == "getGuiderStatus") {
        if (isGuiding) {
            wsThread->sendMessageToClient("GuiderStatus:true");
        } else {
            wsThread->sendMessageToClient("GuiderStatus:false");
        }
    } else if (parts.size() == 4 && trim(parts[0]) == "SolveSYNC") {
        glFocalLength = std::stoi(trim(parts[1]));
        glCameraSize_width = std::stod(trim(parts[2]));
        glCameraSize_height = std::stod(trim(parts[3]));
        TelescopeControl_SolveSYNC();
    } else if (message == "ClearDataPoints") {
        dataPoints.clear();
    } else if (message == "ShowAllImageFolder") {
        std::string allFile = GetAllFile();
        std::cout << allFile << std::endl;
        wsThread->sendMessageToClient("ShowAllImageFolder:" + allFile);
    } else if (parts.size() == 2 && trim(parts[0]) == "MoveFileToUSB") {
        std::vector<std::string> ImagePath = parseString(parts[1], ImageSaveBasePath);
        RemoveImageToUsb(ImagePath);
    } else if (parts.size() == 2 && trim(parts[0]) == "DeleteFile") {
        std::vector<std::string> ImagePath = parseString(parts[1], ImageSaveBasePath);
        DeleteImage(ImagePath);
    } else if (message == "USBCheck") {
        USBCheck();
    }

}

auto Peer::handleTextMessage(const oatpp::Object<MessageDto>& message)
    -> oatpp::async::CoroutineStarter {
    class SendMessageCoroutine
        : public oatpp::async::Coroutine<SendMessageCoroutine> {
    private:
        oatpp::async::Lock* m_lock_;
        std::shared_ptr<AsyncWebSocket> m_websocket_;
        oatpp::String m_message_;
        json m_response_;
        json m_jsonData_;

    public:
        SendMessageCoroutine(oatpp::async::Lock* lock,
                             const std::shared_ptr<AsyncWebSocket>& websocket,
                             oatpp::String message)
            : m_lock_(lock),
              m_websocket_(websocket),
              m_message_(std::move(message)) {}

        auto act() -> Action override {
            auto command = m_message_.getValue("");
            try {
                m_jsonData_ = json::parse(m_message_);
                yieldTo(&SendMessageCoroutine::process);
            } catch (const json::parse_error& e) {
                m_response_["error"] = "Invalid JSON";
                m_response_["message"] = e.what();
            }
            yieldTo(&SendMessageCoroutine::send);
        }

        auto process()  -> Action{
            if (!m_jsonData_.contains("name")) {

            }
        }

        auto send() -> Action {
            return oatpp::async::synchronize(
                       m_lock_,
                       m_websocket_->sendOneFrameTextAsync(m_response_.dump()))
                .next(finish());
        }

    };
    if (m_socket_) {
        m_asyncExecutor->execute<SendMessageCoroutine>(&m_writeLock_, m_socket_,
                                                       message->message);
    }
}

auto Peer::handleMessage(const oatpp::Object<MessageDto>& message)
    -> oatpp::async::CoroutineStarter {
    if (!message->code) {
        return onApiError("No message code provided.");
    }

    switch (*message->code) {
        case MessageCodes::CODE_PEER_MESSAGE:
            m_room_->addHistoryMessage(message);
            m_room_->sendMessageAsync(message);
            ++m_statistics->EVENT_PEER_SEND_MESSAGE;
            break;

        case MessageCodes::CODE_PEER_COMMAND:
            handleTextMessage(message);
            break;

        case MessageCodes::CODE_PEER_IS_TYPING:
            m_room_->sendMessageAsync(message);
            break;

        case MessageCodes::CODE_FILE_SHARE:
            return handleFilesMessage(message);

        case MessageCodes::CODE_FILE_CHUNK_DATA:
            return handleFileChunkMessage(message);

        default:
            return onApiError("Invalid client message code.");
    }

    return nullptr;
}

auto Peer::getRoom() -> std::shared_ptr<Room> { return m_room_; }

auto Peer::getNickname() -> oatpp::String { return m_nickname_; }

auto Peer::getPeerId() -> v_int64 { return m_peerId_; }

void Peer::addFile(const std::shared_ptr<File>& file) {
    m_files_.push_back(file);
}

auto Peer::getFiles() -> const std::list<std::shared_ptr<File>>& {
    return m_files_;
}

void Peer::invalidateSocket() {
    if (m_socket_) {
        m_socket_->getConnection().invalidate();
    }
    m_socket_.reset();
}

auto Peer::onPing(const std::shared_ptr<AsyncWebSocket>& socket,
                  const oatpp::String& message)
    -> oatpp::async::CoroutineStarter {
    return oatpp::async::synchronize(&m_writeLock_,
                                     socket->sendPongAsync(message));
}

auto Peer::onPong(const std::shared_ptr<AsyncWebSocket>& socket,
                  const oatpp::String& message)
    -> oatpp::async::CoroutineStarter {
    --m_pingPoingCounter_;
    return nullptr;  // do nothing
}

auto Peer::onClose(const std::shared_ptr<AsyncWebSocket>& socket, v_uint16 code,
                   const oatpp::String& message)
    -> oatpp::async::CoroutineStarter {
    return nullptr;  // do nothing
}

auto Peer::readMessage(const std::shared_ptr<AsyncWebSocket>& socket,
                       v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
    -> oatpp::async::CoroutineStarter {
    if (m_messageBuffer_.getCurrentPosition() + size >
        m_appConfig->maxMessageSizeBytes) {
        return onApiError("Message size exceeds max allowed size.");
    }

    if (size == 0) {  // message transfer finished

        auto wholeMessage = m_messageBuffer_.toString();
        m_messageBuffer_.setCurrentPosition(0);

        oatpp::Object<MessageDto> message;

        try {
            message = m_objectMapper->readFromString<oatpp::Object<MessageDto>>(
                wholeMessage);
        } catch (const std::runtime_error& e) {
            return onApiError("Can't parse message");
        }

        message->peerName = m_nickname_;
        message->peerId = m_peerId_;
        message->timestamp = oatpp::Environment::getMicroTickCount();

        return handleMessage(message);
    }
    if (size > 0) {  // message frame received
        m_messageBuffer_.writeSimple(data, size);
    }

    return nullptr;  // do nothing
}