#pragma once

#include <algorithm>
#include <cassert>
#include <ferrugo/core/overloaded.hpp>
#include <ferrugo/core/type_traits.hpp>
#include <functional>
#include <iostream>
#include <sstream>
#include <string_view>
#include <variant>
#include <vector>

namespace ferrugo
{
namespace core
{

template <class T, class = void>
struct formatter;

template <class... Args>
std::ostream& write_to(std::ostream& os, Args&&... args)
{
    (formatter<std::decay_t<Args>>{}(os, std::forward<Args>(args)), ...);
    return os;
}

struct format_error : std::runtime_error
{
    format_error(std::string message) : std::runtime_error{ std::move(message) }
    {
    }
};

namespace detail
{

struct arg_ref
{
    using arg_printer = void (*)(std::ostream&, const void*);
    arg_printer m_printer;
    const void* m_ptr;

    template <class T>
    explicit arg_ref(const T& item)
        : m_printer{ [](std::ostream& os, const void* ptr) { write_to(os, *static_cast<const T*>(ptr)); } }
        , m_ptr{ std::addressof(item) }
    {
    }

    arg_ref(const arg_ref&) = default;
    arg_ref(arg_ref&&) = default;

    friend std::ostream& operator<<(std::ostream& os, const arg_ref& item)
    {
        item.m_printer(os, item.m_ptr);
        return os;
    }
};

template <class... Args>
auto wrap_args(const Args&... args) -> std::vector<arg_ref>
{
    std::vector<arg_ref> result;
    result.reserve(sizeof...(args));
    (result.push_back(arg_ref{ args }), ...);
    return result;
}

class format_string
{
private:
    struct print_text
    {
        std::string_view text;
    };

    struct print_argument
    {
        int index;
    };

    using print_action = std::variant<print_text, print_argument>;

public:
    explicit format_string(std::string_view fmt) : m_actions{ parse(fmt) }
    {
    }

    void format(std::ostream& os, const std::vector<arg_ref>& arguments) const
    {
        for (const auto& action : m_actions)
        {
            std::visit(
                ferrugo::core::overloaded{ [&](const print_text& a) { os << a.text; },
                                           [&](const print_argument& a) { os << arguments.at(a.index); } },
                action);
        }
    }

    auto format(const std::vector<arg_ref>& arguments) const -> std::string
    {
        std::stringstream ss;
        format(ss, arguments);
        return std::move(ss).str();
    }

private:
    std::vector<print_action> m_actions;

    static auto parse(std::string_view fmt) -> std::vector<print_action>
    {
        static const auto is_opening_bracket = [](char c) { return c == '{'; };
        static const auto is_closing_bracket = [](char c) { return c == '}'; };
        static const auto is_bracket = [](char c) { return is_opening_bracket(c) || is_closing_bracket(c); };
        static const auto is_colon = [](char c) { return c == ':'; };
        std::vector<print_action> result;
        int arg_index = 0;
        while (!fmt.empty())
        {
            const auto begin = std::begin(fmt);
            const auto end = std::end(fmt);
            const auto bracket = std::find_if(begin, end, is_bracket);
            if (bracket == end)
            {
                result.push_back(print_text{ fmt });
                fmt = make_string_view(bracket, end);
            }
            else if (bracket[0] == bracket[1])
            {
                result.push_back(print_text{ make_string_view(begin, bracket + 1) });
                fmt = make_string_view(bracket + 2, end);
            }
            else if (is_opening_bracket(bracket[0]))
            {
                const auto closing_bracket = std::find_if(bracket + 1, end, is_closing_bracket);
                if (closing_bracket == end)
                {
                    throw format_error{ "unclosed bracket" };
                }
                result.push_back(print_text{ make_string_view(begin, bracket) });

                const auto actual_index = std::invoke(
                    [](std::string_view arg, int current_index) -> int
                    {
                        const auto colon = std::find_if(std::begin(arg), std::end(arg), is_colon);
                        const auto index_part = make_string_view(std::begin(arg), colon);
                        const auto fmt_part = make_string_view(colon != std::end(arg) ? colon + 1 : colon, std::end(arg));
                        return !index_part.empty() ? parse_int(index_part) : current_index;
                    },
                    make_string_view(bracket + 1, closing_bracket),
                    arg_index);
                result.push_back(print_argument{ actual_index });
                fmt = make_string_view(closing_bracket + 1, end);
                ++arg_index;
            }
        }
        return result;
    }

    static auto make_string_view(std::string_view::iterator b, std::string_view::iterator e) -> std::string_view
    {
        if (b < e)
            return { std::addressof(*b), std::string_view::size_type(e - b) };
        else
            return {};
    }

    static auto parse_int(std::string_view txt) -> int
    {
        int result = 0;
        for (char c : txt)
        {
            assert('0' <= c && c <= '9');
            result = result * 10 + (c - '0');
        }
        return result;
    }
};

template <bool NewLine = false>
struct print_to_fn
{
    std::ostream& m_os;
    struct impl
    {
        std::ostream& m_os;
        format_string m_formatter;

        template <class... Args>
        void operator()(Args&&... args) const
        {
            m_formatter.format(m_os, wrap_args(std::forward<Args>(args)...));
            if constexpr (NewLine)
            {
                m_os << '\n';
            }
        }
    };

    auto operator()(std::string_view fmt) const -> impl
    {
        return impl{ m_os, format_string{ fmt } };
    }
};

struct format_fn
{
    struct impl
    {
        format_string m_formatter;

        template <class... Args>
        auto operator()(Args&&... args) const -> std::string
        {
            return m_formatter.format(wrap_args(std::forward<Args>(args)...));
        }
    };

    auto operator()(std::string_view fmt) const -> impl
    {
        return impl{ format_string{ fmt } };
    }
};

}  // namespace detail

template <class T>
struct formatter<T, std::enable_if_t<has_ostream_operator<T>{}>>
{
    void operator()(std::ostream& os, const T& item) const
    {
        os << item;
    }
};

template <>
struct formatter<bool>
{
    void operator()(std::ostream& os, bool item) const
    {
        os << std::boolalpha << item;
    }
};

static constexpr inline auto print = detail::print_to_fn<>{ std::cout };
static constexpr inline auto println = detail::print_to_fn<true>{ std::cout };

static constexpr inline auto format = detail::format_fn{};

}  // namespace core

}  // namespace ferrugo
