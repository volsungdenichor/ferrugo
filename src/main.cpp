#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cuchar>
#include <ferrugo/core/ansi.hpp>
#include <ferrugo/core/arrays/array.hpp>
#include <ferrugo/core/format.hpp>
#include <ferrugo/core/optional.hpp>
#include <ferrugo/core/overloaded.hpp>
#include <ferrugo/core/pipeline.hpp>
#include <ferrugo/core/predicates.hpp>
#include <ferrugo/core/ranges.hpp>
#include <ferrugo/core/type_name.hpp>
#include <ferrugo/core/type_traits.hpp>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>

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
        "L",
        "M",
        "T",
    };
    const auto& values = quantity_t<D...>::values;
    for (std::size_t d = 0; d < values.size(); ++d)
    {
        const short v = values[d];
        const auto name = names[d];
        if (v != 0)
        {
            os << name;
            if (v != 1)
            {
                os << v;
            }
        }
    }
    return os;
}

template <class T, class Q, class = void>
struct point_t;

template <class T, class Q, class = void>
struct delta_t;

template <class T, class Q>
struct point_t<T, Q, std::enable_if_t<is_quality<Q>{}>>
{
    T m_value;

    explicit point_t(T value = {}) : m_value{ value }
    {
    }

    constexpr T get() const
    {
        return m_value;
    }

    friend std::ostream& operator<<(std::ostream& os, const point_t& item)
    {
        return os << item.m_value << " " << Q{};
    }
};

template <class T, class Q>
struct delta_t<T, Q, std::enable_if_t<is_quality<Q>{}>>
{
    T m_value;

    explicit delta_t(T value = {}) : m_value{ value }
    {
    }

    constexpr T get() const
    {
        return m_value;
    }

    friend std::ostream operator<<(std::ostream& os, const delta_t& item)
    {
        return os << item.m_value << " " << Q{};
    }
};

template <class T, class Q>
constexpr auto operator+(point_t<T, Q> item) -> point_t<T, Q>
{
    return item;
}

template <class T, class Q>
constexpr auto operator-(point_t<T, Q> item) -> point_t<T, Q>
{
    return point_t<T, Q>{ -item.get() };
}

template <class L, class R, class Q, class Res = std::invoke_result_t<std::plus<>, L, R>>
constexpr auto operator+(point_t<L, Q> lhs, delta_t<R, Q> rhs) -> point_t<Res, Q>
{
    return point_t<Res, Q>{ lhs.get() + rhs.get() };
}

template <class L, class R, class Q, class Res = std::invoke_result_t<std::plus<>, L, R>>
constexpr auto operator+(delta_t<L, Q> lhs, point_t<R, Q> rhs) -> point_t<Res, Q>
{
    return point_t<Res, Q>{ lhs.get() + rhs.get() };
}

template <class L, class R, class Q, class Res = std::invoke_result_t<std::minus<>, L, R>>
constexpr auto operator-(point_t<L, Q> lhs, point_t<R, Q> rhs) -> delta_t<Res, Q>
{
    return delta_t<Res, Q>{ lhs.get() - rhs.get() };
}

template <class L, class R, class QL, class QR, class Res = std::invoke_result_t<std::minus<>, L, R>>
constexpr auto operator*(point_t<L, QL> lhs, point_t<R, QR> rhs) -> point_t<Res, mul_result_t<QL, QR>>
{
    return point_t<Res, mul_result_t<QL, QR>>{ lhs.get() * rhs.get() };
}

template <class L, class R, class QL, class QR, class Res = std::invoke_result_t<std::divides<>, L, R>>
constexpr auto operator/(point_t<L, QL> lhs, point_t<R, QR> rhs) -> point_t<Res, div_result_t<QL, QR>>
{
    return point_t<Res, div_result_t<QL, QR>>{ lhs.get() / rhs.get() };
}

namespace quantities
{

using scalar = quantity_t<0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
using length = quantity_t<1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
using mass = quantity_t<0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0>;
using time = quantity_t<0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0>;

using area = pow_result_t<length, 2>;
using volume = pow_result_t<length, 3>;

using velocity = div_result_t<length, time>;
using acceleration = div_result_t<velocity, time>;

}  // namespace quantities

void run()
{
    using namespace std::string_literals;
    using namespace std::string_view_literals;

    using namespace ferrugo;

    std::cout << (point_t<double, quantities::length>{ 2.0 }) << std::endl;

    std::vector<int> a = { 1, 2, 3, 4 };
    const std::vector<double> b = { 1.5, 2.5, 3.5, 4.5 };
    const std::array<std::string, 2> c = { "AAA", "BBB" };
    const auto r = core::all(core::ref_range(a), core::ref_range(b), core::ref_range(c), core::owning_range(c));
    auto it = std::begin(r) + 2;
    --it;
    const auto print = core::println("{}");
    print(*it);
    using range_type = decltype(r);
    print(core::type_name<core::range_category_t<range_type>>());
    print(core::type_name<core::range_reference_t<range_type>>());
    for (const auto& item : r)
    {
        print(item);
    }
}

void print_error()
{
    static const auto print_error = ferrugo::core::println(std::cerr, "Error: {}\n");
    using namespace ferrugo;
    try
    {
        std::rethrow_exception(std::current_exception());
    }
    catch (const std::exception& ex)
    {
        print_error(ex.what());
    }
    catch (...)

    {
        print_error("UNKNOWN ERROR");
    }
}

int main(int argc, char const* argv[])
{
    try
    {
        run();
    }
    catch (...)
    {
        print_error();
    }
}
