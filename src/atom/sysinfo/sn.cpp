#include "sn.hpp"

#ifdef _WIN32
#include <Wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

class HardwareInfo::Impl {
public:
    static auto getWmiProperty(const std::wstring& wmiClass,
                               const std::wstring& property) -> std::string {
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
            cleanup(pLoc, pSvc, pEnumerator);
            return "";
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

        cleanup(pLoc, pSvc, pEnumerator);
        return result;
    }

    static auto getWmiPropertyMultiple(const std::wstring& wmiClass,
                                       const std::wstring& property)
        -> std::vector<std::string> {
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
            cleanup(pLoc, pSvc, pEnumerator);
            return results;
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
                results.emplace_back(
                    static_cast<const char*>(_bstr_t(vtProp.bstrVal)));
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }

        cleanup(pLoc, pSvc, pEnumerator);
        return results;
    }

    static auto initializeWmi(IWbemLocator*& pLoc,
                              IWbemServices*& pSvc) -> bool {
        HRESULT hres;

        hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
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

        hres = CoCreateInstance(CLSID_WbemLocator, nullptr,
                                CLSCTX_INPROC_SERVER, IID_IWbemLocator,
                                reinterpret_cast<LPVOID*>(&pLoc));

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

    static void cleanup(IWbemLocator* pLoc, IWbemServices* pSvc,
                        IEnumWbemClassObject* pEnumerator) {
        if (pSvc) {
            pSvc->Release();
        }
        if (pLoc) {
            pLoc->Release();
        }
        if (pEnumerator) {
            pEnumerator->Release();
        }
        CoUninitialize();
    }

    auto getBiosSerialNumber() const -> std::string {
        return getWmiProperty(L"Win32_BIOS", L"SerialNumber");
    }

    auto getMotherboardSerialNumber() const -> std::string {
        return getWmiProperty(L"Win32_BaseBoard", L"SerialNumber");
    }

    auto getCpuSerialNumber() const -> std::string {
        return getWmiProperty(L"Win32_Processor", L"ProcessorId");
    }

    auto getDiskSerialNumbers() const -> std::vector<std::string> {
        return getWmiPropertyMultiple(L"Win32_DiskDrive", L"SerialNumber");
    }
};

#else
#include <fstream>
#include <iostream>

class HardwareInfo::Impl {
public:
    auto readFile(const std::string& path,
                  const std::string& key = "") const -> std::string {
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

    auto getBiosSerialNumber() const -> std::string {
        return readFile("/sys/class/dmi/id/product_serial");
    }

    auto getMotherboardSerialNumber() const -> std::string {
        return readFile("/sys/class/dmi/id/board_serial");
    }

    auto getCpuSerialNumber() const -> std::string {
        return readFile("/proc/cpuinfo", "Serial");
    }

    auto getDiskSerialNumbers() const -> std::vector<std::string> {
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

auto HardwareInfo::getBiosSerialNumber() -> std::string {
    return impl_->getBiosSerialNumber();
}

auto HardwareInfo::getMotherboardSerialNumber() -> std::string {
    return impl_->getMotherboardSerialNumber();
}

auto HardwareInfo::getCpuSerialNumber() -> std::string {
    return impl_->getCpuSerialNumber();
}

auto HardwareInfo::getDiskSerialNumbers() -> std::vector<std::string> {
    return impl_->getDiskSerialNumbers();
}
