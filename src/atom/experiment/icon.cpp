#include <windows.h>
#include <shlobj.h>
#include <shlguid.h>
#include <shlwapi.h>

LPSTR ConvertWCharToLPSTR(LPCWSTR str)
{
    LPSTR result = NULL;
    int size = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
    if (size > 0)
    {
        result = new char[size];
        if (result != NULL)
        {
            WideCharToMultiByte(CP_ACP, 0, str, -1, result, size, NULL, NULL);
        }
    }
    return result;
}

class CShellExt : public IShellIconOverlayIdentifier
{
public:
    // 用于标识扩展程序的 GUID
    static const GUID guid;

    // IUnknown 接口实现
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG)
    AddRef();
    STDMETHODIMP_(ULONG)
    Release();

    // IShellIconOverlayIdentifier 接口实现
    STDMETHODIMP GetOverlayInfo(LPWSTR, int, int *, DWORD *);
    STDMETHODIMP GetPriority(int *);
    STDMETHODIMP IsMemberOf(LPCWSTR, DWORD);
};

// 标识扩展程序的 GUID
const GUID CShellExt::guid = {0xE5A3D2E1, 0x2B9E, 0x4C9A, {0xAC, 0x5B, 0x8D, 0x7C, 0x6D, 0x92, 0x90, 0x81}};

// IUnknown 接口实现
STDMETHODIMP CShellExt::QueryInterface(REFIID riid, void **ppvObject)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IShellIconOverlayIdentifier))
    {
        *ppvObject = static_cast<IShellIconOverlayIdentifier *>(this);
        AddRef();
        return S_OK;
    }
    *ppvObject = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG)
CShellExt::AddRef()
{
    return 2; // 返回固定的引用计数值，避免不必要的内存分配和释放
}

STDMETHODIMP_(ULONG)
CShellExt::Release()
{
    return 1; // 返回固定的引用计数值，避免不必要的内存分配和释放
}

// IShellIconOverlayIdentifier 接口实现
STDMETHODIMP CShellExt::GetOverlayInfo(LPWSTR pwszIconFile, int cchMax, int *pIndex, DWORD *pdwFlags)
{
    wcscpy_s(pwszIconFile, cchMax, L"E:\\chat\\atom.png"); // 设置图标文件的路径
    *pIndex = 0;                                           // 使用第一个图标
    *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;          // 返回图标文件和索引信息
    return S_OK;
}

STDMETHODIMP CShellExt::GetPriority(int *pPriority)
{
    *pPriority = 0; // 优先级为 0
    return S_OK;
}

STDMETHODIMP CShellExt::IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib)
{
    if (PathMatchSpec(ConvertWCharToLPSTR(pwszPath), ConvertWCharToLPSTR(L"*.lithium")))
    {                // 判断文件类型是否为 .txt
        return S_OK; // 是 .txt 文件，支持图标
    }
    return S_FALSE; // 不是 .txt 文件，不支持图标
}

