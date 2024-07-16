#include "atom/function/conversion.hpp"
#include <gtest/gtest.h>

TEST(ConversionTest, StaticConversion) {
    atom::meta::TypeConversionBase* conversion =
        new atom::meta::StaticConversion<int, float>();
    std::any from = 10;
    std::any to = conversion->convert(from);
    EXPECT_EQ(std::any_cast<float>(to), 10);
    delete conversion;
}

class Base {
public:
    virtual ~Base() = default;
    virtual void hello() const { std::cout << "Hello from Base\n"; }
};

class Derived : public Base {
public:
    void hello() const override { std::cout << "Hello from Derived\n"; }
};

TEST(ConversionTest, BaseClassConversion) {
    atom::meta::TypeConversions conversions;
    conversions.addBaseClass<Base, Derived>();
    conversions.addConversion(
        std::make_shared<atom::meta::DynamicConversion<
            std::shared_ptr<Derived>, std::shared_ptr<Base>>>());
    std::any from = std::make_shared<Derived>();
    std::any to =
        conversions.convert<std::shared_ptr<Base>, std::shared_ptr<Derived>>(
            from);
    EXPECT_NO_THROW(std::any_cast<std::shared_ptr<Base>>(to)->hello());
}

TEST(ConversionTest, VectorConversion) {
    atom::meta::TypeConversions conversions;
    conversions.addVectorConversion<Derived, Base>();
    std::vector<std::shared_ptr<Derived>> from;
    from.push_back(std::make_shared<Derived>());
    std::any to =
        conversions.convert<std::vector<std::shared_ptr<Base>>,
                            std::vector<std::shared_ptr<Derived>>>(from);
    EXPECT_EQ(std::any_cast<std::vector<std::shared_ptr<Base>>>(to).size(), 1);
}

/*
TEST(ConversionTest, SequenceConversion) {
    atom::meta::TypeConversions conversions;
    conversions.add_sequence_conversion<std::list, int, float>();
    std::list<int> from;
    from.push_back(10);
    std::any to = conversions.convert<std::list<float>, std::list<int>>(from);
    EXPECT_EQ(std::any_cast<std::list<float>>(to).front(), 10.0);
}


TEST(ConversionTest, SetConversion) {
    atom::meta::TypeConversions conversions;
    conversions.add_set_conversion<std::set, int, float>();
    std::set<int> from;
    from.insert(10);
    std::any to = conversions.convert<std::set<float>, std::set<int>>(from);
    EXPECT_TRUE(std::any_cast<std::set<float>>(to).count(10.0) == 1);
}
*/
