#include <catch2/catch_test_macros.hpp>
#include <ferrugo/core/format_utils.hpp>
#include <ferrugo/core/predicates.hpp>

#include "matchers.hpp"

using namespace ferrugo;
namespace p = ferrugo::predicates;
using namespace std::string_view_literals;

auto divisible_by(int divisor)
{
    return [=](int v) { return v % divisor == 0; };
}

TEST_CASE("predicates - format", "")
{
    REQUIRE_THAT(  //
        (core::str(p::all(p::ge(0), p::lt(5)))),
        equal_to("(all (ge 0) (lt 5))"sv));
    REQUIRE_THAT(  //
        (core::str(p::any(1, 2, 3, p::ge(100)))),
        equal_to("(any 1 2 3 (ge 100))"sv));
    REQUIRE_THAT(  //
        (core::str(p::negate(p::any(1, 2, 3)))),
        equal_to("(not (any 1 2 3))"sv));
    REQUIRE_THAT(  //
        (core::str(p::each(p::any(1, 2, 3)))),
        equal_to("(each (any 1 2 3))"sv));
    REQUIRE_THAT(  //
        (core::str(p::contains(p::any(1, 2, 3)))),
        equal_to("(contains (any 1 2 3))"sv));
    REQUIRE_THAT(  //
        (core::str(p::size_is(p::lt(8)))),
        equal_to("(size_is (lt 8))"sv));
}

TEST_CASE("predicates - eq", "")
{
    const auto pred = p::eq(10);
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(eq 10)"sv));
    REQUIRE_THAT(pred(10), equal_to(true));
    REQUIRE_THAT(pred(5), equal_to(false));
    REQUIRE_THAT(pred(15), equal_to(false));
}

TEST_CASE("predicates - ne", "")
{
    const auto pred = p::ne(10);
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(ne 10)"sv));
    REQUIRE_THAT(pred(10), equal_to(false));
    REQUIRE_THAT(pred(5), equal_to(true));
    REQUIRE_THAT(pred(15), equal_to(true));
}

TEST_CASE("predicates - lt", "")
{
    const auto pred = p::lt(10);
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(lt 10)"sv));
    REQUIRE_THAT(pred(10), equal_to(false));
    REQUIRE_THAT(pred(5), equal_to(true));
    REQUIRE_THAT(pred(15), equal_to(false));
}

TEST_CASE("predicates - gt", "")
{
    const auto pred = p::gt(10);
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(gt 10)"sv));
    REQUIRE_THAT(pred(10), equal_to(false));
    REQUIRE_THAT(pred(5), equal_to(false));
    REQUIRE_THAT(pred(15), equal_to(true));
}

TEST_CASE("predicates - le", "")
{
    const auto pred = p::le(10);
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(le 10)"sv));
    REQUIRE_THAT(pred(10), equal_to(true));
    REQUIRE_THAT(pred(5), equal_to(true));
    REQUIRE_THAT(pred(15), equal_to(false));
}

TEST_CASE("predicates - ge", "")
{
    const auto pred = p::ge(10);
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(ge 10)"sv));
    REQUIRE_THAT(pred(10), equal_to(true));
    REQUIRE_THAT(pred(5), equal_to(false));
    REQUIRE_THAT(pred(15), equal_to(true));
}

TEST_CASE("predicates - all", "")
{
    const auto pred = p::all(p::ge(10), p::lt(20), divisible_by(3));
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(all (ge 10) (lt 20) divisible_by(int)::{lambda(int)#1})"sv));
    REQUIRE_THAT(pred(9), equal_to(false));
    REQUIRE_THAT(pred(10), equal_to(false));
    REQUIRE_THAT(pred(11), equal_to(false));
    REQUIRE_THAT(pred(12), equal_to(true));
    REQUIRE_THAT(pred(13), equal_to(false));
    REQUIRE_THAT(pred(14), equal_to(false));
    REQUIRE_THAT(pred(15), equal_to(true));
    REQUIRE_THAT(pred(16), equal_to(false));
    REQUIRE_THAT(pred(17), equal_to(false));
    REQUIRE_THAT(pred(18), equal_to(true));
    REQUIRE_THAT(pred(19), equal_to(false));
    REQUIRE_THAT(pred(20), equal_to(false));
    REQUIRE_THAT(pred(21), equal_to(false));
}

TEST_CASE("predicates - any", "")
{
    const auto pred = p::any(divisible_by(5), divisible_by(3), 100);
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(any divisible_by(int)::{lambda(int)#1} divisible_by(int)::{lambda(int)#1} 100)"sv));
    REQUIRE_THAT(pred(9), equal_to(true));
    REQUIRE_THAT(pred(10), equal_to(true));
    REQUIRE_THAT(pred(11), equal_to(false));
    REQUIRE_THAT(pred(12), equal_to(true));
    REQUIRE_THAT(pred(13), equal_to(false));
    REQUIRE_THAT(pred(14), equal_to(false));
    REQUIRE_THAT(pred(15), equal_to(true));
    REQUIRE_THAT(pred(16), equal_to(false));
    REQUIRE_THAT(pred(17), equal_to(false));
    REQUIRE_THAT(pred(18), equal_to(true));
    REQUIRE_THAT(pred(19), equal_to(false));
    REQUIRE_THAT(pred(20), equal_to(true));
    REQUIRE_THAT(pred(21), equal_to(true));
    REQUIRE_THAT(pred(100), equal_to(true));
}

TEST_CASE("predicates - negate", "")
{
    const auto pred = p::negate(p::all(p::ge(0), p::lt(5)));
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(not (all (ge 0) (lt 5)))"sv));
    REQUIRE_THAT(pred(-1), equal_to(true));
    REQUIRE_THAT(pred(0), equal_to(false));
    REQUIRE_THAT(pred(1), equal_to(false));
    REQUIRE_THAT(pred(2), equal_to(false));
    REQUIRE_THAT(pred(3), equal_to(false));
    REQUIRE_THAT(pred(4), equal_to(false));
    REQUIRE_THAT(pred(5), equal_to(true));
}

TEST_CASE("predicates - is_empty", "")
{
    const auto pred = p::is_empty();
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(is_empty)"sv));
    REQUIRE_THAT(pred(""sv), equal_to(true));
    REQUIRE_THAT(pred("###"sv), equal_to(false));
}

TEST_CASE("predicates - size_is", "")
{
    const auto pred = p::size_is(p::lt(3));
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(size_is (lt 3))"sv));
    REQUIRE_THAT(pred(""sv), equal_to(true));
    REQUIRE_THAT(pred("#"sv), equal_to(true));
    REQUIRE_THAT(pred("##"sv), equal_to(true));
    REQUIRE_THAT(pred("###"sv), equal_to(false));
}

TEST_CASE("predicates - each", "")
{
    const auto pred = p::each('#');
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(each #)"sv));
    REQUIRE_THAT(pred(""sv), equal_to(true));
    REQUIRE_THAT(pred("#"sv), equal_to(true));
    REQUIRE_THAT(pred("##"sv), equal_to(true));
    REQUIRE_THAT(pred("##__"sv), equal_to(false));
}

TEST_CASE("predicates - contains", "")
{
    const auto pred = p::contains('#');
    REQUIRE_THAT(  //
        (core::str(pred)),
        equal_to("(contains #)"sv));
    REQUIRE_THAT(pred(""sv), equal_to(false));
    REQUIRE_THAT(pred("#"sv), equal_to(true));
    REQUIRE_THAT(pred("##"sv), equal_to(true));
    REQUIRE_THAT(pred("__"sv), equal_to(false));
}
