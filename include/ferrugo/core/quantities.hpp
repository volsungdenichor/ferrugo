#pragma once

#include <array>
#include <iostream>
#include <type_traits>

namespace ferrugo
{

namespace quants
{

template <short... Dims>
struct quantity_t
{
    static constexpr std::size_t size = sizeof...(Dims);
    static constexpr std::array<short, size> values = { Dims... };

    quantity_t() = default;
};

template <class T>
struct is_quality : std::false_type
{
};

template <short... Dims>
struct is_quality<quantity_t<Dims...>> : std::true_type
{
};

template <class Q1, class Q2>
struct mul_result;

template <class Q1, class Q2>
using mul_result_t = typename mul_result<Q1, Q2>::type;

template <class Q1, class Q2>
struct div_result;

template <class Q1, class Q2>
using div_result_t = typename div_result<Q1, Q2>::type;

template <class Q, short N, short D>
struct pow_result;

template <class Q, short N, short D = 1>
using pow_result_t = typename pow_result<Q, N, D>::type;

template <class Q>
using inv_result_t = pow_result_t<Q, -1>;

template <class Q>
using sqr_root_result_t = pow_result_t<Q, 1, 2>;

template <class Q1, class Q2>
using div_result_t = typename div_result<Q1, Q2>::type;

template <short... L, short... R>
struct mul_result<quantity_t<L...>, quantity_t<R...>>
{
    static_assert(sizeof...(L) == sizeof...(R));
    using type = quantity_t<(L + R)...>;
};

template <short... L, short... R>
struct div_result<quantity_t<L...>, quantity_t<R...>>
{
    static_assert(sizeof...(L) == sizeof...(R));
    using type = quantity_t<(L - R)...>;
};

template <short... Dims, short N, short D>
struct pow_result<quantity_t<Dims...>, N, D>
{
    using type = quantity_t<(Dims * N / D)...>;
};

template <short... L, short... R>
constexpr auto operator*(quantity_t<L...>, quantity_t<R...>) -> mul_result_t<quantity_t<L...>, quantity_t<R...>>
{
    return {};
}

template <short... L, short... R>
constexpr auto operator/(quantity_t<L...>, quantity_t<R...>) -> div_result_t<quantity_t<L...>, quantity_t<R...>>
{
    return {};
}

template <short... D>
constexpr auto operator/(short, quantity_t<D...>) -> inv_result_t<quantity_t<D...>>
{
    return {};
}

template <short... D>
std::ostream& operator<<(std::ostream& os, quantity_t<D...>)
{
    static const std::vector<std::string_view> names = {
        "length",
        "mass",
        "time",
        "coords",
        "temperature",
        "angle"
        "electric_current",
        "luminous_intensity",
        "solid_angle",
        "amount",
    };
    const auto& values = quantity_t<D...>::values;
    for (std::size_t d = 0; d < values.size(); ++d)
    {
        const short v = values[d];
        const auto name = names[d];
        if (v != 0)
        {
            os << '{' << name;
            // if (v != 1)
            // {
            //     os << v;
            // }
            if (v > 0)
            {
                os << "+" << v;
            }
            else if (v < 0)
            {
                os << "-" << -v;
            }
            os << '}';
        }
    }
    return os;
}

template <class T, class Q, class = void>
struct value_t;

template <class T, class Q>
struct value_t<T, Q, std::enable_if_t<is_quality<Q>{}>>
{
    T m_value;

    explicit value_t(T value = {}) : m_value{ value }
    {
    }

    constexpr explicit operator bool() const
    {
        return static_cast<bool>(m_value);
    }

    constexpr T get() const
    {
        return m_value;
    }

    template <class U>
    constexpr value_t<U, Q> as() const
    {
        return value_t<U, Q>{ static_cast<U>(m_value) };
    }

