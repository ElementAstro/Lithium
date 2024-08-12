#include "atom/sysinfo/disk.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <fstream>
#endif

using namespace atom::system;
using namespace testing;

#ifdef _WIN32
class MockWindowsApi {
public:
    MOCK_METHOD(DWORD, GetLogicalDrives, (), ());
    MOCK_METHOD(UINT, GetDriveTypeA, (LPCSTR), ());
    MOCK_METHOD(BOOL, GetDiskFreeSpaceExA,
                (LPCSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER),
                ());
    MOCK_METHOD(HANDLE, CreateFileA,
                (LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD,
                 HANDLE),
                ());
    MOCK_METHOD(BOOL, DeviceIoControl,
                (HANDLE, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD,
                 LPOVERLAPPED),
                ());
    MOCK_METHOD(BOOL, CloseHandle, (HANDLE), ());
};

MockWindowsApi* mockWindowsApi;

DWORD WINAPI MockGetLogicalDrives() {
    return mockWindowsApi->GetLogicalDrives();
}

UINT WINAPI MockGetDriveTypeA(LPCSTR drivePath) {
    return mockWindowsApi->GetDriveTypeA(drivePath);
}

BOOL WINAPI MockGetDiskFreeSpaceExA(LPCSTR drivePath,
                                    PULARGE_INTEGER freeBytesAvailable,
                                    PULARGE_INTEGER totalBytes,
                                    PULARGE_INTEGER totalFreeBytes) {
    return mockWindowsApi->GetDiskFreeSpaceExA(drivePath, freeBytesAvailable,
                                               totalBytes, totalFreeBytes);
}

HANDLE WINAPI MockCreateFileA(LPCSTR fileName, DWORD desiredAccess,
                              DWORD shareMode,
                              LPSECURITY_ATTRIBUTES securityAttributes,
                              DWORD creationDisposition,
                              DWORD flagsAndAttributes, HANDLE templateFile) {
    return mockWindowsApi->CreateFileA(fileName, desiredAccess, shareMode,
                                       securityAttributes, creationDisposition,
                                       flagsAndAttributes, templateFile);
}

BOOL WINAPI MockDeviceIoControl(HANDLE device, DWORD controlCode,
                                LPVOID inBuffer, DWORD inBufferSize,
                                LPVOID outBuffer, DWORD outBufferSize,
                                LPDWORD bytesReturned,
                                LPOVERLAPPED overlapped) {
    return mockWindowsApi->DeviceIoControl(
        device, controlCode, inBuffer, inBufferSize, outBuffer, outBufferSize,
        bytesReturned, overlapped);
}

BOOL WINAPI MockCloseHandle(HANDLE handle) {
    return mockWindowsApi->CloseHandle(handle);
}

void setupMockWindowsApi() {
    mockWindowsApi = new MockWindowsApi();
    GetLogicalDrives = MockGetLogicalDrives;
    GetDriveTypeA = MockGetDriveTypeA;
    GetDiskFreeSpaceExA = MockGetDiskFreeSpaceExA;
    CreateFileA = MockCreateFileA;
    DeviceIoControl = MockDeviceIoControl;
    CloseHandle = MockCloseHandle;
}

void cleanupMockWindowsApi() { delete mockWindowsApi; }

#else
class MockFileReader {
public:
    MOCK_METHOD(std::string, ReadFile, (const std::string&), ());
};

MockFileReader* mockFileReader;

std::string MockReadFile(const std::string& path) {
    return mockFileReader->ReadFile(path);
}

void setupMockFileReader() { mockFileReader = new MockFileReader(); }

void cleanupMockFileReader() { delete mockFileReader; }
#endif

class DiskTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef _WIN32
        setupMockWindowsApi();
#else
        setupMockFileReader();
#endif
    }

    void TearDown() override {
#ifdef _WIN32
        cleanupMockWindowsApi();
#else
        cleanupMockFileReader();
#endif
    }
};

