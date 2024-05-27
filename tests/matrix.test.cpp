#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <ferrugo/core/arrays/array.hpp>

#include "matchers.hpp"

using namespace ferrugo;
using matrix = core::matrix<int, core::size<2, 2>>;

struct vector_equal_fn
{
    template <class V>
    struct impl : public Catch::Matchers::MatcherGenericBase
    {
        V m_expected;
        explicit impl(V expected) : m_expected{ expected }
        {
        }

        template <class T>
        bool match(const T& actual) const
        {
            return core::distance(actual, m_expected) < 0.001;
        }

        std::string describe() const
        {
            std::stringstream ss;
            ss << "Equal to " << m_expected;
            return ss.str();
        }
    };

    template <class V>
    auto operator()(V v) const -> impl<V>
    {
        return impl<V>{ v };
    }
};

static constexpr inline auto vector_equal = vector_equal_fn{};

TEST_CASE("matrix negation", "[matrix]")
{
    REQUIRE_THAT(
        (-matrix{ 1, 2, 11, 12 }),  //
        equal_to(matrix{ -1, -2, -11, -12 }));
}

TEST_CASE("matrix addition", "[matrix]")
{
    REQUIRE_THAT(
        (matrix{ 1, 2, 11, 12 } + matrix{ 1, 1, 2, 2 }),  //
        equal_to(matrix{ 2, 3, 13, 14 }));
}

TEST_CASE("matrix addition assignment", "[matrix]")
{
    matrix m{ 1, 2, 11, 12 };
    REQUIRE_THAT(
        (m += matrix{ 1, 1, 1, 1 }),  //
        equal_to(matrix{ 2, 3, 12, 13 }));
}

TEST_CASE("matrix subtraction", "[matrix]")
{
    REQUIRE_THAT(
        (matrix{ 1, 2, 11, 12 } - matrix{ 1, 1, 2, 2 }),  //
        equal_to(matrix{ 0, 1, 9, 10 }));
}

TEST_CASE("matrix subtraction assignment", "[matrix]")
{
    matrix m{ 1, 2, 11, 12 };
    REQUIRE_THAT(
        (m -= matrix{ 1, 1, 1, 1 }),  //
        equal_to(matrix{ 0, 1, 10, 11 }));
}

TEST_CASE("matrix-matrix multiplication", "[matrix]")
{
    REQUIRE_THAT(
        (matrix{ 1, 2, 3, 4 } * matrix{ 5, 6, 7, 8 }),  //
        equal_to(matrix{ 19, 22, 43, 50 }));
}

TEST_CASE("matrix-scalar multiplication", "[matrix]")
{
    REQUIRE_THAT(
        (matrix{ 1, 2, 3, 4 } * 2),  //
        equal_to(matrix{ 2, 4, 6, 8 }));

    REQUIRE_THAT(
        (2 * matrix{ 1, 2, 3, 4 }),  //
        equal_to(matrix{ 2, 4, 6, 8 }));
}

TEST_CASE("matrix-scalar division", "[matrix]")
{
    REQUIRE_THAT(
        (matrix{ 3, 6, -3, -9 } / 3),  //
        equal_to(matrix{ 1, 2, -1, -3 }));
}

TEST_CASE("vector length", "[vector]")
{
    REQUIRE_THAT((core::length(core::vector_2d<float>{ 1, 0 })), equal_to(1.F));
    REQUIRE_THAT(
        (core::length(core::vector_2d<float>{ 1, 1 })), Catch::Matchers::WithinAbs(1.41421F, 0.001F));
    REQUIRE_THAT((core::length(core::vector_2d<float>{ 0, 1 })), equal_to(1.F));
}

TEST_CASE("vector unit", "[vector]")
{
    REQUIRE_THAT(                                                        //
        (core::unit(core::vector_2d<float>{ 2, 0 })),  //
        vector_equal(core::vector_2d<float>{ 1, 0 }));
    REQUIRE_THAT(                                                        //
        (core::unit(core::vector_2d<float>{ 2, 2 })),  //
        vector_equal(core::vector_2d<float>{ 0.707107F, 0.707107F  }));
}
