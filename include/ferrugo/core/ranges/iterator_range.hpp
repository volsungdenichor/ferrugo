#pragma once

#include <ferrugo/core/range_interface.hpp>

namespace ferrugo
{

namespace core
{

namespace detail
{

template <class Iter>
struct iterator_range
{
    Iter m_begin;
    Iter m_end;

    iterator_range() = default;

    iterator_range(Iter b, Iter e) : m_begin(b), m_end(e)
    {
    }

    iterator_range(const std::pair<Iter, Iter>& p) : iterator_range(std::get<0>(p), std::get<1>(p))
    {
    }

    iterator_range(const std::tuple<Iter, Iter>& t) : iterator_range(std::get<0>(t), std::get<1>(t))
    {
    }

    iterator_range(Iter b, iter_difference_t<Iter> n) : iterator_range(b, std::next(b, n))
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
        return m_begin;
    }

    Iter end() const
    {
        return m_end;
    }
};

}  // namespace detail

template <class Iter>
struct iterator_range : range_interface<detail::iterator_range<Iter>>
{
    using base_type = range_interface<detail::iterator_range<Iter>>;
    using base_type::base_type;
};

template <class Iter>
iterator_range(Iter, Iter) -> iterator_range<Iter>;

template <class Range>
iterator_range(Range&&) -> iterator_range<iterator_t<Range>>;

}  // namespace core

}  // namespace ferrugo
