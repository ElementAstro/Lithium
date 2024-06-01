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
    static_assert(is_template_class_v<Wrapper<int>>);
    static_assert(!is_template_class_v<int>);
}

// Tests for template_traits
TEST(TemplateTraitsTest, TemplateTraitsTest) {
    using WrapperTraits = atom::meta::details::template_traits<Wrapper<int>>;
    static_assert(std::is_same_v<typename WrapperTraits::args_type,
                                 std::tuple<atom::meta::identity<int>>>);

    using PairTraits = atom::meta::details::template_traits<Pair<int, double>>;
    static_assert(std::is_same_v<typename PairTraits::args_type,
                                 std::tuple<atom::meta::identity<int>,
                                            atom::meta::identity<double>>>);
}

TEST(TemplateTraitsTest, TemplateArgTest) {
    static_assert(
        std::is_same_v<atom::meta::template_arg_t<0, Pair<int, double>>,
                       atom::meta::identity<int>>);
    static_assert(
        std::is_same_v<atom::meta::template_arg_t<1, Pair<int, double>>,
                       atom::meta::identity<double>>);
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
