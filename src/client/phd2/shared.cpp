#include "shared.hpp"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>
#include <format>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

using namespace std::literals;

class PHDSharedMemoryClient::Impl {
public:
    Impl() { initSharedMemory(); }

    ~Impl() {
        if (sharedMemory_) {
            shmdt(sharedMemory_);
        }
    }

    void initSharedMemory() {
        key_t key = ftok("../", 2015);
        if (key == -1) {
            throw std::runtime_error("Failed to create key for shared memory");
        }

        shmid_ = shmget(key, 4096, IPC_CREAT | 0666);
        if (shmid_ < 0) {
            throw std::runtime_error("Failed to get shared memory");
        }

        sharedMemory_ = static_cast<char*>(shmat(shmid_, nullptr, 0));
        if (sharedMemory_ == nullptr) {
            throw std::runtime_error("Failed to attach shared memory");
        }
    }

    bool sendCommand(unsigned int vendCommand) {
        constexpr unsigned int baseAddress = 0x03;

        std::memset(sharedMemory_, 0, 1024);

        sharedMemory_[1] = msb(vendCommand);
        sharedMemory_[2] = lsb(vendCommand);
        sharedMemory_[0] = 0x01;  // enable command

        return waitForResponse(500ms);
    }

    bool waitForResponse(std::chrono::milliseconds timeout) {
        auto start = std::chrono::steady_clock::now();
        while (sharedMemory_[0] == 0x01) {
            if (std::chrono::steady_clock::now() - start > timeout) {
                return false;
            }
            std::this_thread::sleep_for(1ms);
        }
        return true;
    }

    static unsigned char msb(unsigned int value) {
        return static_cast<unsigned char>((value >> 8) & 0xFF);
    }

    static unsigned char lsb(unsigned int value) {
        return static_cast<unsigned char>(value & 0xFF);
    }

    int shmid_;
    char* sharedMemory_;
    std::mutex mutex_;

    // Additional member variables for new functions
    double starX_ = 0.0;
    double starY_ = 0.0;
    double rmsError_ = 0.0;
};

PHDSharedMemoryClient::PHDSharedMemoryClient()
    : pImpl(std::make_unique<Impl>()) {}

PHDSharedMemoryClient::~PHDSharedMemoryClient() = default;

PHDSharedMemoryClient::PHDSharedMemoryClient(PHDSharedMemoryClient&&) noexcept =
    default;
PHDSharedMemoryClient& PHDSharedMemoryClient::operator=(
    PHDSharedMemoryClient&&) noexcept = default;

bool PHDSharedMemoryClient::connectPHD() {
    std::string versionName;
    if (call_phd_GetVersion(versionName)) {
        std::cout << "QSCOPE|connectPHD|version: " << versionName << std::endl;
        return true;
    } else {
        std::cout << "QSCOPE|connectPHD|error: there is no openPHD2 running"
                  << std::endl;
        return false;
    }
}

bool PHDSharedMemoryClient::call_phd_GetVersion(std::string& versionName) {
    constexpr unsigned int baseAddress = 0x03;
    constexpr unsigned int vendCommand = 0x01;

    std::memset(pImpl->sharedMemory_, 0, 1024);

    pImpl->sharedMemory_[1] = Impl::msb(vendCommand);
    pImpl->sharedMemory_[2] = Impl::lsb(vendCommand);
    pImpl->sharedMemory_[0] = 0x01;  // enable command

    if (!pImpl->waitForResponse(500ms)) {
        versionName.clear();
        return false;
    }

    unsigned char addr = 0;
    uint16_t length;
    std::memcpy(&length, pImpl->sharedMemory_ + baseAddress + addr,
                sizeof(uint16_t));
    addr += sizeof(uint16_t);

    if (length > 0 && length < 1024) {
        versionName.assign(pImpl->sharedMemory_ + baseAddress + addr, length);
        return true;
    } else {
        versionName.clear();
        return false;
    }
}