HRESULT RegOpenKeyExAFromW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    HRESULT hr = S_OK;
    int len = WideCharToMultiByte(CP_ACP, 0, lpSubKey, -1, NULL, 0, NULL, NULL);
    if (len > 0)
    {
        char *lpSubKeyA = new char[len];
        if (lpSubKeyA != NULL)
        {
            WideCharToMultiByte(CP_ACP, 0, lpSubKey, -1, lpSubKeyA, len, NULL, NULL);
            hr = RegOpenKeyExA(hKey, lpSubKeyA, ulOptions, samDesired, phkResult);
            delete[] lpSubKeyA;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

HRESULT GetModuleFileNameAFromW(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
    HRESULT hr = S_OK;
    WCHAR szFilename[MAX_PATH];
    if (GetModuleFileNameW(hModule, szFilename, MAX_PATH) != 0)
    {
        int len = WideCharToMultiByte(CP_ACP, 0, szFilename, -1, NULL, 0, NULL, NULL);
        if (len > 0)
        {
            if (len <= nSize)
            {
                WideCharToMultiByte(CP_ACP, 0, szFilename, -1, lpFilename, nSize, NULL, NULL);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

// 注册扩展程序
HRESULT RegisterShellExt()
{
    HKEY hKey = NULL;
    HRESULT hr = RegOpenKeyExAFromW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved", 0, KEY_WRITE, &hKey);

    if (hr != ERROR_SUCCESS)
    {
        return hr;
    }

    WCHAR szModule[MAX_PATH];
    GetModuleFileNameAFromW(NULL, ConvertWCharToLPSTR(szModule), MAX_PATH);

    WCHAR szCLSID[MAX_PATH];
    StringFromGUID2(CShellExt::guid, szCLSID, MAX_PATH);

    WCHAR szData[MAX_PATH] = L"Test Shell Extension";

    hr = RegSetValueEx(hKey, ConvertWCharToLPSTR(szCLSID), 0, REG_SZ, (LPBYTE)szData, (wcslen(szData) + 1) * sizeof(WCHAR));
    if (hr != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return hr;
    }

    hr = RegCreateKeyEx(HKEY_CLASSES_ROOT, ConvertWCharToLPSTR(L"CLSID\\{E5A3D2E1-2B9E-4C9A-AC5B-8D7C6D929081}"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (hr != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return hr;
    }

    WCHAR szModulePath[MAX_PATH];
    GetModuleFileName(NULL, ConvertWCharToLPSTR(szModulePath), MAX_PATH);
    PathRemoveFileSpec(ConvertWCharToLPSTR(szModulePath));

    WCHAR szInprocServer32[MAX_PATH];
    wcscpy_s(szInprocServer32, szModulePath);
    wcscat_s(szInprocServer32, L"\\testshell.dll");

    hr = RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)L"Test Shell Extension", (wcslen(L"Test Shell Extension") + 1) * sizeof(WCHAR));
    if (hr == ERROR_SUCCESS)
    {
        hr = RegSetValueEx(hKey, ConvertWCharToLPSTR(L"InProcServer32"), 0, REG_SZ, (LPBYTE)szInprocServer32, (wcslen(szInprocServer32) + 1) * sizeof(WCHAR));
    }

    RegCloseKey(hKey);
    return hr;
}

HRESULT RegDeleteTreeAFromW(HKEY hKey, LPCWSTR lpSubKey)
{
    HRESULT hr = S_OK;
    int len = WideCharToMultiByte(CP_ACP, 0, lpSubKey, -1, NULL, 0, NULL, NULL);
    if (len > 0)
    {
        char *lpSubKeyA = new char[len];
        if (lpSubKeyA != NULL)
        {
            WideCharToMultiByte(CP_ACP, 0, lpSubKey, -1, lpSubKeyA, len, NULL, NULL);
            hr = RegDeleteTreeA(hKey, lpSubKeyA);
            delete[] lpSubKeyA;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

// 注销扩展程序
HRESULT UnregisterShellExt()
{
    HKEY hKey = NULL;
    HRESULT hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, ConvertWCharToLPSTR(L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"), 0, KEY_WRITE, &hKey);
    if (hr != ERROR_SUCCESS)
    {
        return hr;
    }

    WCHAR szCLSID[MAX_PATH];
    StringFromGUID2(CShellExt::guid, szCLSID, MAX_PATH);

    hr = RegDeleteValue(hKey, ConvertWCharToLPSTR(szCLSID));
    RegCloseKey(hKey);
    if (hr != ERROR_SUCCESS)
    {
        return hr;
    }

    hr = RegDeleteTreeAFromW(HKEY_CLASSES_ROOT, L"CLSID\\{E5A3D2E1-2B9E-4C9A-AC5B-8D7C6D929081}");

    return hr;
}

// DLL 入口函数，用于注册和注销扩展程序
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // 注册扩展程序
        RegisterShellExt();
        break;
    case DLL_PROCESS_DETACH:
        // 注销扩展程序
        UnregisterShellExt();
        break;
    }
    return TRUE;
}
