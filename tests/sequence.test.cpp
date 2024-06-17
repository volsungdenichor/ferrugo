#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <ferrugo/core/ranges/sequence.hpp>

#include "matchers.hpp"

using namespace std::string_view_literals;

using namespace ferrugo;

TEST_CASE("sequence - iota", "[sequence]")
{
    REQUIRE_THAT(seq::iota(0) |= seq::take(10), matchers::elements_are(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST_CASE("sequence - range", "[sequence]")
{
    REQUIRE_THAT(seq::range(0, 5), matchers::elements_are(0, 1, 2, 3, 4));
    REQUIRE_THAT(seq::range(2, 5), matchers::elements_are(2, 3, 4));
    REQUIRE_THAT(seq::range(5), matchers::elements_are(0, 1, 2, 3, 4));
}

TEST_CASE("sequence - enumerate", "[sequence]")
{
    REQUIRE_THAT(
        seq::range(10, 15) |= core::pipe(seq::enumerate(0)),
        matchers::elements_are(
            std::tuple{ 0, 10 }, std::tuple{ 1, 11 }, std::tuple{ 2, 12 }, std::tuple{ 3, 13 }, std::tuple{ 4, 14 }));
}

TEST_CASE("sequence - take", "[sequence]")
{
    REQUIRE_THAT(seq::range(0, 10) |= seq::take(5), matchers::elements_are(0, 1, 2, 3, 4));
}

TEST_CASE("sequence - drop", "[sequence]")
{
    REQUIRE_THAT(seq::range(0, 10) |= seq::drop(5), matchers::elements_are(5, 6, 7, 8, 9));
}

TEST_CASE("sequence - step", "[sequence]")
{
    REQUIRE_THAT(seq::range(0, 10) |= seq::step(3), matchers::elements_are(0, 3, 6, 9));
}

TEST_CASE("sequence - take_while", "[sequence]")
{
    REQUIRE_THAT(seq::range(0, 10) |= seq::take_while([](int x) { return x < 5; }), matchers::elements_are(0, 1, 2, 3, 4));
}

TEST_CASE("sequence - drop_while", "[sequence]")
{
    REQUIRE_THAT(seq::range(0, 10) |= seq::drop_while([](int x) { return x < 5; }), matchers::elements_are(5, 6, 7, 8, 9));
}

TEST_CASE("sequence - filter", "[sequence]")
{
    REQUIRE_THAT(seq::range(0, 10) |= seq::filter([](int x) { return x % 3 == 0; }), matchers::elements_are(0, 3, 6, 9));
}

TEST_CASE("sequence - transform", "[sequence]")
{
    REQUIRE_THAT(
        seq::range(0, 10) |= seq::transform([](int x) { return std::to_string(x * 10); }),
        matchers::elements_are("0", "10", "20", "30", "40", "50", "60", "70", "80", "90"));
}

TEST_CASE("sequence - chain", "[sequence]")
{
    REQUIRE_THAT(seq::chain(seq::range(0, 5), seq::range(100, 105)), matchers::elements_are(0, 1, 2, 3, 4, 100, 101, 102, 103, 104));
}

TEST_CASE("sequence - zip", "[sequence]")
{
    REQUIRE_THAT(
        seq::zip(seq::range(10, 15), seq::range(100, 110)),
        matchers::elements_are(
            std::tuple{ 10, 100 },
            std::tuple{ 11, 101 },
            std::tuple{ 12, 102 },
            std::tuple{ 13, 103 },
            std::tuple{ 14, 104 }));
}

TEST_CASE("sequence - zip_transform", "[sequence]")
{
    REQUIRE_THAT(
        seq::zip_transform(std::plus<>{}, seq::range(10, 15), seq::range(100, 110)), matchers::elements_are(110, 112, 114, 116, 118));
}

TEST_CASE("sequence - join", "[sequence]")
{
    REQUIRE_THAT(
        seq::range(5) |= seq::transform([](int x) { return seq::range(x); }) |= seq::join(),
        matchers::elements_are(0, 0, 1, 0, 1, 2, 0, 1, 2, 3));
}

TEST_CASE("sequence - transform_join", "[sequence]")
{
    REQUIRE_THAT(
        seq::range(5) |= seq::transform_join([](int x) { return seq::range(x); }),
        matchers::elements_are(0, 0, 1, 0, 1, 2, 0, 1, 2, 3));
}