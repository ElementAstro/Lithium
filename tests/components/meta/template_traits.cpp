#include "atom/function/template_traits.hpp"
#include <gtest/gtest.h>

using namespace atom::meta;

// Helper templates for testing
template <typename T>
struct Wrapper {};

template <typename T, typename U>
struct Pair {};

template <auto V>
struct ValueWrapper {};

// Tests for is_template
TEST(TemplateTraitsTest, IsTemplateTest) {
    static_assert(is_template_v<Wrapper<int>>);
    static_assert(!is_template_v<int>);
}

// Tests for template_traits
TEST(TemplateTraitsTest, TemplateTraitsTest) {
    using Traits = template_traits<Wrapper<int>>;
    static_assert(std::is_same_v<Traits::args_type, std::tuple<int>>);

    using PairTraits = template_traits<Pair<int, double>>;
    static_assert(
        std::is_same_v<PairTraits::args_type, std::tuple<int, double>>);
}

// Tests for template_arity_v
TEST(TemplateTraitsTest, TemplateArityTest) {
    static_assert(template_arity_v<Wrapper<int>> == 1);
    static_assert(template_arity_v<Pair<int, double>> == 2);
}

// Tests for is_specialization_of
TEST(TemplateTraitsTest, IsSpecializationOfTest) {
    static_assert(is_specialization_of_v<Wrapper, Wrapper<int>>);
    static_assert(!is_specialization_of_v<Wrapper, int>);
}

// Tests for template_arg_t
TEST(TemplateTraitsTest, TemplateArgTest) {
    static_assert(std::is_same_v<template_arg_t<0, Pair<int, double>>, int>);
    static_assert(std::is_same_v<template_arg_t<1, Pair<int, double>>, double>);
}

// Tests for is_derived_from_all
struct Base1 {};
struct Base2 {};
struct Derived : Base1, Base2 {};

TEST(TemplateTraitsTest, IsDerivedFromAllTest) {
    static_assert(is_derived_from_all_v<Derived, Base1, Base2>);
    static_assert(!is_derived_from_all_v<Base1, Base1, Base2>);
}

// Tests for is_partial_specialization_of
template <typename T, typename U = int>
struct PartialSpecialization {};

TEST(TemplateTraitsTest, IsPartialSpecializationOfTest) {
    static_assert(is_partial_specialization_of_v<PartialSpecialization<int>,
                                                 PartialSpecialization>);
    static_assert(
        !is_partial_specialization_of_v<Wrapper<int>, PartialSpecialization>);
}

// Tests for is_class_template
TEST(TemplateTraitsTest, IsClassTemplateTest) {
    static_assert(is_class_template_v<Wrapper<int>>);
    static_assert(!is_class_template_v<int>);
}

// Tests for is_variable_template
template <auto... Values>
struct VariableTemplate {};

TEST(TemplateTraitsTest, IsVariableTemplateTest) {
    static_assert(is_variable_template_v<VariableTemplate<1, 2, 3>>);
    static_assert(!is_variable_template_v<int>);
}

// Tests for is_alias_template
template <typename T>
using AliasTemplate = Wrapper<T>;

TEST(TemplateTraitsTest, IsAliasTemplateTest) {
    static_assert(is_alias_template_v<AliasTemplate<int>>);
    static_assert(!is_alias_template_v<int>);
}

// Tests for template_args_as_tuple
TEST(TemplateTraitsTest, TemplateArgsAsTupleTest) {
    using Tuple1 = template_args_as_tuple_t<Wrapper<int>>;
    static_assert(std::is_same_v<Tuple1, std::tuple<int>>);

    using Tuple2 = template_args_as_tuple_t<Pair<int, double>>;
    static_assert(std::is_same_v<Tuple2, std::tuple<int, double>>);
}

// Tests for template_args_as_value_tuple
template <auto... Values>
struct ValueTemplate {};

TEST(TemplateTraitsTest, TemplateArgsAsValueTupleTest) {
    using ValueTuple = template_args_as_value_tuple_t<ValueTemplate<1, 2, 3>>;
    static_assert(
        std::is_same_v<ValueTuple, std::tuple<std::integral_constant<int, 1>,
                                              std::integral_constant<int, 2>,
                                              std::integral_constant<int, 3>>>);
}

// Tests for count_occurrences
TEST(TemplateTraitsTest, CountOccurrencesTest) {
    static_assert(count_occurrences_v<int, double, int, float, int> == 2);
    static_assert(count_occurrences_v<int, double, float, double> == 0);
}

template <class... Args>
struct MyTemplate {
    using type = std::tuple<Args...>;
};

TEST(InstantiatedTraitsTest, BasicTest) {
    using Tuple = std::tuple<int, double, char>;
    using Result = instantiated_t<MyTemplate, Tuple>;

    using Expected = MyTemplate<int, double, char>;

    static_assert(std::is_same_v<Result, Expected>,
                  "Instantiated type is incorrect");
}

struct WellFormedTuple {
    using type = std::tuple<int, double>;
};

struct NotWellFormed {
    // 不是一个元组
};

TEST(IsTupleLikeWellFormedTest, WellFormedTuple) {
    static_assert(is_tuple_like_well_formed<WellFormedTuple::type>(),
                  "WellFormedTuple should be tuple-like");
}

TEST(IsTupleLikeWellFormedTest, NotWellFormed) {
    static_assert(!is_tuple_like_well_formed<NotWellFormed>(),
                  "NotWellFormed should not be tuple-like");
}

struct Copyable {
    Copyable() = default;
    Copyable(const Copyable&) = default;
    Copyable& operator=(const Copyable&) = default;
    ~Copyable() = default;
};

struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    ~NonCopyable() = default;
};

TEST(HasCopyabilityTest, Copyable) {
    EXPECT_TRUE(has_copyability<Copyable>(constraint_level::nontrivial));
    EXPECT_FALSE(has_copyability<NonCopyable>(constraint_level::nontrivial));
}

TEST(HasCopyabilityTest, NonCopyable) {
    EXPECT_FALSE(has_copyability<NonCopyable>(constraint_level::nontrivial));
}

TEST(HasRelocatabilityTest, Copyable) {
    EXPECT_TRUE(has_relocatability<Copyable>(constraint_level::nontrivial));
}

TEST(HasRelocatabilityTest, NonCopyable) {
    EXPECT_TRUE(has_relocatability<NonCopyable>(constraint_level::nontrivial));
}

TEST(HasDestructibilityTest, Copyable) {
    EXPECT_TRUE(has_destructibility<Copyable>(constraint_level::nontrivial));
}

TEST(HasDestructibilityTest, NonCopyable) {
    EXPECT_TRUE(has_destructibility<NonCopyable>(constraint_level::nontrivial));
}