#include "sn.hpp"

#ifdef _WIN32
#include <Wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")
#include <iostream>

class HardwareInfo::Impl {
public:
    std::string GetWmiProperty(const std::wstring& wmiClass,
                               const std::wstring& property) {
        IWbemLocator* pLoc = NULL;
        IWbemServices* pSvc = NULL;
        IEnumWbemClassObject* pEnumerator = NULL;
        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        std::string result;

        if (!InitializeWmi(pLoc, pSvc))
            return "";

        HRESULT hres = pSvc->ExecQuery(
            bstr_t("WQL"), bstr_t((L"SELECT * FROM " + wmiClass).c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
            &pEnumerator);

        if (FAILED(hres))
            goto cleanup;

        while (pEnumerator) {
            HRESULT hr =
                pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (0 == uReturn)
                break;

            VARIANT vtProp;
            hr = pclsObj->Get(property.c_str(), 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr)) {
                result = _bstr_t(vtProp.bstrVal);
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }

    cleanup:
        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        CoUninitialize();

        return result;
    }

    std::vector<std::string> GetWmiPropertyMultiple(
        const std::wstring& wmiClass, const std::wstring& property) {
        IWbemLocator* pLoc = NULL;
        IWbemServices* pSvc = NULL;
        IEnumWbemClassObject* pEnumerator = NULL;
        IWbemClassObject* pclsObj = NULL;
        ULONG uReturn = 0;
        std::vector<std::string> results;

        if (!InitializeWmi(pLoc, pSvc))
            return results;

        HRESULT hres = pSvc->ExecQuery(
            bstr_t("WQL"), bstr_t((L"SELECT * FROM " + wmiClass).c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL,
            &pEnumerator);

        if (FAILED(hres))
            goto cleanup;

        while (pEnumerator) {
            HRESULT hr =
                pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (0 == uReturn)
                break;

            VARIANT vtProp;
            hr = pclsObj->Get(property.c_str(), 0, &vtProp, 0, 0);
            if (SUCCEEDED(hr)) {
                results.push_back(_bstr_t(vtProp.bstrVal));
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }

    cleanup:
        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        CoUninitialize();

        return results;
    }

    bool InitializeWmi(IWbemLocator*& pLoc, IWbemServices*& pSvc) {
        HRESULT hres;

        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres))
            return false;

        hres = CoInitializeSecurity(
            NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

        if (FAILED(hres)) {
            CoUninitialize();
            return false;
        }

        hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                                IID_IWbemLocator, (LPVOID*)&pLoc);

        if (FAILED(hres)) {
            CoUninitialize();
            return false;
        }

        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL,
                                   0, 0, &pSvc);

        if (FAILED(hres)) {
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                 NULL, RPC_C_AUTHN_LEVEL_CALL,
                                 RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

        if (FAILED(hres)) {
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        return true;
    }

public:
    std::string GetBiosSerialNumber() {
        return GetWmiProperty(L"Win32_BIOS", L"SerialNumber");
    }

    std::string GetMotherboardSerialNumber() {
        return GetWmiProperty(L"Win32_BaseBoard", L"SerialNumber");
    }

    std::string GetCpuSerialNumber() {
        return GetWmiProperty(L"Win32_Processor", L"ProcessorId");
    }

    std::vector<std::string> GetDiskSerialNumbers() {
        return GetWmiPropertyMultiple(L"Win32_DiskDrive", L"SerialNumber");
    }
};

#else
#include <fstream>
#include <iostream>

class HardwareInfo::Impl {
public:
    std::string ReadFile(const std::string& path, const std::string& key = "") {
        std::ifstream file(path);
        std::string content;

        if (file.is_open()) {
            if (key.empty()) {
                std::getline(file, content);
            } else {
                std::string line;
                while (std::getline(file, line)) {
                    if (line.find(key) != std::string::npos) {
                        content = line.substr(line.find(":") + 2);
                        break;
                    }
                }
            }
            file.close();
        }

        return content;
    }

public:
    std::string GetBiosSerialNumber() {
        return ReadFile("/sys/class/dmi/id/product_serial");
    }

    std::string GetMotherboardSerialNumber() {
        return ReadFile("/sys/class/dmi/id/board_serial");
    }

    std::string GetCpuSerialNumber() {
        return ReadFile("/proc/cpuinfo", "Serial");
    }

    std::vector<std::string> GetDiskSerialNumbers() {
        std::vector<std::string> serials;
        serials.push_back(ReadFile("/sys/block/sda/device/serial"));
        // 可以根据需要增加更多磁盘
        return serials;
    }
};

#endif

// HardwareInfo类的构造和析构实现
HardwareInfo::HardwareInfo() : pImpl(new Impl()) {}
HardwareInfo::~HardwareInfo() { delete pImpl; }

std::string HardwareInfo::GetBiosSerialNumber() {
    return pImpl->GetBiosSerialNumber();
}

std::string HardwareInfo::GetMotherboardSerialNumber() {
    return pImpl->GetMotherboardSerialNumber();
}

std::string HardwareInfo::GetCpuSerialNumber() {
    return pImpl->GetCpuSerialNumber();
}

std::vector<std::string> HardwareInfo::GetDiskSerialNumbers() {
    return pImpl->GetDiskSerialNumbers();
}
