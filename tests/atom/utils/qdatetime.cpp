#include <gtest/gtest.h>
#include "atom/utils/qdatetime.hpp"
#include "atom/utils/qtimezone.hpp"

using namespace atom::utils;

// Mock MyTimeZone class for testing
class MockTimeZone : public MyTimeZone {
public:
    MockTimeZone(int offset) : offset(offset) {}
    std::chrono::seconds offsetFromUtc(const MyDateTime&) const {
        return std::chrono::seconds(offset);
    }
private:
    int offset;
};

// Test the default constructor
TEST(MyDateTimeTest, DefaultConstructor) {
    MyDateTime dt;
    EXPECT_FALSE(dt.isValid());
}

// Test the parameterized constructor with valid date string
TEST(MyDateTimeTest, ParameterizedConstructorValid) {
    MyDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_TRUE(dt.isValid());
}

// Test the parameterized constructor with invalid date string
TEST(MyDateTimeTest, ParameterizedConstructorInvalid) {
    MyDateTime dt("invalid date", "%Y-%m-%d %H:%M:%S");
    EXPECT_FALSE(dt.isValid());
}

// Test the constructor with timezone
TEST(MyDateTimeTest, ParameterizedConstructorWithTimezone) {
    MockTimeZone tz(3600); // +1 hour
    MyDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S", tz);
    EXPECT_TRUE(dt.isValid());
}

// Test currentDateTime
TEST(MyDateTimeTest, CurrentDateTime) {
    MyDateTime dt = MyDateTime::currentDateTime();
    EXPECT_TRUE(dt.isValid());
}

// Test currentDateTime with timezone
TEST(MyDateTimeTest, CurrentDateTimeWithTimezone) {
    MockTimeZone tz(3600); // +1 hour
    MyDateTime dt = MyDateTime::currentDateTime(tz);
    EXPECT_TRUE(dt.isValid());
}

// Test fromString
TEST(MyDateTimeTest, FromString) {
    MyDateTime dt = MyDateTime::fromString("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_TRUE(dt.isValid());
}

// Test fromString with timezone
TEST(MyDateTimeTest, FromStringWithTimezone) {
    MockTimeZone tz(3600); // +1 hour
    MyDateTime dt = MyDateTime::fromString("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S", tz);
    EXPECT_TRUE(dt.isValid());
}

// Test toString
TEST(MyDateTimeTest, ToString) {
    MyDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    std::string dateString = dt.toString("%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(dateString, "2023-07-18 12:34:56");
}

// Test toString with timezone
TEST(MyDateTimeTest, ToStringWithTimezone) {
    MockTimeZone tz(3600); // +1 hour
    MyDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    std::string dateString = dt.toString("%Y-%m-%d %H:%M:%S", tz);
    EXPECT_EQ(dateString, "2023-07-18 13:34:56");
}

// Test addDays
TEST(MyDateTimeTest, AddDays) {
    MyDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    MyDateTime newDt = dt.addDays(2);
    EXPECT_EQ(newDt.toString("%Y-%m-%d %H:%M:%S"), "2023-07-20 12:34:56");
}

// Test addSecs
TEST(MyDateTimeTest, AddSecs) {
    MyDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    MyDateTime newDt = dt.addSecs(3600);
    EXPECT_EQ(newDt.toString("%Y-%m-%d %H:%M:%S"), "2023-07-18 13:34:56");
}

// Test daysTo
TEST(MyDateTimeTest, DaysTo) {
    MyDateTime dt1("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    MyDateTime dt2("2023-07-20 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(dt1.daysTo(dt2), 2);
}

// Test secsTo
TEST(MyDateTimeTest, SecsTo) {
    MyDateTime dt1("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    MyDateTime dt2("2023-07-18 13:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(dt1.secsTo(dt2), 3600);
}

// Test operator<=>
TEST(MyDateTimeTest, ThreeWayComparison) {
    MyDateTime dt1("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    MyDateTime dt2("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    MyDateTime dt3("2023-07-18 13:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_TRUE((dt1 <=> dt2) == 0);
    EXPECT_TRUE((dt1 <=> dt3) < 0);
    EXPECT_TRUE((dt3 <=> dt1) > 0);
}
