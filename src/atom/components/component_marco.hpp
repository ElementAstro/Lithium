#define GET_COMPONENT_ARG(name, type)                              \
    if (!m_params.get<type>(#name).has_value())                    \
    {                                                              \
        LOG_F(ERROR, "{}: Missing argument: {}", __func__, #name); \
        return;                                                    \
    }                                                              \
    const type name = m_params.get<type>(#name).value();
