#include "bios.hpp"

#ifdef _WIN32
#include <comdef.h>
#include <wbemidl.h>

#if defined(_MSC_VER)
#pragma comment(lib, "wbemuuid.lib")
#endif
#endif

#include <memory>
#include <sstream>

#include "atom/log/loguru.hpp"

namespace atom::system {
#ifdef _WIN32
auto getBiosInfo() -> BiosInfoData {
    LOG_F(INFO, "Starting getBiosInfo function");
    BiosInfoData biosInfo = {"", "", ""};  // Initialize structure
    HRESULT hresult;

    // Initialize COM library
    hresult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hresult)) {
        LOG_F(ERROR, "Failed to initialize COM library. Error code = 0x{:x}",
              hresult);
        return biosInfo;
    }

    // Set COM security
    hresult = CoInitializeSecurity(
        nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);

    if (FAILED(hresult)) {
        LOG_F(ERROR, "Failed to initialize security. Error code = 0x{:x}",
              hresult);
        CoUninitialize();
        return biosInfo;
    }

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = nullptr;
    hresult = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                               IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hresult)) {
        LOG_F(ERROR,
              "Failed to create IWbemLocator object. Error code = 0x{:x}",
              hresult);
        CoUninitialize();
        return biosInfo;
    }

    // Connect to WMI namespace
    IWbemServices *pSvc = nullptr;
    hresult = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0,
                                  0, 0, 0, &pSvc);
    if (FAILED(hresult)) {
        LOG_F(ERROR, "Could not connect to WMI namespace. Error code = 0x{:x}",
              hresult);
        pLoc->Release();
        CoUninitialize();
        return biosInfo;
    }

    // Set security levels on the proxy
    hresult =
        CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
                          RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
                          nullptr, EOAC_NONE);

    if (FAILED(hresult)) {
        LOG_F(ERROR, "Could not set proxy blanket. Error code = 0x{:x}",
              hresult);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return biosInfo;
    }

    // Execute WQL query to get BIOS information
    IEnumWbemClassObject *pEnumerator = nullptr;
    hresult =
        pSvc->ExecQuery(_bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM Win32_BIOS"),
                        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                        nullptr, &pEnumerator);

    if (FAILED(hresult)) {
        LOG_F(ERROR, "Query for BIOS information failed. Error code = 0x{:x}",
              hresult);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return biosInfo;
    }

    // Parse query results
    IWbemClassObject *pclsObj = nullptr;
    ULONG uReturn = 0;

    while (pEnumerator != nullptr) {
        HRESULT hresult =
            pEnumerator->Next(WBEM_NO_WAIT, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;

        // Get BIOS version
        hresult = pclsObj->Get(L"Version", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hresult) && vtProp.vt == VT_BSTR) {
            biosInfo.version = _com_util::ConvertBSTRToString(vtProp.bstrVal);
            LOG_F(INFO, "BIOS Version: {}", biosInfo.version);
        }
        VariantClear(&vtProp);

        // Get BIOS manufacturer
        hresult = pclsObj->Get(L"Manufacturer", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hresult) && vtProp.vt == VT_BSTR) {
            biosInfo.manufacturer =
                _com_util::ConvertBSTRToString(vtProp.bstrVal);
            LOG_F(INFO, "BIOS Manufacturer: {}", biosInfo.manufacturer);
        }
        VariantClear(&vtProp);

        // Get BIOS release date
        hresult = pclsObj->Get(L"ReleaseDate", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hresult) && vtProp.vt == VT_BSTR) {
            biosInfo.releaseDate =
                _com_util::ConvertBSTRToString(vtProp.bstrVal);
            LOG_F(INFO, "BIOS Release Date: {}", biosInfo.releaseDate);
        }
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    LOG_F(INFO, "Finished getBiosInfo function");
    return biosInfo;
}

#elif __linux__
BiosInfoData getBiosInfo() {
    LOG_F(INFO, "Starting getBiosInfo function");
    BiosInfoData biosInfo = {"", "", ""};
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen("sudo dmidecode -t bios", "r"), pclose);

    if (!pipe) {
        LOG_F(ERROR, "popen() failed!");
        return biosInfo;
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    std::istringstream iss(result);
    std::string line;

    while (std::getline(iss, line)) {
        if (line.find("Version:") != std::string::npos) {
            biosInfo.version = line.substr(line.find(":") + 2);
            LOG_F(INFO, "BIOS Version: {}", biosInfo.version);
        } else if (line.find("Vendor:") != std::string::npos) {
            biosInfo.manufacturer = line.substr(line.find(":") + 2);
            LOG_F(INFO, "BIOS Manufacturer: {}", biosInfo.manufacturer);
        } else if (line.find("Release Date:") != std::string::npos) {
            biosInfo.releaseDate = line.substr(line.find(":") + 2);
            LOG_F(INFO, "BIOS Release Date: {}", biosInfo.releaseDate);
        }
    }

    LOG_F(INFO, "Finished getBiosInfo function");
    return biosInfo;
}

#endif

}  // namespace atom::system
