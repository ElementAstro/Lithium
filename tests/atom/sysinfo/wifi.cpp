#include "atom/sysinfo/wifi.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>


#ifdef _WIN32
#include <iphlpapi.h>
#include <windows.h>
#include <wlanapi.h>


class MockWindowsApi {
public:
    MOCK_METHOD(DWORD, WlanOpenHandle, (DWORD, PVOID, PDWORD, PHANDLE), ());
    MOCK_METHOD(DWORD, WlanEnumInterfaces,
                (HANDLE, PVOID, PWLAN_INTERFACE_INFO_LIST*), ());
    MOCK_METHOD(DWORD, WlanQueryInterface,
                (HANDLE, const GUID*, WLAN_INTF_OPCODE, PVOID, PDWORD, PVOID*,
                 PVOID),
                ());
    MOCK_METHOD(DWORD, WlanCloseHandle, (HANDLE, PVOID), ());
    MOCK_METHOD(DWORD, GetAdaptersInfo, (PIP_ADAPTER_INFO, PULONG), ());
};

MockWindowsApi* mockWindowsApi;

DWORD WINAPI MockWlanOpenHandle(DWORD dwClientVersion, PVOID pReserved,
                                PDWORD pdwNegotiatedVersion,
                                PHANDLE phClientHandle) {
    return mockWindowsApi->WlanOpenHandle(dwClientVersion, pReserved,
                                          pdwNegotiatedVersion, phClientHandle);
}

DWORD WINAPI
MockWlanEnumInterfaces(HANDLE hClientHandle, PVOID pReserved,
                       PWLAN_INTERFACE_INFO_LIST* ppInterfaceList) {
    return mockWindowsApi->WlanEnumInterfaces(hClientHandle, pReserved,
                                              ppInterfaceList);
}

DWORD WINAPI MockWlanQueryInterface(HANDLE hClientHandle,
                                    const GUID* pInterfaceGuid,
                                    WLAN_INTF_OPCODE OpCode, PVOID pReserved,
                                    PDWORD pdwDataSize, PVOID* ppData,
                                    PVOID pWlanOpcodeValueType) {
    return mockWindowsApi->WlanQueryInterface(hClientHandle, pInterfaceGuid,
                                              OpCode, pReserved, pdwDataSize,
                                              ppData, pWlanOpcodeValueType);
}

DWORD WINAPI MockWlanCloseHandle(HANDLE hClientHandle, PVOID pReserved) {
    return mockWindowsApi->WlanCloseHandle(hClientHandle, pReserved);
}

DWORD WINAPI MockGetAdaptersInfo(PIP_ADAPTER_INFO pAdapterInfo,
                                 PULONG pOutBufLen) {
    return mockWindowsApi->GetAdaptersInfo(pAdapterInfo, pOutBufLen);
}

void setupMockWindowsApi() {
    mockWindowsApi = new MockWindowsApi();
    WlanOpenHandle = MockWlanOpenHandle;
    WlanEnumInterfaces = MockWlanEnumInterfaces;
    WlanQueryInterface = MockWlanQueryInterface;
    WlanCloseHandle = MockWlanCloseHandle;
    GetAdaptersInfo = MockGetAdaptersInfo;
}

void cleanupMockWindowsApi() { delete mockWindowsApi; }

#else
#include <fstream>

class MockFileReader {
public:
    MOCK_METHOD(std::string, ReadFile, (const std::string&), ());
};

MockFileReader* mockFileReader;

std::string MockReadFile(const std::string& path) {
    return mockFileReader->ReadFile(path);
}

void setupMockFileReader() { mockFileReader = new MockFileReader(); }

void cleanupMockFileReader() { delete mockFileReader; }
#endif

class WifiTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef _WIN32
        setupMockWindowsApi();
#else
        setupMockFileReader();
#endif
    }

    void TearDown() override {
#ifdef _WIN32
        cleanupMockWindowsApi();
#else
        cleanupMockFileReader();
#endif
    }
};

#ifdef _WIN32
TEST_F(WifiTest, GetCurrentWifi_Windows) {
    HANDLE handle = reinterpret_cast<HANDLE>(1);
    WLAN_INTERFACE_INFO_LIST interfaceInfoList;
    WLAN_INTERFACE_INFO interfaceInfo;
    interfaceInfo.isState = wlan_interface_state_connected;
    WLAN_CONNECTION_ATTRIBUTES connectionAttributes;
    connectionAttributes.wlanAssociationAttributes.dot11Ssid.uSSIDLength = 4;
    memcpy(connectionAttributes.wlanAssociationAttributes.dot11Ssid.ucSSID,
           "Test", 4);

    interfaceInfoList.dwNumberOfItems = 1;
    interfaceInfoList.InterfaceInfo[0] = interfaceInfo;

    EXPECT_CALL(*mockWindowsApi, WlanOpenHandle(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(handle), Return(ERROR_SUCCESS)));
    EXPECT_CALL(*mockWindowsApi, WlanEnumInterfaces(_, _, _))
        .WillOnce(
            DoAll(SetArgPointee<2>(&interfaceInfoList), Return(ERROR_SUCCESS)));
    EXPECT_CALL(*mockWindowsApi, WlanQueryInterface(_, _, _, _, _, _, _))
        .WillOnce(DoAll(SetArgPointee<5>(&connectionAttributes),
                        Return(ERROR_SUCCESS)));
    EXPECT_CALL(*mockWindowsApi, WlanCloseHandle(_, _))
        .WillOnce(Return(ERROR_SUCCESS));

    std::string wifiName = getCurrentWifi();
    EXPECT_EQ(wifiName, "Test");
}

TEST_F(WifiTest, GetCurrentWiredNetwork_Windows) {
    IP_ADAPTER_INFO adapterInfo;
    adapterInfo.Type = MIB_IF_TYPE_ETHERNET;
    strcpy_s(adapterInfo.AdapterName, "Ethernet");

    EXPECT_CALL(*mockWindowsApi, GetAdaptersInfo(_, _))
        .WillOnce(DoAll(SetArgPointee<0>(&adapterInfo), Return(NO_ERROR)));

    std::string wiredNetworkName = getCurrentWiredNetwork();
    EXPECT_EQ(wiredNetworkName, "Ethernet");
}

#else
TEST_F(WifiTest, GetCurrentWifi_Linux) {
    std::string mockWirelessInfo =
        "Inter-| sta-|   Quality        | Discarded packets               | "
        "Missed | WE\n"
        " face | tus | link level noise |  nwid  crypt   frag  retry   misc | "
        "beacon | 22\n"
        "wlan0: 0000   54.  -61.  -256        0      0      0      0      0    "
        "    0\n";

    EXPECT_CALL(*mockFileReader, ReadFile("/proc/net/wireless"))
        .WillOnce(Return(mockWirelessInfo));

    std::string wifiName = getCurrentWifi();
    EXPECT_EQ(wifiName, "wlan0");
}

TEST_F(WifiTest, GetCurrentWiredNetwork_Linux) {
    std::string mockSysClassNet = "eth0\n";

    EXPECT_CALL(*mockFileReader, ReadFile("/sys/class/net"))
        .WillOnce(Return(mockSysClassNet));

    std::string wiredNetworkName = getCurrentWiredNetwork();
    EXPECT_EQ(wiredNetworkName, "eth0");
}

#endif
