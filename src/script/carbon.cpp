#include "carbon.hpp"

#include "carbon/carbon.hpp"
#include "carbon/extra/math.hpp"
#include "carbon/extra/string.hpp"

#include "atom/log/loguru.hpp"

namespace Lithium
{
    // Pimpl implementation
    class CarbonScriptImpl
    {
    public:
        CarbonScriptImpl()
            : m_carbon(std::make_unique<Carbon::CarbonScript>())      {
        }

        void InitSubModules()
        {
            m_carbon->add(Carbon::extras::math::bootstrap());
            m_carbon->add(Carbon::extras::string_methods::bootstrap());

            // Add additional sub-modules if needed
        }

        void InitMyApp()
        {
            LOG_F(INFO, "CarbonScriptr initializing ...");
            InitSubModules();
            LOG_F(INFO, "CarbonScript initialized");
            //m_carbon->add_global(Carbon::var(MyApp), "app");
        }

        bool LoadScriptFile(const std::string &filename)
        {
            std::ifstream file(filename);
            if (file)
            {
                std::string script((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
                file.close();
                m_carbon->eval(script);
            }
            else
            {
                LOG_F(ERROR, "Failed to open script file: %s", filename.c_str());
                return false;
            }
            return true;
        }

        bool RunCommand(const std::string &command)
        {
            try
            {
                m_carbon->eval(command);
            }
            catch (Carbon::exception::eval_error &e)
            {
                LOG_F(ERROR, "Failed to eval %s : %s", e.filename.c_str(), e.what());
                return false;
            };
            return true;
        }

        bool RunScript(const std::string &filename)
        {
            try
            {
                m_carbon->eval_file(filename);
            }
            catch (Carbon::exception::eval_error &e)
            {
                LOG_F(ERROR, "Failed to run %s : %s", e.filename.c_str(), e.what());
                return false;
            }
            return true;
        }

        bool RunMultiCommand(const std::vector<std::string> &commands)
        {
            for (auto command : commands)
            {
                try
                {
                    m_carbon->eval(command);
                }
                catch (Carbon::exception::eval_error &e)
                {
                    LOG_F(ERROR, "Failed to run: %s", e.what());
                    return false;
                }
            }
            return true;
        }

    private:
        std::unique_ptr<Carbon::CarbonScript> m_carbon;
    };

    CarbonScript::CarbonScript()
        : impl_(std::make_unique<CarbonScriptImpl>())
    {
    }

    std::shared_ptr<CarbonScript> CarbonScript::createShared()
    {
        return std::make_shared<CarbonScript>();
    }

    void CarbonScript::Init()
    {
        impl_->InitMyApp();
    }

    bool CarbonScript::LoadScriptFile(const std::string &filename)
    {
        return impl_->LoadScriptFile(filename);
    }

    bool CarbonScript::RunCommand(const std::string &command)
    {
        return impl_->RunCommand(command);
    }

    bool CarbonScript::RunMultiCommand(const std::vector<std::string> &commands)
    {
        return impl_->RunMultiCommand(commands);
    }

    bool CarbonScript::RunScript(const std::string &filename)
    {
        return impl_->RunScript(filename);
    }

}
