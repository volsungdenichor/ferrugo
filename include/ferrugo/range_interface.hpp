#pragma once

#include <ferrugo/type_traits.hpp>

namespace ferrugo
{

template <class Impl>
struct range_interface
{
    Impl impl_;

    using begin_iterator = decltype(std::declval<const Impl&>().begin());
    using end_iterator = decltype(std::declval<const Impl&>().end());

    using iterator = begin_iterator;
    using const_iterator = iterator;

    using reference = typename std::iterator_traits<iterator>::reference;
    using difference_type = iter_difference_t<iterator>;
    using size_type = difference_type;

    template <class... Args, require<std::is_constructible<Impl, Args...>::value> = {}>
    range_interface(Args&&... args) : impl_{ std::forward<Args>(args)... }
    {
    }

    iterator begin() const
    {
        return impl_.begin();
    }

    iterator end() const
    {
        return impl_.end();
    }

    template <class Container, require<std::is_constructible<Container, iterator, iterator>::value> = {}>
    operator Container() const
    {
        return Container{ begin(), end() };
    }

    bool empty() const
    {
        return begin() == end();
    }

    reference front() const
    {
        return *begin();
    }

    template <class It = iterator, require<is_bidirectional_iterator<It>::value> = {}>
    reference back() const
    {
        return *std::prev(end());
    }

    template <class It = iterator, require<is_random_access_iterator<It>::value> = {}>
    reference operator[](difference_type n) const
    {
        return *std::next(begin(), n);
    }

    template <class It = iterator, require<is_random_access_iterator<It>::value> = {}>
    reference at(difference_type n) const
    {
        if (0 <= n && n < size())
        {
            return (*this)[n];
        }
        throw std::out_of_range{ "index out of range" };
    }

    // template <class It = iterator, require<is_random_access_iterator<It>::value> = {}>
    size_type size() const
    {
        return std::distance(begin(), end());
    }
};

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

}  // namespace ferrugo
