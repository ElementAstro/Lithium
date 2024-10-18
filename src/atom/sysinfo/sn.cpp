#include "sn.hpp"

#include "atom/log/loguru.hpp"

#ifdef _WIN32
#include <Wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

class HardwareInfo::Impl {
public:
    static auto getWmiProperty(const std::wstring& wmiClass,
                               const std::wstring& property) -> std::string {
        LOG_F(INFO, "Getting WMI property: Class = {}, Property = {}",
              wmiClass.c_str(), property.c_str());
        IWbemLocator* pLoc = nullptr;
        IWbemServices* pSvc = nullptr;
        IEnumWbemClassObject* pEnumerator = nullptr;
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;
        std::string result;

        if (!initializeWmi(pLoc, pSvc)) {
            LOG_F(ERROR, "Failed to initialize WMI");
            return "";
        }

        HRESULT hres = pSvc->ExecQuery(
            bstr_t("WQL"), bstr_t((L"SELECT * FROM " + wmiClass).c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
            &pEnumerator);

        if (FAILED(hres)) {
            LOG_F(ERROR, "WMI query execution failed");
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
                LOG_F(INFO, "Retrieved WMI property value: {}", result.c_str());
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
        LOG_F(INFO,
              "Getting multiple WMI properties: Class = {}, Property = {}",
              wmiClass.c_str(), property.c_str());
        IWbemLocator* pLoc = nullptr;
        IWbemServices* pSvc = nullptr;
        IEnumWbemClassObject* pEnumerator = nullptr;
        IWbemClassObject* pclsObj = nullptr;
        ULONG uReturn = 0;
        std::vector<std::string> results;

        if (!initializeWmi(pLoc, pSvc)) {
            LOG_F(ERROR, "Failed to initialize WMI");
            return results;
        }

        HRESULT hres = pSvc->ExecQuery(
            bstr_t("WQL"), bstr_t((L"SELECT * FROM " + wmiClass).c_str()),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
            &pEnumerator);

        if (FAILED(hres)) {
            LOG_F(ERROR, "WMI query execution failed");
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
                LOG_F(INFO, "Retrieved WMI property value: {}", results.back());
            }
            VariantClear(&vtProp);
            pclsObj->Release();
        }

        cleanup(pLoc, pSvc, pEnumerator);
        return results;
    }

    static auto initializeWmi(IWbemLocator*& pLoc,
                              IWbemServices*& pSvc) -> bool {
        LOG_F(INFO, "Initializing WMI");
        HRESULT hres;

        hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hres)) {
            LOG_F(ERROR, "Failed to initialize COM library. Error code = 0x{}",
                  hres);
            return false;
        }

        hres = CoInitializeSecurity(
            nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT,
            RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, nullptr);

        if (FAILED(hres)) {
            LOG_F(ERROR, "Failed to initialize security. Error code = 0x{}",
                  hres);
            CoUninitialize();
            return false;
        }

        hres = CoCreateInstance(CLSID_WbemLocator, nullptr,
                                CLSCTX_INPROC_SERVER, IID_IWbemLocator,
                                reinterpret_cast<LPVOID*>(&pLoc));

        if (FAILED(hres)) {
            LOG_F(ERROR,
                  "Failed to create IWbemLocator object. Error code = 0x{}",
                  hres);
            CoUninitialize();
            return false;
        }

        hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0,
                                   0, 0, 0, &pSvc);

        if (FAILED(hres)) {
            LOG_F(ERROR,
                  "Could not connect to WMI namespace. Error code = 0x{}",
                  hres);
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        hres =
            CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                              nullptr, RPC_C_AUTHN_LEVEL_CALL,
                              RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

        if (FAILED(hres)) {
            LOG_F(ERROR, "Could not set proxy blanket. Error code = 0x{}",
                  hres);
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();
            return false;
        }

        LOG_F(INFO, "WMI initialized successfully");
        return true;
    }

    static void cleanup(IWbemLocator* pLoc, IWbemServices* pSvc,
                        IEnumWbemClassObject* pEnumerator) {
        LOG_F(INFO, "Cleaning up WMI resources");
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
        LOG_F(INFO, "Getting BIOS serial number");
        return getWmiProperty(L"Win32_BIOS", L"SerialNumber");
    }

    auto getMotherboardSerialNumber() const -> std::string {
        LOG_F(INFO, "Getting motherboard serial number");
        return getWmiProperty(L"Win32_BaseBoard", L"SerialNumber");
    }

    auto getCpuSerialNumber() const -> std::string {
        LOG_F(INFO, "Getting CPU serial number");
        return getWmiProperty(L"Win32_Processor", L"ProcessorId");
    }

    auto getDiskSerialNumbers() const -> std::vector<std::string> {
        LOG_F(INFO, "Getting disk serial numbers");
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
        LOG_F(INFO, "Reading file: {}", path.c_str());
        std::ifstream file(path);
        std::string content;

        if (file.is_open()) {
            if (key.empty()) {
                std::getline(file, content);
                LOG_F(INFO, "Read content: {}", content.c_str());
            } else {
                std::string line;
                while (std::getline(file, line)) {
                    if (line.find(key) != std::string::npos) {
                        content = line.substr(line.find(":") + 2);
                        LOG_F(INFO, "Found key {} with value: {}", key.c_str(),
                              content.c_str());
                        break;
                    }
                }
            }
            file.close();
        } else {
            LOG_F(ERROR, "Failed to open file: {}", path.c_str());
        }

        return content;
    }

    auto getBiosSerialNumber() const -> std::string {
        LOG_F(INFO, "Getting BIOS serial number");
        return readFile("/sys/class/dmi/id/product_serial");
    }

    auto getMotherboardSerialNumber() const -> std::string {
        LOG_F(INFO, "Getting motherboard serial number");
        return readFile("/sys/class/dmi/id/board_serial");
    }

    auto getCpuSerialNumber() const -> std::string {
        LOG_F(INFO, "Getting CPU serial number");
        return readFile("/proc/cpuinfo", "Serial");
    }

    auto getDiskSerialNumbers() const -> std::vector<std::string> {
        LOG_F(INFO, "Getting disk serial numbers");
        std::vector<std::string> serials;
        serials.push_back(readFile("/sys/block/sda/device/serial"));
        // 可以根据需要增加更多磁盘
        return serials;
    }
};

#endif

// HardwareInfo类的构造和析构实现
HardwareInfo::HardwareInfo() : impl_(new Impl()) {
    LOG_F(INFO, "HardwareInfo constructor called");
}

HardwareInfo::~HardwareInfo() {
    LOG_F(INFO, "HardwareInfo destructor called");
    delete impl_;
}

auto HardwareInfo::getBiosSerialNumber() -> std::string {
    LOG_F(INFO, "Getting BIOS serial number from HardwareInfo");
    return impl_->getBiosSerialNumber();
}

auto HardwareInfo::getMotherboardSerialNumber() -> std::string {
    LOG_F(INFO, "Getting motherboard serial number from HardwareInfo");
    return impl_->getMotherboardSerialNumber();
}

auto HardwareInfo::getCpuSerialNumber() -> std::string {
    LOG_F(INFO, "Getting CPU serial number from HardwareInfo");
    return impl_->getCpuSerialNumber();
}

auto HardwareInfo::getDiskSerialNumbers() -> std::vector<std::string> {
    LOG_F(INFO, "Getting disk serial numbers from HardwareInfo");
    return impl_->getDiskSerialNumbers();
}