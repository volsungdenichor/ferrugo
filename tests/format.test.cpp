#include <catch2/catch_test_macros.hpp>
#include <ferrugo/core/format.hpp>
#include <ferrugo/core/format_utils.hpp>

#include "matchers.hpp"

using namespace std::string_view_literals;

using namespace ferrugo;

TEST_CASE("format - no explicit indices", "[format]")
{
    REQUIRE_THAT(core::str(core::detail::format_string("{} has {}.")), equal_to("{0} has {1}."));
}

TEST_CASE("format - some explicit indices", "[format]")
{
    REQUIRE_THAT(core::str(core::detail::format_string("{1} has {}.")), equal_to("{1} has {1}."));
}

TEST_CASE("format - all explicit indices", "[format]")
{
    REQUIRE_THAT(core::str(core::detail::format_string("{1} has {0}.")), equal_to("{1} has {0}."));
}

TEST_CASE("format - argument format specifiers", "[format]")
{
    REQUIRE_THAT(core::str(core::detail::format_string("{:abc} has {:def}.")), equal_to("{0:abc} has {1:def}."));
}

TEST_CASE("format - explicit indices and argument format specifiers", "[format]")
{
    REQUIRE_THAT(core::str(core::detail::format_string("{1:abc} has {0:def}.")), equal_to("{1:abc} has {0:def}."));
}

TEST_CASE("format", "")
{
    REQUIRE_THAT(  //
        core::format("{} has {}.")("Alice", "a cat"),
        equal_to("Alice has a cat."sv));
}

TEST_CASE("format - a vector", "")
{
    REQUIRE_THAT(  //
        core::format("{} has the following animals: {}.")("Alice", std::vector{ "a cat", "a dog" }),
        equal_to("Alice has the following animals: [a cat, a dog]."sv));
}

TEST_CASE("print", "")
{
    std::stringstream ss;
    core::print(ss, "{} has {}.")("Alice", "a cat");
    REQUIRE_THAT(ss.str(), equal_to("Alice has a cat."sv));
}

TEST_CASE("println", "")
{
    std::stringstream ss;
    core::println(ss, "{} has {}.")("Alice", "a cat");
    REQUIRE_THAT(ss.str(), equal_to("Alice has a cat.\n"sv));
}

TEST_CASE("join", "")
{
    REQUIRE_THAT(
        core::format("{} has {}.")("Alice", core::join(std::vector{ "a cat", "a dog", "a turtle" }, ", ")),
        equal_to("Alice has a cat, a dog, a turtle."sv));

}