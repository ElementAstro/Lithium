#include "sn.hpp"

#ifdef _WIN32
#include <Wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

class HardwareInfo::Impl {
public:
    std::string getWmiProperty(const std::wstring& wmiClass,
                               const std::wstring& property) {
        IWbemLocator* pLoc = nullptr;
        IWbemServices* pSvc = nullptr;
        IEnumWbemClassObject* pEnumerator = nullptr;
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;
        std::string result;

        if (!initializeWmi(pLoc, pSvc)) {
            return "";
        }

        HRESULT hres = pSvc->ExecQuery(
            bstr_t("WQL"), bstr_t((L"SELECT * FROM " + wmiClass).c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
            &pEnumerator);

        if (FAILED(hres)) {
            goto cleanup;
        }

        while (pEnumerator) {
            HRESULT hr =
                pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (0 == uReturn) {
                break;
            }

            VARIANT vtProp;
            hr = pclsObj->Get(property.c_str(), 0, &vtProp, nullptr, nullptr);
            if (SUCCEEDED(hr)) {
                result = _bstr_t(vtProp.bstrVal);
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }

    cleanup:
        if (pSvc)
            pSvc->Release();
        if (pLoc)
            pLoc->Release();
        if (pEnumerator)
            pEnumerator->Release();
        CoUninitialize();

        return result;
    }

    std::vector<std::string> getWmiPropertyMultiple(
        const std::wstring& wmiClass, const std::wstring& property) {
        IWbemLocator* pLoc = nullptr;
        IWbemServices* pSvc = nullptr;
        IEnumWbemClassObject* pEnumerator = nullptr;
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;
        std::vector<std::string> results;

        if (!initializeWmi(pLoc, pSvc)) {
            return results;
        }

        HRESULT hres = pSvc->ExecQuery(
            bstr_t("WQL"), bstr_t((L"SELECT * FROM " + wmiClass).c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
            &pEnumerator);

        if (FAILED(hres)) {
            goto cleanup;
        }

        while (pEnumerator) {
            HRESULT hr =
                pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (0 == uReturn) {
                break;
            }

            VARIANT vtProp;
            hr = pclsObj->Get(property.c_str(), 0, &vtProp, nullptr, nullptr);
            if (SUCCEEDED(hr)) {
                results.push_back(
                    static_cast<const char*>(_bstr_t(vtProp.bstrVal)));
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }

    cleanup:
        if (pSvc)
            pSvc->Release();
        if (pLoc)
            pLoc->Release();
        if (pEnumerator)
            pEnumerator->Release();
        CoUninitialize();

        return results;
    }

    static bool initializeWmi(IWbemLocator*& pLoc, IWbemServices*& pSvc) {
        HRESULT hres;

        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            return false;
        }

        hres = CoInitializeSecurity(
            nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);

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

        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0,
                                   0, 0, 0, &pSvc);

        if (FAILED(hres)) {
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        hres =
            CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                              nullptr, RPC_C_AUTHN_LEVEL_CALL,
                              RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

        if (FAILED(hres)) {
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        return true;
    }

    std::string getBiosSerialNumber() {
        return getWmiProperty(L"Win32_BIOS", L"SerialNumber");
    }

    std::string getMotherboardSerialNumber() {
        return getWmiProperty(L"Win32_BaseBoard", L"SerialNumber");
    }

    std::string getCpuSerialNumber() {
        return getWmiProperty(L"Win32_Processor", L"ProcessorId");
    }

    std::vector<std::string> getDiskSerialNumbers() {
        return getWmiPropertyMultiple(L"Win32_DiskDrive", L"SerialNumber");
    }
};

#else
#include <fstream>
#include <iostream>

class HardwareInfo::Impl {
public:
    std::string readFile(const std::string& path, const std::string& key = "") {
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

    std::string getBiosSerialNumber() {
        return readFile("/sys/class/dmi/id/product_serial");
    }

    std::string getMotherboardSerialNumber() {
        return readFile("/sys/class/dmi/id/board_serial");
    }

    std::string getCpuSerialNumber() {
        return readFile("/proc/cpuinfo", "Serial");
    }

    std::vector<std::string> getDiskSerialNumbers() {
        std::vector<std::string> serials;
        serials.push_back(readFile("/sys/block/sda/device/serial"));
        // 可以根据需要增加更多磁盘
        return serials;
    }
};

#endif

// HardwareInfo类的构造和析构实现
HardwareInfo::HardwareInfo() : impl_(new Impl()) {}
HardwareInfo::~HardwareInfo() { delete impl_; }

std::string HardwareInfo::getBiosSerialNumber() {
    return impl_->getBiosSerialNumber();
}

std::string HardwareInfo::getMotherboardSerialNumber() {
    return impl_->getMotherboardSerialNumber();
}

std::string HardwareInfo::getCpuSerialNumber() {
    return impl_->getCpuSerialNumber();
}

std::vector<std::string> HardwareInfo::getDiskSerialNumbers() {
    return impl_->getDiskSerialNumbers();
}