

#ifndef CARBON_UNKNOWN_HPP
#define CARBON_UNKNOWN_HPP

namespace Carbon {
  namespace detail {
    struct Loadable_Module {
      Loadable_Module(const std::string &, const std::string &) {
#ifdef CARBON_NO_DYNLOAD
        throw Carbon::exception::load_module_error("Loadable module support was disabled (CARBON_NO_DYNLOAD)");
#else
        throw Carbon::exception::load_module_error("Loadable module support not available for your platform");
#endif
      }

      ModulePtr m_moduleptr;
    };
  } // namespace detail
} // namespace Carbon
#endif
