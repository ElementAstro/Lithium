#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>

class Serial {
public:
    Serial(const std::string& deviceName, int baudRate);
    ~Serial();
    bool IsConnected() const;
    bool WriteLine(const std::string& data);
    std::string ReadLine();

private:
    HANDLE hComm;
};

#else
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

class Serial {
public:
    Serial(const std::string& deviceName, int baudRate);
    ~Serial();
    bool IsConnected() const;
    void Close();
    bool WriteLine(const std::string& data);
    std::string ReadLine();

private:
    int fd;
};
#endif

class ArduinoWrapper {
public:
    bool connect(const std::string& deviceName);
    void disconnect();
    bool writeData(int value);
    bool readData(int& result);
    bool isConnected() const;

private:
    Serial* serialPort = nullptr;
};

