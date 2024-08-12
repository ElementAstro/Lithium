#include "ttybase.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

#include "atom/log/loguru.hpp"

TTYBase::~TTYBase() {
    if (m_PortFD != -1) {
        disconnect();
    }
}

TTYBase::TTYResponse TTYBase::checkTimeout(uint8_t timeout) {
#ifdef _WIN32
    // Windows specific implementation
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = timeout;
    timeouts.ReadTotalTimeoutConstant = timeout * 1000;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = timeout * 1000;
    timeouts.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(reinterpret_cast<HANDLE>(m_PortFD), &timeouts))
        return TTYResponse::Errno;

    return TTYResponse::OK;
#else
    if (m_PortFD == -1) {
        return TTYResponse::Errno;
    }

    struct timeval tv;
    fd_set readout;
    int retval;

    FD_ZERO(&readout);
    FD_SET(m_PortFD, &readout);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    retval = select(m_PortFD + 1, &readout, nullptr, nullptr, &tv);

    if (retval > 0) {
        return TTYResponse::OK;
    }
    if (retval == -1) {
        return TTYResponse::SelectError;
    }
    return TTYResponse::Timeout;
#endif
}

TTYBase::TTYResponse TTYBase::write(const uint8_t* buffer, uint32_t nbytes,
                                    uint32_t& nbytesWritten) {
    if (m_PortFD == -1)
        return TTYResponse::Errno;

#ifdef _WIN32
    // Windows specific write implementation
    DWORD bytesWritten;
    if (!WriteFile(reinterpret_cast<HANDLE>(m_PortFD), buffer, nbytes,
                   &bytesWritten, nullptr))
        return TTYResponse::WriteError;

    nbytesWritten = bytesWritten;
    return TTYResponse::OK;
#else
    int bytesW = 0;
    nbytesWritten = 0;

    while (nbytes > 0) {
        bytesW = ::write(m_PortFD, buffer + nbytesWritten, nbytes);

        if (bytesW < 0) {
            return TTYResponse::WriteError;
        }

        nbytesWritten += bytesW;
        nbytes -= bytesW;
    }

    return TTYResponse::OK;
#endif
}

TTYBase::TTYResponse TTYBase::writeString(std::string_view string,
                                          uint32_t& nbytesWritten) {
    return write(reinterpret_cast<const uint8_t*>(string.data()), string.size(),
                 nbytesWritten);
}

TTYBase::TTYResponse TTYBase::read(uint8_t* buffer, uint32_t nbytes,
                                   uint8_t timeout, uint32_t& nbytesRead) {
    if (m_PortFD == -1) {
        return TTYResponse::Errno;
    }

#ifdef _WIN32
    // Windows specific read implementation
    DWORD bytesRead;
    if (!ReadFile(reinterpret_cast<HANDLE>(m_PortFD), buffer, nbytes,
                  &bytesRead, nullptr))
        return TTYResponse::ReadError;

    nbytesRead = bytesRead;
    return TTYResponse::OK;
#else
    uint32_t numBytesToRead = nbytes;
    int bytesRead = 0;
    TTYResponse timeoutResponse = TTYResponse::OK;
    nbytesRead = 0;

    while (numBytesToRead > 0) {
        if ((timeoutResponse = checkTimeout(timeout)) != TTYResponse::OK) {
            return timeoutResponse;
        }

        bytesRead = ::read(m_PortFD, buffer + nbytesRead, numBytesToRead);

        if (bytesRead < 0) {
            return TTYResponse::ReadError;
        }

        nbytesRead += bytesRead;
        numBytesToRead -= bytesRead;
    }

    return TTYResponse::OK;
#endif
}

TTYBase::TTYResponse TTYBase::readSection(uint8_t* buffer, uint32_t nsize,
                                          uint8_t stopByte, uint8_t timeout,
                                          uint32_t& nbytesRead) {
    if (m_PortFD == -1) {
        return TTYResponse::Errno;
    }

    nbytesRead = 0;
    memset(buffer, 0, nsize);

    while (nbytesRead < nsize) {
        if (auto timeoutResponse = checkTimeout(timeout);
            timeoutResponse != TTYResponse::OK) {
            return timeoutResponse;
        }

        uint8_t readChar;
        int bytesRead = ::read(m_PortFD, &readChar, 1);

        if (bytesRead < 0) {
            return TTYResponse::ReadError;
        }

        buffer[nbytesRead++] = readChar;

        if (readChar == stopByte) {
            return TTYResponse::OK;
        }
    }

    return TTYResponse::Overflow;
}

