// indihub_agent.cpp
#include "indihub_agent.hpp"
#include "async_system_command.hpp"

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/env.hpp"

#include <sstream>

namespace {
constexpr const char* INDIHUB_AGENT_OFF = "off";
constexpr const char* INDIHUB_AGENT_DEFAULT_MODE = "solo";

std::string getConfigPath() {
    std::string configPath;
    try {
        atom::utils::Env env;
        configPath = env.getEnv("HOME");
        configPath += "/.indihub";
    } catch (const std::exception&) {
        configPath = "/tmp/indihub";
    }

    if (!atom::io::isFolderExists(configPath)) {
        atom::io::createDirectory(configPath);
    }

    return configPath + "/indihub.json";
}

const std::string INDIHUB_AGENT_CONFIG = getConfigPath();
}  // namespace

IndiHubAgent::IndiHubAgent(const std::string& web_addr,
                           const std::string& hostname, int port)
    : web_addr_(web_addr),
      hostname_(hostname),
      port_(port),
      mode_(INDIHUB_AGENT_OFF),
      async_cmd_(nullptr),
      command_thread_(nullptr) {}

IndiHubAgent::~IndiHubAgent() { stop(); }

void IndiHubAgent::run(const std::string& profile, const std::string& mode,
                       const std::string& conf) {
    std::stringstream cmd;
    cmd << "indihub-agent"
        << " -indi-server-manager=" << web_addr_ << " -indi-profile=" << profile
        << " -mode=" << mode << " -conf=" << conf
        << " -api-origins=" << hostname_ << ":" << port_ << "," << hostname_
        << ".local:" << port_ << " > /tmp/indihub-agent.log 2>&1 &";

    LOG_F(INFO, "Running command: {}", cmd.str());

    async_cmd_ = std::make_unique<AsyncSystemCommand>(cmd.str());
    command_thread_ =
        std::make_unique<std::thread>([this]() { async_cmd_->run(); });
}

void IndiHubAgent::start(const std::string& profile, const std::string& mode,
                         const std::string& conf) {
    if (isRunning()) {
        stop();
    }
    run(profile, mode, conf);
    mode_ = mode;
}

void IndiHubAgent::stop() {
    if (!async_cmd_) {
        LOG_F(INFO, "indihub_agent: not running");
        return;
    }

    try {
        async_cmd_->terminate();
        if (command_thread_ && command_thread_->joinable()) {
            command_thread_->join();
        }
        LOG_F(INFO, "indihub_agent: terminated successfully");
    } catch (const std::exception& e) {
        LOG_F(WARNING, "indihub_agent: termination failed with error {}",
              e.what());
    }
}

bool IndiHubAgent::isRunning() const {
    return async_cmd_ && async_cmd_->isRunning();
}

std::string IndiHubAgent::getMode() const { return mode_; }