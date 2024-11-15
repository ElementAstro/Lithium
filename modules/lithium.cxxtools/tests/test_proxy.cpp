// test_proxy.cpp
#include "proxy.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;
using namespace lithium::cxxtools;

class NetworkProxyTest : public ::testing::Test {
protected:
    NetworkProxy proxy;

    void SetUp() override {}

    void TearDown() override {
        // Clean up any changes made to the system during tests
        proxy.disableProxy();
    }
};

TEST_F(NetworkProxyTest, SetProxy_ValidProxy_ReturnsTrue) {
    std::string proxyAddress = "http://127.0.0.1:8080";
    NetworkProxy::ProxyMode mode = NetworkProxy::ProxyMode::System;
    std::string listenIP = "127.0.0.1";
    std::string dns = "8.8.8.8";

    bool result = proxy.setProxy(proxyAddress, mode, listenIP, dns);
    EXPECT_TRUE(result);
}

TEST_F(NetworkProxyTest, SetProxy_InvalidProxy_ReturnsFalse) {
    std::string proxyAddress = "invalid_proxy";
    NetworkProxy::ProxyMode mode = NetworkProxy::ProxyMode::System;
    std::string listenIP = "127.0.0.1";
    std::string dns = "8.8.8.8";

    bool result = proxy.setProxy(proxyAddress, mode, listenIP, dns);
    EXPECT_FALSE(result);
}

TEST_F(NetworkProxyTest, DisableProxy_ReturnsTrue) {
    bool result = proxy.disableProxy();
    EXPECT_TRUE(result);
}

TEST_F(NetworkProxyTest, GetCurrentProxy_ReturnsProxy) {
    std::string proxyAddress = "http://127.0.0.1:8080";
    NetworkProxy::ProxyMode mode = NetworkProxy::ProxyMode::System;
    std::string listenIP = "127.0.0.1";
    std::string dns = "8.8.8.8";

    proxy.setProxy(proxyAddress, mode, listenIP, dns);
    std::string currentProxy = proxy.getCurrentProxy();
    EXPECT_EQ(currentProxy, proxyAddress);
}

TEST_F(NetworkProxyTest, InstallCertificate_ValidCert_ReturnsTrue) {
    std::string certPath = "test_cert.pem";
    std::ofstream certFile(certPath);
    certFile << "dummy certificate content";
    certFile.close();

    bool result = proxy.installCertificate(certPath);
    EXPECT_TRUE(result);

    fs::remove(certPath);
}

TEST_F(NetworkProxyTest, InstallCertificate_InvalidCert_ReturnsFalse) {
    std::string certPath = "/invalid/path/to/cert.pem";

    bool result = proxy.installCertificate(certPath);
    EXPECT_FALSE(result);
}

TEST_F(NetworkProxyTest, UninstallCertificate_ValidCert_ReturnsTrue) {
    std::string certName = "test_cert";

    bool result = proxy.uninstallCertificate(certName);
    EXPECT_TRUE(result);
}

TEST_F(NetworkProxyTest, UninstallCertificate_InvalidCert_ReturnsFalse) {
    std::string certName = "invalid_cert";

    bool result = proxy.uninstallCertificate(certName);
    EXPECT_FALSE(result);
}

TEST_F(NetworkProxyTest, ViewCertificateInfo_ValidCert_ReturnsInfo) {
    std::string certName = "test_cert";

    std::string certInfo = proxy.viewCertificateInfo(certName);
    EXPECT_FALSE(certInfo.empty());
}

TEST_F(NetworkProxyTest, ViewCertificateInfo_InvalidCert_ReturnsEmpty) {
    std::string certName = "invalid_cert";

    std::string certInfo = proxy.viewCertificateInfo(certName);
    EXPECT_TRUE(certInfo.empty());
}

TEST_F(NetworkProxyTest, EditHostsFile_ValidEntries_Success) {
    std::vector<std::pair<std::string, std::string>> hostsEntries = {
        {"127.0.0.1", "test.local"}, {"127.0.0.1", "example.local"}};

    proxy.editHostsFile(hostsEntries);

    // Verify the entries were added to the hosts file
    std::ifstream hostsFile("/etc/hosts");
    std::string line;
    bool foundTestLocal = false;
    bool foundExampleLocal = false;
    while (std::getline(hostsFile, line)) {
        if (line.find("test.local") != std::string::npos) {
            foundTestLocal = true;
        }
        if (line.find("example.local") != std::string::npos) {
            foundExampleLocal = true;
        }
    }
    hostsFile.close();

    EXPECT_TRUE(foundTestLocal);
    EXPECT_TRUE(foundExampleLocal);
}

TEST_F(NetworkProxyTest, ResetHostsFile_Success) {
    proxy.resetHostsFile();

    // Verify the hosts file was reset
    std::ifstream hostsFile("/etc/hosts");
    std::string line;
    bool foundLocalhost = false;
    while (std::getline(hostsFile, line)) {
        if (line.find("127.0.0.1   localhost") != std::string::npos) {
            foundLocalhost = true;
        }
    }
    hostsFile.close();

    EXPECT_TRUE(foundLocalhost);
}

TEST_F(NetworkProxyTest, EnableHttpToHttpsRedirect_Enable) {
    proxy.enableHttpToHttpsRedirect(true);
    // Additional checks can be added here if needed
}

TEST_F(NetworkProxyTest, EnableHttpToHttpsRedirect_Disable) {
    proxy.enableHttpToHttpsRedirect(false);
    // Additional checks can be added here if needed
}

TEST_F(NetworkProxyTest, SetCustomDoH_ValidUrl) {
    std::string dohUrl = "https://dns.google/dns-query";
    proxy.setCustomDoH(dohUrl);
    // Additional checks can be added here if needed
}

int main(int argc, char** argv) {
    loguru::init(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}