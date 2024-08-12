#include "atom/function/conversion.hpp"
#include <gtest/gtest.h>
#include <any>
#include <map>
#include <memory>
#include <set>
#include <vector>

using namespace atom::meta;

// 测试类定义
class Base {
public:
    virtual ~Base() = default;
    virtual std::string getName() const { return "Base"; }
};

class Derived : public Base {
public:
    std::string getName() const override { return "Derived"; }
};

class Unrelated {};

// 测试类型转换
TEST(TypeConversionTest, StaticConversionPointerTypes) {
    Derived derived;
    std::any derivedPtr = &derived;

    StaticConversion<Derived*, Base*> converter;

    std::any basePtr = converter.convert(derivedPtr);
    EXPECT_EQ(std::any_cast<Base*>(basePtr)->getName(), "Derived");

    // 反向转换
    std::any convertedBackPtr = converter.convertDown(basePtr);
    EXPECT_EQ(std::any_cast<Derived*>(convertedBackPtr)->getName(), "Derived");
}

TEST(TypeConversionTest, StaticConversionReferenceTypes) {
    Derived derived;
    std::any derivedPtr = &derived;

    StaticConversion<Derived*, Base*> converter;

    std::any basePtr = converter.convert(derivedPtr);
    EXPECT_EQ(std::any_cast<Base*>(basePtr)->getName(), "Derived");

    // 反向转换
    std::any convertedBackPtr = converter.convertDown(basePtr);
    EXPECT_EQ(std::any_cast<Derived*>(convertedBackPtr)->getName(), "Derived");
}

TEST(TypeConversionTest, DynamicConversionPointerTypes) {
    Derived derived;
    std::any derivedPtr = &derived;

    DynamicConversion<Derived*, Base*> converter;

    std::any basePtr = converter.convert(derivedPtr);
    EXPECT_EQ(std::any_cast<Base*>(basePtr)->getName(), "Derived");

    // 反向转换
    std::any convertedBackPtr = converter.convertDown(basePtr);
    EXPECT_EQ(std::any_cast<Derived*>(convertedBackPtr)->getName(), "Derived");
}

TEST(TypeConversionTest, VectorConversion) {
    auto derived1 = std::make_shared<Derived>();
    auto derived2 = std::make_shared<Derived>();
    std::vector<std::shared_ptr<Derived>> derivedVec = {derived1, derived2};
    std::any derivedAnyVec = derivedVec;

    VectorConversion<std::shared_ptr<Derived>, std::shared_ptr<Base>> converter;

    std::any baseAnyVec = converter.convert(derivedAnyVec);
    auto baseVec =
        std::any_cast<std::vector<std::shared_ptr<Base>>>(baseAnyVec);
    ASSERT_EQ(baseVec.size(), 2);
    EXPECT_EQ(baseVec[0]->getName(), "Derived");
    EXPECT_EQ(baseVec[1]->getName(), "Derived");

    // 反向转换
    std::any convertedBackAnyVec = converter.convertDown(baseAnyVec);
    auto derivedVecBack = std::any_cast<std::vector<std::shared_ptr<Derived>>>(
        convertedBackAnyVec);
    ASSERT_EQ(derivedVecBack.size(), 2);
    EXPECT_EQ(derivedVecBack[0]->getName(), "Derived");
    EXPECT_EQ(derivedVecBack[1]->getName(), "Derived");
}

TEST(TypeConversionTest, MapConversion) {
    std::map<int, std::shared_ptr<Derived>> derivedMap = {
        {1, std::make_shared<Derived>()}, {2, std::make_shared<Derived>()}};
    std::any derivedAnyMap = derivedMap;

    MapConversion<std::map, int, std::shared_ptr<Derived>, int,
                  std::shared_ptr<Base>>
        converter;

    std::any baseAnyMap = converter.convert(derivedAnyMap);
    auto baseMap =
        std::any_cast<std::map<int, std::shared_ptr<Base>>>(baseAnyMap);
    ASSERT_EQ(baseMap.size(), 2);
    EXPECT_EQ(baseMap[1]->getName(), "Derived");
    EXPECT_EQ(baseMap[2]->getName(), "Derived");

    // 反向转换
    std::any convertedBackAnyMap = converter.convertDown(baseAnyMap);
    auto derivedMapBack =
        std::any_cast<std::map<int, std::shared_ptr<Derived>>>(
            convertedBackAnyMap);
    ASSERT_EQ(derivedMapBack.size(), 2);
    EXPECT_EQ(derivedMapBack[1]->getName(), "Derived");
    EXPECT_EQ(derivedMapBack[2]->getName(), "Derived");
}

TEST(TypeConversionTest, SetConversion) {
    auto derived1 = std::make_shared<Derived>();
    auto derived2 = std::make_shared<Derived>();
    std::set<std::shared_ptr<Derived>> derivedSet = {derived1, derived2};
    std::any derivedAnySet = derivedSet;

    SetConversion<std::set, std::shared_ptr<Derived>, std::shared_ptr<Base>>
        converter;

    std::any baseAnySet = converter.convert(derivedAnySet);
    auto baseSet = std::any_cast<std::set<std::shared_ptr<Base>>>(baseAnySet);
    ASSERT_EQ(baseSet.size(), 2);
    for (const auto& elem : baseSet) {
        EXPECT_EQ(elem->getName(), "Derived");
    }

    // 反向转换
    std::any convertedBackAnySet = converter.convertDown(baseAnySet);
    auto derivedSetBack =
        std::any_cast<std::set<std::shared_ptr<Derived>>>(convertedBackAnySet);
    ASSERT_EQ(derivedSetBack.size(), 2);
    for (const auto& elem : derivedSetBack) {
        EXPECT_EQ(elem->getName(), "Derived");
    }
}

TEST(TypeConversionTest, TypeMismatchThrows) {
    std::any wrongType = Unrelated();

    StaticConversion<Derived*, Base*> converter;

    // 使用未使用的变量来接收返回值，避免忽略nodiscrd警告
    EXPECT_THROW(
        {
            try {
                auto unused = converter.convert(wrongType);
            } catch (const BadConversionException& e) {
                throw;
            }
        },
        BadConversionException);

    EXPECT_THROW(
        {
            try {
                auto unused = converter.convertDown(wrongType);
            } catch (const BadConversionException& e) {
                throw;
            }
        },
        BadConversionException);
}

TEST(TypeConversionTest, UnsupportedConversionThrows) {
    std::any derivedPtr = std::make_shared<Derived>();

    TypeConversions typeConversions;

    EXPECT_THROW(({
                     try {
                         auto unused =
                             typeConversions.convert<Base, Derived>(derivedPtr);
                     } catch (const BadConversionException& e) {
                         throw;
                     }
                 }),
                 BadConversionException);
}

TEST(TypeConversionTest, ConversionExistsCheck) {
    TypeConversions typeConversions;

    typeConversions.addBaseClass<Base, Derived>();

    EXPECT_TRUE(
        typeConversions.canConvert(userType<Derived>(), userType<Base>()));
    EXPECT_FALSE(
        typeConversions.canConvert(userType<Unrelated>(), userType<Base>()));
}
