#pragma once

#include <ferrugo/iterator_range.hpp>
#include <ferrugo/owning_range.hpp>
#include <ferrugo/range_ref.hpp>

namespace ferrugo
{

struct make_range_fn
{
    template <class Range, require<is_range_interface<decay_t<Range>>::value> = {}>
    auto make(Range&& range) const -> decay_t<Range>
    {
        return std::forward<Range>(range);
    }

    template <class Range, require<!is_range_interface<decay_t<Range>>::value && std::is_lvalue_reference<Range>::value> = {}>
    auto make(Range&& range) const -> range_ref<typename std::remove_reference<Range>::type>
    {
        return make_range_ref(std::forward<Range>(range));
    }

    template <class Range, require<!is_range_interface<decay_t<Range>>::value && !std::is_lvalue_reference<Range>::value> = {}>
    auto make(Range&& range) const -> owning_range<decay_t<Range>>
    {
        return make_owning_range(std::forward<Range>(range));
    }

    template <class Range>
    auto operator()(Range&& range) const -> decltype(this->make(std::forward<Range>(range)))
    {
        return this->make(std::forward<Range>(range));
    }
};

constexpr auto make_range = make_range_fn{};

}  // namespace ferrugo