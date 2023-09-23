#include "device_utils.hpp"

#ifdef _WIN32
#include <windows.h>
#else

#endif
#include <iostream>
#include <array>
#include <memory>
#include <stdexcept>

#include <sstream>

std::string execute_command(const std::string &cmd)
{
    std::array<char, 128> buffer;
    std::string result = "";

    HANDLE pipeOutRead, pipeOutWrite;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&pipeOutRead, &pipeOutWrite, &saAttr, 0))
    {
        throw std::runtime_error("Failed to create pipe!");
    }

    // 设置管道输出为非继承模式
    if (!SetHandleInformation(pipeOutRead, HANDLE_FLAG_INHERIT, 0))
    {
        throw std::runtime_error("Failed to set pipe handle information!");
    }

    // 创建进程并重定向标准输出到管道写入端
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = pipeOutWrite;
    si.hStdOutput = pipeOutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, const_cast<char *>(cmd.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        throw std::runtime_error("Failed to execute command!");
    }

    // 关闭管道写入端
    CloseHandle(pipeOutWrite);

    // 读取命令输出
    DWORD bytesRead;
    while (ReadFile(pipeOutRead, buffer.data(), buffer.size(), &bytesRead, NULL))
    {
        if (bytesRead == 0)
        {
            break;
        }
        result.append(buffer.data(), bytesRead);
    }

    // 关闭管道读取端
    CloseHandle(pipeOutRead);

    // 等待进程结束
    WaitForSingleObject(pi.hProcess, INFINITE);

    // 关闭进程和线程句柄
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return result;
}