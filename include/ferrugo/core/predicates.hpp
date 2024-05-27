#pragma once

#include <ferrugo/core/type_traits.hpp>
#include <ferrugo/core/format_utils.hpp>
#include <functional>

namespace ferrugo
{
namespace predicates
{

template <char... Ch>
struct static_string
{
    static constexpr std::size_t size = sizeof...(Ch);
    static constexpr std::array<char, size> value = { Ch... };

    operator std::string_view() const
    {
        return std::string_view{ value.data(), value.size() };
    }

    friend std::ostream& operator<<(std::ostream& os, const static_string& item)
    {
        return os << static_cast<std::string_view>(item);
    }
};

template <class T, class U>
using is_equal_comparable = std::invoke_result_t<std::equal_to<>, T, U>;

template <class Pred, class T>
constexpr bool invoke_pred(Pred&& pred, T&& item)
{
    if constexpr (std::is_invocable_v<Pred, T>)
    {
        return std::invoke(std::forward<Pred>(pred), std::forward<T>(item));
    }
    else if constexpr (::ferrugo::core::is_detected<is_equal_comparable, T, Pred>{})
    {
        return pred == item;
    }
}

template <class T, class Impl = void>
struct predicate_t;

template <class T>
struct predicate_t<T, void>
{
    using predicate_type = std::function<bool(T)>;
    using value_type = std::decay_t<T>;

    using impl_type = std::variant<predicate_type, value_type>;

    impl_type m_impl;

    template <class Pred, core::require<std::is_constructible_v<predicate_type, Pred>> = 0>
    constexpr predicate_t(Pred&& pred) : m_impl{ std::in_place_type<predicate_type>, std::forward<Pred>(pred) }
    {
    }

    template <
        class U,
        core::require<!std::is_constructible_v<predicate_type, U> && std::is_constructible_v<value_type, U>> = 0>
    constexpr predicate_t(U&& value) : m_impl{ std::in_place_type<value_type>, std::forward<U>(value) }
    {
    }

    template <class U>
    constexpr bool operator()(U&& item) const
    {
        if (const auto p = std::get_if<value_type>(&m_impl))
        {
            return *p == std::forward<U>(item);
        }
        if (const auto p = std::get_if<predicate_type>(&m_impl))
        {
            return (*p)(std::forward<U>(item));
        }
        return false;
    }

    friend std::ostream& operator<<(std::ostream& os, const predicate_t& item)
    {
        if (const auto p = std::get_if<value_type>(&item.m_impl))
        {
            return os << *p;
        }
        else
        {
            return os << "???";
        }
    }
};

template <class Impl>
struct predicate_t<void, Impl>
{
    using predicate_type = Impl;

    predicate_type m_pred;

    constexpr predicate_t(predicate_type pred) : m_pred(std::move(pred))
    {
    }

    template <class U>
    constexpr bool operator()(U&& item) const
    {
        return invoke_pred(m_pred, std::forward<U>(item));
    }

    friend std::ostream& operator<<(std::ostream& os, const predicate_t& item)
    {
        if constexpr (core::is_detected<core::has_ostream_operator, Impl>{})
        {
            return os << item.m_pred;
        }
        else
        {
            return os << "??";
        }
    }
};

struct to_predicate_fn
{
    template <class Pred>
    constexpr auto operator()(Pred&& pred) const -> predicate_t<void, std::decay_t<Pred>>
    {
        return { std::forward<Pred>(pred) };
    }

    template <class... Args>
    constexpr auto operator()(predicate_t<Args...> pred) const -> predicate_t<Args...>
    {
        return pred;
    }
};

static constexpr inline auto to_predicate = to_predicate_fn{};

struct all_tag
{
};
struct any_tag
{
};

template <class Tag, class Name>
struct compound_fn
{
    template <class... Preds>
    struct impl
    {
        std::tuple<Preds...> m_preds;

        template <class U>
        constexpr bool operator()(U&& item) const
        {
            if constexpr (std::is_same_v<Tag, any_tag>)
            {
                return std::apply(
                    [&](const auto&... preds) { return (... || invoke_pred(preds, std::forward<U>(item))); }, m_preds);
            }
            else if constexpr (std::is_same_v<Tag, all_tag>)
            {
                return std::apply(
                    [&](const auto&... preds) { return (... && invoke_pred(preds, std::forward<U>(item))); }, m_preds);
            }
        }

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            static const auto name = Name{};

            os << "(" << name;
            std::apply([&](const auto&... preds) { ((os << " " << ::ferrugo::core::safe_format(preds)), ...); }, item.m_preds);
            os << ")";
            return os;
        }
    };

