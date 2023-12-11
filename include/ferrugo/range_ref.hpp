#pragma once

#include <ferrugo/range_interface.hpp>

namespace ferrugo
{

namespace detail
{

template <class Range>
struct range_ref
{
    Range* range_;

    range_ref() = default;

    range_ref(Range& range) : range_(range)
    {
    }

    range_ref(const range_ref&) = default;
    range_ref(range_ref&&) = default;

    iterator_t<Range> begin() const
    {
        return std::begin(*range_);
    }

    iterator_t<Range> end() const
    {
        return std::end(*range_);
    }
};

}  // namespace detail

template <class Range>
using range_ref = range_interface<detail::range_ref<Range>>;

template <class Range>
auto make_range_ref(Range& range) -> range_ref<Range>
{
    return { range };
}

}  // namespace ferrugo
