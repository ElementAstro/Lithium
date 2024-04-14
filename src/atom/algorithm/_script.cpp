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
            m_module->add(Carbon::fun(&base64Encode), "base64encode");
            m_module->add(Carbon::fun(&base64Decode), "base64decode");
            m_module->add(Carbon::fun(&base85Encode), "base85encode");
            m_module->add(Carbon::fun(&base85Decode), "base85decode");
            m_module->add(Carbon::fun(&base91Encode), "base91encode");
            m_module->add(Carbon::fun(&base91Decode), "base91decode");
            m_module->add(Carbon::fun(&base128Encode), "base128encode");
            m_module->add(Carbon::fun(&base128Decode), "base128decode");

            return m_module;
        } catch (const std::bad_any_cast &e) {
            LOG_F(ERROR, "Failed to load config manager");
            return nullptr;
        }
    }
}


