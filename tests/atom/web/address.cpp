#include <gtest/gtest.h>
#include "atom/web/address.hpp"

using namespace atom::web;

// Test cases for the Address class
class MockAddress : public Address {
public:
    bool parse(const std::string& address) override { return true; }
    void printAddressType() const override {}
    bool isInRange(const std::string& start, const std::string& end) override { return true; }
    std::string toBinary() const override { return "binary"; }
    bool isEqual(const Address& other) const override { return true; }
    std::string getType() const override { return "Mock"; }
    std::string getNetworkAddress(const std::string& mask) const override { return "network"; }
    std::string getBroadcastAddress(const std::string& mask) const override { return "broadcast"; }
    bool isSameSubnet(const Address& other, const std::string& mask) const override { return true; }
    std::string toHex() const override { return "hex"; }
};

/*
TODO: Fix this test
TEST(AddressTest, GetAddress) {
    MockAddress address;
    address.addressStr = "127.0.0.1";
    EXPECT_EQ(address.getAddress(), "127.0.0.1");
}
*/

// Test cases for the IPv4 class
TEST(IPv4Test, Constructor) {
    IPv4 address("192.168.1.1");
    EXPECT_EQ(address.getAddress(), "192.168.1.1");
}

TEST(IPv4Test, Parse) {
    IPv4 address;
    EXPECT_TRUE(address.parse("192.168.1.1"));
}

TEST(IPv4Test, PrintAddressType) {
    IPv4 address;
    testing::internal::CaptureStdout();
    address.printAddressType();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "IPv4\n");
}

TEST(IPv4Test, IsInRange) {
    IPv4 address("192.168.1.5");
    EXPECT_TRUE(address.isInRange("192.168.1.0", "192.168.1.10"));
}

TEST(IPv4Test, ToBinary) {
    IPv4 address("192.168.1.1");
    EXPECT_EQ(address.toBinary(), "11000000101010000000000100000001");
}

TEST(IPv4Test, IsEqual) {
    IPv4 address1("192.168.1.1");
    IPv4 address2("192.168.1.1");
    EXPECT_TRUE(address1.isEqual(address2));
}

TEST(IPv4Test, GetType) {
    IPv4 address;
    EXPECT_EQ(address.getType(), "IPv4");
}

TEST(IPv4Test, GetNetworkAddress) {
    IPv4 address("192.168.1.1");
    EXPECT_EQ(address.getNetworkAddress("255.255.255.0"), "192.168.1.0");
}

TEST(IPv4Test, GetBroadcastAddress) {
    IPv4 address("192.168.1.1");
    EXPECT_EQ(address.getBroadcastAddress("255.255.255.0"), "192.168.1.255");
}

TEST(IPv4Test, IsSameSubnet) {
    IPv4 address1("192.168.1.1");
    IPv4 address2("192.168.1.2");
    EXPECT_TRUE(address1.isSameSubnet(address2, "255.255.255.0"));
}

TEST(IPv4Test, ToHex) {
    IPv4 address("192.168.1.1");
    EXPECT_EQ(address.toHex(), "C0A80101");
}

TEST(IPv4Test, ParseCIDR) {
    IPv4 address;
    EXPECT_TRUE(address.parseCIDR("192.168.1.1/24"));
}

// Test cases for the IPv6 class
TEST(IPv6Test, Constructor) {
    IPv6 address("::1");
    EXPECT_EQ(address.getAddress(), "::1");
}

TEST(IPv6Test, Parse) {
    IPv6 address;
    EXPECT_TRUE(address.parse("::1"));
}

TEST(IPv6Test, PrintAddressType) {
    IPv6 address;
    testing::internal::CaptureStdout();
    address.printAddressType();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "IPv6\n");
}

TEST(IPv6Test, IsInRange) {
    IPv6 address("::5");
    EXPECT_TRUE(address.isInRange("::0", "::10"));
}

TEST(IPv6Test, ToBinary) {
    IPv6 address("::1");
    EXPECT_EQ(address.toBinary(), "00000000000000000000000000000001");
}

TEST(IPv6Test, IsEqual) {
    IPv6 address1("::1");
    IPv6 address2("::1");
    EXPECT_TRUE(address1.isEqual(address2));
}

TEST(IPv6Test, GetType) {
    IPv6 address;
    EXPECT_EQ(address.getType(), "IPv6");
}

TEST(IPv6Test, GetNetworkAddress) {
    IPv6 address("::1");
    EXPECT_EQ(address.getNetworkAddress("ffff:ffff:ffff:ffff::"), "::");
}

TEST(IPv6Test, GetBroadcastAddress) {
    IPv6 address("::1");
    EXPECT_EQ(address.getBroadcastAddress("ffff:ffff:ffff:ffff::"), "::ffff:ffff:ffff:ffff");
}

TEST(IPv6Test, IsSameSubnet) {
    IPv6 address1("::1");
    IPv6 address2("::2");
    EXPECT_TRUE(address1.isSameSubnet(address2, "ffff:ffff:ffff:ffff::"));
}

TEST(IPv6Test, ToHex) {
    IPv6 address("::1");
    EXPECT_EQ(address.toHex(), "00000000000000000000000000000001");
}

TEST(IPv6Test, ParseCIDR) {
    IPv6 address;
    EXPECT_TRUE(address.parseCIDR("::1/128"));
}

// Test cases for the UnixDomain class
TEST(UnixDomainTest, Constructor) {
    UnixDomain address("/tmp/socket");
    EXPECT_EQ(address.getAddress(), "/tmp/socket");
}

TEST(UnixDomainTest, Parse) {
    UnixDomain address;
    EXPECT_TRUE(address.parse("/tmp/socket"));
}

TEST(UnixDomainTest, PrintAddressType) {
    UnixDomain address;
    testing::internal::CaptureStdout();
    address.printAddressType();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "UnixDomain\n");
}

TEST(UnixDomainTest, IsInRange) {
    UnixDomain address("/tmp/socket");
    EXPECT_TRUE(address.isInRange("/tmp/socket1", "/tmp/socket2"));
}

TEST(UnixDomainTest, ToBinary) {
    UnixDomain address("/tmp/socket");
    EXPECT_EQ(address.toBinary(), "binary");
}

TEST(UnixDomainTest, IsEqual) {
    UnixDomain address1("/tmp/socket");
    UnixDomain address2("/tmp/socket");
    EXPECT_TRUE(address1.isEqual(address2));
}

TEST(UnixDomainTest, GetType) {
    UnixDomain address;
    EXPECT_EQ(address.getType(), "UnixDomain");
}

TEST(UnixDomainTest, GetNetworkAddress) {
    UnixDomain address("/tmp/socket");
    EXPECT_EQ(address.getNetworkAddress("mask"), "network");
}

TEST(UnixDomainTest, GetBroadcastAddress) {
    UnixDomain address("/tmp/socket");
    EXPECT_EQ(address.getBroadcastAddress("mask"), "broadcast");
}

TEST(UnixDomainTest, IsSameSubnet) {
    UnixDomain address1("/tmp/socket1");
    UnixDomain address2("/tmp/socket2");
    EXPECT_TRUE(address1.isSameSubnet(address2, "mask"));
}

TEST(UnixDomainTest, ToHex) {
    UnixDomain address("/tmp/socket");
    EXPECT_EQ(address.toHex(), "hex");
}