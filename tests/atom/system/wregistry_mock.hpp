#ifndef MOCK_WINDOWS_REGISTRY_HPP
#define MOCK_WINDOWS_REGISTRY_HPP

#include <gmock/gmock.h>
#include <windows.h>

class MockWindowsRegistry {
public:
    MOCK_METHOD(LSTATUS, RegOpenKeyExA, (HKEY, LPCSTR, DWORD, REGSAM, PHKEY), (const));
    MOCK_METHOD(LSTATUS, RegQueryValueExA, (HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD), (const));
    MOCK_METHOD(LSTATUS, RegSetValueExA, (HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD), (const));
    MOCK_METHOD(LSTATUS, RegDeleteKeyA, (HKEY, LPCSTR), (const));
    MOCK_METHOD(LSTATUS, RegDeleteValueA, (HKEY, LPCSTR), (const));
    MOCK_METHOD(LSTATUS, RegEnumKeyExA, (HKEY, DWORD, LPSTR, LPDWORD, LPDWORD, LPSTR, LPDWORD, PFILETIME), (const));
    MOCK_METHOD(LSTATUS, RegEnumValueA, (HKEY, DWORD, LPSTR, LPDWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD), (const));
    MOCK_METHOD(LSTATUS, RegCloseKey, (HKEY), (const));
};

#endif  // MOCK_WINDOWS_REGISTRY_HPP
