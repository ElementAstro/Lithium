// test_vany.hpp
#ifndef TEST_ATOM_META_VANY_HPP
#define TEST_ATOM_META_VANY_HPP

#include <gtest/gtest.h>
#include "atom/function/vany.hpp"
#include <string>
#include <vector>

namespace atom::meta::test {

class AnyTest : public ::testing::Test {
protected:
    struct LargeObject {
        std::array<char, 64> data;
        std::string name;
        
        LargeObject(const std::string& n = "test") : name(n) {}
        bool operator==(const LargeObject& other) const {
            return name == other.name;
        }
    };
    
    struct SmallObject {
        int value;
        SmallObject(int v = 0) : value(v) {}
        bool operator==(const SmallObject& other) const {
            return value == other.value;
        }
    };
};

// Construction Tests
TEST_F(AnyTest, DefaultConstruction) {
    Any any;
    EXPECT_FALSE(any.hasValue());
}

TEST_F(AnyTest, SmallObjectConstruction) {
    SmallObject obj(42);
    Any any(obj);
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<SmallObject>());
    EXPECT_EQ(any.cast<SmallObject>(), obj);
}

TEST_F(AnyTest, LargeObjectConstruction) {
    LargeObject obj("test");
    Any any(obj);
    EXPECT_TRUE(any.hasValue());
    EXPECT_TRUE(any.is<LargeObject>());
    EXPECT_EQ(any.cast<LargeObject>(), obj);
}

// Copy Tests
/*
TEST_F(AnyTest, CopyConstructionSmall) {
    Any original(SmallObject(42));
    Any copy(original);
    EXPECT_TRUE(copy.hasValue());
    EXPECT_TRUE(copy.is<SmallObject>());
    EXPECT_EQ(copy.cast<SmallObject>(), SmallObject(42));
}

TEST_F(AnyTest, CopyConstructionLarge) {
    Any original(LargeObject("test"));
    Any copy(original);
    EXPECT_TRUE(copy.hasValue());
    EXPECT_TRUE(copy.is<LargeObject>());
    EXPECT_EQ(copy.cast<LargeObject>(), LargeObject("test"));
}
*/


// Move Tests
TEST_F(AnyTest, MoveConstructionSmall) {
    Any original(SmallObject(42));
    Any moved(std::move(original));
    EXPECT_TRUE(moved.hasValue());
    EXPECT_FALSE(original.hasValue());
    EXPECT_EQ(moved.cast<SmallObject>(), SmallObject(42));
}

TEST_F(AnyTest, MoveConstructionLarge) {
    Any original(LargeObject("test"));
    Any moved(std::move(original));
    EXPECT_TRUE(moved.hasValue());
    EXPECT_FALSE(original.hasValue());
    EXPECT_EQ(moved.cast<LargeObject>(), LargeObject("test"));
}

// Assignment Tests
/*
TEST_F(AnyTest, CopyAssignment) {
    Any original(SmallObject(42));
    Any copy;
    copy = original;
    EXPECT_TRUE(copy.hasValue());
    EXPECT_EQ(copy.cast<SmallObject>(), SmallObject(42));
}
*/


TEST_F(AnyTest, MoveAssignment) {
    Any original(SmallObject(42));
    Any target;
    target = std::move(original);
    EXPECT_TRUE(target.hasValue());
    EXPECT_FALSE(original.hasValue());
}

// Type Safety Tests
TEST_F(AnyTest, TypeChecking) {
    Any any(42);
    EXPECT_TRUE(any.is<int>());
    EXPECT_FALSE(any.is<double>());
    EXPECT_FALSE(any.is<std::string>());
}

TEST_F(AnyTest, BadCast) {
    Any any(42);
    EXPECT_THROW(any.cast<std::string>(), std::bad_cast);
}

// String Conversion Tests
TEST_F(AnyTest, ToStringTest) {
    Any empty;
    EXPECT_EQ(empty.toString(), "Empty Any");

    Any intAny(42);
    EXPECT_EQ(intAny.toString(), "42");

    Any strAny(std::string("test"));
    EXPECT_EQ(strAny.toString(), "test");
}

// Iterator Support Tests
TEST_F(AnyTest, ForeachVector) {
    std::vector<int> vec{1, 2, 3};
    Any any(vec);
    std::vector<int> result;
    
    any.foreach([&result](const Any& item) {
        result.push_back(item.cast<int>());
    });
    
    EXPECT_EQ(result, vec);
}

TEST_F(AnyTest, ForeachNonIterable) {
    Any any(42);
    EXPECT_THROW(any.foreach([](const Any&) {}), std::invalid_argument);
}

// Reset and Value Management Tests
TEST_F(AnyTest, ResetTest) {
    Any any(42);
    EXPECT_TRUE(any.hasValue());
    any.reset();
    EXPECT_FALSE(any.hasValue());
}

/*
TEST_F(AnyTest, SwapTest) {
    Any a1(42);
    Any a2(std::string("test"));
    
    a1.swap(a2);
    
    EXPECT_TRUE(a1.is<std::string>());
    EXPECT_TRUE(a2.is<int>());
    EXPECT_EQ(a1.cast<std::string>(), "test");
    EXPECT_EQ(a2.cast<int>(), 42);
}
*/


// Memory Leak Tests
TEST_F(AnyTest, NoMemoryLeakOnException) {
    struct ThrowOnCopy {
        ThrowOnCopy() = default;
        ThrowOnCopy(const ThrowOnCopy&) { throw std::runtime_error("copy error"); }
    };
    
    Any original(ThrowOnCopy{});
    EXPECT_THROW(Any copy(original), std::runtime_error);
}

// Invoke Tests
TEST_F(AnyTest, InvokeTest) {
    int value = 42;
    Any any(value);
    bool invoked = false;
    
    any.invoke([&invoked](const void* ptr) {
        invoked = (*static_cast<const int*>(ptr) == 42);
    });
    
    EXPECT_TRUE(invoked);
}

TEST_F(AnyTest, InvokeEmpty) {
    Any any;
    EXPECT_THROW(any.invoke([](const void*) {}), std::invalid_argument);
}

// Type Info Tests
TEST_F(AnyTest, TypeInfoTest) {
    Any any(42);
    EXPECT_EQ(any.type(), typeid(int));
    
    any = std::string("test");
    EXPECT_EQ(any.type(), typeid(std::string));
}

TEST_F(AnyTest, TypeInfoEmptyThrows) {
    Any any;
    EXPECT_THROW(any.type(), std::bad_typeid);
}

}  // namespace atom::meta::test

#endif // TEST_ATOM_META_VANY_HPP