#include <ferrugo/core/format.hpp>
#include <ferrugo/core/optional.hpp>
#include <ferrugo/core/pipeline.hpp>
#include <ferrugo/core/ranges.hpp>
#include <ferrugo/core/type_name.hpp>
#include <ferrugo/core/type_traits.hpp>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

template <class T>
struct ferrugo::core::formatter<std::vector<T>>
{
    void operator()(std::ostream& os, const std::vector<T>& item) const
    {
        const auto b = std::begin(item);
        const auto e = std::end(item);
        format_to(os, "[");
        for (auto it = b; it != e; ++it)
        {
            if (it != b)
            {
                format_to(os, ", ");
            }
            format_to(os, *it);
        }
        format_to(os, "]");
    }
};

template <class F, class S>
struct ferrugo::core::formatter<std::pair<F, S>>
{
    void operator()(std::ostream& os, const std::pair<F, S>& item) const
    {
        format_to(os, "(", item.first, ", ", item.second, ")");
    }
};

template <class... Args>
struct ferrugo::core::formatter<std::tuple<Args...>>
{
    void operator()(std::ostream& os, const std::tuple<Args...>& item) const
    {
        format_to(os, "(");
        std::apply(
            [&os](const auto&... args)
            {
                auto n = 0u;
                ((format_to(os, args) << (++n != sizeof...(args) ? ", " : "")), ...);
            },
            item);
        format_to(os, ")");
    }
};

template <class T>
struct ferrugo::core::formatter<std::optional<T>>
{
    void operator()(std::ostream& os, const std::optional<T>& item) const
    {
        if (item)
        {
            format_to(os, "some(", *item, ")");
        }
        else
        {
            format_to(os, "none");
        }
    }
};

template <class T>
struct ferrugo::core::formatter<std::reference_wrapper<T>>
{
    void operator()(std::ostream& os, const std::reference_wrapper<T>& item) const
    {
        format_to(os, item.get());
    }
};

void handle(ferrugo::core::random_access_iterable<int> rng)
{
    for (auto&& x : rng)
    {
        ferrugo::core::print(x, '\n');
    }
    ferrugo::core::print("size=", rng.size(), '\n');
}

void run()
{
    using namespace ferrugo::core;
    const auto p = pipe(
        [=](int x) { return 10 * x; },  //
        [=](int x) { return x + 1; },
        [=](int x) { return x / 10.0; },
        [=](double x) { return std::to_string(x); },
        [=](const std::string& x) { return format('_', x, '_'); },
        [=](const std::string& x) { cout(x, '\n'); });

    int v[] = { 2, 3, 5, 7, 11, 13, 17, -1 };

    handle(ref_range(v));
}

void print_error()
{
    using namespace ferrugo;
    try
    {
        std::rethrow_exception(std::current_exception());
    }
    catch (const std::exception& ex)
    {
        core::cerr("\nError: ", ex.what(), "\n");
    }
    catch (...)
    {
        core::cerr("\nError: ", "UNKNOWN ERROR", "\n");
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
