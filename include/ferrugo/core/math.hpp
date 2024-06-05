#pragma once

#include <cmath>
#include <ferrugo/core/quantities.hpp>

namespace ferrugo
{
namespace math
{

struct sqrt_fn
{
    template <class T>
    using result_type = decltype(::sqrt(std::declval<T>()));

    template <class T>
    auto operator()(T v) const -> result_type<T>
    {
        return sqrt(v);
    }

    template <class T, class Q>
    auto operator()(quants::value_t<T, Q> v) const -> quants::value_t<result_type<T>, quants::sqr_root_result_t<Q>>
    {
        return quants::value_t<result_type<T>, quants::sqr_root_result_t<Q>>{ (*this)(v.get()) };
    }
};

static constexpr inline auto sqrt = sqrt_fn{};

struct abs_fn
{
    template <class T>
    auto operator()(T v) const -> T
    {
        return abs(v);
    }

    template <class T, class Q>
    auto operator()(quants::value_t<T, Q> v) const -> quants::value_t<T, Q>
    {
        return quants::value_t<T, Q>{ (*this)(v.get()) };
    }
};

static constexpr inline auto abs = abs_fn{};

struct sin_fn
{
    template <class T>
    auto operator()(T v) const -> T
    {
        return sin(v);
    }
};

static constexpr inline auto sin = sin_fn{};

struct cos_fn
{
    template <class T>
    auto operator()(T v) const -> T
    {
        return cos(v);
    }
};

static constexpr inline auto cos = cos_fn{};

struct tan_fn
{
    template <class T>
    auto operator()(T v) const -> T
    {
        return tan(v);
    }
};

static constexpr inline auto tan = tan_fn{};

struct cot_fn
{
    template <class T>
    auto operator()(T v) const -> T
    {
        return cot(v);
    }
};

static constexpr inline auto cot = cot_fn{};

struct asin_fn
{
    template <class T>
    auto operator()(T v) const -> T
    {
        return asin(v);
    }
};

static constexpr inline auto asin = asin_fn{};

struct acos_fn
{
    template <class T>
    auto operator()(T v) const -> T
    {
        return acos(v);
    }
};

static constexpr inline auto acos = acos_fn{};

struct floor_fn
{
    template <class T>
    auto operator ()(T x) const -> T
    {
        return floor(x);
    }

    template <class T, class Q>
    auto operator()(quants::value_t<T, Q> v) const -> quants::value_t<T, Q>
    {
        return quants::value_t<T, Q>{ (*this)(v.get()) };
    }
};

static constexpr inline auto floor = floor_fn{};

struct ceil_fn
{
    template <class T>
    auto operator ()(T x) const -> T
    {
        return ceil(x);
    }

    template <class T, class Q>
    auto operator()(quants::value_t<T, Q> v) const -> quants::value_t<T, Q>
    {
        return quants::value_t<T, Q>{ (*this)(v.get()) };
    }
};

static constexpr inline auto ceil = ceil_fn{};

struct round_fn
{
    template <class T>
    auto operator ()(T x) const -> T
    {
        return round(x);
    }

    template <class T, class Q>
    auto operator()(quants::value_t<T, Q> v) const -> quants::value_t<T, Q>
    {
        return quants::value_t<T, Q>{ (*this)(v.get()) };
    }
};

static constexpr inline auto round = round_fn{};

}  // namespace math
}  // namespace ferrugo