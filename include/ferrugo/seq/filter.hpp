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
struct filter_fn
{
    template <class Range, class Pred>
    struct view
    {
        Range range;
        Pred pred;

        view(Range range, Pred pred) : range{ std::move(range) }, pred{ std::move(pred) }
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
                update();
            }

            auto deref() const -> range_reference_t<Range>
            {
                return *it;
            }

            void inc()
            {
                ++it;
                update();
            }

            bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

            template <class It = inner_iterator, require<is_bidirectional_iterator<It>::value> = {}>
            void dec()
            {
                --it;
                while (!invoke(parent->pred, *it))
                {
                    --it;
                }
            }

            void update()
            {
                while (it != std::end(parent->range) && !invoke(parent->pred, *it))
                {
                    ++it;
                }
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

    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class R, class P>
        static auto make_view(R&& r, P&& p) -> view<decay_t<R>, decay_t<P>>
        {
            return { std::forward<R>(r), std::forward<P>(p) };
        }

        template <class Range>
        auto operator()(Range&& range) const
            -> decltype(make_range_interface(make_view(make_range(std::forward<Range>(range)), pred)))
        {
            return make_range_interface(make_view(make_range(std::forward<Range>(range)), pred));
        }
    };

    template <class P>
    static auto make_impl(P&& p) -> impl<decay_t<P>>
    {
        return { std::forward<P>(p) };
    }

    template <class Pred>
    auto operator()(Pred&& pred) const -> decltype(fn(make_impl(std::forward<Pred>(pred))))
    {
        return fn(make_impl(std::forward<Pred>(pred)));
    }
};

}  // namespace detail

constexpr auto filter = detail::filter_fn{};

}  // namespace seq
}  // namespace ferrugo
