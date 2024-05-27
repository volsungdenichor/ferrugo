#pragma once

#include <catch2/matchers/catch_matchers_templated.hpp>

template <class Op>
struct compare_matcher
{
    std::string_view m_name;

    template <class T>
    struct impl : public Catch::Matchers::MatcherGenericBase
    {
        T m_expected;
        std::string_view m_name;

        impl(T expected, std::string_view name) : m_expected{ std::move(expected) }, m_name{ name }
        {
        }

        template <class U>
        bool match(U&& actual) const
        {
            return Op{}(actual, m_expected);
        }

        std::string describe() const override
        {
            std::stringstream ss;
            ss << m_name << " " << m_expected;
            return ss.str();
        }
    };

    template <class T>
    auto operator()(T expected) const -> impl<T>
    {
        return impl<T>{ std::move(expected), m_name };
    }
};

static constexpr inline auto equal_to = compare_matcher<std::equal_to<>>{ "equal to" };
static constexpr inline auto not_equal_to = compare_matcher<std::not_equal_to<>>{ "not equal to" };
static constexpr inline auto less = compare_matcher<std::less<>>{ "less than" };
static constexpr inline auto greater = compare_matcher<std::greater<>>{ "greater than" };
static constexpr inline auto less_equal = compare_matcher<std::less_equal<>>{ "less than or equal to" };
static constexpr inline auto greater_equal = compare_matcher<std::greater_equal<>>{ "greater than or equal to" };
