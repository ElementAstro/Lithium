#pragma once

#define GET_PARAM_VALUE(param_name, param_type, var_name) \
    var_name = m_params[param_name].get<param_type>();

#define GET_STRING_PARAM_VALUE(param_name, var_name) \
    std::string var_name = m_params[param_name].get<std::string>();

#define SET_STRING_PARAM_VALUE(param_name, var_name) \
    var_name = m_params[param_name].get<std::string>();

#define GET_INT_PARAM_VALUE(param_name, var_name) \
    int var_name = m_params[param_name].get<int>();

#define SET_INT_PARAM_VALUE(param_name, var_name) \
    var_name = m_params[param_name].get<int>();

#define GET_FLOAT_PARAM_VALUE(param_name, var_name) \
    float var_name = m_params[param_name].get<float>();

#define SET_FLOAT_PARAM_VALUE(param_name, var_name) \
    var_name = m_params[param_name].get<float>();

#define GET_DOUBLE_PARAM_VALUE(param_name, var_name) \
    double var_name = m_params[param_name].get<double>();

#define SET_DOUBLE_PARAM_VALUE(param_name, var_name) \
    var_name = m_params[param_name].get<double>();

#define GET_BOOL_PARAM_VALUE(param_name, var_name) \
    bool var_name = m_params[param_name].get<bool>();

#define SET_BOOL_PARAM_VALUE(param_name, var_name) \
    var_name = m_params[param_name].get<bool>();

#define CHECK_PARAM_EXISTS(param_name)                                                   \
    if (!m_params.contains(#param_name))                                                 \
    {                                                                                    \
        RESPONSE_ERROR(res, ServerError::MissingParameters, #param_name " is required"); \
    }