bool PHDSharedMemoryClient::call_phd_StartLooping() {
    return pImpl->sendCommand(0x03);
}

bool PHDSharedMemoryClient::call_phd_StopLooping() {
    return pImpl->sendCommand(0x04);
}

bool PHDSharedMemoryClient::call_phd_AutoFindStar() {
    return pImpl->sendCommand(0x05);
}

bool PHDSharedMemoryClient::call_phd_StartGuiding() {
    return pImpl->sendCommand(0x06);
}

bool PHDSharedMemoryClient::call_phd_checkStatus(unsigned char& status) {
    constexpr unsigned int baseAddress = 0x03;
    constexpr unsigned int vendCommand = 0x07;

    std::memset(pImpl->sharedMemory_, 0, 1024);

    pImpl->sharedMemory_[1] = Impl::msb(vendCommand);
    pImpl->sharedMemory_[2] = Impl::lsb(vendCommand);
    pImpl->sharedMemory_[0] = 0x01;  // enable command

    if (!pImpl->waitForResponse(500ms)) {
        status = 0;
        return false;
    }

    status = pImpl->sharedMemory_[3];
    return true;
}

bool PHDSharedMemoryClient::call_phd_setExposureTime(unsigned int expTime) {
    constexpr unsigned int baseAddress = 0x03;
    constexpr unsigned int vendCommand = 0x0b;

    std::memset(pImpl->sharedMemory_, 0, 1024);

    pImpl->sharedMemory_[1] = Impl::msb(vendCommand);
    pImpl->sharedMemory_[2] = Impl::lsb(vendCommand);

    std::memcpy(pImpl->sharedMemory_ + baseAddress, &expTime,
                sizeof(unsigned int));

    pImpl->sharedMemory_[0] = 0x01;  // enable command

    return pImpl->waitForResponse(500ms);
}

bool PHDSharedMemoryClient::call_phd_whichCamera(const std::string& camera) {
    constexpr unsigned int baseAddress = 0x03;
    constexpr unsigned int vendCommand = 0x0d;

    std::memset(pImpl->sharedMemory_, 0, 1024);

    pImpl->sharedMemory_[1] = Impl::msb(vendCommand);
    pImpl->sharedMemory_[2] = Impl::lsb(vendCommand);

    int length = camera.length() + 1;
    unsigned char addr = 0;

    std::memcpy(pImpl->sharedMemory_ + baseAddress + addr, &length,
                sizeof(int));
    addr += sizeof(int);
    std::memcpy(pImpl->sharedMemory_ + baseAddress + addr, camera.c_str(),
                length);

    pImpl->sharedMemory_[0] = 0x01;  // enable command

    return pImpl->waitForResponse(500ms);
}

bool PHDSharedMemoryClient::call_phd_ChackControlStatus(int sdk_num) {
    constexpr unsigned int baseAddress = 0x03;
    constexpr unsigned int vendCommand = 0x0e;

    std::memset(pImpl->sharedMemory_, 0, 1024);

    pImpl->sharedMemory_[1] = Impl::msb(vendCommand);
    pImpl->sharedMemory_[2] = Impl::lsb(vendCommand);

    std::memcpy(pImpl->sharedMemory_ + baseAddress, &sdk_num, sizeof(int));

    pImpl->sharedMemory_[0] = 0x01;  // enable command

    return pImpl->waitForResponse(500ms);
}

bool PHDSharedMemoryClient::call_phd_ClearCalibration() {
    return pImpl->sendCommand(0x02);
}

