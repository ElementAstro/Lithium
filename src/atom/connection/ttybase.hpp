#ifndef ATOM_CONNECTION_TTYBASE_HPP
#define ATOM_CONNECTION_TTYBASE_HPP

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <string_view>

// Windows specific includes
#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

class TTYBase {
public:
    enum class TTYResponse {
        OK = 0,
        ReadError = -1,
        WriteError = -2,
        SelectError = -3,
        Timeout = -4,
        PortFailure = -5,
        ParamError = -6,
        Errno = -7,
        Overflow = -8
    };

    explicit TTYBase(std::string_view driverName) : m_DriverName(driverName) {}
    virtual ~TTYBase();

    TTYResponse read(uint8_t* buffer, uint32_t nbytes, uint8_t timeout,
                     uint32_t& nbytesRead);
    TTYResponse readSection(uint8_t* buffer, uint32_t nsize, uint8_t stopByte,
                            uint8_t timeout, uint32_t& nbytesRead);
    TTYResponse write(const uint8_t* buffer, uint32_t nbytes,
                      uint32_t& nbytesWritten);
    TTYResponse writeString(std::string_view string, uint32_t& nbytesWritten);
    TTYResponse connect(std::string_view device, uint32_t bitRate,
                        uint8_t wordSize, uint8_t parity, uint8_t stopBits);
    TTYResponse disconnect();
    void setDebug(bool enabled);
    std::string getErrorMessage(TTYResponse code) const;

    int getPortFD() const { return m_PortFD; }

private:
    TTYResponse checkTimeout(uint8_t timeout);

    int m_PortFD{-1};
    bool m_Debug{false};
    std::string_view m_DriverName;
};

#endif
