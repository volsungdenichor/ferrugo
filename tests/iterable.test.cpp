#include <gmock/gmock.h>

#include <ferrugo/iterable.hpp>

TEST(iterable, iota)
{
    EXPECT_THAT(ferrugo::iota(0).take(5), testing::ElementsAre(0, 1, 2, 3, 4));
}

TEST(iterable, range)
{
    EXPECT_THAT(ferrugo::range(0, 5), testing::ElementsAre(0, 1, 2, 3, 4));
}

TEST(iterable, repeat)
{
    EXPECT_THAT(ferrugo::repeat('x', 5), testing::ElementsAre('x', 'x', 'x', 'x', 'x'));
    EXPECT_THAT(ferrugo::repeat('x').take(5), testing::ElementsAre('x', 'x', 'x', 'x', 'x'));
}

TEST(iterable, empty)
{
    EXPECT_THAT(ferrugo::empty<int>(), testing::IsEmpty());
}

TEST(iterable, transform)
{
    static const auto sqr = [](int x) { return x * x; };
    EXPECT_THAT(ferrugo::range(0, 5).transform(sqr), testing::ElementsAre(0, 1, 4, 9, 16));
}

TEST(iterable, filter)
{
    static const auto is_prime = [](int n)
    {
        const auto r = ferrugo::range(2, (n / 2) + 1);
        return n >= 2 && std::none_of(r.begin(), r.end(), [=](int d) { return n % d == 0; });
    };
    EXPECT_THAT(ferrugo::range(0, 20).filter(is_prime), testing::ElementsAre(2, 3, 5, 7, 11, 13, 17, 19));
}
