
#pragma once

#include <ferrugo/iterator_interface.hpp>
#include <ferrugo/make_unique.hpp>
#include <ferrugo/ranges.hpp>
namespace ferrugo
{

namespace detail
{

template <class T>
struct i_iterator
{
    virtual ~i_iterator() = default;
    virtual T deref() const = 0;
    virtual void inc() = 0;
    virtual bool is_equal(const i_iterator& other) const = 0;
    virtual std::unique_ptr<i_iterator> clone() const = 0;
};

template <class T>
struct i_range
{
    virtual ~i_range() = default;
    virtual std::unique_ptr<i_iterator<T>> begin() const = 0;
    virtual std::unique_ptr<i_iterator<T>> end() const = 0;
};

template <class T, class Inner>
struct iterator_wrapper : public i_iterator<T>
{
    using inner_iter = Inner;
    inner_iter it;

    iterator_wrapper(inner_iter it) : it{ it }
    {
    }

    T deref() const override
    {
        return *it;
    }

    void inc() override
    {
        ++it;
    }

    bool is_equal(const i_iterator<T>& other) const override
    {
        return it == static_cast<const iterator_wrapper&>(other).it;
    }

    std::unique_ptr<i_iterator<T>> clone() const override
    {
        return std::make_unique<iterator_wrapper>(it);
    }
};

template <class T, class Range>
struct range_wrapper : public i_range<T>
{
    Range range_;

    range_wrapper(Range range) : range_{ std::move(range) }
    {
    }

    std::unique_ptr<i_iterator<T>> begin() const override
    {
        return std::make_unique<iterator_wrapper<T, iterator_t<Range>>>(std::begin(range_));
    }

    std::unique_ptr<i_iterator<T>> end() const override
    {
        return std::make_unique<iterator_wrapper<T, iterator_t<Range>>>(std::end(range_));
    }
};

template <class T>
struct iterable
{
    struct iter
    {
        std::unique_ptr<i_iterator<T>> it;

        iter() = default;

        iter(std::unique_ptr<i_iterator<T>> it) : it{ std::move(it) }
        {
        }

        iter(const iter& other) : it{ other.it ? other.it->clone() : nullptr }
        {
        }

        iter(iter&& other) : it{ std::move(other.it) }
        {
        }

        T deref() const
        {
            return it->deref();
        }

        void inc()
        {
            it->inc();
        }

        bool is_equal(const iter& other) const
        {
            return it->is_equal(*other.it);
        }
    };

    std::unique_ptr<i_range<T>> impl_;

    using iterator = iterator_interface<iter>;

    template <class Range>
    iterable(Range&& range) : impl_(create(make_range(std::forward<Range>(range))))
    {
    }

    iterable(const iterable&) = delete;
    iterable(iterable&&) = default;

    iterator begin() const
    {
        return iterator{ impl_->begin() };
    }

    iterator end() const
    {
        return iterator{ impl_->end() };
    }

    template <class Range>
    static std::unique_ptr<i_range<T>> create(Range range)
    {
        return std::make_unique<range_wrapper<T, Range>>(std::move(range));
    }
};

}  // namespace detail

template <class T>
using iterable = range_interface<detail::iterable<T>>;

}  // namespace ferrugo
