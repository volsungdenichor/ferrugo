
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
struct i_forward_iterator : i_cloneable<i_forward_iterator<T>>
{
    virtual ~i_forward_iterator() = default;

    virtual T deref() const = 0;
    virtual void inc() = 0;
    virtual bool is_equal(const i_forward_iterator& other) const = 0;
};

template <class T>
using i_forward_range = i_range<i_forward_iterator<T>>;

template <class T, class Iter>
struct forward_iterator_impl : public i_forward_iterator<T>
{
    Iter m_it;

    forward_iterator_impl(Iter it) : m_it{ it }
    {
    }

    std::unique_ptr<i_forward_iterator<T>> clone() const override
    {
        return std::make_unique<forward_iterator_impl>(m_it);
    }

    T deref() const override
    {
        return *m_it;
    }

    void inc() override
    {
        ++m_it;
    }

    bool is_equal(const i_forward_iterator<T>& other) const override
    {
        return m_it == static_cast<const forward_iterator_impl&>(other).m_it;
    }
};

template <class T, class Range>
struct forward_range_impl : public i_forward_range<T>
{
    Range m_range;

    forward_range_impl(Range range) : m_range{ std::move(range) }
    {
    }

    std::unique_ptr<i_forward_iterator<T>> begin() const override
    {
        return std::make_unique<forward_iterator_impl<T, iterator_t<Range>>>(std::begin(m_range));
    }

    std::unique_ptr<i_forward_iterator<T>> end() const override
    {
        return std::make_unique<forward_iterator_impl<T, iterator_t<Range>>>(std::end(m_range));
    }
};

template <class T>
struct forward_iterable
{
    struct iter
    {
        std::unique_ptr<i_forward_iterator<T>> m_it;

        iter() = default;

        iter(std::unique_ptr<i_forward_iterator<T>> it) : m_it{ std::move(it) }
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

        void inc()
        {
            m_it->inc();
        }

        bool is_equal(const iter& other) const
        {
            return m_it->is_equal(*other.m_it);
        }
    };

    std::unique_ptr<i_forward_range<T>> m_impl;

    using iterator = iterator_interface<iter>;

    template <class Range>
    forward_iterable(Range range) : m_impl{ std::make_unique<forward_range_impl<T, Range>>(std::move(range)) }
    {
        static_assert(is_forward_range<std::decay_t<Range>>{}, "forward range required");
    }

    forward_iterable(const forward_iterable&) = delete;
    forward_iterable(forward_iterable&&) = default;

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
struct forward_iterable : range_interface<detail::forward_iterable<T>>
{
    using base_type = range_interface<detail::forward_iterable<T>>;
    using base_type::base_type;
};

template <class T>
using iterable = forward_iterable<T>;

}  // namespace core

}  // namespace ferrugo
