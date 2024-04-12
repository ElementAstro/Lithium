#include "carbon/carbon.hpp"

#include "base.hpp"
#include <any>

#include "atom/log/loguru.hpp"

using namespace Atom::Algorithm;

CARBON_MODULE_EXPORT {
    Carbon::ModulePtr exportModule([[maybe_unused]] const std::any &m_params) {
        try {
            Carbon::ModulePtr m_module = std::make_shared<Carbon::Module>();

            m_module->add(Carbon::fun(&base16Encode), "base16encode");
            m_module->add(Carbon::fun(&base16Decode), "base16decode");
            m_module->add(Carbon::fun(&base32Encode), "base32encode");
            m_module->add(Carbon::fun(&base32Decode), "base32decode");

            return m_module;
        } catch (const std::bad_any_cast &e) {
            LOG_F(ERROR, "Failed to load config manager");
            return nullptr;
        }
    }
}


