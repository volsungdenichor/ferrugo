#pragma once

#include <ferrugo/core/range_interface.hpp>
#include <memory>

namespace ferrugo
{
namespace core
{

namespace detail
{

template <class Range>
struct owning_range
{
    std::shared_ptr<Range> m_range;

    owning_range(Range&& range) : m_range{ std::make_shared<Range>(std::move(range)) }
    {
    }

    owning_range(const Range& range) : m_range{ std::make_shared<Range>(range) }
    {
    }

    owning_range(const owning_range&) = default;
    owning_range(owning_range&&) = default;

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
struct owning_range : range_interface<detail::owning_range<Range>>
{
    using base_type = range_interface<detail::owning_range<Range>>;
    using base_type::base_type;
};

template <class Range>
owning_range(Range&&) -> owning_range<std::decay_t<Range>>;

}  // namespace core

}  // namespace ferrugo