TTYBase::TTYResponse TTYBase::connect(std::string_view device, uint32_t bitRate,
                                      uint8_t wordSize, uint8_t parity,
                                      uint8_t stopBits) {
#ifdef _WIN32
    // Windows specific implementation
    HANDLE hSerial =
        CreateFile(device.data(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hSerial == INVALID_HANDLE_VALUE)
        return TTYResponse::PortFailure;

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        return TTYResponse::PortFailure;
    }

    dcbSerialParams.BaudRate = bitRate;
    dcbSerialParams.ByteSize = wordSize;
    dcbSerialParams.StopBits = (stopBits == 1) ? ONESTOPBIT : TWOSTOPBITS;
    dcbSerialParams.Parity = parity;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        CloseHandle(hSerial);
        return TTYResponse::PortFailure;
    }

    m_PortFD = reinterpret_cast<int>(hSerial);
    return TTYResponse::OK;
#elif defined(BSD) && !defined(__GNU__)
    int t_fd = -1;
    int bps;
    int handshake;
    struct termios tty_setting;

    // Open the serial port read/write, with no controlling terminal, and don't
    // wait for a connection. The O_NONBLOCK flag also causes subsequent I/O on
    // the device to be non-blocking. See open(2) ("man 2 open") for details.

    t_fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (t_fd == -1) {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error opening serial port (%s) - %s(%d).", device,
                     strerror(errno), errno);
        goto error;
    }

    // Note that open() follows POSIX semantics: multiple open() calls to the
    // same file will succeed unless the TIOCEXCL ioctl is issued. This will
    // prevent additional opens except by root-owned processes. See tty(4) ("man
    // 4 tty") and ioctl(2) ("man 2 ioctl") for details.

    if (ioctl(t_fd, TIOCEXCL) == -1) {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error setting TIOCEXCL on %s - %s(%d).", device,
                     strerror(errno), errno);
        goto error;
    }

    // Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O
    // will block. See fcntl(2) ("man 2 fcntl") for details.

    if (fcntl(t_fd, F_SETFL, 0) == -1) {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error clearing O_NONBLOCK %s - %s(%d).", device,
                     strerror(errno), errno);
        goto error;
    }

    // Get the current options and save them so we can restore the default
    // settings later.
    if (tcgetattr(t_fd, &tty_setting) == -1) {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error getting tty attributes %s - %s(%d).", device,
                     strerror(errno), errno);
        goto error;
    }

    // Set raw input (non-canonical) mode, with reads blocking until either a
    // single character has been received or a one second timeout expires. See
    // tcsetattr(4) ("man 4 tcsetattr") and termios(4) ("man 4 termios") for
    // details.

    cfmakeraw(&tty_setting);
    tty_setting.c_cc[VMIN] = 1;
    tty_setting.c_cc[VTIME] = 10;

    // The baud rate, word length, and handshake options can be set as follows:
    switch (bit_rate) {
        case 0:
            bps = B0;
            break;
        case 50:
            bps = B50;
            break;
        case 75:
            bps = B75;
            break;
        case 110:
            bps = B110;
            break;
        case 134:
            bps = B134;
            break;
        case 150:
            bps = B150;
            break;
        case 200:
            bps = B200;
            break;
        case 300:
            bps = B300;
            break;
        case 600:
            bps = B600;
            break;
        case 1200:
            bps = B1200;
            break;
        case 1800:
            bps = B1800;
            break;
        case 2400:
            bps = B2400;
            break;
        case 4800:
            bps = B4800;
            break;
        case 9600:
            bps = B9600;
            break;
        case 19200:
            bps = B19200;
            break;
        case 38400:
            bps = B38400;
            break;
        case 57600:
            bps = B57600;
            break;
        case 115200:
            bps = B115200;
            break;
        case 230400:
            bps = B230400;
            break;
        default:
            DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                         "connect: %d is not a valid bit rate.", bit_rate);
            return TTY_PARAM_ERROR;
    }

    cfsetspeed(&tty_setting, bps);  // Set baud rate
    /* word size */
    switch (word_size) {
        case 5:
            tty_setting.c_cflag |= CS5;
            break;
        case 6:
            tty_setting.c_cflag |= CS6;
            break;
        case 7:
            tty_setting.c_cflag |= CS7;
            break;
        case 8:
            tty_setting.c_cflag |= CS8;
            break;
        default:
            DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                         "connect: %d is not a valid data bit count.",
                         word_size);
            return TTY_PARAM_ERROR;
    }

    /* parity */
    switch (parity) {
        case PARITY_NONE:
            break;
        case PARITY_EVEN:
            tty_setting.c_cflag |= PARENB;
            break;
        case PARITY_ODD:
            tty_setting.c_cflag |= PARENB | PARODD;
            break;
        default:
            DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                         "connect: %d is not a valid parity selection value.",
                         parity);
            return TTY_PARAM_ERROR;
    }

    /* stop_bits */
    switch (stop_bits) {
        case 1:
            break;
        case 2:
            tty_setting.c_cflag |= CSTOPB;
            break;
        default:
            DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                         "connect: %d is not a valid stop bit count.",
                         stop_bits);
            return TTY_PARAM_ERROR;
    }

