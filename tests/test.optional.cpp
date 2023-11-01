#include <gmock/gmock.h>

#include <ferrugo/optional.hpp>

TEST(optional, default_constructed_has_no_value)
{
    ferrugo::optional<std::string> opt{};
    EXPECT_THAT(opt.has_value(), false);
}

TEST(optional, initialized_from_value_has_value)
{
    ferrugo::optional<std::string> opt{ "Ala ma kota" };
    EXPECT_THAT(opt.has_value(), true);
}

TEST(optional, initialized_from_none_has_no_value)
{
    ferrugo::optional<std::string> opt{ ferrugo::none };
    EXPECT_THAT(opt.has_value(), false);
}

TEST(optional, copy_constructor)
{
    const ferrugo::optional<std::string> original{ "abc" };
    ferrugo::optional<std::string> opt = original;

    EXPECT_THAT(opt.has_value(), true);
    EXPECT_THAT(*opt, "abc");

    EXPECT_THAT(*original, "abc");
    EXPECT_THAT(original.has_value(), true);
}

TEST(optional, move_constructor)
{
    ferrugo::optional<std::string> original{ "abc" };
    ferrugo::optional<std::string> opt = std::move(original);
    EXPECT_THAT(opt.has_value(), true);
    EXPECT_THAT(*opt, "abc");

    EXPECT_THAT(original.has_value(), false);
}

TEST(optional, copy_constructor_from_optional_reference)
{
    std::string text = "abc";
    ferrugo::optional<std::string&> original{ text };
    ferrugo::optional<std::string> opt = original;
    EXPECT_THAT(opt.has_value(), true);
    EXPECT_THAT(*opt, "abc");
}

TEST(optional, comparison_operators)
{
    ferrugo::optional<std::string> opt = "ABC";
    EXPECT_THAT(opt == ferrugo::none, false);
    EXPECT_THAT(opt != ferrugo::none, true);
    EXPECT_THAT(ferrugo::none == opt, false);
    EXPECT_THAT(ferrugo::none != opt, true);
    EXPECT_THAT(opt == "ABC", true);
    EXPECT_THAT(opt == "XYZ", false);
    EXPECT_THAT(opt != "ABC", false);
    EXPECT_THAT(opt != "XYZ", true);
    EXPECT_THAT("ABC" == opt, true);
    EXPECT_THAT("XYZ" == opt, false);
    EXPECT_THAT("ABC" != opt, false);
    EXPECT_THAT("XYZ" != opt, true);
}