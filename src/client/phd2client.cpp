#include "phd2client.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include "loguru/loguru.hpp"

#define CONNECT_CHECK(func)          \
    if (!phd2_client->IsConnected()) \
    {                                \
        return false;                \
    }                                \
    func

using json = nlohmann::json;

SocketClient::SocketClient()
    : socket_(INVALID_SOCKET), isRunning_(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG_F(ERROR, "Failed to initialize Winsock");
        throw std::runtime_error("Failed to initialize Winsock");
    }
#endif
}

SocketClient::~SocketClient()
{
    Disconnect();
}

bool SocketClient::Connect(const std::string &serverIP, int serverPort)
{
#ifdef _WIN32
    socket_ = socket(AF_INET, SOCK_STREAM, 0);
#else
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
    if (socket_ == INVALID_SOCKET)
    {
        LOG_F(ERROR, "Failed to create socket");
#ifdef _WIN32
        WSACleanup();
#endif
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

#ifdef _WIN32
    if (InetPton(AF_INET, serverIP.c_str(), &(serverAddress.sin_addr)) <= 0)
    {
        LOG_F(ERROR, "Invalid server IP address");
        closesocket(socket_);
        WSACleanup();
        throw std::runtime_error("Invalid server IP address");
    }
#else
    if (inet_pton(AF_INET, serverIP.c_str(), &(serverAddress.sin_addr)) <= 0)
    {
        LOG_F(ERROR, "Invalid server IP address");
        throw std::runtime_error("Invalid server IP address");
    }
#endif

    if (connect(socket_, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        LOG_F(ERROR, "Failed to connect to server");
        throw std::runtime_error("Failed to connect to server");
    }

    isRunning_ = true;
    receiveThread_ = std::thread([&]()
                                 { ReceiveThread(); });

    return true;
}

void SocketClient::Disconnect()
{
    if (socket_ != INVALID_SOCKET)
    {
#ifdef _WIN32
        closesocket(socket_);
#else
        close(socket_);
#endif
        socket_ = INVALID_SOCKET;
    }

    if (isRunning_)
    {
        isRunning_ = false;
        if (receiveThread_.joinable())
        {
            receiveThread_.join();
        }
    }
}

void SocketClient::Send(const std::string &message)
{
    if (socket_ == INVALID_SOCKET)
    {
        LOG_F(ERROR, "Not connected to server");
        return;
    }

    if (send(socket_, message.c_str(), message.length(), 0) < 0)
    {
        LOG_F(ERROR, "Failed to send data");
        throw std::runtime_error("Failed to send data");
    }
}

void SocketClient::SetMessageHandler(std::function<void(const json &)> handler)
{
    messageHandler_ = std::move(handler);
}

bool SocketClient::IsConnected() const
{
    return socket_ != INVALID_SOCKET;
}

void SocketClient::StopReceiveThread()
{
    if (isRunning_)
    {
        isRunning_ = false;

        // 等待接收线程退出
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock);
    }
}

void SocketClient::ReceiveThread()
{
    while (isRunning_)
    {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        int bytesRead = recv(socket_, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0)
        {
            if (bytesRead < 0)
            {
                LOG_F(ERROR, "Failed to receive data: %d", bytesRead);
            }
            else
            {
                DLOG_F(INFO, "Connection closed by server");
            }
            break;
        }

        std::string receivedData(buffer);
        std::istringstream iss(receivedData);
        std::string line;

        while (std::getline(iss, line))
        {
            json jsonData;
            try
            {
                jsonData = json::parse(line);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to parse JSON data: " << e.what() << std::endl;
                continue;
            }

            // 调用消息处理函数
            if (messageHandler_)
            {
                messageHandler_(jsonData);
            }
        }
    }

    // 停止接收线程后通知等待的条件变量
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.notify_all();
}

PHD2Client::PHD2Client()
{
    phd2_client = std::make_shared<SocketClient>();

    _is_star_locked = false;
    _is_star_selected = false;
    _is_calibrating = false;
    _is_calibrated = false;
    _is_selected = false;

    _is_guiding = false;
    _is_looping = false;
    _is_settling = false;
    _is_settled = false;
    _guiding_error = 0;
    _dither_dx = 0;
    _dither_dy = 0;
    _last_error = "";

    _is_calibration_flipped = false;
    _calibrated_error = "";

    _settle_error = "";
    _starlost_error = "";

    RegisterFunc("Version", &PHD2Client::_version);
    RegisterFunc("LockPositionSet", &PHD2Client::_lock_position_set);
    RegisterFunc("Calibrating", &PHD2Client::_calibrating);
    RegisterFunc("CalibrationComplete", &PHD2Client::_calibration_completed);
    RegisterFunc("StarSelected", &PHD2Client::_star_selected);
    RegisterFunc("StartGuiding", &PHD2Client::_start_guiding);
    RegisterFunc("Paused", &PHD2Client::_paused);
    RegisterFunc("StartCalibration", &PHD2Client::_start_calibration);
    RegisterFunc("AppState", &PHD2Client::_app_state);
    RegisterFunc("CalibrationFailed", &PHD2Client::_calibration_failed);
    RegisterFunc("CalibrationDataFlipped", &PHD2Client::_calibration_data_flipped);
    RegisterFunc("LockPositionShiftLimitReached", &PHD2Client::_lock_position_shift_limit_reached);
    RegisterFunc("LoopingExposures", &PHD2Client::_looping_exposures);
    RegisterFunc("LoopingExposuresStopped", &PHD2Client::_looping_exposures_stopped);
    RegisterFunc("SettleBegin", &PHD2Client::_settle_begin);
    RegisterFunc("Settling", &PHD2Client::_settling);
    RegisterFunc("SettleDone", &PHD2Client::_settle_done);
    RegisterFunc("StarLost", &PHD2Client::_star_lost);
    RegisterFunc("GuidingStopped", &PHD2Client::_guiding_stopped);
    RegisterFunc("Resumed", &PHD2Client::_resumed);
    RegisterFunc("GuideStep", &PHD2Client::_guide_step);
    RegisterFunc("GuidingDithered", &PHD2Client::_guiding_dithered);
    RegisterFunc("LockPositionLost", &PHD2Client::_lock_position_lost);
    RegisterFunc("Alert", &PHD2Client::_alert);
    RegisterFunc("GuideParamChange", &PHD2Client::_guide_param_change);
    RegisterFunc("ConfigurationChange", &PHD2Client::_configuration_change);

    phd2_client->SetMessageHandler(std::bind(&PHD2Client::parser_json, this, std::placeholders::_1));
}

PHD2Client::~PHD2Client()
{
}

bool PHD2Client::RunFunc(const std::string &name, const json &params)
{
    if (m_CommandDispatcher->HasHandler(name))
    {
        m_CommandDispatcher->Dispatch(name, params);
        return true;
    }
    return false;
}

bool PHD2Client::connect(const std::string &host, int port)
{
    return true;
}

bool PHD2Client::disconnect()
{
    return true;
}

bool PHD2Client::reconnect()
{
    return true;
}

bool PHD2Client::is_connected()
{
    return _is_connected.load();
}

void PHD2Client::parser_json(const json &message)
{
    if (!message.empty())
    {
        if (message.contains("Event"))
        {
            const std::string name = message["Event"].get<std::string>();
            if (m_CommandDispatcher->HasHandler(name))
            {
                m_CommandDispatcher->Dispatch(name, message);
            }
        }
    }
}

void PHD2Client::_version(const json &message)
{
    _host = message["Host"];
    _lightguiderversion = message["LGuiderVersion"];
    _subversion = message["LGuiderSubver"];
    _msgversion = message["MsgVersion"];
}

void PHD2Client::_lock_position_set(const json &message)
{
    _star_position["X"] = message["X"];
    _star_position["Y"] = message["Y"];
    _is_star_locked = true;
}

void PHD2Client::_calibrating(const json &message)
{
    _calibrated_status["direction"] = message["dir"];
    _calibrated_status["distance"] = message["dist"];
    _calibrated_status["dx"] = message["dx"];
    _calibrated_status["dy"] = message["dy"];
    _calibrated_status["position"] = message["pos"];
    _calibrated_status["stop"] = message["step"];
    _calibrated_status["state"] = message["State"];
}

void PHD2Client::_calibration_completed(const json &message)
{
    _mount = message["Mount"];
}

void PHD2Client::_star_selected(const json &message)
{
    _star_position["X"] = message["X"];
    _star_position["Y"] = message["Y"];
    _is_star_selected = true;
}

void PHD2Client::_start_guiding(const json &message)
{
    _is_guiding = true;
}

void PHD2Client::_paused(const json &message)
{
    _is_guiding = false;
    _is_calibrating = false;
}

void PHD2Client::_start_calibration(const json &message)
{
    _mount = message["Mount"];
    _is_calibrating = true;
    _is_guiding = false;
}

void PHD2Client::_app_state(const json &message)
{
    std::string state = message["State"];

    if (state == "Stopped")
    {
        _is_calibrating = false;
        _is_looping = false;
        _is_guiding = false;
        _is_settling = false;
    }
    else if (state == "Selected")
    {
        _is_selected = true;
        _is_looping = false;
        _is_guiding = false;
        _is_settling = false;
        _is_calibrating = false;
    }
    else if (state == "Calibrating")
    {
        _is_calibrating = true;
        _is_guiding = false;
    }
    else if (state == "Guiding")
    {
        _is_guiding = true;
        _is_calibrating = false;
    }
    else if (state == "LostLock")
    {
        _is_guiding = true;
        _is_star_locked = false;
    }
    else if (state == "Paused")
    {
        _is_guiding = false;
        _is_calibrating = false;
    }
    else if (state == "Looping")
    {
        _is_looping = true;
    }
}

void PHD2Client::_calibration_failed(const json &message)
{
    _calibrated_error = message["Reason"];
    _is_calibrating = false;
    _is_calibrated = false;
}

void PHD2Client::_calibration_data_flipped(const json &message)
{
    _is_calibration_flipped = true;
}

void PHD2Client::_lock_position_shift_limit_reached(const json &message)
{
    DLOG_F(WARNING, "Star locked position reached the edge of the camera frame");
}

void PHD2Client::_looping_exposures(const json &message)
{
    _is_looping = true;
}

void PHD2Client::_looping_exposures_stopped(const json &message)
{
    _is_looping = false;
}

void PHD2Client::_settle_begin(const json &message)
{
    _is_settling = true;
}

void PHD2Client::_settling(const json &message)
{
    _settle_status["distance"] = message["Distance"];
    _settle_status["time"] = message["SettleTime"];
    _settle_status["locked"] = message["StarLocked"];
    _is_settling = true;
}

void PHD2Client::_settle_done(const json &message)
{
    int status = message["Status"];

    if (status == 0)
    {
        DLOG_F(INFO, "Settle succeeded");
        _is_settled = true;
    }
    else
    {
        _settle_error = message["Error"];
        DLOG_F(INFO, "Settle failed, error: {}", message["Error"].dump(4));
        _is_settled = false;
    }
    _is_settling = false;
}

void PHD2Client::_star_lost(const json &message)
{
    _starlost_status["snr"] = message["SNR"];
    _starlost_status["star_mass"] = message["StarMass"];
    _starlost_status["avg_dist"] = message["AvgDist"];
    _starlost_error = message["Status"];

    LOG_F(ERROR, "Star Lost, SNR: {}, StarMass: {}, AvgDist: {}",
          _starlost_status["snr"], _starlost_status["star_mass"], _starlost_status["avg_dist"]);

    _is_guiding = false;
    _is_calibrating = false;
}

void PHD2Client::_guiding_stopped(const json &message)
{
    _is_guiding = false;
    DLOG_F(INFO, "Guiding Stopped");
}

void PHD2Client::_resumed(const json &message)
{
    DLOG_F(INFO, "Guiding Resumed");
    _is_guiding = true;
}

void PHD2Client::_guide_step(const json &message)
{
    _mount = message["Mount"];
    DLOG_F(INFO, "Guide step mount: %d", _mount);
    _guiding_error = message["ErrorCode"];
    DLOG_F(INFO, "Guide step error: %d", _guiding_error);

    _guiding_status["avg_dist"] = message["AvgDist"];
    DLOG_F(INFO, "Guide step average distance: %f", _guiding_status["avg_dist"]);

    _guiding_status["dx"] = message["dx"];
    DLOG_F(INFO, "Guide step dx: %f", _guiding_status["dx"]);
    _guiding_status["dy"] = message["dy"];
    DLOG_F(INFO, "Guide step dy: %f", _guiding_status["dy"]);

    _guiding_status["ra_raw_distance"] = message["RADistanceRaw"];
    DLOG_F(INFO, "Guide step RADistanceRaw: %f", _guiding_status["ra_raw_distance"]);
    _guiding_status["dec_raw_distance"] = message["DECDistanceRaw"];
    DLOG_F(INFO, "Guide step DECDistanceRaw: %f", _guiding_status["dec_raw_distance"]);

    _guiding_status["ra_distance"] = message["RADistanceGuide"];
    DLOG_F(INFO, "Guide step RADistanceGuide: %f", _guiding_status["ra_distance"]);
    _guiding_status["dec_distance"] = message["DECDistanceGuide"];
    DLOG_F(INFO, "Guide step DECDistanceGuide: %f", _guiding_status["dec_distance"]);

    _guiding_status["ra_duration"] = message["RADuration"];
    DLOG_F(INFO, "Guide step RADuration: %f", _guiding_status["ra_duration"]);
    _guiding_status["dec_duration"] = message["DECDuration"];
    DLOG_F(INFO, "Guide step DECDuration: %f", _guiding_status["dec_duration"]);

    _guiding_status["ra_direction"] = message["RADirection"];
    DLOG_F(INFO, "Guide step RADirection: %f", _guiding_status["ra_direction"]);
    _guiding_status["dec_direction"] = message["DECDirection"];
    DLOG_F(INFO, "Guide step DECDirection: %f", _guiding_status["dec_direction"]);

    _guiding_status["snr"] = message["SNR"];
    DLOG_F(INFO, "Guide step SNR: %f", _guiding_status["snr"]);
    _guiding_status["starmass"] = message["StarMass"];
    DLOG_F(INFO, "Guide step StarMass: %f", _guiding_status["starmass"]);
    _guiding_status["hfd"] = message["HFD"];
    DLOG_F(INFO, "Guide step HFD: %f", _guiding_status["hfd"]);
}

void PHD2Client::_guiding_dithered(const json &message)
{
    _dither_dx = message["dx"];
    _dither_dy = message["dy"];
}

void PHD2Client::_lock_position_lost(const json &message)
{
    _is_star_locked = true;
    LOG_F(ERROR, "Star Lock Position Lost");
}

void PHD2Client::_alert(const json &message)
{
    _last_error = message["Msg"];
    LOG_F(ERROR, "Alert: %s", _last_error.c_str());
}

void PHD2Client::_guide_param_change(const json &message)
{
}

void PHD2Client::_configuration_change(const json &message)
{
}

json PHD2Client::GenerateCommand(const std::string &command, const json &params)
{
    json res = {{"method", command}, {"id", 1}};
    if (!params.empty())
    {
        res["params"] = params;
    }
    return res;
}

bool PHD2Client::SendCommand(const json &command)
{
    CONNECT_CHECK(
        phd2_client->Send(command.dump());
        return true;)
}

bool PHD2Client::GetProfiles()
{
    CONNECT_CHECK(
        return SendCommand(GenerateCommand("get_profiles", {}));)
}

bool PHD2Client::GetCurrentProfile()
{
    CONNECT_CHECK(
        return SendCommand(GenerateCommand("get_profile", {}));)
}

bool PHD2Client::SetProfile(int profileId)
{
    CONNECT_CHECK(
        return SendCommand(GenerateCommand("set_profile", {{"profile_id", profileId}}));)
}

bool PHD2Client::generateProfile(const json &profile)
{
    CONNECT_CHECK(
        std::string name = profile["name"];
        int id = profile["id"];
        std::string camera = profile["camera"];
        std::string mount = profile["mount"];

        if (name.empty() || id == 0 || camera.empty() || mount.empty()) {
            return false;
        }

        return true;)
}

bool PHD2Client::exportProfile()
{
    CONNECT_CHECK(
        return SendCommand(GenerateCommand("export_config_settings", {}));)
}

bool PHD2Client::connectDevice()
{
    CONNECT_CHECK(
        if (_current_profile.empty()) {
            return false;
        }

        json command;
        command["command"] = "set_connected";
        command["params"] = true;

        return SendCommand(command);)
}

bool PHD2Client::disconnectDevice()
{
    CONNECT_CHECK(
        json command;
        command["command"] = "set_connected";
        command["params"] = false;
        return SendCommand(command);)
}

bool PHD2Client::reconnectDevice()
{
    CONNECT_CHECK(
        if (disconnectDevice()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return connectDevice();
        } return false;)
}

bool PHD2Client::checkConnected()
{
    CONNECT_CHECK(
        json command;
        command["command"] = "get_connected";
        command["params"] = json::object();
        return SendCommand(command);)
}