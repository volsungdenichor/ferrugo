#pragma once

#include <array>
#include <cmath>
#include <ferrugo/core/math.hpp>
#include <ferrugo/core/optional.hpp>
#include <functional>
#include <iostream>
#include <numeric>

namespace ferrugo
{
namespace core
{

struct uninitialized_t
{
};
static constexpr inline uninitialized_t uninitialized{};

template <std::size_t... V>
struct size
{
    static constexpr std::size_t dim_count = sizeof...(V);
    static constexpr std::array<std::size_t, sizeof...(V)> value = { V... };
    static constexpr std::size_t volume = (V * ...);

    static constexpr std::size_t get(std::size_t index)
    {
        return value[index];
    }
};

template <class T>
struct is_matrix : std::false_type
{
};

template <class T>
struct is_scalar : std::bool_constant<!is_matrix<T>{}>
{
};

template <class T, class Size>
struct matrix
{
    using value_type = T;
    using const_reference = const T&;
    using reference = T&;
    using location_type = std::array<std::size_t, Size::dim_count>;

    static constexpr std::size_t row_count = Size::get(0);
    static constexpr std::size_t col_count = Size::get(1);
    static constexpr std::size_t volume = Size::volume;

    matrix()
    {
        std::fill(std::begin(m_data), std::end(m_data), value_type{});
    }

    matrix(uninitialized_t)
    {
    }

    matrix(const matrix& other)
    {
        std::copy(std::begin(other.m_data), std::end(other.m_data), std::begin(m_data));
    }

    matrix(std::initializer_list<T> init)
    {
        std::copy(std::begin(init), std::end(init), std::begin(m_data));
    }

    template <class U>
    matrix(const matrix<U, Size>& other)
    {
        std::copy(std::begin(other.m_data), std::end(other.m_data), std::begin(m_data));
    }

    const_reference operator[](std::size_t n) const
    {
        return m_data[n];
    }

    reference operator[](std::size_t n)
    {
        return m_data[n];
    }

    const_reference operator[](const location_type& loc) const
    {
        return m_data[get_offset(loc)];
    }

    reference operator[](const location_type& loc)
    {
        return m_data[get_offset(loc)];
    }

    static constexpr std::ptrdiff_t get_offset(const location_type& loc)
    {
        return loc[0] * col_count + loc[1];
    }

