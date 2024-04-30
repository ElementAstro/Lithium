#include "carbon.hpp"

#include "carbon/carbon.hpp"
#include "carbon/extra/math.hpp"
#include "carbon/extra/stdlib.hpp"
#include "carbon/extra/string.hpp"

#include "atom/algorithm/_script.hpp"
#include "atom/error/_script.hpp"
#include "atom/io/_script.hpp"
#include "atom/system/_script.hpp"
#include "atom/type/_script.hpp"

#include "config/_script.hpp"

#include "atom/async/pool.hpp"
#include "atom/type/noncopyable.hpp"
#include "atom/log/loguru.hpp"

namespace lithium {
// Pimpl implementation
class CarbonScriptImpl : public NonCopyable {
public:
    explicit CarbonScriptImpl(const fs::path &script_dir = "./script",
                              int cache_expiry_seconds = 60)
        : m_carbon(std::make_unique<Carbon::CarbonScript>()) {
        if (!atom::io::isFolderExists(script_dir)) {
            throw std::runtime_error("Script directory does not exist: " +
                                     script_dir_.string());
        }
        cache_cleanup_thread_ =
            std::thread(&CarbonScriptImpl::CacheCleanupLoop, this);
        LOG_F(INFO, "CarbonScript initializing ...");
        InitMyApp();
        LOG_F(INFO, "CarbonScript initialized");
    }

    void InitSubModules() {
        m_carbon->add(Carbon::extras::math::bootstrap());
        m_carbon->add(Carbon::extras::string_methods::bootstrap());
        m_carbon->add(Carbon::extras::stdlib::bootstrap());

        m_carbon->add(Atom::_Script::Algorithm::bootstrap());
        m_carbon->add(Atom::_Script::Error::bootstrap());
        m_carbon->add(Atom::_Script::IO::bootstrap());
        m_carbon->add(Atom::_Script::System::bootstrap());
        m_carbon->add(Atom::_Script::Type::bootstrap());

        m_carbon->add(lithium::_Script::Config::bootstrap());

        // Add additional sub-modules if needed
    }

    void InitMyApp() {
        LOG_F(INFO, "CarbonScriptr initializing ...");
        InitSubModules();
        LOG_F(INFO, "CarbonScript initialized");
        // m_carbon->add_global(Carbon::var(MyApp), "app");
    }

    bool LoadScriptFile(const std::string &filename) {
        std::ifstream file(filename);
        if (file) {
            std::string script((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();
            m_carbon->eval(script);
        } else {
            LOG_F(ERROR, "Failed to open script file: {}", filename.c_str());
            return false;
        }
        return true;
    }

    bool RunCommand(const std::string &command) {
        try {
            m_carbon->eval(command);
        } catch (Carbon::exception::eval_error &e) {
            LOG_F(ERROR, "Failed to eval {} : {}", e.filename.c_str(),
                  e.what());
            return false;
        };
        return true;
    }

    bool RunScript(const std::string &filename) {
        try {
            m_carbon->eval_file(filename);
        } catch (Carbon::exception::eval_error &e) {
            LOG_F(ERROR, "Failed to run {} : {}", e.filename.c_str(), e.what());
            return false;
        }
        return true;
    }

    bool RunMultiCommand(const std::vector<std::string> &commands) {
        for (auto command : commands) {
            try {
                m_carbon->eval(command);
            } catch (Carbon::exception::eval_error &e) {
                LOG_F(ERROR, "Failed to run: {}", e.what());
                return false;
            }
        }
        return true;
    }

    void cacheScript(const std::string &script_name) {
        auto script_path = script_dir_ / (script_name + ".li");

        if (!atom::io::isFileExists(script_path)) {
            LOG_F(ERROR, "Script not found: {}", script_name);
            return;
        }
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = script_cache_.find(script_path);
        if (it == script_cache_.end()) {
            std::ifstream script_file(script_path);
            std::string script_content(
                (std::istreambuf_iterator<char>(script_file)),
                std::istreambuf_iterator<char>());
            m_carbon->eval(script_content);
            script_cache_[script_path] = std::chrono::steady_clock::now();
        } else {
            script_cache_[script_path] = std::chrono::steady_clock::now();
        }
    }

private:
    void CacheCleanupLoop() {
        while (!stop_cleanup_thread_) {
            std::this_thread::sleep_for(
                std::chrono::seconds(cache_expiry_seconds_));
            std::lock_guard<std::mutex> lock(cache_mutex_);
            auto now = std::chrono::steady_clock::now();
            for (auto it = script_cache_.begin(); it != script_cache_.end();) {
                if (now - it->second >
                    std::chrono::seconds(cache_expiry_seconds_)) {
                    it = script_cache_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

private:
    std::unique_ptr<Carbon::CarbonScript> m_carbon;

    fs::path script_dir_;
    std::unordered_map<fs::path, std::chrono::steady_clock::time_point>
        script_cache_;
    std::mutex cache_mutex_;
    std::thread cache_cleanup_thread_;
    bool stop_cleanup_thread_ = false;
    int cache_expiry_seconds_;
};

CarbonScript::CarbonScript() : impl_(std::make_unique<CarbonScriptImpl>()) {}

std::shared_ptr<CarbonScript> CarbonScript::createShared() {
    return std::make_shared<CarbonScript>();
}

void CarbonScript::Init() { impl_->InitMyApp(); }

bool CarbonScript::LoadScriptFile(const std::string &filename) {
    return impl_->LoadScriptFile(filename);
}

bool CarbonScript::RunCommand(const std::string &command) {
    return impl_->RunCommand(command);
}

bool CarbonScript::RunMultiCommand(const std::vector<std::string> &commands) {
    return impl_->RunMultiCommand(commands);
}

bool CarbonScript::RunScript(const std::string &filename) {
    return impl_->RunScript(filename);
}

}  // namespace lithium
