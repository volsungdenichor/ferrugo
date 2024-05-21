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

void run()
{
    using namespace ferrugo;

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
