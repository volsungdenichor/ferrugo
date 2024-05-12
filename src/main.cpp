#include <cassert>
#include <ferrugo/core/format.hpp>
#include <ferrugo/core/optional.hpp>
#include <ferrugo/core/overloaded.hpp>
#include <ferrugo/core/pipeline.hpp>
#include <ferrugo/core/ranges.hpp>
#include <ferrugo/core/type_name.hpp>
#include <ferrugo/core/type_traits.hpp>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>

enum class sex_t
{
    male,
    female
};

struct person
{
    std::string name;
    int age;
    sex_t sex;
};

template <class T>
struct enum_formatter
{
    std::map<T, std::string_view> m_values;

    template <class... Tail>
    explicit enum_formatter(T value, std::string_view name, const Tail&... tail)
    {
        static_assert(sizeof...(tail) % 2 == 0, "Sequence of (value, name) required");
        init(m_values, value, name, tail...);
    }

    void parse(std::string_view)
    {
    }

    void format(std::ostream& os, const T& item) const
    {
        ferrugo::core::write_to(os, m_values.at(item));
    }

    template <class... Tail>
    static void init(std::map<T, std::string_view>& result, T value, std::string_view name, const Tail&... tail)
    {
        result.emplace(value, name);
        if constexpr (sizeof...(tail) > 0)
        {
            init(result, tail...);
        }
    }
};

template <class T>
struct struct_formatter
{
    using field_info = std::tuple<std::function<void(std::ostream&, const T&)>, std::string_view>;
    std::vector<field_info> m_values;

    template <class Type, class... Tail>
    explicit struct_formatter(Type T::*field, std::string_view name, const Tail&... tail)
    {
        static_assert(sizeof...(tail) % 2 == 0, "Sequence of (field, name) required");
        init(m_values, field, name, tail...);
    }

    void parse(std::string_view)
    {
    }

    void format(std::ostream& os, const T& item) const
    {
        os << "{";
        for (std::size_t i = 0; i < m_values.size(); ++i)
        {
            if (i > 0)
            {
                os << "; ";
            }
            const auto& [func, name] = m_values[i];
            os << name << "=";
            std::invoke(func, os, item);
        }
        os << "}";
    }

    template <class Type, class... Tail>
    static void init(std::vector<field_info>& result, Type T::*field, std::string_view name, const Tail&... tail)
    {
        auto f
            = [=](std::ostream& os, const T& item) { ferrugo::core::write_to(os, std::invoke(std::mem_fn(field), item)); };
        result.emplace_back(f, name);
        if constexpr (sizeof...(tail) > 0)
        {
            init(result, tail...);
        }
    }
};

template <>
struct ferrugo::core::formatter<sex_t> : enum_formatter<sex_t>
{
    formatter() : enum_formatter{ sex_t::male, "male", sex_t::female, "female" }
    {
    }
};

template <>
struct ferrugo::core::formatter<person> : struct_formatter<person>
{
    formatter() : struct_formatter{ &person::name, "name", &person::age, "age", &person::sex, "sex" }
    {
    }
};

void run()
{
    const std::vector<std::string> v{ "Ala", "Beata", "Celina", "Dezyderiusz" };
    const auto p = ferrugo::core::println("{0:Bonjour}");
    std::cout << p << std::endl;
    p(v);
    p(std::pair{ 1, 'X' });
    p(std::tuple{ 'X', 3.14, 10, true, std::cref(v) });
    p(person{ "Adam", 57, sex_t::male });
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
