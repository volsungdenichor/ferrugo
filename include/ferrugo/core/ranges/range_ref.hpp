#pragma once

#include <ferrugo/core/range_interface.hpp>

namespace ferrugo
{
namespace core
{

namespace detail
{

template <class Range>
struct ref_range
{
    Range* m_range;

    ref_range(Range& range) : m_range{ &range }
    {
    }

    ref_range(const ref_range&) = default;
    ref_range(ref_range&&) = default;

    iterator_t<Range> begin() const
    {
        return std::begin(*m_range);
    }

    iterator_t<Range> end() const
    {
        return std::end(*m_range);
    }
};

}  // namespace detail

template <class Range>
struct ref_range : range_interface<detail::ref_range<Range>>
{
    using base_type = range_interface<detail::ref_range<Range>>;
    using base_type::base_type;
};

template <class Range>
ref_range(Range&) -> ref_range<Range>;

}  // namespace core

}  // namespace ferrugo