#ifdef _WIN32
TEST_F(DiskTest, GetDiskUsage_Windows) {
    EXPECT_CALL(*mockWindowsApi, GetLogicalDrives())
        .WillOnce(Return(0b101));  // Drives A and C are available
    EXPECT_CALL(*mockWindowsApi, GetDriveTypeA(_))
        .WillRepeatedly(Return(DRIVE_FIXED));

    ULARGE_INTEGER totalSpaceC, freeSpaceC;
    totalSpaceC.QuadPart = 100 * 1024 * 1024;
    freeSpaceC.QuadPart = 50 * 1024 * 1024;

    EXPECT_CALL(*mockWindowsApi, GetDiskFreeSpaceExA("A:\\", _, _, _))
        .WillOnce(Return(FALSE));
    EXPECT_CALL(*mockWindowsApi, GetDiskFreeSpaceExA("C:\\", _, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(totalSpaceC),
                        SetArgPointee<3>(freeSpaceC), Return(TRUE)));

    auto diskUsage = getDiskUsage();

    ASSERT_EQ(diskUsage.size(), 1);
    EXPECT_EQ(diskUsage[0].first, "C:\\");
    EXPECT_NEAR(diskUsage[0].second, 50.0, 1e-5);
}

TEST_F(DiskTest, GetDriveModel_Windows) {
    HANDLE mockHandle = reinterpret_cast<HANDLE>(1);

    EXPECT_CALL(*mockWindowsApi, CreateFileA(_, _, _, _, _, _, _))
        .WillOnce(Return(mockHandle));
    std::array<char, 1024> buffer = {};
    std::string_view vendorId = "VENDOR";
    std::string_view productId = "PRODUCT";
    std::string_view productRevision = "REVISION";
    std::memcpy(
        buffer.data() + offsetof(STORAGE_DEVICE_DESCRIPTOR, VendorIdOffset),
        vendorId.data(), vendorId.size());
    std::memcpy(
        buffer.data() + offsetof(STORAGE_DEVICE_DESCRIPTOR, ProductIdOffset),
        productId.data(), productId.size());
    std::memcpy(buffer.data() +
                    offsetof(STORAGE_DEVICE_DESCRIPTOR, ProductRevisionOffset),
                productRevision.data(), productRevision.size());

    DWORD bytesReturned = 0;
    EXPECT_CALL(*mockWindowsApi, DeviceIoControl(_, _, _, _, _, _, _, _))
        .WillOnce(DoAll(SetArrayArgument<4>(buffer.begin(), buffer.end()),
                        Return(TRUE)));
    EXPECT_CALL(*mockWindowsApi, CloseHandle(_)).WillOnce(Return(TRUE));

    std::string model = getDriveModel("C:\\");

    EXPECT_EQ(model, "VENDOR PRODUCT REVISION");
}

#else

TEST_F(DiskTest, GetDiskUsage_Linux) {
    std::string mockProcMounts = "dev/sda1 / ext4 rw,relatime 0 0\n";
    EXPECT_CALL(*mockFileReader, ReadFile("/proc/mounts"))
        .WillOnce(Return(mockProcMounts));

    std::string mockStatfs = "1024 512 256 128";
    EXPECT_CALL(*mockFileReader, ReadFile("/proc/stat"))
        .WillOnce(Return(mockStatfs));

    auto diskUsage = getDiskUsage();

    ASSERT_EQ(diskUsage.size(), 1);
    EXPECT_EQ(diskUsage[0].first, "/");
    EXPECT_NEAR(diskUsage[0].second, 75.0,
                1e-5);  // Adjust the expected value based on the mock data
}

TEST_F(DiskTest, GetDriveModel_Linux) {
    std::string mockDriveModel = "MockModel";
    EXPECT_CALL(*mockFileReader, ReadFile("/sys/block/sda/device/model"))
        .WillOnce(Return(mockDriveModel));

    std::string model = getDriveModel("sda");

    EXPECT_EQ(model, "MockModel");
}

#endif
