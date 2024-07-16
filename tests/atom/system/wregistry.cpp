#include "atom/system/wregistry.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "wregistry_mock.hpp"


using namespace atom::system;
using namespace testing;

MockWindowsRegistry* mockRegistry;

LSTATUS WINAPI MockRegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions,
                                 REGSAM samDesired, PHKEY phkResult) {
    return mockRegistry->RegOpenKeyExA(hKey, lpSubKey, ulOptions, samDesired,
                                       phkResult);
}

LSTATUS WINAPI MockRegQueryValueExA(HKEY hKey, LPCSTR lpValueName,
                                    LPDWORD lpReserved, LPDWORD lpType,
                                    LPBYTE lpData, LPDWORD lpcbData) {
    return mockRegistry->RegQueryValueExA(hKey, lpValueName, lpReserved, lpType,
                                          lpData, lpcbData);
}

LSTATUS WINAPI MockRegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved,
                                  DWORD dwType, const BYTE* lpData,
                                  DWORD cbData) {
    return mockRegistry->RegSetValueExA(hKey, lpValueName, Reserved, dwType,
                                        lpData, cbData);
}

LSTATUS WINAPI MockRegDeleteKeyA(HKEY hKey, LPCSTR lpSubKey) {
    return mockRegistry->RegDeleteKeyA(hKey, lpSubKey);
}

LSTATUS WINAPI MockRegDeleteValueA(HKEY hKey, LPCSTR lpValueName) {
    return mockRegistry->RegDeleteValueA(hKey, lpValueName);
}

LSTATUS WINAPI MockRegEnumKeyExA(HKEY hKey, DWORD dwIndex, LPSTR lpName,
                                 LPDWORD lpcName, LPDWORD lpReserved,
                                 LPSTR lpClass, LPDWORD lpcClass,
                                 PFILETIME lpftLastWriteTime) {
    return mockRegistry->RegEnumKeyExA(hKey, dwIndex, lpName, lpcName,
                                       lpReserved, lpClass, lpcClass,
                                       lpftLastWriteTime);
}

LSTATUS WINAPI MockRegEnumValueA(HKEY hKey, DWORD dwIndex, LPSTR lpValueName,
                                 LPDWORD lpcchValueName, LPDWORD lpReserved,
                                 LPDWORD lpType, LPBYTE lpData,
                                 LPDWORD lpcbData) {
    return mockRegistry->RegEnumValueA(hKey, dwIndex, lpValueName,
                                       lpcchValueName, lpReserved, lpType,
                                       lpData, lpcbData);
}

LSTATUS WINAPI MockRegCloseKey(HKEY hKey) {
    return mockRegistry->RegCloseKey(hKey);
}

class WRegistryTest : public ::testing::Test {
protected:
    void SetUp() override { mockRegistry = new MockWindowsRegistry(); }

    void TearDown() override { delete mockRegistry; }
};

// Test getRegistrySubKeys function
TEST_F(WRegistryTest, GetRegistrySubKeys) {
    EXPECT_CALL(*mockRegistry, RegOpenKeyExA(_, _, _, _, _))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*mockRegistry, RegEnumKeyExA(_, _, _, _, _, _, _, _))
        .WillOnce(Return(ERROR_NO_MORE_ITEMS));
    EXPECT_CALL(*mockRegistry, RegCloseKey(_)).WillOnce(Return(ERROR_SUCCESS));

    HKEY hRootKey = HKEY_CURRENT_USER;
    std::string_view subKey = "Software\\Test";
    std::vector<std::string> subKeys;
    EXPECT_TRUE(getRegistrySubKeys(hRootKey, subKey, subKeys));
}

// Test getRegistryValues function
TEST_F(WRegistryTest, GetRegistryValues) {
    EXPECT_CALL(*mockRegistry, RegOpenKeyExA(_, _, _, _, _))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*mockRegistry, RegEnumValueA(_, _, _, _, _, _, _, _))
        .WillOnce(Return(ERROR_NO_MORE_ITEMS));
    EXPECT_CALL(*mockRegistry, RegCloseKey(_)).WillOnce(Return(ERROR_SUCCESS));

    HKEY hRootKey = HKEY_CURRENT_USER;
    std::string_view subKey = "Software\\Test";
    std::vector<std::pair<std::string, std::string>> values;
    EXPECT_TRUE(getRegistryValues(hRootKey, subKey, values));
}

// Test modifyRegistryValue function
TEST_F(WRegistryTest, ModifyRegistryValue) {
    EXPECT_CALL(*mockRegistry, RegOpenKeyExA(_, _, _, _, _))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*mockRegistry, RegSetValueExA(_, _, _, _, _, _))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*mockRegistry, RegCloseKey(_)).WillOnce(Return(ERROR_SUCCESS));

    HKEY hRootKey = HKEY_CURRENT_USER;
    std::string_view subKey = "Software\\Test";
    std::string_view valueName = "TestValue";
    std::string_view newValue = "NewData";
    EXPECT_TRUE(modifyRegistryValue(hRootKey, subKey, valueName, newValue));
}

// Test deleteRegistrySubKey function
TEST_F(WRegistryTest, DeleteRegistrySubKey) {
    EXPECT_CALL(*mockRegistry, RegDeleteKeyA(_, _))
        .WillOnce(Return(ERROR_SUCCESS));
    HKEY hRootKey = HKEY_CURRENT_USER;
    std::string_view subKey = "Software\\Test";
    EXPECT_TRUE(deleteRegistrySubKey(hRootKey, subKey));
}

// Test deleteRegistryValue function
TEST_F(WRegistryTest, DeleteRegistryValue) {
    EXPECT_CALL(*mockRegistry, RegOpenKeyExA(_, _, _, _, _))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*mockRegistry, RegDeleteValueA(_, _))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*mockRegistry, RegCloseKey(_)).WillOnce(Return(ERROR_SUCCESS));

    HKEY hRootKey = HKEY_CURRENT_USER;
    std::string_view subKey = "Software\\Test";
    std::string_view valueName = "TestValue";
    EXPECT_TRUE(deleteRegistryValue(hRootKey, subKey, valueName));
}
