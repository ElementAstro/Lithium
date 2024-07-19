#include "atom/utils/qtimezone.hpp"
#include <gtest/gtest.h>
#include "atom/utils/qdatetime.hpp"

using namespace atom::utils;

// Test the default constructor
TEST(MyTimeZoneTest, DefaultConstructor) {
    MyTimeZone tz;
    EXPECT_EQ(tz.id(), "UTC");
    EXPECT_TRUE(tz.isValid());
    EXPECT_EQ(tz.offsetFromUtc(MyDateTime::currentDateTime()).count(), 0);
}

// Test the parameterized constructor with valid time zone ID
TEST(MyTimeZoneTest, ParameterizedConstructorValid) {
    MyTimeZone tz("PST");
    EXPECT_EQ(tz.id(), "PST");
    EXPECT_TRUE(tz.isValid());
}

// Test the parameterized constructor with invalid time zone ID
TEST(MyTimeZoneTest, ParameterizedConstructorInvalid) {
    EXPECT_THROW(MyTimeZone tz("InvalidID"), std::invalid_argument);
}

// Test availableTimeZoneIds
TEST(MyTimeZoneTest, AvailableTimeZoneIds) {
    std::vector<std::string> expectedIds = {"UTC", "PST", "EST", "CST", "MST"};
    std::vector<std::string> actualIds = MyTimeZone::availableTimeZoneIds();
    EXPECT_EQ(actualIds, expectedIds);
}

// Test id
TEST(MyTimeZoneTest, Id) {
    MyTimeZone tz("EST");
    EXPECT_EQ(tz.id(), "EST");
}

// Test displayName
TEST(MyTimeZoneTest, DisplayName) {
    MyTimeZone tz("CST");
    EXPECT_EQ(tz.displayName(), "Central Standard Time");
}

// Test isValid
TEST(MyTimeZoneTest, IsValid) {
    MyTimeZone tz("MST");
    EXPECT_TRUE(tz.isValid());
}

// Test offsetFromUtc
TEST(MyTimeZoneTest, OffsetFromUtc) {
    MyTimeZone tz("PST");
    MyDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(tz.offsetFromUtc(dt).count(), tz.standardTimeOffset().count());
}

// Test standardTimeOffset
TEST(MyTimeZoneTest, StandardTimeOffset) {
    MyTimeZone tz("UTC");
    EXPECT_EQ(tz.standardTimeOffset().count(), 0);
}

// Test daylightTimeOffset
TEST(MyTimeZoneTest, DaylightTimeOffset) {
    MyTimeZone tz("UTC");
    EXPECT_EQ(tz.daylightTimeOffset().count(), 0);
}

// Test hasDaylightTime
TEST(MyTimeZoneTest, HasDaylightTime) {
    MyTimeZone tz("UTC");
    EXPECT_FALSE(tz.hasDaylightTime());
}

// Test isDaylightTime
TEST(MyTimeZoneTest, IsDaylightTime) {
    MyTimeZone tz("UTC");
    MyDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_FALSE(tz.isDaylightTime(dt));
}

// Test operator<=>
TEST(MyTimeZoneTest, ThreeWayComparison) {
    MyTimeZone tz1("PST");
    MyTimeZone tz2("PST");
    MyTimeZone tz3("EST");
    EXPECT_TRUE((tz1 <=> tz2) == 0);
    EXPECT_TRUE((tz1 <=> tz3) != 0);
}
