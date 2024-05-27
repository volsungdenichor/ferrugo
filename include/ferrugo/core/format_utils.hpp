#pragma once

#include <ferrugo/core/type_name.hpp>
#include <ferrugo/core/type_traits.hpp>
#include <sstream>

namespace ferrugo
{
namespace core
{

struct str_fn
{
    template <class... Args>
    std::string operator()(Args&&... args) const
    {
        std::stringstream ss;
        ((ss << std::forward<Args>(args)), ...);
        return ss.str();
    }
};

static constexpr inline auto str = str_fn{};

struct safe_format_fn
{
    template <class T>
    struct impl
    {
        const T& m_value;

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            if constexpr (is_detected<has_ostream_operator, T>{})
            {
                return os << item.m_value;
            }
            else
            {
                return os << type_name<T>();
            }
        }
    };

    template <class T>
    auto operator()(const T& item) const -> impl<T>
    {
        return impl<T>{ item };
    }
};

static constexpr inline auto safe_format = safe_format_fn{};

struct delimit_fn
{
    template <class Iter>
    struct impl
    {
        Iter begin;
        Iter end;
        std::string_view delimiter;

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            for (auto it = item.begin; it != item.end; ++it)
            {
                if (it != item.begin)
                {
                    os << item.delimiter;
                }
                os << *it;
            }
            return os;
        }
    };
    template <class Range>
    auto operator()(Range&& range, std::string_view delimiter) const
    {
        auto b = std::begin(range);
        auto e = std::end(range);
        return impl<decltype(b)>{ b, e, delimiter };
    }
};

static constexpr auto delimit = delimit_fn{};

}  // namespace core
}  // namespace ferrugo