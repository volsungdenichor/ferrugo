
#pragma once

#include <ferrugo/core/iterator_interface.hpp>
#include <ferrugo/core/range_interface.hpp>
#include <ferrugo/core/ranges/iterable.hpp>

namespace ferrugo
{

namespace core
{

namespace detail
{

template <class T>
struct i_random_access_iterator : i_cloneable<i_random_access_iterator<T>>
{
    virtual ~i_random_access_iterator() = default;

    virtual T deref() const = 0;
    virtual std::ptrdiff_t distance_to(const i_random_access_iterator& other) const = 0;
    virtual void advance(std::ptrdiff_t offset) = 0;
};

template <class T>
using i_random_access_range = i_range<i_random_access_iterator<T>>;

template <class T, class Iter>
struct random_access_iterator_impl : public i_random_access_iterator<T>
{
    Iter m_it;

    random_access_iterator_impl(Iter it) : m_it{ it }
    {
    }

    std::unique_ptr<i_random_access_iterator<T>> clone() const override
    {
        return std::make_unique<random_access_iterator_impl>(m_it);
    }

    T deref() const override
    {
        return *m_it;
    }

    std::ptrdiff_t distance_to(const i_random_access_iterator<T>& other) const override
    {
        return std::distance(m_it, static_cast<const random_access_iterator_impl&>(other).m_it);
    }

    void advance(std::ptrdiff_t offset) override
    {
        std::advance(m_it, offset);
    }
};

template <class T, class Range>
struct random_access_range_impl : public i_random_access_range<T>
{
    Range range_;

    random_access_range_impl(Range range) : range_{ std::move(range) }
    {
    }

    std::unique_ptr<i_random_access_iterator<T>> begin() const override
    {
        return std::make_unique<random_access_iterator_impl<T, iterator_t<Range>>>(std::begin(range_));
    }

    std::unique_ptr<i_random_access_iterator<T>> end() const override
    {
        return std::make_unique<random_access_iterator_impl<T, iterator_t<Range>>>(std::end(range_));
    }
};

template <class T>
struct random_access_iterable
{
    struct iter
    {
        std::unique_ptr<i_random_access_iterator<T>> m_it;

        iter() = default;

        iter(std::unique_ptr<i_random_access_iterator<T>> it) : m_it{ std::move(it) }
        {
        }

        iter(const iter& other) : m_it{ other.m_it ? other.m_it->clone() : nullptr }
        {
        }

        iter(iter&& other) : m_it{ std::move(other.m_it) }
        {
        }

        T deref() const
        {
            return m_it->deref();
        }

        std::ptrdiff_t distance_to(const iter& other) const
        {
            return m_it->distance_to(*other.m_it);
        }

        void advance(std::ptrdiff_t offset)
        {
            m_it->advance(offset);
        }
    };

    std::unique_ptr<i_random_access_range<T>> m_impl;

    using iterator = iterator_interface<iter>;

    template <class Range>
    random_access_iterable(Range range) : m_impl{ std::make_unique<random_access_range_impl<T, Range>>(std::move(range)) }
    {
        static_assert(is_random_access_range<std::decay_t<Range>>{}, "random access range required");
    }

    random_access_iterable(const random_access_iterable&) = delete;
    random_access_iterable(random_access_iterable&&) = default;

    iterator begin() const
    {
        return iterator{ m_impl->begin() };
    }

    iterator end() const
    {
        return iterator{ m_impl->end() };
    }
};

}  // namespace detail

template <class T>
struct random_access_iterable : range_interface<detail::random_access_iterable<T>>
{
    using base_type = range_interface<detail::random_access_iterable<T>>;
    using base_type::base_type;
};

}  // namespace core

}  // namespace ferrugo
