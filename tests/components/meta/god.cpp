#include "atom/function/god.hpp"
#include <gtest/gtest.h>

TEST(GodLibraryTest, Cast) {
    int x = atom::meta::cast<int>(1.23);
    EXPECT_EQ(x, 1);

    double y = atom::meta::cast<double>(x);
    EXPECT_EQ(y, 1.0);
}

TEST(GodLibraryTest, AlignUp) {
    EXPECT_EQ(atom::meta::alignUp<64>(123), 128);
    EXPECT_EQ(atom::meta::alignUp<32>(33), 64);

    int* ptr = reinterpret_cast<int*>(123);
    EXPECT_EQ(atom::meta::alignUp<64>(ptr), reinterpret_cast<int*>(128));
}

TEST(GodLibraryTest, AlignDown) {
    EXPECT_EQ(atom::meta::alignDown<64>(123), 64);
    EXPECT_EQ(atom::meta::alignDown<32>(33), 32);

    int* ptr = reinterpret_cast<int*>(123);
    EXPECT_EQ(atom::meta::alignDown<64>(ptr), reinterpret_cast<int*>(64));
}

TEST(GodLibraryTest, Log2) {
    EXPECT_EQ(atom::meta::log2(1), 0);
    EXPECT_EQ(atom::meta::log2(2), 1);
    EXPECT_EQ(atom::meta::log2(8), 3);
}

TEST(GodLibraryTest, Nb) {
    EXPECT_EQ(atom::meta::nb<16>(32), 2);
    EXPECT_EQ(atom::meta::nb<32>(33), 2);
}

TEST(GodLibraryTest, Eq) {
    uint32_t a = 0x12345678;
    uint32_t b = 0x12345678;
    EXPECT_TRUE(atom::meta::eq<uint32_t>(&a, &b));

    uint64_t c = 0x123456789ABCDEF0;
    uint64_t d = 0x123456789ABCDEF1;
    EXPECT_FALSE(atom::meta::eq<uint64_t>(&c, &d));
}

TEST(GodLibraryTest, Copy) {
    char src[] = "Hello, World!";
    char dst[sizeof(src)] = {0};
    atom::meta::copy<sizeof(src)>(dst, src);
    EXPECT_STREQ(dst, src);
}

TEST(GodLibraryTest, Swap) {
    int x = 42;
    int y = atom::meta::swap(&x, 24);
    EXPECT_EQ(x, 24);
    EXPECT_EQ(y, 42);
}

TEST(GodLibraryTest, FetchAdd) {
    int x = 42;
    int y = atom::meta::fetchAdd(&x, 8);
    EXPECT_EQ(x, 50);
    EXPECT_EQ(y, 42);
}

TEST(GodLibraryTest, FetchSub) {
    int x = 42;
    int y = atom::meta::fetchSub(&x, 8);
    EXPECT_EQ(x, 34);
    EXPECT_EQ(y, 42);
}

TEST(GodLibraryTest, FetchAnd) {
    int x = 42;
    int y = atom::meta::fetchAnd(&x, 8);
    EXPECT_EQ(x, 8);
    EXPECT_EQ(y, 42);
}

TEST(GodLibraryTest, FetchOr) {
    int x = 42;
    int y = atom::meta::fetchOr(&x, 8);
    EXPECT_EQ(x, 42 | 8);
    EXPECT_EQ(y, 42);
}

TEST(GodLibraryTest, FetchXor) {
    int x = 42;
    int y = atom::meta::fetchXor(&x, 8);
    EXPECT_EQ(x, 42 ^ 8);
    EXPECT_EQ(y, 42);
}

TEST(GodLibraryTest, TypeTraits) {
    EXPECT_TRUE((atom::meta::isSame<int, int>()));
    EXPECT_FALSE((atom::meta::isSame<int, double>()));
    EXPECT_TRUE((atom::meta::isRef<int&>()));
    EXPECT_FALSE((atom::meta::isRef<int>()));
    EXPECT_TRUE((atom::meta::isArray<int[]>()));
    EXPECT_FALSE((atom::meta::isArray<int>()));
    EXPECT_TRUE((atom::meta::isClass<std::string>()));
    EXPECT_FALSE((atom::meta::isClass<int>()));
    EXPECT_TRUE((atom::meta::isScalar<int>()));
    EXPECT_FALSE((atom::meta::isScalar<std::string>()));
    EXPECT_TRUE((atom::meta::isTriviallyCopyable<int>()));
    EXPECT_FALSE((atom::meta::isTriviallyCopyable<std::string>()));
    EXPECT_TRUE((atom::meta::isTriviallyDestructible<int>()));
    EXPECT_FALSE((atom::meta::isTriviallyDestructible<std::string>()));
    EXPECT_TRUE((atom::meta::isBaseOf<std::ios_base, std::istream>()));
    EXPECT_FALSE((atom::meta::isBaseOf<std::istream, std::ios_base>()));
    EXPECT_TRUE((atom::meta::hasVirtualDestructor<std::iostream>()));
    EXPECT_FALSE((atom::meta::hasVirtualDestructor<int>()));
}
