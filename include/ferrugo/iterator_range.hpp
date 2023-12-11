#pragma once

#include <ferrugo/range_interface.hpp>

namespace ferrugo
{

namespace detail
{

template <class Iter>
struct iterator_range
{
    Iter b_;
    Iter e_;

    iterator_range() = default;

    iterator_range(Iter b, Iter e) : b_(b), e_(e)
    {
    }

    iterator_range(std::pair<Iter, Iter> p) : iterator_range(std::get<0>(p), std::get<1>(p))
    {
    }

    iterator_range(std::tuple<Iter, Iter> t) : iterator_range(std::get<0>(t), std::get<1>(t))
    {
    }

    iterator_range(Iter b, typename std::iterator_traits<Iter>::difference_type n) : iterator_range(b, std::next(b, n))
    {
    }

    template <class Range>
    iterator_range(Range&& range) : iterator_range(std::begin(range), std::end(range))
    {
    }

    iterator_range(const iterator_range&) = default;
    iterator_range(iterator_range&&) = default;

    Iter begin() const
    {
        return b_;
    }

    Iter end() const
    {
        return e_;
    }
};

}  // namespace detail

template <class Iter>
using iterator_range = range_interface<detail::iterator_range<Iter>>;

template <class Range>
using subrange = iterator_range<iterator_t<Range>>;

template <class Iter>
auto make_iterator_range(Iter b, Iter e) -> iterator_range<Iter>
{
    return { b, e };
}

template <class Iter>
auto make_iterator_range(std::pair<Iter, Iter> p) -> iterator_range<Iter>
{
    return { p };
}

template <class Iter>
auto make_iterator_range(std::tuple<Iter, Iter> p) -> iterator_range<Iter>
{
    return { p };
}

template <class Iter>
auto make_iterator_range(Iter b, typename std::iterator_traits<Iter>::difference_type n) -> iterator_range<Iter>
{
    return { b, n };
}

template <class Range>
auto make_iterator_range(Range&& range) -> iterator_range<iterator_t<Range>>
{
    return { std::forward<Range>(range) };
}

}  // namespace ferrugo
