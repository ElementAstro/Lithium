#define GET_PARAM(type, name)      \
    if (!params.contains(#name)) { \
        return false;              \
    }                              \
    type name = params[#name].get<type>();

#define TOGGLE_DEBUG(debug)                    \
    if (debug) {                               \
        LOG_F(INFO, "Debug mode is enabled");  \
        m_debug.store(true);                   \
    } else {                                   \
        LOG_F(INFO, "Debug mode is disabled"); \
        m_debug.store(false);                  \
    }

#define TOGGLE_TIMEOUT(timeout)   \
    if (timeout > 0) {            \
        m_timeout.store(timeout); \
    } else {                      \
        m_timeout.store(30);      \
    }