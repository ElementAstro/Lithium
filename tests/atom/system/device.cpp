#include "atom/system/device.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>


#ifdef _WIN32
#include <windows.h>
#include <bluetoothapis.h>
#include <setupapi.h>
#else
#include <fcntl.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

#if __has_include(<bluetooth/bluetooth.h>)
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#endif
#endif

using namespace atom::system;
using namespace testing;

#ifdef _WIN32
// Mock functions for Windows
class MockWindowsApi {
public:
    MOCK_METHOD(HDEVINFO, SetupDiGetClassDevs,
                (const GUID*, PCSTR, HWND, DWORD), ());
    MOCK_METHOD(BOOL, SetupDiEnumDeviceInfo,
                (HDEVINFO, DWORD, PSP_DEVINFO_DATA), ());
    MOCK_METHOD(BOOL, SetupDiGetDeviceRegistryProperty,
                (HDEVINFO, PSP_DEVINFO_DATA, DWORD, PDWORD, PBYTE, DWORD,
                 PDWORD),
                ());
    MOCK_METHOD(BOOL, SetupDiDestroyDeviceInfoList, (HDEVINFO), ());
    MOCK_METHOD(HBLUETOOTH_DEVICE_FIND, BluetoothFindFirstDevice,
                (BLUETOOTH_DEVICE_SEARCH_PARAMS*, BLUETOOTH_DEVICE_INFO*), ());
    MOCK_METHOD(BOOL, BluetoothFindNextDevice,
                (HBLUETOOTH_DEVICE_FIND, BLUETOOTH_DEVICE_INFO*), ());
    MOCK_METHOD(BOOL, BluetoothFindDeviceClose, (HBLUETOOTH_DEVICE_FIND), ());
};

MockWindowsApi* mockWindowsApi;

// Define mock functions to use in the tests
HDEVINFO WINAPI MockSetupDiGetClassDevs(const GUID* ClassGuid, PCSTR Enumerator,
                                        HWND hwndParent, DWORD Flags) {
    return mockWindowsApi->SetupDiGetClassDevs(ClassGuid, Enumerator,
                                               hwndParent, Flags);
}

BOOL WINAPI MockSetupDiEnumDeviceInfo(HDEVINFO DeviceInfoSet, DWORD MemberIndex,
                                      PSP_DEVINFO_DATA DeviceInfoData) {
    return mockWindowsApi->SetupDiEnumDeviceInfo(DeviceInfoSet, MemberIndex,
                                                 DeviceInfoData);
}

BOOL WINAPI MockSetupDiGetDeviceRegistryProperty(
    HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, DWORD Property,
    PDWORD PropertyRegDataType, PBYTE PropertyBuffer, DWORD PropertyBufferSize,
    PDWORD RequiredSize) {
    return mockWindowsApi->SetupDiGetDeviceRegistryProperty(
        DeviceInfoSet, DeviceInfoData, Property, PropertyRegDataType,
        PropertyBuffer, PropertyBufferSize, RequiredSize);
}

BOOL WINAPI MockSetupDiDestroyDeviceInfoList(HDEVINFO DeviceInfoSet) {
    return mockWindowsApi->SetupDiDestroyDeviceInfoList(DeviceInfoSet);
}

HBLUETOOTH_DEVICE_FIND WINAPI MockBluetoothFindFirstDevice(
    BLUETOOTH_DEVICE_SEARCH_PARAMS* pbtsp, BLUETOOTH_DEVICE_INFO* pbtdi) {
    return mockWindowsApi->BluetoothFindFirstDevice(pbtsp, pbtdi);
}

BOOL WINAPI MockBluetoothFindNextDevice(HBLUETOOTH_DEVICE_FIND hFind,
                                        BLUETOOTH_DEVICE_INFO* pbtdi) {
    return mockWindowsApi->BluetoothFindNextDevice(hFind, pbtdi);
}

BOOL WINAPI MockBluetoothFindDeviceClose(HBLUETOOTH_DEVICE_FIND hFind) {
    return mockWindowsApi->BluetoothFindDeviceClose(hFind);
}

// Setup the mock environment
void setupMockWindowsApi() {
    mockWindowsApi = new MockWindowsApi();
    SetupDiGetClassDevs = MockSetupDiGetClassDevs;
    SetupDiEnumDeviceInfo = MockSetupDiEnumDeviceInfo;
    SetupDiGetDeviceRegistryProperty = MockSetupDiGetDeviceRegistryProperty;
    SetupDiDestroyDeviceInfoList = MockSetupDiDestroyDeviceInfoList;
    BluetoothFindFirstDevice = MockBluetoothFindFirstDevice;
    BluetoothFindNextDevice = MockBluetoothFindNextDevice;
    BluetoothFindDeviceClose = MockBluetoothFindDeviceClose;
}

// Cleanup the mock environment
void cleanupMockWindowsApi() { delete mockWindowsApi; }

