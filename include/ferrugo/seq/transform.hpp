#pragma once

#include <ferrugo/iterator_interface.hpp>
#include <ferrugo/pipeline.hpp>
#include <ferrugo/ranges.hpp>

namespace ferrugo
{
namespace seq
{
namespace detail
{
struct transform_fn
{
    template <class Range, class Func>
    struct view
    {
        Range range;
        Func func;

        view(Range range, Func func) : range{ std::move(range) }, func{ std::move(func) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            inner_iterator it;

            iter() = default;

            iter(const view* parent, inner_iterator it) : parent{ parent }, it{ it }
            {
            }

            auto deref() const -> decltype(invoke(parent->func, *it))
            {
                return invoke(parent->func, *it);
            }

            void inc()
            {
                ++it;
            }

            bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

            template <class It = inner_iterator, require<is_bidirectional_iterator<It>::value> = {}>
            void dec()
            {
                --it;
            }

            template <class It = inner_iterator, require<is_random_access_iterator<It>::value> = {}>
            void advance(iter_difference_t<It> offset)
            {
                it += offset;
            }

            template <class It = inner_iterator, require<is_random_access_iterator<It>::value> = {}>
            iter_difference_t<It> distance_to(const iter& other) const
            {
                return other.it - it;
            }
        };

        using iterator = iterator_interface<iter>;

        iterator begin() const
        {
            return iterator{ this, std::begin(range) };
        }

        iterator end() const
        {
            return iterator{ this, std::end(range) };
        }
    };

    template <class Func>
    struct impl
    {
        Func func;

        template <class R, class F>
        static auto make_view(R&& r, F&& f) -> view<decay_t<R>, decay_t<F>>
        {
            return { std::forward<R>(r), std::forward<F>(f) };
        }

        template <class Range>
        auto operator()(Range&& range) const
            -> decltype(make_range_interface(make_view(make_range(std::forward<Range>(range)), func)))
        {
            return make_range_interface(make_view(make_range(std::forward<Range>(range)), func));
        }
    };

    template <class F>
    static auto make_impl(F&& f) -> impl<decay_t<F>>
    {
        return { std::forward<F>(f) };
    }

    template <class Func>
    auto operator()(Func&& func) const -> decltype(fn(make_impl(std::forward<Func>(func))))
    {
        return fn(make_impl(std::forward<Func>(func)));
    }
};

}  // namespace detail

constexpr auto transform = detail::transform_fn{};

}  // namespace seq
}  // namespace ferrugo
