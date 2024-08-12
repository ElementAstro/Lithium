#include "addon/version.hpp"

#include <gtest/gtest.h>

using namespace lithium;

TEST(VersionTest, ParseVersion) {
    Version v1 = Version::parse("1.2.3");
    EXPECT_EQ(v1.major, 1);
    EXPECT_EQ(v1.minor, 2);
    EXPECT_EQ(v1.patch, 3);
    EXPECT_EQ(v1.prerelease, "");
    EXPECT_EQ(v1.build, "");

    Version v2 = Version::parse("2.0.0-alpha.1+exp.sha.5114f85");
    EXPECT_EQ(v2.major, 2);
    EXPECT_EQ(v2.minor, 0);
    EXPECT_EQ(v2.patch, 0);
    EXPECT_EQ(v2.prerelease, "alpha.1");
    EXPECT_EQ(v2.build, "exp.sha.5114f85");

    // 测试无效版本号格式
    EXPECT_THROW(Version::parse("1.2"), std::invalid_argument);
    EXPECT_THROW(Version::parse("1.2.3.4"), std::invalid_argument);
    EXPECT_THROW(Version::parse("abc.def.ghi"), std::invalid_argument);
}

// 测试Version比较功能
TEST(VersionTest, CompareVersions) {
    Version v1 = Version::parse("1.0.0");
    Version v2 = Version::parse("2.0.0");
    EXPECT_LT(v1, v2);

    Version v3 = Version::parse("1.2.3");
    Version v4 = Version::parse("1.2.4");
    EXPECT_LT(v3, v4);

    Version v5 = Version::parse("1.2.3-alpha");
    Version v6 = Version::parse("1.2.3");
    EXPECT_LT(v5, v6);  // prerelease版本应小于正式版本

    Version v7 = Version::parse("1.2.3-alpha");
    Version v8 = Version::parse("1.2.3-beta");
    EXPECT_LT(v7, v8);  // "alpha" < "beta"
}

// 测试Version校验功能
TEST(VersionTest, CheckVersion) {
    Version actualVersion = Version::parse("1.2.3");

    EXPECT_TRUE(checkVersion(actualVersion, "^1.0.0"));
    EXPECT_FALSE(checkVersion(actualVersion, "^2.0.0"));

    EXPECT_TRUE(checkVersion(actualVersion, "~1.2.0"));
    EXPECT_FALSE(checkVersion(actualVersion, "~1.3.0"));

    EXPECT_TRUE(checkVersion(actualVersion, ">=1.2.3"));
    EXPECT_FALSE(checkVersion(actualVersion, ">=2.0.0"));

    EXPECT_TRUE(checkVersion(actualVersion, "<2.0.0"));
    EXPECT_FALSE(checkVersion(actualVersion, "<1.2.3"));

    EXPECT_TRUE(checkVersion(actualVersion, "=1.2.3"));
    EXPECT_FALSE(checkVersion(actualVersion, "=1.2.4"));
}

// 测试DateVersion解析功能
TEST(DateVersionTest, ParseDateVersion) {
    DateVersion dv1 = DateVersion::parse("2024-08-15");
    EXPECT_EQ(dv1.year, 2024);
    EXPECT_EQ(dv1.month, 8);
    EXPECT_EQ(dv1.day, 15);

    EXPECT_THROW(DateVersion::parse("2024/08/15"), std::invalid_argument);
    EXPECT_THROW(DateVersion::parse("2024-15-08"), std::invalid_argument);
    EXPECT_THROW(DateVersion::parse("abcd-ef-gh"), std::invalid_argument);
}

// 测试DateVersion比较功能
TEST(DateVersionTest, CompareDateVersions) {
    DateVersion dv1 = DateVersion::parse("2024-08-15");
    DateVersion dv2 = DateVersion::parse("2024-08-16");
    EXPECT_LT(dv1, dv2);

    DateVersion dv3 = DateVersion::parse("2024-07-15");
    DateVersion dv4 = DateVersion::parse("2024-08-15");
    EXPECT_LT(dv3, dv4);

    DateVersion dv5 = DateVersion::parse("2024-08-15");
    DateVersion dv6 = DateVersion::parse("2024-08-15");
    EXPECT_EQ(dv5, dv6);
}

// 测试DateVersion校验功能
TEST(DateVersionTest, CheckDateVersion) {
    DateVersion actualDateVersion = DateVersion::parse("2024-08-15");

    EXPECT_TRUE(checkDateVersion(actualDateVersion, ">=2024-08-15"));
    EXPECT_FALSE(checkDateVersion(actualDateVersion, ">=2024-08-16"));

    EXPECT_TRUE(checkDateVersion(actualDateVersion, "<=2024-08-15"));
    EXPECT_FALSE(checkDateVersion(actualDateVersion, "<2024-08-15"));

    EXPECT_TRUE(checkDateVersion(actualDateVersion, "=2024-08-15"));
    EXPECT_FALSE(checkDateVersion(actualDateVersion, "=2024-08-14"));
}

// 测试边缘情况
TEST(EdgeCaseTest, EdgeCases) {
    Version v1 = Version::parse("0.0.0");
    Version v2 = Version::parse("0.0.1");
    EXPECT_LT(v1, v2);

    Version v3 = Version::parse("1.0.0-alpha");
    Version v4 = Version::parse("1.0.0-alpha.1");
    EXPECT_LT(v3, v4);

    DateVersion dv1 = DateVersion::parse("2024-12-31");
    DateVersion dv2 = DateVersion::parse("2025-01-01");
    EXPECT_LT(dv1, dv2);

    DateVersion dv3 = DateVersion::parse("2024-02-29");
    EXPECT_EQ(dv3.year, 2024);
    EXPECT_EQ(dv3.month, 2);
    EXPECT_EQ(dv3.day, 29);  // 闰年测试
}