#else
// Mock functions for Linux
class MockLibusbApi {
public:
    MOCK_METHOD(int, libusb_init, (libusb_context**), ());
    MOCK_METHOD(ssize_t, libusb_get_device_list,
                (libusb_context*, libusb_device***), ());
    MOCK_METHOD(void, libusb_free_device_list, (libusb_device**, int), ());
    MOCK_METHOD(void, libusb_exit, (libusb_context*), ());
    MOCK_METHOD(int, libusb_get_device_descriptor,
                (libusb_device*, libusb_device_descriptor*), ());
    MOCK_METHOD(int, libusb_get_config_descriptor,
                (libusb_device*, uint8_t, libusb_config_descriptor**), ());
    MOCK_METHOD(void, libusb_free_config_descriptor,
                (libusb_config_descriptor*), ());
    MOCK_METHOD(int, libusb_open, (libusb_device*, libusb_device_handle**), ());
    MOCK_METHOD(void, libusb_close, (libusb_device_handle*), ());
    MOCK_METHOD(int, libusb_get_string_descriptor_ascii,
                (libusb_device_handle*, uint8_t, unsigned char*, int), ());
};

MockLibusbApi* mockLibusbApi;

// Define mock functions to use in the tests
int MockLibusbInit(libusb_context** ctx) {
    return mockLibusbApi->libusb_init(ctx);
}

ssize_t MockLibusbGetDeviceList(libusb_context* ctx, libusb_device*** list) {
    return mockLibusbApi->libusb_get_device_list(ctx, list);
}

void MockLibusbFreeDeviceList(libusb_device** list, int unrefDevices) {
    mockLibusbApi->libusb_free_device_list(list, unrefDevices);
}

void MockLibusbExit(libusb_context* ctx) { mockLibusbApi->libusb_exit(ctx); }

int MockLibusbGetDeviceDescriptor(libusb_device* dev,
                                  libusb_device_descriptor* desc) {
    return mockLibusbApi->libusb_get_device_descriptor(dev, desc);
}

int MockLibusbGetConfigDescriptor(libusb_device* dev, uint8_t configIndex,
                                  libusb_config_descriptor** config) {
    return mockLibusbApi->libusb_get_config_descriptor(dev, configIndex,
                                                       config);
}

void MockLibusbFreeConfigDescriptor(libusb_config_descriptor* config) {
    mockLibusbApi->libusb_free_config_descriptor(config);
}

int MockLibusbOpen(libusb_device* dev, libusb_device_handle** handle) {
    return mockLibusbApi->libusb_open(dev, handle);
}

void MockLibusbClose(libusb_device_handle* handle) {
    mockLibusbApi->libusb_close(handle);
}

int MockLibusbGetStringDescriptorAscii(libusb_device_handle* dev,
                                       uint8_t descIndex, unsigned char* data,
                                       int length) {
    return mockLibusbApi->libusb_get_string_descriptor_ascii(dev, descIndex,
                                                             data, length);
}

// Setup the mock environment
void setupMockLibusbApi() {
    mockLibusbApi = new MockLibusbApi();
    libusb_init = MockLibusbInit;
    libusb_get_device_list = MockLibusbGetDeviceList;
    libusb_free_device_list = MockLibusbFreeDeviceList;
    libusb_exit = MockLibusbExit;
    libusb_get_device_descriptor = MockLibusbGetDeviceDescriptor;
    libusb_get_config_descriptor = MockLibusbGetConfigDescriptor;
    libusb_free_config_descriptor = MockLibusbFreeConfigDescriptor;
    libusb_open = MockLibusbOpen;
    libusb_close = MockLibusbClose;
    libusb_get_string_descriptor_ascii = MockLibusbGetStringDescriptorAscii;
}

// Cleanup the mock environment
void cleanupMockLibusbApi() { delete mockLibusbApi; }
#endif

// Test fixture for setting up common test environment
class DeviceTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef _WIN32
        setupMockWindowsApi();
#else
        setupMockLibusbApi();
#endif
    }

    void TearDown() override {
#ifdef _WIN32
        cleanupMockWindowsApi();
#else
        cleanupMockLibusbApi();
#endif
    }
};

#ifdef _WIN32
// Test enumerateUsbDevices function for Windows
TEST_F(DeviceTest, EnumerateUsbDevices) {
    EXPECT_CALL(*mockWindowsApi, SetupDiGetClassDevs(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HDEVINFO>(1)));
    EXPECT_CALL(*mockWindowsApi, SetupDiEnumDeviceInfo(_, _, _))
        .WillOnce(Return(TRUE))
        .WillOnce(Return(FALSE));
    EXPECT_CALL(*mockWindowsApi,
                SetupDiGetDeviceRegistryProperty(_, _, _, _, _, _, _))
        .WillOnce(Return(TRUE));
    EXPECT_CALL(*mockWindowsApi, SetupDiDestroyDeviceInfoList(_))
        .WillOnce(Return(TRUE));

    auto devices = enumerateUsbDevices();
    EXPECT_FALSE(devices.empty());
    EXPECT_EQ(devices.size(), 1);
}

