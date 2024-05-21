#pragma once

#include <ferrugo/core/iterator_interface.hpp>
#include <ferrugo/core/range_interface.hpp>
#include <ferrugo/core/ranges/owning_range.hpp>
#include <ferrugo/core/ranges/range_ref.hpp>
#include <ferrugo/core/type_traits.hpp>

namespace ferrugo
{
namespace core
{

namespace detail
{

template <class... Ranges>
struct ranges_pack
{
    using ranges_type = std::tuple<Ranges...>;
    ranges_type m_ranges;
    using index_seq = std::index_sequence_for<Ranges...>;

    ranges_pack(std::tuple<Ranges...> ranges) : m_ranges{ std::move(ranges) }
    {
    }

    struct iter
    {
        using iterators_type = std::tuple<iterator_t<Ranges>...>;

        iterators_type m_iters;

        iter() = default;

        iter(iterators_type iters) : m_iters{ iters }
        {
        }

    private:
        template <class Iters, std::size_t... I>
        static auto deref(const Iters& iters, std::index_sequence<I...>)
            -> std::tuple<iter_reference_t<std::tuple_element_t<I, Iters>>...>
        {
            return { *std::get<I>(iters)... };
        }

    public:
        auto deref() const -> decltype(deref(m_iters, index_seq{}))
        {
            return deref(m_iters, index_seq{});
        }

        template <class It = iterators_type, require<each_satisfies<It, pre_increment>{}> = 0>
        void inc()
        {
            static const auto impl = []<std::size_t... I>(auto& it, std::index_sequence<I...>)
            {
                (++std::get<I>(it), ...);
            };
            return impl(m_iters, index_seq{});
        }

        template <class It = iterators_type, require<each_satisfies<It, pre_decrement>{}> = 0>
        void dec()
        {
            static const auto impl = []<std::size_t... I>(auto& it, std::index_sequence<I...>)
            {
                (--std::get<I>(it), ...);
            };
            return impl(m_iters, index_seq{});
        }

        template <class It = iterators_type, require<each_satisfies<It, equal_to>{}> = 0>
        bool is_equal(const iter& other) const
        {
            static const auto impl = []<std::size_t... I>(auto& it, auto& o, std::index_sequence<I...>)
            {
                return (... || (std::get<I>(it) == std::get<I>(o)));
            };
            return impl(m_iters, other.m_iters, index_seq{});
        }

        template <class It = iterators_type, require<each_satisfies<It, less_than>{}> = 0>
        bool is_less(const iter& other) const
        {
            static const auto impl = []<std::size_t... I>(auto& it, auto& o, std::index_sequence<I...>)
            {
                return (... || (std::get<I>(it) < std::get<I>(o)));
            };
            return impl(m_iters, other.m_iters, index_seq{});
        }

        template <class It = iterators_type, require<each_satisfies<It, subtraction>{}> = 0>
        std::ptrdiff_t distance_to(const iter& other) const
        {
            static const auto min_value = [](std::ptrdiff_t v0, auto... tail) -> std::ptrdiff_t
            {
                if constexpr (sizeof...(tail) > 0)
                {
                    return std::min(v0, min_value(tail...));
                }
                else
                {
                    return v0;
                }
            };

            static const auto impl =
                []<std::size_t... I>(const auto& it, const auto& o, std::index_sequence<I...>)->std::ptrdiff_t
            {
                return min_value(std::distance(std::get<I>(it), std::get<I>(o))...);
            };
            return impl(m_iters, other.m_iters, index_seq{});
        }

        template <class T>
        using advance_by_offset = addition_assignment<T, std::ptrdiff_t>;

        template <class It = iterators_type, require<each_satisfies<It, advance_by_offset>{}> = 0>
        void advance(std::ptrdiff_t offset)
        {
            static const auto impl = []<std::size_t... I>(auto& it, std::ptrdiff_t off, std::index_sequence<I...>)
            {
                ((std::get<I>(it) += off), ...);
            };
            impl(m_iters, offset, index_seq{});
        }
    };

    using iterator = iterator_interface<iter>;

    iterator begin() const
    {
        static const auto impl = []<std::size_t... I>(const auto& r, std::index_sequence<I...>)
        {
            return iterator{ std::tuple{ std::begin(std::get<I>(r))... } };
        };
        return impl(m_ranges, index_seq{});
    }

    iterator end() const
    {
        static const auto impl = []<std::size_t... I>(const auto& r, std::index_sequence<I...>)
        {
            return iterator{ std::tuple{ std::end(std::get<I>(r))... } };
        };
        return impl(m_ranges, index_seq{});
    }
};

struct all_fn
{
    template <class... Ranges>
    auto operator()(Ranges... ranges) const
    {
        return range_interface<ranges_pack<Ranges...>>{ std::tuple{ std::move(ranges)... } };
    }
};

}  // namespace detail

static constexpr inline auto all = detail::all_fn{};

}  // namespace core
}  // namespace ferrugo