    std::array<value_type, volume> m_data;
};

template <class T, class Size>
struct is_matrix<matrix<T, Size>> : std::true_type
{
};

template <class T, std::size_t D>
using square_matrix = matrix<T, size<D, D>>;

template <class T, std::size_t D>
using vector = matrix<T, size<1, D>>;

template <class T>
using square_matrix_2d = square_matrix<T, 3>;

template <class T>
using square_matrix_3d = square_matrix<T, 4>;

template <class T>
using vector_2d = vector<T, 2>;

template <class T>
using vector_3d = vector<T, 3>;

template <class T, std::size_t R, std::size_t C>
std::ostream& operator<<(std::ostream& os, const matrix<T, size<R, C>>& item)
{
    os << "[";

    for (std::size_t r = 0; r < R; ++r)
    {
        os << "[";

        for (std::size_t c = 0; c < C; ++c)
        {
            if (c != 0)
            {
                os << " ";
            }

            os << item[{ r, c }];
        }

        os << "]";
    }

    os << "]";

    return os;
}

template <class T, std::size_t D>
std::ostream& operator<<(std::ostream& os, const vector<T, D>& item)
{
    os << "[";

    for (std::size_t d = 0; d < D; ++d)
    {
        if (d != 0)
        {
            os << " ";
        }

        os << item[d];
    }

    os << "]";

    return os;
}

template <class T, class Size>
auto operator+(const matrix<T, Size>& item) -> matrix<T, Size>
{
    return item;
}

template <class T, class Size>
auto operator-(const matrix<T, Size>& item) -> matrix<T, Size>
{
    matrix<T, Size> result{ uninitialized };
    std::transform(std::begin(item.m_data), std::end(item.m_data), std::begin(result.m_data), std::negate<>{});
    return result;
}

template <class T, class U, class Size, class Res = std::invoke_result_t<std::plus<>, T, U>>
auto operator+=(matrix<T, Size>& lhs, const matrix<U, Size>& rhs) -> matrix<T, Size>&
{
    std::transform(
        std::begin(lhs.m_data), std::end(lhs.m_data), std::begin(rhs.m_data), std::begin(lhs.m_data), std::plus<>{});
    return lhs;
}

template <class T, class U, class Size, class Res = std::invoke_result_t<std::plus<>, T, U>>
auto operator+(const matrix<T, Size>& lhs, const matrix<U, Size>& rhs) -> matrix<Res, Size>
{
    matrix<Res, Size> result{ uninitialized };
    std::transform(
        std::begin(lhs.m_data), std::end(lhs.m_data), std::begin(rhs.m_data), std::begin(result.m_data), std::plus<>{});
    return result;
}

template <class T, class U, class Size, class Res = std::invoke_result_t<std::minus<>, T, U>>
auto operator-=(matrix<T, Size>& lhs, const matrix<U, Size>& rhs) -> matrix<T, Size>&
{
    std::transform(
        std::begin(lhs.m_data), std::end(lhs.m_data), std::begin(rhs.m_data), std::begin(lhs.m_data), std::minus<>{});
    return lhs;
}

template <class T, class U, class Size, class Res = std::invoke_result_t<std::minus<>, T, U>>
auto operator-(const matrix<T, Size>& lhs, const matrix<U, Size>& rhs) -> matrix<Res, Size>
{
    matrix<Res, Size> result{ uninitialized };
    std::transform(
        std::begin(lhs.m_data), std::end(lhs.m_data), std::begin(rhs.m_data), std::begin(result.m_data), std::minus<>{});
    return result;
}

template <
    class T,
    class U,
    class Size,
    class Res = std::invoke_result_t<std::multiplies<>, T, U>,
    class = std::enable_if_t<is_scalar<U>{}>>
auto operator*=(matrix<T, Size>& lhs, U rhs) -> matrix<T, Size>&
{
    std::transform(
        std::begin(lhs.m_data),
        std::end(lhs.m_data),
        std::begin(lhs.m_data),
        std::bind(std::multiplies<>{}, std::placeholders::_1, rhs));
    return lhs;
}

template <
    class T,
    class U,
    class Size,
    class Res = std::invoke_result_t<std::multiplies<>, T, U>,
    class = std::enable_if_t<is_scalar<U>{}>>
auto operator*(const matrix<T, Size>& lhs, U rhs) -> matrix<Res, Size>
{
    matrix<Res, Size> result{ uninitialized };
    std::transform(
        std::begin(lhs.m_data),
        std::end(lhs.m_data),
        std::begin(result.m_data),
        std::bind(std::multiplies<>{}, std::placeholders::_1, rhs));
    return result;
}

template <
    class T,
    class U,
    class Size,
    class Res = std::invoke_result_t<std::multiplies<>, T, U>,
    class = std::enable_if_t<is_scalar<T>{}>>
auto operator*(T lhs, const matrix<U, Size>& rhs) -> matrix<Res, Size>
{
    return rhs * lhs;
}

template <
    class T,
    class U,
    std::size_t R,
    std::size_t C,
    std::size_t D,
    class Res = std::invoke_result_t<std::multiplies<>, T, U>>
auto operator*(const matrix<T, size<R, D>>& lhs, const matrix<U, size<D, C>>& rhs) -> matrix<Res, size<R, C>>
{
    matrix<Res, size<R, C>> result{};
    for (std::size_t r = 0; r < R; ++r)
    {
        for (std::size_t c = 0; c < C; ++c)
        {
            Res sum = {};

            for (std::size_t i = 0; i < D; ++i)
            {
                sum += lhs[{ r, i }] * rhs[{ i, c }];
            }

            result[{ r, c }] = sum;
        }
    }
    return result;
}

template <class T, class U, std::size_t D, class Res = std::invoke_result_t<std::multiplies<>, T, U>>
auto operator*(const vector<T, D>& lhs, const square_matrix<U, D + 1>& rhs) -> vector<Res, D>
{
    vector<Res, D> result{};

    for (size_t d = 0; d < lhs.size(); ++d)
    {
        Res sum = static_cast<Res>(rhs[{ D, d }]);

        for (std::size_t i = 0; i < D; ++i)
        {
            sum += lhs[i] * rhs[{ i, d }];
        }

        result[d] = sum;
    }

    return result;
}

template <class T, class U, std::size_t D, class Res = std::invoke_result_t<std::multiplies<>, T, U>>
auto operator*(const square_matrix<U, D + 1>& lhs, const vector<T, D>& rhs) -> vector<Res, D>
{
    return rhs * lhs;
}

template <class T, class U, std::size_t D, class Res = std::invoke_result_t<std::multiplies<>, T, U>>
auto& operator*=(vector<T, D>& lhs, const square_matrix<U, D + 1>& rhs)
{
    return lhs = lhs * rhs;
}

template <
    class T,
    class U,
    class Size,
    class Res = std::invoke_result_t<std::divides<>, T, U>,
    class = std::enable_if_t<is_scalar<U>{}>>
auto operator/=(matrix<T, Size>& lhs, U rhs) -> matrix<T, Size>&
{
    std::transform(
        std::begin(lhs.m_data),
        std::end(lhs.m_data),
        std::begin(lhs.m_data),
        std::bind(std::divides<>{}, std::placeholders::_1, rhs));
    return lhs;
}

template <
    class T,
    class U,
    class Size,
    class Res = std::invoke_result_t<std::divides<>, T, U>,
    class = std::enable_if_t<is_scalar<U>{}>>
auto operator/(const matrix<T, Size>& lhs, U rhs) -> matrix<Res, Size>
{
    matrix<Res, Size> result{ uninitialized };
    std::transform(
        std::begin(lhs.m_data),
        std::end(lhs.m_data),
        std::begin(result.m_data),
        std::bind(std::divides<>{}, std::placeholders::_1, rhs));
    return result;
}

template <class T, class U, class Size>
bool operator==(const matrix<T, Size>& lhs, const matrix<U, Size>& rhs)
{
    return std::equal(std::begin(lhs.m_data), std::end(lhs.m_data), std::begin(rhs.m_data));
}

template <class T, class U, class Size>
bool operator!=(const matrix<T, Size>& lhs, const matrix<U, Size>& rhs)
{
    return !(lhs == rhs);
}

struct minor_fn
{
    template <class T, std::size_t R, std::size_t C>
    auto operator()(const matrix<T, size<R, C>>& item, std::size_t row, std::size_t col) const
        -> matrix<T, size<R - 1, C - 1>>
    {
        static_assert(R > 1, "minor: invalid row");
        static_assert(C > 1, "minor: invalid col");

        matrix<T, size<R - 1, C - 1>> result;

        for (std::size_t r = 0; r < R - 1; ++r)
        {
            for (std::size_t c = 0; c < C - 1; ++c)
            {
                result[{ r, c }] = item[{ r + (r < row ? 0 : 1), c + (c < col ? 0 : 1) }];
            }
        }

        return result;
    }
};

static constexpr inline auto minor = minor_fn{};

struct det_fn
{
    template <class T>
    auto operator()(const square_matrix<T, 1>& item) const -> T
    {
        return item[{ 0, 0 }];
    }

