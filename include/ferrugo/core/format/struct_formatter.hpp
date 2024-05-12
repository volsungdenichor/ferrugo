#pragma once

#include <ferrugo/core/format/format.hpp>

namespace ferrugo
{
namespace core
{

template <class T>
struct struct_formatter
{
    using field_info = std::tuple<std::function<void(format_context&, const T&)>, std::string_view>;
    std::vector<field_info> m_values;

    template <class Type, class... Tail>
    explicit struct_formatter(Type T::*field, std::string_view name, const Tail&... tail)
    {
        static_assert(sizeof...(tail) % 2 == 0, "Sequence of (field, name) required");
        init(m_values, field, name, tail...);
    }

    void parse(const parse_context&)
    {
    }

    void format(format_context& ctx, const T& item) const
    {
        write_to(ctx, "{");
        for (std::size_t i = 0; i < m_values.size(); ++i)
        {
            if (i > 0)
            {
                write_to(ctx, "; ");
            }
            const auto& [func, name] = m_values[i];
            write_to(ctx, name, "=");
            std::invoke(func, ctx, item);
        }
        write_to(ctx, "}");
    }

    template <class Type, class... Tail>
    static void init(std::vector<field_info>& result, Type T::*field, std::string_view name, const Tail&... tail)
    {
        auto f = [=](format_context& ctx, const T& item) { write_to(ctx, std::invoke(std::mem_fn(field), item)); };
        result.emplace_back(std::move(f), name);
        if constexpr (sizeof...(tail) > 0)
        {
            init(result, tail...);
        }
    }
};

}  // namespace core
}  // namespace ferrugo