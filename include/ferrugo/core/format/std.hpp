#pragma once

#include <ferrugo/core/format/format.hpp>
#include <optional>
#include <tuple>
#include <vector>

namespace ferrugo
{

namespace core
{

template <class Range>
struct range_formatter
{
    void parse(std::string_view)
    {
    }

    void format(std::ostream& os, const Range& item) const
    {
        const auto b = std::begin(item);
        const auto e = std::end(item);
        write_to(os, "[");
        for (auto it = b; it != e; ++it)
        {
            if (it != b)
            {
                write_to(os, ", ");
            }
            write_to(os, *it);
        }
        write_to(os, "]");
    }
};

template <class... Args>
struct formatter<std::vector<Args...>> : range_formatter<std::vector<Args...>>
{
};

template <class Tuple>
struct tuple_formatter
{
    void parse(std::string_view)
    {
    }

    void format(std::ostream& os, const Tuple& item) const
    {
        write_to(os, "(");
        std::apply(
            [&os](const auto&... args)
            {
                auto n = 0u;
                ((write_to(os, args) << (++n != sizeof...(args) ? ", " : "")), ...);
            },
            item);
        write_to(os, ")");
    }
};

template <class F, class S>
struct formatter<std::pair<F, S>> : tuple_formatter<std::pair<F, S>>
{
};

template <class... Args>
struct formatter<std::tuple<Args...>> : tuple_formatter<std::tuple<Args...>>
{
};

template <class T>
struct formatter<std::optional<T>>
{
    formatter<T> m_inner = {};

    void parse(std::string_view ctx)
    {
        m_inner.parse(ctx);
    }

    void format(std::ostream& os, const std::optional<T>& item) const
    {
        if (item)
        {
            write_to(os, "some(");
            m_inner.format(os, *item);
            write_to(os, ")");
        }
        else
        {
            write_to(os, "none");
        }
    }
};

template <class T>
struct formatter<std::reference_wrapper<T>>
{
    formatter<std::remove_const_t<T>> m_inner = {};

    void parse(std::string_view ctx)
    {
        m_inner.parse(ctx);
    }

    void format(std::ostream& os, const std::reference_wrapper<T>& item) const
    {
        m_inner.format(os, item.get());
    }
};

}  // namespace core
}  // namespace ferrugo