#if defined(MAC_OS_X_VERSION_10_4) && \
    (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_4)
    // Starting with Tiger, the IOSSIOSPEED ioctl can be used to set arbitrary
    // baud rates other than those specified by POSIX. The driver for the
    // underlying serial hardware ultimately determines which baud rates can be
    // used. This ioctl sets both the input and output speed.

    speed_t speed = 14400;  // Set 14400 baud
    if (ioctl(t_fd, IOSSIOSPEED, &speed) == -1) {
        IDLog("Error calling ioctl(..., IOSSIOSPEED, ...) - %s(%d).\n",
              strerror(errno), errno);
    }
#endif

    // Cause the new options to take effect immediately.
    if (tcsetattr(t_fd, TCSANOW, &tty_setting) == -1) {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error setting tty attributes %s - %s(%d).", device,
                     strerror(errno), errno);
        goto error;
    }

    // To set the modem handshake lines, use the following ioctls.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.

    if (ioctl(t_fd, TIOCSDTR) == -1)  // Assert Data Terminal Ready (DTR)
    {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error asserting DTR %s - %s(%d).", device,
                     strerror(errno), errno);
    }

    if (ioctl(t_fd, TIOCCDTR) == -1)  // Clear Data Terminal Ready (DTR)
    {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error clearing DTR %s - %s(%d).", device, strerror(errno),
                     errno);
    }

    handshake = TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR;
    if (ioctl(t_fd, TIOCMSET, &handshake) == -1)
    // Set the modem lines depending on the bits set in handshake
    {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error setting handshake lines %s - %s(%d).", device,
                     strerror(errno), errno);
    }

    // To read the state of the modem lines, use the following ioctl.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.

    if (ioctl(t_fd, TIOCMGET, &handshake) == -1)
    // Store the state of the modem lines in handshake
    {
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error getting handshake lines %s - %s(%d).", device,
                     strerror(errno), errno);
    }

    DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                 "Handshake lines currently set to %d", handshake);

#if defined(MAC_OS_X_VERSION_10_3) && \
    (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_3)
    unsigned long mics = 1UL;

    // Set the receive latency in microseconds. Serial drivers use this value to
    // determine how often to dequeue characters received by the hardware. Most
    // applications don't need to set this value: if an app reads lines of
    // characters, the app can't do anything until the line termination
    // character has been received anyway. The most common applications which
    // are sensitive to read latency are MIDI and IrDA applications.

    if (ioctl(t_fd, IOSSDATALAT, &mics) == -1) {
        // set latency to 1 microsecond
        DEBUGFDEVICE(m_DriverName, m_DebugChannel,
                     "Error setting read latency %s - %s(%d).\n", device,
                     strerror(errno), errno);
        goto error;
    }
#endif

    m_PortFD = t_fd;
    /* return success */
    return TTY_OK;

    // Failure path
error:
    if (t_fd != -1) {
        close(t_fd);
        m_PortFD = -1;
    }

    return TTY_PORT_FAILURE;
