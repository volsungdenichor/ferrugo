#include <cassert>
#include <ferrugo/core/format.hpp>
#include <ferrugo/core/format/enum_formatter.hpp>
#include <ferrugo/core/format/struct_formatter.hpp>
#include <ferrugo/core/optional.hpp>
#include <ferrugo/core/overloaded.hpp>
#include <ferrugo/core/pipeline.hpp>
#include <ferrugo/core/ranges.hpp>
#include <ferrugo/core/type_name.hpp>
#include <ferrugo/core/type_traits.hpp>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>

template <template <class...> class Op, class... Args>
using is_detected = ferrugo::core::is_detected<Op, Args...>;
template <class T>
struct type
{
    template <template <class...> class... Ops>
    using satisfies_all = std::conjunction<is_detected<Ops, T>...>;

    template <template <class...> class... Ops>
    using satisfies_any = std::disjunction<is_detected<Ops, T>...>;

    template <template <class...> class Op>
    using satisfies = satisfies_all<Op>;
};

template <class... Types>
struct type_pack
{
    template <template <class...> class Op>
    using each_satisfies = std::conjunction<is_detected<Op, Types>...>;

    template <template <class...> class Op>
    using any_satisfies = std::disjunction<is_detected<Op, Types>...>;
};

template <class T>
struct type_pack_for_impl;

template <class... Types>
struct type_pack_for_impl<std::tuple<Types...>>
{
    using type = type_pack<Types...>;
};

template <class... Types>
struct type_pack_for_impl<std::pair<Types...>>
{
    using type = type_pack<Types...>;
};

template <class T>
using type_pack_for = typename type_pack_for_impl<T>::type;

template <class T>
using is_arithmetic = std::enable_if_t<std::is_arithmetic_v<T>>;

template <class T>
using is_unsigned = std::enable_if_t<std::is_unsigned_v<T>>;

void run()
{
    using namespace ferrugo;
    std::cout << type_pack<int, double>::each_satisfies<is_arithmetic>{} << std::endl;
    std::cout << type_pack<int, double>::each_satisfies<is_unsigned>{} << std::endl;
    return;

    std::vector<int> a = { 1, 2, 3, 4 };
    const std::vector<double> b = { 1.5, 2.5, 3.5, 4.5 };
    const std::array<std::string, 2> c = { "AAA", "BBB" };
    const auto r = core::all(core::ref_range(a), core::ref_range(b), core::ref_range(c));
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
    static const auto format_error = ferrugo::core::format("Error: {}\n");
    using namespace ferrugo;
    try
    {
        std::rethrow_exception(std::current_exception());
    }
    catch (const std::exception& ex)
    {
        std::cerr << format_error(ex.what());
    }
    catch (...)
    {
        std::cerr << format_error("UNKNOWN ERROR");
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
