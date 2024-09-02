#include "atom/utils/qtimezone.hpp"
#include <gtest/gtest.h>
#include "atom/utils/qdatetime.hpp"

using namespace atom::utils;

// Test the default constructor
TEST(QTimeZoneTest, DefaultConstructor) {
    QTimeZone tz;
    EXPECT_EQ(tz.id(), "UTC");
    EXPECT_TRUE(tz.isValid());
    EXPECT_EQ(tz.offsetFromUtc(QDateTime::currentDateTime()).count(), 0);
}

// Test the parameterized constructor with valid time zone ID
TEST(QTimeZoneTest, ParameterizedConstructorValid) {
    QTimeZone tz("PST");
    EXPECT_EQ(tz.id(), "PST");
    EXPECT_TRUE(tz.isValid());
}

// Test the parameterized constructor with invalid time zone ID
TEST(QTimeZoneTest, ParameterizedConstructorInvalid) {
    EXPECT_THROW(QTimeZone tz("InvalidID"), std::invalid_argument);
}

// Test availableTimeZoneIds
TEST(QTimeZoneTest, AvailableTimeZoneIds) {
    std::vector<std::string> expectedIds = {"UTC", "PST", "EST", "CST", "MST"};
    std::vector<std::string> actualIds = QTimeZone::availableTimeZoneIds();
    EXPECT_EQ(actualIds, expectedIds);
}

// Test id
TEST(QTimeZoneTest, Id) {
    QTimeZone tz("EST");
    EXPECT_EQ(tz.id(), "EST");
}

// Test displayName
TEST(QTimeZoneTest, DisplayName) {
    QTimeZone tz("CST");
    EXPECT_EQ(tz.displayName(), "Central Standard Time");
}

// Test isValid
TEST(QTimeZoneTest, IsValid) {
    QTimeZone tz("MST");
    EXPECT_TRUE(tz.isValid());
}

// Test offsetFromUtc
TEST(QTimeZoneTest, OffsetFromUtc) {
    QTimeZone tz("PST");
    QDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(tz.offsetFromUtc(dt).count(), tz.standardTimeOffset().count());
}

// Test standardTimeOffset
TEST(QTimeZoneTest, StandardTimeOffset) {
    QTimeZone tz("UTC");
    EXPECT_EQ(tz.standardTimeOffset().count(), 0);
}

// Test daylightTimeOffset
TEST(QTimeZoneTest, DaylightTimeOffset) {
    QTimeZone tz("UTC");
    EXPECT_EQ(tz.daylightTimeOffset().count(), 0);
}

// Test hasDaylightTime
TEST(QTimeZoneTest, HasDaylightTime) {
    QTimeZone tz("UTC");
    EXPECT_FALSE(tz.hasDaylightTime());
}

// Test isDaylightTime
TEST(QTimeZoneTest, IsDaylightTime) {
    QTimeZone tz("UTC");
    QDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_FALSE(tz.isDaylightTime(dt));
}

// Test operator<=>
TEST(QTimeZoneTest, ThreeWayComparison) {
    QTimeZone tz1("PST");
    QTimeZone tz2("PST");
    QTimeZone tz3("EST");
    EXPECT_TRUE((tz1 <=> tz2) == 0);
    EXPECT_TRUE((tz1 <=> tz3) != 0);
}