void PHDSharedMemoryClient::showPHDData() {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);

    if (pImpl->sharedMemory_[2047] != 0x02)
        return;

    unsigned int currentPHDSizeX, currentPHDSizeY;
    unsigned char bitDepth;
    double dRa, dDec, SNR, MASS, RMSErrorX, RMSErrorY, RMSErrorTotal,
        PixelRatio;
    int RADUR, DECDUR;
    char RADIR, DECDIR;
    bool StarLostAlert, InGuiding;

    unsigned int mem_offset = 1024;
    std::memcpy(&currentPHDSizeX, pImpl->sharedMemory_ + mem_offset,
                sizeof(unsigned int));
    mem_offset += sizeof(unsigned int);
    std::memcpy(&currentPHDSizeY, pImpl->sharedMemory_ + mem_offset,
                sizeof(unsigned int));
    mem_offset += sizeof(unsigned int);
    std::memcpy(&bitDepth, pImpl->sharedMemory_ + mem_offset,
                sizeof(unsigned char));
    mem_offset += sizeof(unsigned char);

    // Skip sdk_num, sdk_direction, sdk_duration
    mem_offset += 3 * sizeof(int);

    // Guide error data
    mem_offset += sizeof(unsigned char);  // guideDataIndicator
    std::memcpy(&dRa, pImpl->sharedMemory_ + mem_offset, sizeof(double));
    mem_offset += sizeof(double);
    std::memcpy(&dDec, pImpl->sharedMemory_ + mem_offset, sizeof(double));
    mem_offset += sizeof(double);
    std::memcpy(&SNR, pImpl->sharedMemory_ + mem_offset, sizeof(double));
    mem_offset += sizeof(double);
    std::memcpy(&MASS, pImpl->sharedMemory_ + mem_offset, sizeof(double));
    mem_offset += sizeof(double);
    std::memcpy(&RADUR, pImpl->sharedMemory_ + mem_offset, sizeof(int));
    mem_offset += sizeof(int);
    std::memcpy(&DECDUR, pImpl->sharedMemory_ + mem_offset, sizeof(int));
    mem_offset += sizeof(int);
    std::memcpy(&RADIR, pImpl->sharedMemory_ + mem_offset, sizeof(char));
    mem_offset += sizeof(char);
    std::memcpy(&DECDIR, pImpl->sharedMemory_ + mem_offset, sizeof(char));
    mem_offset += sizeof(char);
    std::memcpy(&RMSErrorX, pImpl->sharedMemory_ + mem_offset, sizeof(double));
    mem_offset += sizeof(double);
    std::memcpy(&RMSErrorY, pImpl->sharedMemory_ + mem_offset, sizeof(double));
    mem_offset += sizeof(double);
    std::memcpy(&RMSErrorTotal, pImpl->sharedMemory_ + mem_offset,
                sizeof(double));
    mem_offset += sizeof(double);
    std::memcpy(&PixelRatio, pImpl->sharedMemory_ + mem_offset, sizeof(double));
    mem_offset += sizeof(double);
    std::memcpy(&StarLostAlert, pImpl->sharedMemory_ + mem_offset,
                sizeof(bool));
    mem_offset += sizeof(bool);
    std::memcpy(&InGuiding, pImpl->sharedMemory_ + mem_offset, sizeof(bool));

    // Update member variables
    pImpl->starX_ = dRa;
    pImpl->starY_ = dDec;
    pImpl->rmsError_ = RMSErrorTotal;

    // Process and use the data as needed
    std::cout << std::format("RMSErrorX: {:.3f}, RMSErrorY: {:.3f}", RMSErrorX,
                             RMSErrorY)
              << std::endl;

    // Clear the data indicator
    pImpl->sharedMemory_[2047] = 0x00;
}

