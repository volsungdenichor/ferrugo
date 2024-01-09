
#pragma once

#include <ferrugo/iterator_interface.hpp>
#include <ferrugo/make_unique.hpp>
#include <ferrugo/ranges.hpp>
namespace ferrugo
{

namespace detail
{

template <class T>
struct i_forward_iterator
{
    virtual ~i_forward_iterator() = default;
    virtual std::unique_ptr<i_forward_iterator> clone() const = 0;

    virtual T deref() const = 0;
    virtual void inc() = 0;
    virtual bool is_equal(const i_forward_iterator& other) const = 0;
};

template <class T>
struct i_random_access_iterator
{
    virtual ~i_random_access_iterator() = default;
    virtual std::unique_ptr<i_random_access_iterator> clone() const = 0;

    virtual T deref() const = 0;
    virtual std::ptrdiff_t distance_to(const i_random_access_iterator& other) const = 0;
    virtual void advance(std::ptrdiff_t offset) = 0;
};

template <class It>
struct i_range
{
    virtual ~i_range() = default;
    virtual std::unique_ptr<It> begin() const = 0;
    virtual std::unique_ptr<It> end() const = 0;
};

template <class T, class Inner>
struct forward_iterator_impl : public i_forward_iterator<T>
{
    using inner_iter = Inner;
    inner_iter it;

    forward_iterator_impl(inner_iter it) : it{ it }
    {
    }

    std::unique_ptr<i_forward_iterator<T>> clone() const override
    {
        return std::make_unique<forward_iterator_impl>(it);
    }

    T deref() const override
    {
        return *it;
    }

    void inc() override
    {
        ++it;
    }

    bool is_equal(const i_forward_iterator<T>& other) const override
    {
        return it == static_cast<const forward_iterator_impl&>(other).it;
    }
};

template <class T, class Inner>
struct random_access_iterator_impl : public i_random_access_iterator<T>
{
    using inner_iter = Inner;
    inner_iter it;

    random_access_iterator_impl(inner_iter it) : it{ it }
    {
    }

    std::unique_ptr<i_random_access_iterator<T>> clone() const override
    {
        return std::make_unique<random_access_iterator_impl>(it);
    }

    T deref() const override
    {
        return *it;
    }

    std::ptrdiff_t distance_to(const i_random_access_iterator<T>& other) const override
    {
        return std::distance(it, static_cast<const random_access_iterator_impl&>(other).it);
    }

    void advance(std::ptrdiff_t offset) override
    {
        std::advance(it, offset);
    }
};

template <class T, class Range>
struct forward_range_impl : public i_range<i_forward_iterator<T>>
{
    Range range_;

    forward_range_impl(Range range) : range_{ std::move(range) }
    {
    }

    std::unique_ptr<i_forward_iterator<T>> begin() const override
    {
        return std::make_unique<forward_iterator_impl<T, iterator_t<Range>>>(std::begin(range_));
    }

    std::unique_ptr<i_forward_iterator<T>> end() const override
    {
        return std::make_unique<forward_iterator_impl<T, iterator_t<Range>>>(std::end(range_));
    }
};

template <class T, class Range>
struct random_access_range_impl : public i_range<i_random_access_iterator<T>>
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
struct forward_iterable
{
    struct iter
    {
        std::unique_ptr<i_forward_iterator<T>> it;

        iter() = default;

        iter(std::unique_ptr<i_forward_iterator<T>> it) : it{ std::move(it) }
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

    std::unique_ptr<i_range<i_forward_iterator<T>>> impl_;

    using iterator = iterator_interface<iter>;

    template <class Range>
    forward_iterable(Range&& range) : impl_(create(make_range(std::forward<Range>(range))))
    {
        static_assert(is_forward_range<decay_t<Range>>::value, "forward range required");
    }

    forward_iterable(const forward_iterable&) = delete;
    forward_iterable(forward_iterable&&) = default;

    iterator begin() const
    {
        return iterator{ impl_->begin() };
    }

    iterator end() const
    {
        return iterator{ impl_->end() };
    }

    template <class Range>
    static std::unique_ptr<i_range<i_forward_iterator<T>>> create(Range range)
    {
        return std::make_unique<forward_range_impl<T, Range>>(std::move(range));
    }
};

template <class T>
struct random_access_iterable
{
    struct iter
    {
        std::unique_ptr<i_random_access_iterator<T>> it;

        iter() = default;

        iter(std::unique_ptr<i_random_access_iterator<T>> it) : it{ std::move(it) }
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

        std::ptrdiff_t distance_to(const iter& other) const
        {
            return it->distance_to(*other.it);
        }

        void advance(std::ptrdiff_t offset)
        {
            it->advance(offset);
        }
    };

    std::unique_ptr<i_range<i_random_access_iterator<T>>> impl_;

    using iterator = iterator_interface<iter>;

    template <class Range>
    random_access_iterable(Range&& range) : impl_(create(make_range(std::forward<Range>(range))))
    {
        static_assert(is_random_access_range<decay_t<Range>>::value, "random access range required");
    }

    random_access_iterable(const random_access_iterable&) = delete;
    random_access_iterable(random_access_iterable&&) = default;

    iterator begin() const
    {
        return iterator{ impl_->begin() };
    }

    iterator end() const
    {
        return iterator{ impl_->end() };
    }

    template <class Range>
    static std::unique_ptr<i_range<i_random_access_iterator<T>>> create(Range range)
    {
        return std::make_unique<random_access_range_impl<T, Range>>(std::move(range));
    }
};

}  // namespace detail

template <class T>
using forward_iterable = range_interface<detail::forward_iterable<T>>;

template <class T>
using random_access_iterable = range_interface<detail::random_access_iterable<T>>;

template <class T>
using iterable = forward_iterable<T>;

}  // namespace ferrugo
