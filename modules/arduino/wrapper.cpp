#include "wrapper.hpp"



bool ArduinoWrapper::connect(const std::string& deviceName) {
    if (isConnected()) {
        return true;
    }

    serialPort = new Serial(deviceName, 9600);
    if (serialPort->IsConnected()) {
        return true;
    }
    else {
        return false;
    }
}

void ArduinoWrapper::disconnect() {
    if (isConnected()) {
        serialPort->Close();
        delete serialPort;
        serialPort = nullptr;
    }
}

bool ArduinoWrapper::writeData(int value) {
    if (!isConnected()) {
        return false;
    }

    std::string cmd = "write:" + std::to_string(value) + ";";
    serialPort->WriteLine(cmd);
    return true;
}

bool ArduinoWrapper::readData(int& result) {
    if (!isConnected()) {
        return false;
    }

    std::string response = serialPort->ReadLine();
    if (response.empty()) {
        return false;
    }

    result = std::stoi(response);
    return true;
}

bool ArduinoWrapper::isConnected() const {
    if (serialPort == nullptr) {
        return false;
    }
    else {
        return serialPort->IsConnected();
    }
}

#ifdef _WIN32

Serial::Serial(const std::string& deviceName, int baudRate)
    : hComm(INVALID_HANDLE_VALUE) {
    hComm = CreateFileA(
        deviceName.c_str(), GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hComm == INVALID_HANDLE_VALUE) {
        return;
    }

    DCB dcb;
    GetCommState(hComm, &dcb);
    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(hComm, &dcb);

    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hComm, &timeouts);

    PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

Serial::~Serial() {
    if (hComm != INVALID_HANDLE_VALUE) {
        CloseHandle(hComm);
    }
}

bool Serial::IsConnected() const {
    return hComm != INVALID_HANDLE_VALUE;
}

bool Serial::WriteLine(const std::string& data) {
    if (!IsConnected()) {
        return false;
    }

    DWORD bytesWritten;
    std::string buffer = data + "\r\n";
    bool success = WriteFile(hComm, buffer.c_str(), buffer.length(), &bytesWritten, NULL);
    return success && bytesWritten == buffer.length();
}

std::string Serial::ReadLine() {
    if (!IsConnected()) {
        return "";
    }

    std::string result = "";
    char buffer[1];
    DWORD bytesRead;

    do {
        if (ReadFile(hComm, buffer, 1, &bytesRead, NULL) && bytesRead > 0) {
            result += buffer[0];
        }
        else {
            break;
        }
    } while (buffer[0] != '\n' && buffer[0] != '\r');

    return result;
}

#else
Serial::Serial(const std::string& deviceName, int baudRate)
    : fd(-1) {
    fd = open(deviceName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        return;
    }

    fcntl(fd, F_SETFL, 0);

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    tcsetattr(fd, TCSANOW, &options);

    tcflush(fd, TCIOFLUSH);
}

Serial::~Serial() {
    if (fd != -1) {
        close(fd);
    }
}

bool Serial::IsConnected() const {
    return fd != -1;
}

void Serial::Close() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

bool Serial::WriteLine(const std::string& data) {
    if (!IsConnected()) {
        return false;
    }

    std::string buffer = data + "\r\n";
    ssize_t bytesWritten = write(fd, buffer.c_str(), buffer.length());
    return bytesWritten == buffer.length();
}

std::string Serial::ReadLine() {
    if (!IsConnected()) {
        return "";
    }

    std::string result = "";
    char buffer[1];

    do {
        ssize_t bytesRead = read(fd, buffer, 1);
        if (bytesRead > 0) {
            result += buffer[0];
        }
        else {
            break;
        }
    } while (buffer[0] != '\n' && buffer[0] != '\r');

    return result;
}
#endif
