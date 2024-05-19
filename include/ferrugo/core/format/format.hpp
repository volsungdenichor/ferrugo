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

class parse_context
{
public:
    explicit parse_context(std::string_view specifier) : m_specifier{ specifier }
    {
    }

    std::string_view specifier() const
    {
        return m_specifier;
    }

private:
    std::string_view m_specifier;
};

class format_context
{
public:
    constexpr explicit format_context(std::ostream& os) : m_os{ os }
    {
    }

    std::ostream& stream() const
    {
        return m_os;
    }

private:
    std::ostream& m_os;
};

template <class... Args>
format_context& write_to(format_context& ctx, Args&&... args)
{
    (formatter<std::decay_t<Args>>{}.format(ctx, std::forward<Args>(args)), ...);
    return ctx;
}

struct format_error : std::runtime_error
{
    explicit format_error(std::string message) : std::runtime_error{ std::move(message) }
    {
    }
};

namespace detail
{

struct arg_ref
{
    using arg_printer = void (*)(format_context&, const void*, const parse_context&);
    arg_printer m_printer;
    const void* m_ptr;

    template <class T>
    explicit arg_ref(const T& item)
        : m_printer{ [](format_context& format_ctx, const void* ptr, const parse_context& parse_ctx)
                     {
                         formatter<T> f{};
                         f.parse(parse_ctx);
                         f.format(format_ctx, *static_cast<const T*>(ptr));
                     } }
        , m_ptr{ std::addressof(item) }
    {
    }

    arg_ref(const arg_ref&) = default;
    arg_ref(arg_ref&&) = default;

    void print(format_context& format_ctx, const parse_context& parse_ctx) const
    {
        m_printer(format_ctx, m_ptr, parse_ctx);
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
        parse_context context;
        // std::string_view specifier;
    };

    using print_action = std::variant<print_text, print_argument>;

public:
    explicit format_string(std::string_view fmt) : m_actions{ parse(fmt) }
    {
    }

    void format(format_context& format_ctx, const std::vector<arg_ref>& arguments) const
    {
        for (const auto& action : m_actions)
        {
            std::visit(
                ferrugo::core::overloaded{ [&](const print_text& a) { format_ctx.stream() << a.text; },
                                           [&](const print_argument& a)
                                           { arguments.at(a.index).print(format_ctx, a.context); } },
                action);
        }
    }

    auto format(const std::vector<arg_ref>& arguments) const -> std::string
    {
        std::stringstream ss;
        format_context format_ctx{ ss };
        format(format_ctx, arguments);
        return std::move(ss).str();
    }

    friend std::ostream& operator<<(std::ostream& os, const format_string& item)
    {
        for (const auto& action : item.m_actions)
        {
            std::visit(
                ferrugo::core::overloaded{ [&](const print_text& a) { os << a.text; },
                                           [&](const print_argument& a)
                                           {
                                               if (a.context.specifier().empty())
                                               {
                                                   os << "{" << a.index << "}";
                                               }
                                               else
                                               {
                                                   os << "{" << a.index << ":" << a.context.specifier() << "}";
                                               }
                                           } },
                action);
        }
        return os;
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

                const auto [actual_index, fmt_specifer] = std::invoke(
                    [](std::string_view arg, int current_index) -> std::tuple<int, std::string_view>
                    {
                        const auto colon = std::find_if(std::begin(arg), std::end(arg), is_colon);
                        const auto index_part = make_string_view(std::begin(arg), colon);
                        const auto fmt_part = make_string_view(colon != std::end(arg) ? colon + 1 : colon, std::end(arg));
                        const auto index = !index_part.empty() ? parse_int(index_part) : current_index;
                        return { index, fmt_part };
                    },
                    make_string_view(bracket + 1, closing_bracket),
                    arg_index);
                result.push_back(print_argument{ actual_index, parse_context{ fmt_specifer } });
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
            format_context format_ctx{ m_os };
            m_formatter.format(format_ctx, wrap_args(std::forward<Args>(args)...));
            if constexpr (NewLine)
            {
                write_to(format_ctx, '\n');
            }
        }

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            return os << item.m_formatter;
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

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            return os << item.m_formatter;
        }
    };

    auto operator()(std::string_view fmt) const -> impl
    {
        return impl{ format_string{ fmt } };
    }
};

}  // namespace detail

template <class T>
struct formatter<T, std::enable_if_t<typename type<T>::satisfies<has_ostream_operator>{}>>
{
    void parse(const parse_context&)
    {
    }

    void format(format_context& ctx, const T& item) const
    {
        ctx.stream() << item;
    }
};

template <>
struct formatter<bool>
{
    void parse(const parse_context&)
    {
    }

    void format(format_context& ctx, bool item) const
    {
        ctx.stream() << std::boolalpha << item;
    }
};

static constexpr inline auto print = detail::print_to_fn<>{ std::cout };
static constexpr inline auto println = detail::print_to_fn<true>{ std::cout };

static constexpr inline auto format = detail::format_fn{};

}  // namespace core

}  // namespace ferrugo