    template <class T>
    auto operator()(const square_matrix<T, 2>& item) const
    {
        return +item[{ 0, 0 }] * item[{ 1, 1 }] - item[{ 0, 1 }] * item[{ 1, 0 }];
    }

    template <class T>
    auto operator()(const square_matrix<T, 3>& item) const
    {
        return +item[{ 0, 0 }] * item[{ 1, 1 }] * item[{ 2, 2 }] + item[{ 0, 1 }] * item[{ 1, 2 }] * item[{ 2, 0 }]
               + item[{ 0, 2 }] * item[{ 1, 0 }] * item[{ 2, 1 }] - item[{ 0, 2 }] * item[{ 1, 1 }] * item[{ 2, 0 }]
               - item[{ 0, 0 }] * item[{ 1, 2 }] * item[{ 2, 1 }] - item[{ 0, 1 }] * item[{ 1, 0 }] * item[{ 2, 2 }];
    }

    template <class T, std::size_t D>
    auto operator()(const square_matrix<T, D>& item) const
    {
        using Res = decltype(item[{ 0, 0 }] * (*this)(minor(item, 0, 0)));
        auto sum = Res{};

        for (std::size_t i = 0; i < D; ++i)
        {
            sum += (i % 2 == 0 ? 1 : -1) * item[{ 0, i }] * (*this)(minor(item, 0, i));
        }

        return sum;
    }
};

static constexpr inline auto det = det_fn{};

struct invert_fn
{
    template <class T, std::size_t D>
    auto operator()(const square_matrix<T, D>& value) const -> core::optional<square_matrix<T, D>>
    {
        auto d = det(value);

        if (!d)
        {
            return {};
        }

        square_matrix<T, D> result;

        for (std::size_t r = 0; r < D; ++r)
        {
            for (std::size_t c = 0; c < D; ++c)
            {
                result[{ c, r }] = T((r + c) % 2 == 0 ? 1 : -1) * det(minor(value, r, c)) / d;
            }
        }

        return result;
    }
};

static constexpr inline auto invert = invert_fn{};

struct dot_fn
{
    template <class T, class U, std::size_t D, class Res = std::invoke_result_t<std::multiplies<>, T, U>>
    auto operator()(const vector<T, D>& lhs, const vector<U, D>& rhs) const -> Res
    {
        return std::inner_product(std::begin(lhs.m_data), std::end(lhs.m_data), std::begin(rhs.m_data), Res{});
    }
};

static constexpr inline auto dot = dot_fn{};

struct norm_fn
{
    template <class T, std::size_t D, class Res = std::invoke_result_t<std::multiplies<>, T, T>>
    auto operator()(const vector<T, D>& item) const -> Res
    {
        return dot(item, item);
    }
};

static constexpr inline auto norm = norm_fn{};

struct length_fn
{
    template <class T, std::size_t D>
    auto operator()(const vector<T, D>& item) const
    {
        return math::sqrt(norm(item));
    }
};

static constexpr inline auto length = length_fn{};

struct normalize_fn
{
    template <class T, std::size_t D>
    auto operator()(vector<T, D>& item, T expected = T{ 1 }) const -> vector<T, D>&
    {
        auto len = length(item);

        if (len)
        {
            item = item * (expected / len);
        }

        return item;
    }
};

static constexpr inline auto normalize = normalize_fn{};

struct unit_fn
{
    template <class T, std::size_t D>
    auto operator()(vector<T, D> item, T expected = T{ 1 }) const -> vector<T, D>
    {
        normalize(item, expected);
        return item;
    }
};

static constexpr inline auto unit = unit_fn{};

struct distance_fn
{
    template <class T, class U, size_t D>
    auto operator()(const vector<T, D>& lhs, const vector<U, D>& rhs) const
    {
        return length(rhs - lhs);
    }
};

static constexpr inline auto distance = distance_fn{};

struct cross_fn
{
    template <class T, class U, class Res = std::invoke_result_t<std::multiplies<>, T, U>>
    auto operator()(const vector_2d<T>& lhs, const vector_2d<U>& rhs) const -> Res
    {
        return lhs[0] * rhs[1] - lhs[1] * rhs[0];
    }

    template <class T, class U, class Res = std::invoke_result_t<std::multiplies<>, T, U>>
    auto operator()(const vector_3d<T>& lhs, const vector_3d<U>& rhs) const -> vector<Res, 3>
    {
        return vector<Res, 3>{ lhs[1] * rhs[2] - lhs[2] * rhs[1],
                               lhs[2] * rhs[0] - lhs[0] * rhs[2],
                               lhs[0] * rhs[1] - lhs[1] * rhs[0] };
    }
};

static constexpr inline auto cross = cross_fn{};

struct projection_fn
{
    template <class T, size_t D>
    auto operator()(const vector<T, D>& lhs, const vector<T, D>& rhs) const
    {
        return rhs * (dot(rhs, lhs) / norm(rhs));
    }
};

static constexpr inline auto projection = projection_fn{};

struct rejection_fn
{
    template <class T, size_t D>
    auto operator()(const vector<T, D>& lhs, const vector<T, D>& rhs) const
    {
        return lhs - projection(lhs, rhs);
    }
};

static constexpr inline auto rejection = rejection_fn{};

}  // namespace core
}  // namespace ferrugo
