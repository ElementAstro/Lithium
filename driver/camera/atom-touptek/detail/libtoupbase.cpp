/*
 * libtoupbase.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 20234-3-1

Description: ToupBase Library

**************************************************/

#include "libtoupbase.hpp"
#include <unordered_map>

std::string errorCodes(HRESULT rc) {
    static std::unordered_map<HRESULT, std::string> errCodes = {
        {0x00000000, "Success"},
        {0x00000001, "Yet another success"},
        {0x8000ffff, "Catastrophic failure"},
        {0x80004001, "Not supported or not implemented"},
        {0x80070005, "Permission denied"},
        {0x8007000e, "Out of memory"},
        {0x80070057, "One or more arguments are not valid"},
        {0x80004003, "Pointer that is not valid"},
        {0x80004005, "Generic failure"},
        {0x8001010e, "Call function in the wrong thread"},
        {0x8007001f, "Device not functioning"},
        {0x800700aa, "The requested resource is in use"},
        {0x8000000a,
         "The data necessary to complete this operation is not yet available"},
        {0x8001011f,
         "This operation returned because the timeout period expired"}};

    const std::unordered_map<HRESULT, std::string>::iterator it =
        errCodes.find(rc);
    if (it != errCodes.end())
        return it->second;
    else {
        char str[256];
        sprintf(str, "Unknown error: 0x%08x", rc);
        return std::string(str);
    }
}