#else
    int tFd = open(device.data(), O_RDWR | O_NOCTTY);
    if (tFd == -1) {
        LOG_F(ERROR, "Error opening {}: {}", device.data(), strerror(errno));
        m_PortFD = -1;
        return TTYResponse::PortFailure;
    }

    termios ttySetting{};
    if (tcgetattr(tFd, &ttySetting) == -1) {
        LOG_F(ERROR, "Error getting {} tty attributes: {}", device.data(),
              strerror(errno));
        return TTYResponse::PortFailure;
    }

    int bps;
    switch (bitRate) {
        case 0:
            bps = B0;
            break;
        case 50:
            bps = B50;
            break;
        case 75:
            bps = B75;
            break;
        case 110:
            bps = B110;
            break;
        case 134:
            bps = B134;
            break;
        case 150:
            bps = B150;
            break;
        case 200:
            bps = B200;
            break;
        case 300:
            bps = B300;
            break;
        case 600:
            bps = B600;
            break;
        case 1200:
            bps = B1200;
            break;
        case 1800:
            bps = B1800;
            break;
        case 2400:
            bps = B2400;
            break;
        case 4800:
            bps = B4800;
            break;
        case 9600:
            bps = B9600;
            break;
        case 19200:
            bps = B19200;
            break;
        case 38400:
            bps = B38400;
            break;
        case 57600:
            bps = B57600;
            break;
        case 115200:
            bps = B115200;
            break;
        case 230400:
            bps = B230400;
            break;
        default:
            LOG_F(ERROR, "connect: {} is not a valid bit rate.", bitRate);
            return TTYResponse::ParamError;
    }

    // Set baud rate
    if ((cfsetispeed(&ttySetting, bps) < 0) ||
        (cfsetospeed(&ttySetting, bps) < 0)) {
        LOG_F(ERROR, "connect: failed setting bit rate.");
        return TTYResponse::PortFailure;
    }

    ttySetting.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD | HUPCL | CRTSCTS);
    ttySetting.c_cflag |= (CLOCAL | CREAD);

    // Set word size
    switch (wordSize) {
        case 5:
            ttySetting.c_cflag |= CS5;
            break;
        case 6:
            ttySetting.c_cflag |= CS6;
            break;
        case 7:
            ttySetting.c_cflag |= CS7;
            break;
        case 8:
            ttySetting.c_cflag |= CS8;
            break;
        default:
            LOG_F(ERROR, "connect: {} is not a valid data bit count.",
                  wordSize);
            return TTYResponse::ParamError;
    }

    // Set parity
    if (parity == 1) {
        ttySetting.c_cflag |= PARENB;
    } else if (parity == 2) {
        ttySetting.c_cflag |= PARENB | PARODD;
    } else {
        LOG_F(ERROR, "connect: {} is not a valid parity setting.", parity);
        return TTYResponse::ParamError;
    }

    // Set stop bits
    if (stopBits == 2) {
        ttySetting.c_cflag |= CSTOPB;
    } else if (stopBits != 1) {
        LOG_F(ERROR, "connect: {} is not a valid stop bit count.", stopBits);
        return TTYResponse::ParamError;
    }

    /* Ignore bytes with parity errors and make terminal raw and dumb.*/
    ttySetting.c_iflag &=
        ~(PARMRK | ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON | IXANY);
    ttySetting.c_iflag |= INPCK | IGNPAR | IGNBRK;

    /* Raw output.*/
    ttySetting.c_oflag &= ~(OPOST | ONLCR);

    /* Local Modes
    Don't echo characters. Don't generate signals.
    Don't process any characters.*/
    ttySetting.c_lflag &=
        ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN | NOFLSH | TOSTOP);
    ttySetting.c_lflag |= NOFLSH;

    /* blocking read until 1 char arrives */
    ttySetting.c_cc[VMIN] = 1;
    ttySetting.c_cc[VTIME] = 0;

    tcflush(tFd, TCIOFLUSH);

    // Set raw input mode (non-canonical)
    cfmakeraw(&ttySetting);

    // Set the new attributes for the port
    if (tcsetattr(tFd, TCSANOW, &ttySetting) != 0) {
        close(tFd);
        return TTYResponse::PortFailure;
    }

    m_PortFD = tFd;
    return TTYResponse::OK;
#endif
}

TTYBase::TTYResponse TTYBase::disconnect() {
    if (m_PortFD == -1) {
        return TTYResponse::Errno;
    }

#ifdef _WIN32
    // Windows specific disconnection
    if (!CloseHandle(reinterpret_cast<HANDLE>(m_PortFD)))
        return TTYResponse::Errno;

    m_PortFD = -1;
    return TTYResponse::OK;
#else
    if (tcflush(m_PortFD, TCIOFLUSH) != 0 || close(m_PortFD) != 0) {
        return TTYResponse::Errno;
    }

    m_PortFD = -1;
    return TTYResponse::OK;
#endif
}

void TTYBase::setDebug(bool enabled) {
    m_Debug = enabled;
    if (m_Debug)
        LOG_F(INFO, "Debugging enabled.");
    else
        LOG_F(INFO, "Debugging disabled.");
}

std::string TTYBase::getErrorMessage(TTYResponse code) const {
    switch (code) {
        case TTYResponse::OK:
            return "No Error";
        case TTYResponse::ReadError:
            return "Read Error: " + std::string(strerror(errno));
        case TTYResponse::WriteError:
            return "Write Error: " + std::string(strerror(errno));
        case TTYResponse::SelectError:
            return "Select Error: " + std::string(strerror(errno));
        case TTYResponse::Timeout:
            return "Timeout Error";
        case TTYResponse::PortFailure:
            if (errno == EACCES) {
                return "Port failure: Access denied. Try adding your user to "
                       "the dialout group and restart (sudo adduser $USER "
                       "dialout)";
            } else {
                return "Port failure: " + std::string(strerror(errno)) +
                       ". Check if the device is connected to this port.";
            }
        case TTYResponse::ParamError:
            return "Parameter Error";
        case TTYResponse::Errno:
            return "Error: " + std::string(strerror(errno));
        case TTYResponse::Overflow:
            return "Read Overflow Error";
        default:
            return "Unknown Error";
    }
}
