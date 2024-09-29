#include "atom/extra/boost/uuid.hpp"
#include <gtest/gtest.h>


using namespace atom::extra::boost;

class UUIDTest : public ::testing::Test {
protected:
    UUID uuid;
};

TEST_F(UUIDTest, DefaultConstructor) {
    UUID uuid;
    EXPECT_FALSE(uuid.isNil());
    EXPECT_EQ(uuid.version(), 4);
}

TEST_F(UUIDTest, StringConstructorValid) {
    std::string uuidStr = "550e8400-e29b-41d4-a716-446655440000";
    UUID uuid(uuidStr);
    EXPECT_EQ(uuid.toString(), uuidStr);
}

TEST_F(UUIDTest, StringConstructorInvalid) {
    EXPECT_THROW(UUID("invalid-uuid-string"), std::runtime_error);
}

TEST_F(UUIDTest, EqualityOperator) {
    UUID uuid1("550e8400-e29b-41d4-a716-446655440000");
    UUID uuid2("550e8400-e29b-41d4-a716-446655440000");
    UUID uuid3("550e8400-e29b-41d4-a716-446655440001");
    EXPECT_EQ(uuid1, uuid2);
    EXPECT_NE(uuid1, uuid3);
}

TEST_F(UUIDTest, ComparisonOperator) {
    UUID uuid1("550e8400-e29b-41d4-a716-446655440000");
    UUID uuid2("550e8400-e29b-41d4-a716-446655440001");
    EXPECT_LT(uuid1, uuid2);
}

TEST_F(UUIDTest, IsNil) {
    UUID nil_uuid("00000000-0000-0000-0000-000000000000");
    EXPECT_TRUE(nil_uuid.isNil());
}

TEST_F(UUIDTest, Format) {
    UUID uuid("550e8400-e29b-41d4-a716-446655440000");
    EXPECT_EQ(uuid.format(), "{550e8400-e29b-41d4-a716-446655440000}");
}

TEST_F(UUIDTest, ToAndFromBytes) {
    UUID uuid("550e8400-e29b-41d4-a716-446655440000");
    auto bytes = uuid.toBytes();
    UUID uuid_from_bytes = UUID::fromBytes(bytes);
    EXPECT_EQ(uuid, uuid_from_bytes);
}

TEST_F(UUIDTest, VersionAndVariant) {
    UUID uuid("550e8400-e29b-41d4-a716-446655440000");
    EXPECT_EQ(uuid.version(), 4);
    EXPECT_EQ(uuid.variant(), 1);
}

TEST_F(UUIDTest, NamespaceUUIDs) {
    UUID dns_ns = UUID::namespaceDNS();
    UUID url_ns = UUID::namespaceURL();
    UUID oid_ns = UUID::namespaceOID();
    EXPECT_FALSE(dns_ns.isNil());
    EXPECT_FALSE(url_ns.isNil());
    EXPECT_FALSE(oid_ns.isNil());
}

TEST_F(UUIDTest, UUIDGeneration) {
    UUID v1 = UUID::v1();
    UUID v4 = UUID::v4();
    EXPECT_EQ(v1.version(), 1);
    EXPECT_EQ(v4.version(), 4);

    UUID ns = UUID::namespaceDNS();
    UUID v3 = UUID::v3(ns, "example.com");
    UUID v5 = UUID::v5(ns, "example.com");
    EXPECT_EQ(v3.version(), 3);
    EXPECT_EQ(v5.version(), 5);
}

TEST_F(UUIDTest, ToBase64) {
    UUID uuid("550e8400-e29b-41d4-a716-446655440000");
    std::string base64 = uuid.toBase64();
    EXPECT_EQ(base64.size(), 22);
}

TEST_F(UUIDTest, GetTimestamp) {
    UUID v1 = UUID::v1();
    EXPECT_NO_THROW(v1.getTimestamp());
    UUID v4 = UUID::v4();
    EXPECT_THROW(v4.getTimestamp(), std::runtime_error);
}

TEST_F(UUIDTest, Hashing) {
    UUID uuid("550e8400-e29b-41d4-a716-446655440000");
    std::hash<UUID> hash_fn;
    size_t hash = hash_fn(uuid);
    EXPECT_NE(hash, 0);
}