// Test enumerateSerialPorts function for Windows
TEST_F(DeviceTest, EnumerateSerialPorts) {
    EXPECT_CALL(*mockWindowsApi, SetupDiGetClassDevs(_, _, _, _))
        .WillOnce(Return(reinterpret_cast<HDEVINFO>(1)));
    EXPECT_CALL(*mockWindowsApi, SetupDiEnumDeviceInfo(_, _, _))
        .WillOnce(Return(TRUE))
        .WillOnce(Return(FALSE));
    EXPECT_CALL(*mockWindowsApi,
                SetupDiGetDeviceRegistryProperty(_, _, _, _, _, _, _))
        .WillOnce(Return(TRUE));
    EXPECT_CALL(*mockWindowsApi, SetupDiDestroyDeviceInfoList(_))
        .WillOnce(Return(TRUE));

    auto devices = enumerateSerialPorts();
    EXPECT_FALSE(devices.empty());
    EXPECT_EQ(devices.size(), 1);
}

// Test enumerateBluetoothDevices function for Windows
TEST_F(DeviceTest, EnumerateBluetoothDevices) {
    BLUETOOTH_DEVICE_INFO deviceInfo;
    deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
    std::wstring deviceName = L"TestDevice";
    wcscpy(deviceInfo.szName, deviceName.c_str());
    deviceInfo.Address.ullLong = 0x123456789ABC;

    EXPECT_CALL(*mockWindowsApi, BluetoothFindFirstDevice(_, _))
        .WillOnce(Return(reinterpret_cast<HBLUETOOTH_DEVICE_FIND>(1)));
    EXPECT_CALL(*mockWindowsApi, BluetoothFindNextDevice(_, _))
        .WillOnce(DoAll(SetArgPointee<1>(deviceInfo), Return(TRUE)))
        .WillOnce(Return(FALSE));
    EXPECT_CALL(*mockWindowsApi, BluetoothFindDeviceClose(_))
        .WillOnce(Return(TRUE));

    auto devices = enumerateBluetoothDevices();
    EXPECT_FALSE(devices.empty());
    EXPECT_EQ(devices.size(), 1);
    EXPECT_EQ(devices[0].description, "TestDevice");
    EXPECT_EQ(devices[0].address, "12:34:56:78:9A:BC");
}

#else
// Test enumerateUsbDevices function for Linux
TEST_F(DeviceTest, EnumerateUsbDevices) {
    libusb_device_descriptor desc;
    desc.idVendor = 0x1234;
    desc.idProduct = 0x5678;

    libusb_device* devList[2] = {reinterpret_cast<libusb_device*>(1), nullptr};

    EXPECT_CALL(*mockLibusbApi, libusb_init(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockLibusbApi, libusb_get_device_list(_, _))
        .WillOnce(DoAll(SetArgPointee<1>(devList), Return(1)));
    EXPECT_CALL(*mockLibusbApi, libusb_get_device_descriptor(_, _))
        .WillOnce(DoAll(SetArgPointee<1>(desc), Return(0)));
    EXPECT_CALL(*mockLibusbApi, libusb_free_device_list(_, _)).Times(1);
    EXPECT_CALL(*mockLibusbApi, libusb_exit(_)).Times(1);

    auto devices = enumerate_usb_devices();
    EXPECT_FALSE(devices.empty());
    EXPECT_EQ(devices.size(), 1);
    EXPECT_EQ(devices[0].description, "Bus 0 Device 0 (0x1234:0x5678)");
}

// Test enumerateSerialPorts function for Linux
TEST_F(DeviceTest, EnumerateSerialPorts) {
    DIR* dir = opendir("/dev");
    ASSERT_NE(dir, nullptr);
    struct dirent entry;
    entry.d_name[0] = 't';
    entry.d_name[1] = 't';
    entry.d_name[2] = 'y';
    entry.d_name[3] = 'S';
    entry.d_name[4] = '0';
    entry.d_name[5] = '\0';

    EXPECT_CALL(*mockLibusbApi, libusb_init(_))
        .Times(0);  // Not needed for serial ports

    auto devices = enumerate_serial_ports();
    EXPECT_FALSE(devices.empty());
    EXPECT_EQ(devices.size(), 1);
    EXPECT_EQ(devices[0].description, "ttyS0");
}

// Test enumerateBluetoothDevices function for Linux
TEST_F(DeviceTest, EnumerateBluetoothDevices) {
#if __has_include(<bluetooth/bluetooth.h>)
    int dev_id = 0;
    int num_rsp = 1;
    inquiry_info devices_info[1];
    bdaddr_t bdaddr;
    str2ba("01:23:45:67:89:AB", &bdaddr);
    devices_info[0].bdaddr = bdaddr;

    EXPECT_CALL(*mockLibusbApi, libusb_init(_))
        .Times(0);  // Not needed for Bluetooth devices

    auto devices = enumerate_bluetooth_devices();
    EXPECT_FALSE(devices.empty());
    EXPECT_EQ(devices.size(), 1);
    EXPECT_EQ(devices[0].description, "01:23:45:67:89:AB");
#endif
}

#endif