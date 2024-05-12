#pragma once

#include <ferrugo/core/format/format.hpp>
#include <map>

namespace ferrugo
{
namespace core
{

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

    void parse(const parse_context&)
    {
    }

    void format(format_context& ctx, const T& item) const
    {
        write_to(ctx, m_values.at(item));
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

}  // namespace core
}  // namespace ferrugo