    template <class Pipe>
    auto to_tuple(Pipe pipe) const -> std::tuple<Pipe>
    {
        return std::tuple<Pipe>{ std::move(pipe) };
    }

    template <class... Pipes>
    auto to_tuple(impl<Pipes...> pipe) const -> std::tuple<Pipes...>
    {
        return pipe.m_preds;
    }

    template <class... Pipes>
    auto from_tuple(std::tuple<Pipes...> tuple) const -> impl<Pipes...>
    {
        return impl<Pipes...>{ std::move(tuple) };
    }

    template <class... Pipes>
    auto operator()(Pipes... pipes) const -> decltype(from_tuple(std::tuple_cat(to_tuple(std::move(pipes))...)))
    {
        return from_tuple(std::tuple_cat(to_tuple(std::move(pipes))...));
    }
};

static constexpr inline auto any = compound_fn<any_tag, static_string<'a', 'n', 'y'>>{};
static constexpr inline auto all = compound_fn<all_tag, static_string<'a', 'l', 'l'>>{};

struct negate_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class U>
        bool operator()(U&& item) const
        {
            return !invoke_pred(pred, std::forward<U>(item));
        }

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            return os << "(not " << item.pred << ")";
        }
    };

    template <class Pred>
    auto operator()(Pred&& pred) const -> impl<std::decay_t<Pred>>
    {
        return impl<std::decay_t<Pred>>{ std::forward<Pred>(pred) };
    }
};

static constexpr inline auto negate = negate_fn{};

template <class Op, class Name>
struct compare_fn
{
    template <class T>
    struct impl
    {
        T value;

        template <class U>
        bool operator()(U&& item) const
        {
            static const auto op = Op{};
            return op(std::forward<U>(item), value);
        }

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            static const auto name = Name{};
            return os << "(" << name << " " << item.value << ")";
        }
    };

    template <class T>
    auto operator()(T&& value) const -> impl<std::decay_t<T>>
    {
        return impl<std::decay_t<T>>{ std::forward<T>(value) };
    }
};

static constexpr inline auto eq = compare_fn<std::equal_to<>, static_string<'e', 'q'>>{};
static constexpr inline auto ne = compare_fn<std::not_equal_to<>, static_string<'n', 'e'>>{};
static constexpr inline auto lt = compare_fn<std::less<>, static_string<'l', 't'>>{};
static constexpr inline auto gt = compare_fn<std::greater<>, static_string<'g', 't'>>{};
static constexpr inline auto le = compare_fn<std::less_equal<>, static_string<'l', 'e'>>{};
static constexpr inline auto ge = compare_fn<std::greater_equal<>, static_string<'g', 'e'>>{};

struct size_is_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class U>
        bool operator()(U&& item) const
        {
            return invoke_pred(pred, std::distance(std::begin(item), std::end(item)));
        }

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            return os << "(size_is " << item.pred << ")";
        }
    };

    template <class Pred>
    auto operator()(Pred&& pred) const -> impl<std::decay_t<Pred>>
    {
        return impl<std::decay_t<Pred>>{ std::forward<Pred>(pred) };
    }
};

struct is_empty_fn
{
    struct impl
    {
        template <class U>
        bool operator()(U&& item) const
        {
            return std::begin(item) == std::end(item);
        }

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            return os << "(is_empty)";
        }
    };

    auto operator()() const -> impl
    {
        return impl{};
    }
};

static constexpr inline auto size_is = size_is_fn{};
static constexpr inline auto is_empty = is_empty_fn{};

struct each_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class U>
        bool operator()(U&& item) const
        {
            return std::all_of(
                std::begin(item), std::end(item), [&](auto&& v) { return invoke_pred(pred, std::forward<decltype(v)>(v)); });
        }

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            return os << "(each " << item.pred << ")";
        }
    };

    template <class Pred>
    auto operator()(Pred&& pred) const -> impl<std::decay_t<Pred>>
    {
        return impl<std::decay_t<Pred>>{ std::forward<Pred>(pred) };
    }
};

struct contains_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class U>
        bool operator()(U&& item) const
        {
            return std::any_of(
                std::begin(item), std::end(item), [&](auto&& v) { return invoke_pred(pred, std::forward<decltype(v)>(v)); });
        }

        friend std::ostream& operator<<(std::ostream& os, const impl& item)
        {
            return os << "(contains " << item.pred << ")";
        }
    };

    template <class Pred>
    auto operator()(Pred&& pred) const -> impl<std::decay_t<Pred>>
    {
        return impl<std::decay_t<Pred>>{ std::forward<Pred>(pred) };
    }
};

static constexpr inline auto each = each_fn{};
static constexpr inline auto contains = contains_fn{};

}  // namespace predicates
}  // namespace ferrugo
