#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

#include <functional>
#include <string>
#include <condition_variable>
#include <map>
#include <atomic>
#include <thread>

#include "atom/type/json.hpp"
using json = nlohmann::json;

/**
 * @brief 类 VCommandDispatcher 负责命令的派发和处理。
 */
class CommandDispatcher
{
public:
    /**
     * @brief HandlerFunc 是用于处理命令的函数类型。
     *
     * 该函数应该接受一个 `json` 类型的参数，表示命令所携带的数据。
     */
    using HandlerFunc = std::function<void(const json &)>;

    /**
     * @brief RegisterHandler 函数用于将一个命令处理程序注册到 `CommandDispatcher` 中。
     *
     * @tparam ClassType 命令处理程序所属的类类型。
     * @param name 命令的名称。
     * @param handler 处理命令的成员函数指针。
     * @param instance 处理命令的对象指针。
     */
    template <typename ClassType>
    void RegisterHandler(const std::string &name, void (ClassType::*handler)(const json &), ClassType *instance)
    {
        auto hash_value = Djb2Hash(name.c_str());
        handlers_[hash_value] = std::bind(handler, instance, std::placeholders::_1);
    }

    /**
     * @brief HasHandler 函数用于检查是否有名为 `name` 的命令处理程序。
     *
     * @param name 要检查的命令名称。
     * @return 如果存在名为 `name` 的命令处理程序，则返回 `true`；否则返回 `false`。
     */
    bool HasHandler(const std::string &name);

    /**
     * @brief Dispatch 函数用于派发一个命令，并将它交给相应的处理程序处理。
     *
     * @param name 要派发的命令的名称。
     * @param data 命令所携带的数据。
     */
    void Dispatch(const std::string &name, const json &data);

private:
    /**
     * @brief handlers_ 是一个哈希表，存储了所有已注册的命令处理程序。
     *
     * 键值为哈希值，值为命令处理程序本身。
     */
#if ENABLE_FASTHASH
    emhash8::HashMap<std::size_t, HandlerFunc> handlers_;
#else
    std::unordered_map<std::size_t, HandlerFunc> handlers_;
#endif

    /**
     * @brief Djb2Hash 函数是一个字符串哈希函数，用于将字符串转换成哈希值。
     *
     * @param str 要转换的字符串。
     * @return 转换后的哈希值。
     */
    static std::size_t Djb2Hash(const char *str);
};

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#endif

/**
 * @brief Represents a client socket for connecting to a server.
 */
class SocketClient
{
public:
    /**
     * @brief Default constructor.
     */
    SocketClient();

    /**
     * @brief Destructor.
     */
    ~SocketClient();

    /**
     * @brief Connects to the specified server.
     *
     * @param serverIP The IP address of the server.
     * @param serverPort The port number of the server.
     * @return `true` if the connection is successful, `false` otherwise.
     */
    bool Connect(const std::string &serverIP, int serverPort);

    /**
     * @brief Disconnects from the server.
     */
    void Disconnect();

    /**
     * @brief Sends a message to the server.
     *
     * @param message The message to send.
     */
    void Send(const std::string &message);

    /**
     * @brief Sets the message handler function to process received messages.
     *
     * @param handler The message handler function.
     */
    void SetMessageHandler(std::function<void(const json &)> handler);

    /**
     * @brief Checks if the client is currently connected to the server.
     *
     * @return `true` if connected, `false` otherwise.
     */
    bool IsConnected() const;

    /**
     * @brief Stops the receive thread.
     */
    void StopReceiveThread();

private:
    SOCKET socket_;                                    ///< The socket descriptor.
    std::thread receiveThread_;                        ///< The receive thread.
    std::function<void(const json &)> messageHandler_; ///< The message handler function.
    bool isRunning_;                                   ///< Flag to indicate if the client is running.
    std::mutex mutex_;                                 ///< Mutex for thread synchronization.
    std::condition_variable cv_;                       ///< Conditional variable for thread synchronization.

    /**
     * @brief The receive thread function.
     */
    void ReceiveThread();
};

class PHD2Client
{
public:
    PHD2Client();
    ~PHD2Client();

    bool connect(const std::string &host = "127.0.0.1", int port = 4400);
    bool disconnect();
    bool reconnect();
    bool is_connected();

    bool start_guiding();

    void parser_json(const json &message);

    void _version(const json &message);
    void _lock_position_set(const json &message);
    void _calibrating(const json &message);
    void _calibration_completed(const json &message);
    void _star_selected(const json &message);
    void _start_guiding(const json &message);
    void _paused(const json &message);
    void _start_calibration(const json &message);
    void _app_state(const json &message);
    void _calibration_failed(const json &message);
    void _calibration_data_flipped(const json &message);
    void _lock_position_shift_limit_reached(const json &message);
    void _looping_exposures(const json &message);
    void _looping_exposures_stopped(const json &message);
    void _settle_begin(const json &message);
    void _settling(const json &message);
    void _settle_done(const json &message);
    void _star_lost(const json &message);
    void _guiding_stopped(const json &message);
    void _resumed(const json &message);
    void _guide_step(const json &message);
    void _guiding_dithered(const json &message);
    void _lock_position_lost(const json &message);
    void _alert(const json &message);
    void _guide_param_change(const json &message);
    void _configuration_change(const json &message);

    json GenerateCommand(const std::string &command, const json &params);
    bool SendCommand(const json &command);

    bool GetProfiles();
    bool GetCurrentProfile();
    bool SetProfile(int profileId);
    bool generateProfile(const json &profile);
    bool exportProfile();

    bool connectDevice();
    bool disconnectDevice();
    bool reconnectDevice();
    bool checkConnected();

private:
    std::shared_ptr<SocketClient> phd2_client;
    std::unique_ptr<CommandDispatcher> m_CommandDispatcher;

    template <typename ClassType>
    void RegisterFunc(const std::string &name, void (ClassType::*handler)(const json &))
    {
        m_CommandDispatcher->RegisterHandler(name, handler, this);
    }

    bool RunFunc(const std::string &name, const json &params);

    std::atomic_bool _is_connected;

    std::string _host;
    std::string _lightguiderversion;
    std::string _subversion;
    std::string _msgversion;

    // 与选星和校准相关的变量
    std::map<std::string, double> _star_position;
    std::map<std::string, std::string> _calibrated_status;
    std::string _mount;
    bool _is_star_locked;
    bool _is_star_selected;
    bool _is_calibrating;
    bool _is_calibrated;
    bool _is_selected;

    std::string _current_profile;

    // 与循迹、反漂和向导错误相关的变量
    bool _is_guiding;
    bool _is_looping;
    bool _is_settling;
    bool _is_settled;
    int _guiding_error;
    std::map<std::string, double> _guiding_status;
    int _dither_dx;
    int _dither_dy;
    std::string _last_error;

    // 与标定过程相关的变量
    bool _is_calibration_flipped;
    std::string _calibrated_error;

    // 与调焦状态相关的变量
    std::map<std::string, double> _settle_status;
    std::string _settle_error;

    // 与星体丢失相关的变量
    std::map<std::string, double> _starlost_status;
    std::string _starlost_error;
};

#endif /* SOCKETCLIENT_H_ */
