#include <gtest/gtest.h>
#include "atom/utils/qdatetime.hpp"
#include "atom/utils/qtimezone.hpp"

using namespace atom::utils;

// Mock QTimeZone class for testing
class MockTimeZone : public QTimeZone {
public:
    MockTimeZone(int offset) : offset(offset) {}
    std::chrono::seconds offsetFromUtc(const QDateTime&) const {
        return std::chrono::seconds(offset);
    }
private:
    int offset;
};

// Test the default constructor
TEST(QDateTimeTest, DefaultConstructor) {
    QDateTime dt;
    EXPECT_FALSE(dt.isValid());
}

// Test the parameterized constructor with valid date string
TEST(QDateTimeTest, ParameterizedConstructorValid) {
    QDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_TRUE(dt.isValid());
}

// Test the parameterized constructor with invalid date string
TEST(QDateTimeTest, ParameterizedConstructorInvalid) {
    QDateTime dt("invalid date", "%Y-%m-%d %H:%M:%S");
    EXPECT_FALSE(dt.isValid());
}

// Test the constructor with timezone
TEST(QDateTimeTest, ParameterizedConstructorWithTimezone) {
    MockTimeZone tz(3600); // +1 hour
    QDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S", tz);
    EXPECT_TRUE(dt.isValid());
}

// Test currentDateTime
TEST(QDateTimeTest, CurrentDateTime) {
    QDateTime dt = QDateTime::currentDateTime();
    EXPECT_TRUE(dt.isValid());
}

// Test currentDateTime with timezone
TEST(QDateTimeTest, CurrentDateTimeWithTimezone) {
    MockTimeZone tz(3600); // +1 hour
    QDateTime dt = QDateTime::currentDateTime(tz);
    EXPECT_TRUE(dt.isValid());
}

// Test fromString
TEST(QDateTimeTest, FromString) {
    QDateTime dt = QDateTime::fromString("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_TRUE(dt.isValid());
}

// Test fromString with timezone
TEST(QDateTimeTest, FromStringWithTimezone) {
    MockTimeZone tz(3600); // +1 hour
    QDateTime dt = QDateTime::fromString("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S", tz);
    EXPECT_TRUE(dt.isValid());
}

// Test toString
TEST(QDateTimeTest, ToString) {
    QDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    std::string dateString = dt.toString("%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(dateString, "2023-07-18 12:34:56");
}

// Test toString with timezone
TEST(QDateTimeTest, ToStringWithTimezone) {
    MockTimeZone tz(3600); // +1 hour
    QDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    std::string dateString = dt.toString("%Y-%m-%d %H:%M:%S", tz);
    EXPECT_EQ(dateString, "2023-07-18 13:34:56");
}

// Test addDays
TEST(QDateTimeTest, AddDays) {
    QDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    QDateTime newDt = dt.addDays(2);
    EXPECT_EQ(newDt.toString("%Y-%m-%d %H:%M:%S"), "2023-07-20 12:34:56");
}

// Test addSecs
TEST(QDateTimeTest, AddSecs) {
    QDateTime dt("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    QDateTime newDt = dt.addSecs(3600);
    EXPECT_EQ(newDt.toString("%Y-%m-%d %H:%M:%S"), "2023-07-18 13:34:56");
}

// Test daysTo
TEST(QDateTimeTest, DaysTo) {
    QDateTime dt1("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    QDateTime dt2("2023-07-20 12:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(dt1.daysTo(dt2), 2);
}

// Test secsTo
TEST(QDateTimeTest, SecsTo) {
    QDateTime dt1("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    QDateTime dt2("2023-07-18 13:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_EQ(dt1.secsTo(dt2), 3600);
}

// Test operator<=>
TEST(QDateTimeTest, ThreeWayComparison) {
    QDateTime dt1("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    QDateTime dt2("2023-07-18 12:34:56", "%Y-%m-%d %H:%M:%S");
    QDateTime dt3("2023-07-18 13:34:56", "%Y-%m-%d %H:%M:%S");
    EXPECT_TRUE((dt1 <=> dt2) == 0);
    EXPECT_TRUE((dt1 <=> dt3) < 0);
    EXPECT_TRUE((dt3 <=> dt1) > 0);
}
