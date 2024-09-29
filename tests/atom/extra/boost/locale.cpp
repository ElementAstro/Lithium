#include "atom/extra/boost/locale.hpp"

#include <gtest/gtest.h>

using namespace atom::extra::boost;

class LocaleWrapperTest : public ::testing::Test {
protected:
    LocaleWrapper localeWrapper;
};

TEST_F(LocaleWrapperTest, ConstructorDefault) {
    EXPECT_NO_THROW(LocaleWrapper());
}

TEST_F(LocaleWrapperTest, ConstructorWithLocale) {
    EXPECT_NO_THROW(LocaleWrapper("en_US.UTF-8"));
}

TEST_F(LocaleWrapperTest, ToUtf8) {
    std::string str = "Hello, 世界";
    std::string utf8Str = LocaleWrapper::toUtf8(str, "ISO-8859-1");
    EXPECT_EQ(utf8Str, "Hello, 世界");
}

TEST_F(LocaleWrapperTest, FromUtf8) {
    std::string str = "Hello, 世界";
    std::string utf8Str = LocaleWrapper::fromUtf8(str, "ISO-8859-1");
    EXPECT_EQ(utf8Str, "Hello, 世界");
}

TEST_F(LocaleWrapperTest, Normalize) {
    std::string str = "e\u0301";  // é in decomposed form
    std::string normalizedStr = LocaleWrapper::normalize(str);
    EXPECT_EQ(normalizedStr, "é");
}

TEST_F(LocaleWrapperTest, Tokenize) {
    std::string str = "Hello, world!";
    auto tokens = LocaleWrapper::tokenize(str);
    ASSERT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], "Hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST_F(LocaleWrapperTest, Translate) {
    std::string str = "Hello";
    std::string translatedStr =
        LocaleWrapper::translate(str, "messages", "en_US.UTF-8");
    EXPECT_EQ(translatedStr, "Hello");
}

TEST_F(LocaleWrapperTest, ToUpper) {
    std::string str = "hello";
    std::string upperStr = localeWrapper.toUpper(str);
    EXPECT_EQ(upperStr, "HELLO");
}

TEST_F(LocaleWrapperTest, ToLower) {
    std::string str = "HELLO";
    std::string lowerStr = localeWrapper.toLower(str);
    EXPECT_EQ(lowerStr, "hello");
}

TEST_F(LocaleWrapperTest, ToTitle) {
    std::string str = "hello world";
    std::string titleStr = localeWrapper.toTitle(str);
    EXPECT_EQ(titleStr, "Hello World");
}

TEST_F(LocaleWrapperTest, Compare) {
    std::string str1 = "apple";
    std::string str2 = "banana";
    int result = localeWrapper.compare(str1, str2);
    EXPECT_LT(result, 0);
}

TEST_F(LocaleWrapperTest, FormatDate) {
    ::boost::posix_time::ptime dateTime(boost::gregorian::date(2023, 10, 1));
    std::string formattedDate = LocaleWrapper::formatDate(dateTime, "%Y-%m-%d");
    EXPECT_EQ(formattedDate, "2023-10-01");
}

TEST_F(LocaleWrapperTest, FormatNumber) {
    double number = 1234.5678;
    std::string formattedNumber = LocaleWrapper::formatNumber(number, 2);
    EXPECT_EQ(formattedNumber, "1234.57");
}

TEST_F(LocaleWrapperTest, FormatCurrency) {
    double amount = 1234.56;
    std::string formattedCurrency =
        LocaleWrapper::formatCurrency(amount, "USD");
    EXPECT_EQ(formattedCurrency, "$1,234.56");
}

TEST_F(LocaleWrapperTest, RegexReplace) {
    std::string str = "Hello, world!";
    ::boost::regex regex("world");
    std::string replacedStr = LocaleWrapper::regexReplace(str, regex, "Boost");
    EXPECT_EQ(replacedStr, "Hello, Boost!");
}

TEST_F(LocaleWrapperTest, Format) {
    std::string formatString = "Hello, {1}!";
    std::string formattedStr = localeWrapper.format(formatString, "world");
    EXPECT_EQ(formattedStr, "Hello, world!");
}
