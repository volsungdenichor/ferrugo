#pragma once

#include <ferrugo/range_interface.hpp>
#include <memory>

namespace ferrugo
{

namespace detail
{

template <class Range>
struct owning_range
{
    std::shared_ptr<Range> range_;

    owning_range() = default;

    owning_range(std::shared_ptr<Range> range) : range_(std::move(range))
    {
    }

    owning_range(Range&& range) : owning_range(std::make_shared<Range>(std::move(range)))
    {
    }

    owning_range(const Range& range) : owning_range(std::make_shared<Range>(range))
    {
    }

    owning_range(const owning_range&) = default;
    owning_range(owning_range&&) = default;

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
using owning_range = range_interface<detail::owning_range<Range>>;

template <class Range>
auto make_owning_range(Range&& range) -> owning_range<decay_t<Range>>
{
    return { std::forward<Range>(range) };
}

}  // namespace ferrugo