    constexpr value_t& reset(value_t value)
    {
        m_value = value.get();
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const value_t& item)
    {
        return os << item.m_value << " " << Q{};
    }
};

template <class T, class Q>
value_t(T) -> value_t<T, Q>;

template <class T, class Q>
constexpr auto operator+(value_t<T, Q> item) -> value_t<T, Q>
{
    return item;
}

template <class T, class Q>
constexpr auto operator-(value_t<T, Q> item) -> value_t<T, Q>
{
    return value_t<T, Q>{ -item.get() };
}

template <class L, class R, class Q, class Res = std::invoke_result_t<std::plus<>, L, R>>
constexpr auto operator+(value_t<L, Q> lhs, value_t<R, Q> rhs) -> value_t<Res, Q>
{
    return value_t<Res, Q>{ lhs.get() + rhs.get() };
}

template <class L, class R, class Q, class Res = std::invoke_result_t<std::plus<>, L, R>>
constexpr auto operator+=(value_t<L, Q>& lhs, value_t<R, Q> rhs) -> value_t<L, Q>&
{
    return lhs.reset((lhs + rhs).template as<L>());
}

template <class L, class R, class Q, class Res = std::invoke_result_t<std::minus<>, L, R>>
constexpr auto operator-(value_t<L, Q> lhs, value_t<R, Q> rhs) -> value_t<Res, Q>
{
    return value_t<Res, Q>{ lhs.get() - rhs.get() };
}

template <class L, class R, class Q, class Res = std::invoke_result_t<std::minus<>, L, R>>
constexpr auto operator-=(value_t<L, Q>& lhs, value_t<R, Q> rhs) -> value_t<L, Q>&
{
    return lhs.reset((lhs - rhs).template as<L>());
}

template <class L, class R, class QL, class QR, class Res = std::invoke_result_t<std::multiplies<>, L, R>>
constexpr auto operator*(value_t<L, QL> lhs, value_t<R, QR> rhs) -> value_t<Res, mul_result_t<QL, QR>>
{
    return value_t<Res, mul_result_t<QL, QR>>{ lhs.get() * rhs.get() };
}

template <class L, class R, class QR, class Res = std::invoke_result_t<std::minus<>, L, R>>
constexpr auto operator*(L lhs, value_t<R, QR> rhs) -> value_t<Res, QR>
{
    return value_t<Res, QR>{ lhs * rhs.get() };
}

template <class L, class R, class QL, class Res = std::invoke_result_t<std::multiplies<>, L, R>>
constexpr auto operator*(value_t<L, QL> lhs, R rhs) -> value_t<Res, QL>
{
    return rhs * lhs;
}

template <class L, class R, class QL, class Res = std::invoke_result_t<std::multiplies<>, L, R>>
constexpr auto operator*=(value_t<L, QL>& lhs, R rhs) -> value_t<L, QL>&
{
    return lhs.reset((lhs * rhs).template as<L>());
}

template <class L, class R, class QL, class QR, class Res = std::invoke_result_t<std::divides<>, L, R>>
constexpr auto operator/(value_t<L, QL> lhs, value_t<R, QR> rhs) -> value_t<Res, div_result_t<QL, QR>>
{
    return value_t<Res, div_result_t<QL, QR>>{ lhs.get() / rhs.get() };
}

template <class L, class R, class Q, class Res = std::invoke_result_t<std::divides<>, L, R>>
constexpr auto operator/(value_t<L, Q> lhs, value_t<R, Q> rhs) -> Res
{
    return lhs.get() / rhs.get();
}

template <class L, class R, class QL, class Res = std::invoke_result_t<std::divides<>, L, R>>
constexpr auto operator/=(value_t<L, QL>& lhs, R rhs) -> value_t<L, QL>&
{
    return lhs.reset((lhs / rhs).template as<L>());
}

template <class L, class R, class QL, class Res = std::invoke_result_t<std::divides<>, L, R>>
constexpr auto operator/(value_t<L, QL> lhs, R rhs) -> value_t<Res, QL>
{
    return value_t<Res, QL>{ lhs.get() / rhs };
}

template <class L, class R, class QR, class Res = std::invoke_result_t<std::divides<>, L, R>>
constexpr auto operator/(L lhs, value_t<R, QR> rhs) -> value_t<Res, inv_result_t<QR>>
{
    return value_t<Res, inv_result_t<QR>>{ lhs / rhs.get() };
}

template <class L, class R, class Q>
constexpr auto operator==(value_t<L, Q> lhs, value_t<R, Q> rhs) -> bool
{
    return lhs.get() == rhs.get();
}

template <class L, class R, class Q>
constexpr auto operator!=(value_t<L, Q> lhs, value_t<R, Q> rhs) -> bool
{
    return lhs.get() != rhs.get();
}

template <class L, class R, class Q>
constexpr auto operator<(value_t<L, Q> lhs, value_t<R, Q> rhs) -> bool
{
    return lhs.get() < rhs.get();
}

template <class L, class R, class Q>
constexpr auto operator>(value_t<L, Q> lhs, value_t<R, Q> rhs) -> bool
{
    return lhs.get() > rhs.get();
}

template <class L, class R, class Q>
constexpr auto operator<=(value_t<L, Q> lhs, value_t<R, Q> rhs) -> bool
{
    return lhs.get() <= rhs.get();
}

template <class L, class R, class Q>
constexpr auto operator>=(value_t<L, Q> lhs, value_t<R, Q> rhs) -> bool
{
    return lhs.get() >= rhs.get();
}

namespace quantities
{

// clang-format off
using scalar                = quantity_t<0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
using length                = quantity_t<1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
using mass                  = quantity_t<0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
using time                  = quantity_t<0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
using coords                = quantity_t<0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0>;
using temperature           = quantity_t<0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0>;
using angle                 = quantity_t<0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0>;
using electric_current      = quantity_t<0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0>;
using luminous_intensity    = quantity_t<0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0>;
using solid_angle           = quantity_t<0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0>;
using amount                = quantity_t<0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0>;

using location              = length;

using area                  = pow_result_t<length, 2>;
using volume                = pow_result_t<length, 3>;

using velocity              = div_result_t<length, time>;
using acceleration          = div_result_t<velocity, time>;

using momentum              = mul_result_t<mass, velocity>;
using force                 = mul_result_t<mass, acceleration>;
using energy                = mul_result_t<force, length>;
using power                 = div_result_t<energy, time>;

using linear_density        = div_result_t<mass, length>;
using area_density          = div_result_t<mass, area>;
using density               = div_result_t<mass, volume>;
using pressure              = div_result_t<force, area>;
using frequency             = inv_result_t<time>;

using charge                = mul_result_t<electric_current, time>;
using voltage               = div_result_t<power, electric_current>;

using resistance            = mul_result_t<voltage, electric_current>;
using conductance           = inv_result_t<resistance>;

using capacitance           = div_result_t<charge, voltage>;

using magnetic_flux         = mul_result_t<voltage, time>;
using magnetic_flux_density = div_result_t<magnetic_flux, area>;
using inductance            = div_result_t<magnetic_flux, electric_current>;

using luminous_flux         = mul_result_t<luminous_intensity, solid_angle>;
using illuminance           = div_result_t<luminous_flux, area>;

using inertia               = mul_result_t<mass, area>;
using angular_velocity      = div_result_t<angle, time>;
using angular_acceleration  = div_result_t<angular_velocity, time>;

using angular_momentum      = mul_result_t<inertia, angular_velocity>;
using torque                = mul_result_t<inertia, angular_acceleration>;
using angular_energy        = mul_result_t<torque, angle>;

// clang-format on

}  // namespace quantities

#define DEFINE_QUANTITY(NAME)                                         \
                                                                      \
    template <class T>                                                \
    using NAME##_t = value_t<T, quantities::NAME>;                    \
    namespace literals                                                \
    {                                                                 \
    auto operator"" _##NAME(double long v) -> NAME##_t<double>        \
    {                                                                 \
        return NAME##_t<double>(double(v));                           \
    }                                                                 \
    auto operator"" _##NAME(unsigned long long v) -> NAME##_t<double> \
    {                                                                 \
        return NAME##_t<double>(double(v));                           \
    }                                                                 \
    }

DEFINE_QUANTITY(scalar)

DEFINE_QUANTITY(length)
DEFINE_QUANTITY(location)
DEFINE_QUANTITY(mass)
DEFINE_QUANTITY(time)
DEFINE_QUANTITY(coords)
DEFINE_QUANTITY(temperature)
DEFINE_QUANTITY(angle)
DEFINE_QUANTITY(electric_current)
DEFINE_QUANTITY(luminous_intensity)
DEFINE_QUANTITY(solid_angle)
DEFINE_QUANTITY(amount)

DEFINE_QUANTITY(area)
DEFINE_QUANTITY(volume)

DEFINE_QUANTITY(velocity)
DEFINE_QUANTITY(acceleration)

DEFINE_QUANTITY(momentum)
DEFINE_QUANTITY(force)
DEFINE_QUANTITY(energy)
DEFINE_QUANTITY(power)

DEFINE_QUANTITY(linear_density)
DEFINE_QUANTITY(area_density)
DEFINE_QUANTITY(density)
DEFINE_QUANTITY(pressure)
DEFINE_QUANTITY(frequency)

DEFINE_QUANTITY(charge)
DEFINE_QUANTITY(voltage)

DEFINE_QUANTITY(resistance)
DEFINE_QUANTITY(conductance)

DEFINE_QUANTITY(capacitance)

DEFINE_QUANTITY(magnetic_flux)
DEFINE_QUANTITY(magnetic_flux_density)
DEFINE_QUANTITY(inductance)

DEFINE_QUANTITY(luminous_flux)
DEFINE_QUANTITY(illuminance)

DEFINE_QUANTITY(inertia)
DEFINE_QUANTITY(angular_velocity)
DEFINE_QUANTITY(angular_acceleration)

DEFINE_QUANTITY(angular_momentum)
DEFINE_QUANTITY(torque)
DEFINE_QUANTITY(angular_energy)

#undef DEFINE_QUANTITY

}  // namespace quants
}  // namespace ferrugo