void PHDSharedMemoryClient::controlGuide(int direction, int duration) {
    // Implement the guide control logic here
    // This might involve sending commands to the mount or updating shared
    // memory
    std::cout << std::format("ControlGuide: Direction={}, Duration={}",
                             direction, duration)
              << std::endl;

    // Example implementation (you may need to adjust this based on your
    // specific requirements):
    constexpr unsigned int baseAddress = 0x03;
    constexpr unsigned int vendCommand =
        0x0F;  // Assuming 0x0F is the command for guide control

    std::memset(pImpl->sharedMemory_, 0, 1024);

    pImpl->sharedMemory_[1] = Impl::msb(vendCommand);
    pImpl->sharedMemory_[2] = Impl::lsb(vendCommand);

    std::memcpy(pImpl->sharedMemory_ + baseAddress, &direction, sizeof(int));
    std::memcpy(pImpl->sharedMemory_ + baseAddress + sizeof(int), &duration,
                sizeof(int));

    pImpl->sharedMemory_[0] = 0x01;  // enable command

    pImpl->waitForResponse(500ms);
}

void PHDSharedMemoryClient::getPHD2ControlInstruct() {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);

    unsigned int mem_offset =
        1024 + 2 * sizeof(unsigned int) + sizeof(unsigned char);

    int controlInstruct = 0;
    std::memcpy(&controlInstruct, pImpl->sharedMemory_ + mem_offset,
                sizeof(int));

    int sdk_num = (controlInstruct >> 24) & 0xFFF;
    int sdk_direction = (controlInstruct >> 12) & 0xFFF;
    int sdk_duration = controlInstruct & 0xFFF;

    if (sdk_num != 0) {
        std::cout
            << std::format(
                   "PHD2ControlTelescope: num={}, direction={}, duration={}",
                   sdk_num, sdk_direction, sdk_duration)
            << std::endl;
    }

    if (sdk_duration != 0) {
        controlGuide(sdk_direction, sdk_duration);

        int zero = 0;
        std::memcpy(pImpl->sharedMemory_ + mem_offset, &zero, sizeof(int));

        call_phd_ChackControlStatus(sdk_num);
    }
}

// New functions implementation

bool PHDSharedMemoryClient::startCalibration() {
    constexpr unsigned int vendCommand =
        0x10;  // Assuming 0x10 is the command for starting calibration
    return pImpl->sendCommand(vendCommand);
}

bool PHDSharedMemoryClient::abortCalibration() {
    constexpr unsigned int vendCommand =
        0x11;  // Assuming 0x11 is the command for aborting calibration
    return pImpl->sendCommand(vendCommand);
}

bool PHDSharedMemoryClient::dither(double pixels) {
    constexpr unsigned int baseAddress = 0x03;
    constexpr unsigned int vendCommand =
        0x12;  // Assuming 0x12 is the command for dithering

    std::memset(pImpl->sharedMemory_, 0, 1024);

    pImpl->sharedMemory_[1] = Impl::msb(vendCommand);
    pImpl->sharedMemory_[2] = Impl::lsb(vendCommand);

    std::memcpy(pImpl->sharedMemory_ + baseAddress, &pixels, sizeof(double));

    pImpl->sharedMemory_[0] = 0x01;  // enable command

    return pImpl->waitForResponse(500ms);
}

bool PHDSharedMemoryClient::setLockPosition(double x, double y) {
    constexpr unsigned int baseAddress = 0x03;
    constexpr unsigned int vendCommand =
        0x13;  // Assuming 0x13 is the command for setting lock position

    std::memset(pImpl->sharedMemory_, 0, 1024);

    pImpl->sharedMemory_[1] = Impl::msb(vendCommand);
    pImpl->sharedMemory_[2] = Impl::lsb(vendCommand);

    std::memcpy(pImpl->sharedMemory_ + baseAddress, &x, sizeof(double));
    std::memcpy(pImpl->sharedMemory_ + baseAddress + sizeof(double), &y,
                sizeof(double));

    pImpl->sharedMemory_[0] = 0x01;  // enable command

    return pImpl->waitForResponse(500ms);
}

std::pair<double, double> PHDSharedMemoryClient::getStarPosition() const {
    return {pImpl->starX_, pImpl->starY_};
}

double PHDSharedMemoryClient::getGuideRMSError() const {
    return pImpl->rmsError_;
}