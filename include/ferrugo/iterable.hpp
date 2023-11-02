#pragma once

#include <ferrugo/optional.hpp>
#include <functional>
#include <limits>
#include <memory>

namespace ferrugo
{

struct stop_iteration_t
{
};

static constexpr stop_iteration_t stop_iteration;

template <class T>
struct iteration_result
{
    using reference = T;
    struct pointer
    {
        T value;

        T* operator->()
        {
            return &value;
        }
    };

    optional<T> value_;

    template <class U, typename std::enable_if<std::is_constructible<T, U>::value, int>::type = 0>
    iteration_result(U&& value) : value_{ std::forward<U>(value) }
    {
    }

    iteration_result(stop_iteration_t) : value_{}
    {
    }

    explicit operator bool() const
    {
        return value_.has_value();
    }

    reference operator*() const&
    {
        return *value_;
    }

    reference operator*() &&
    {
        return *std::move(value_);
    }

    pointer operator->() const&
    {
        return pointer{ **this };
    }

    pointer operator->() &&
    {
        return pointer{ *std::move(*this) };
    }
};

template <class T>
struct iteration_result<T&>
{
    using reference = T&;
    using pointer = T*;

    optional<T&> value_;

    iteration_result(T& value) : value_{ value }
    {
    }

    iteration_result(stop_iteration_t) : value_{}
    {
    }

    explicit operator bool() const
    {
        return value_.has_value();
    }

    reference operator*() const
    {
        return *value_;
    }

    pointer operator->() const
    {
        return value_.operator->();
    }
};

template <class T, class Container, class Iter>
struct iterable_impl
{
    std::shared_ptr<Container> container_;
    mutable Iter iter_;

    iterable_impl(std::shared_ptr<Container> container) : container_(std::move(container)), iter_(std::begin(*container_))
    {
    }

    iteration_result<T> operator()() const
    {
        if (iter_ == std::end(*container_))
        {
            return stop_iteration;
        }
        return iteration_result<T>{ *iter_++ };
    }
};

template <class T>
using next_function = std::function<iteration_result<T>()>;

template <class T>
struct iterable;

template <class T, class S>
iterable<T> unfold(S state, const std::function<iteration_result<std::tuple<T, S>>(const S&)>& func)
{
    auto next = [=]() mutable -> iteration_result<T>
    {
        auto result = func(state);
        if (!result)
        {
            return stop_iteration;
        }
        state = std::get<1>(*result);
        return iteration_result<T>{ std::get<0>(*result) };
    };
    return iterable<T>{ std::move(next) };
}

template <class T>
iterable<T> empty()
{
    return iterable<T>{ []() { return stop_iteration; } };
}

template <class T>
iterable<T> repeat(T value, int count)
{
    return unfold<T, int>(
        count,
        [=](int n) -> iteration_result<std::tuple<T, int>>
        {
            if (n == 0)
            {
                return ferrugo::stop_iteration;
            }
            return std::tuple<T, int>{ value, n - 1 };
        });
}

template <class T>
iterable<T> repeat(T value)
{
    return iterable<T>{ [=]() -> iteration_result<T> { return value; } };
}

template <class T>
iterable<T> single(T value)
{
    return repeat(std::move(value), 1);
}

template <class T>
iterable<T> range(T lo, T up, T step = 1)
{
    return unfold<T, T>(
        lo,
        [=](T n) -> iteration_result<std::tuple<T, T>>
        {
            if (n >= up)
            {
                return stop_iteration;
            }
            return std::tuple<T, T>{ n, n + step };
        });
}

template <class T>
iterable<T> iota(T start)
{
    return iterable<T>{ [=]() mutable -> iteration_result<T> { return start++; } };
}

template <class Func, class L, class R, class Out = decltype(std::declval<Func>()(std::declval<L>(), std::declval<R>()))>
iterable<Out> zip_transform(Func func, iterable<L> lhs, iterable<R> rhs)
{
    auto lhs_next = lhs.get_next();
    auto rhs_next = rhs.get_next();

    return iterable<Out>{ [=]() -> iteration_result<Out>
                          {
                              auto lt = lhs_next();
                              auto rt = rhs_next();
                              if (lt && rt)
                              {
                                  return func(*std::move(lt), *std::move(rt));
                              }
                              return stop_iteration;
                          } };
}

template <class L, class R>
struct as_tuple
{
    template <class... Args>
    std::tuple<L, R> operator()(Args&&... args) const
    {
        return std::forward_as_tuple(std::forward<Args>(args)...);
    }
};

template <class L, class R>
iterable<std::tuple<L, R>> zip(iterable<L> lhs, iterable<R> rhs)
{
    return zip_transform(as_tuple<L, R>{}, std::move(lhs), std::move(rhs));
}

template <class T>
struct transform_mixin
{
    template <class Func, class Out>
    struct implementation
    {
        Func func;
        next_function<T> next;

        iteration_result<Out> operator()() const
        {
            if (auto res = next())
            {
                return iteration_result<Out>{ func(*res) };
            }
            else
            {
                return stop_iteration;
            }
        }
    };

    template <class Func, class Out = decltype(std::declval<Func>()(std::declval<T>()))>
    iterable<Out> transform(Func func) const
    {
        return iterable<Out>{ implementation<Func, Out>{ std::move(func),
                                                         static_cast<const iterable<T>&>(*this).get_next() } };
    }
};

template <class T>
struct filter_mixin
{
    template <class Pred>
    struct implementation
    {
        Pred pred;
        next_function<T> next;

        iteration_result<T> operator()() const
        {
            while (auto res = next())
            {
                if (pred(*res))
                {
                    return iteration_result<T>{ *res };
                }
            }
            return stop_iteration;
        }
    };

    template <class Pred>
    iterable<T> filter(Pred pred) const
    {
        return iterable<T>{ implementation<Pred>{ std::move(pred), static_cast<const iterable<T>&>(*this).get_next() } };
    }
};

template <class T>
struct take_mixin
{
    struct implementation
    {
        mutable std::ptrdiff_t count;
        next_function<T> next;

        iteration_result<T> operator()() const
        {
            if (--count < 0)
            {
                return stop_iteration;
            }
            return next();
        }
    };

    iterable<T> take(std::ptrdiff_t count) const
    {
        return iterable<T>{ implementation{ count, static_cast<const iterable<T>&>(*this).get_next() } };
    }
};

template <class T>
struct drop_mixin
{
    struct implementation
    {
        mutable std::ptrdiff_t count;
        next_function<T> next;

        iteration_result<T> operator()() const
        {
            while (count-- > 0)
            {
                next();
            }
            return next();
        }
    };

    iterable<T> drop(std::ptrdiff_t count) const
    {
        return iterable<T>{ implementation{ count, static_cast<const iterable<T>&>(*this).get_next() } };
    }
};

template <class T>
struct take_while_mixin
{
    template <class Pred>
    struct implementation
    {
        Pred pred;
        next_function<T> next;

        iteration_result<T> operator()() const
        {
            auto res = next();
            if (!res || !pred(*res))
            {
                return stop_iteration;
            }
            return *res;
        }
    };

    template <class Pred>
    iterable<T> take_while(Pred pred) const
    {
        return iterable<T>{ implementation<Pred>{ std::move(pred), static_cast<const iterable<T>&>(*this).get_next() } };
    }
};

template <class T>
struct drop_while_mixin
{
    template <class Pred>
    struct implementation
    {
        Pred pred;
        next_function<T> next;
        mutable bool init;

        iteration_result<T> operator()() const
        {
            if (!init)
            {
                while (auto res = next())
                {
                    if (!pred(*res))
                    {
                        break;
                    }
                }
                init = true;
            }
            return next();
        }
    };

    template <class Pred>
    iterable<T> drop_while(Pred pred) const
    {
        return iterable<T>{ implementation<Pred>{
            std::move(pred), static_cast<const iterable<T>&>(*this).get_next(), false } };
    }
};

template <class T>
struct join_mixin
{
};

template <class T>
struct join_mixin<iterable<T>>
{
    struct implementation
    {
        next_function<iterable<T>> outer_next;
        mutable next_function<T> inner_next;

        iteration_result<T> operator()() const
        {
            while (true)
            {
                auto in = inner_next();
                if (in)
                {
                    return iteration_result<T>{ *in };
                }
                auto out = outer_next();
                if (out)
                {
                    inner_next = out->get_next();
                }
                else
                {
                    return stop_iteration;
                }
            }

            return stop_iteration;
        }
    };

    iterable<T> join() const
    {
        return iterable<T>{ implementation{ static_cast<const iterable<iterable<T>>&>(*this).get_next(),
                                            empty<T>().get_next() } };
    }
};

template <class T>
struct enumerate_mixin
{
    struct implementation
    {
        next_function<T> next;
        mutable std::ptrdiff_t index;

        iteration_result<std::tuple<std::ptrdiff_t, T>> operator()() const
        {
            auto res = next();
            if (!res)
            {
                return stop_iteration;
            }
            return std::tuple<std::ptrdiff_t, T>{ index++, *res };
        };
    };

    iterable<std::tuple<std::ptrdiff_t, T>> enumerate(std::ptrdiff_t start = 0) const
    {
        return zip(iota(start), static_cast<const iterable<T>&>(*this));
    }
};

template <class T>
struct iterable : transform_mixin<T>,
                  filter_mixin<T>,
                  take_mixin<T>,
                  drop_mixin<T>,
                  take_while_mixin<T>,
                  drop_while_mixin<T>,
                  join_mixin<T>,
                  enumerate_mixin<T>
{
    using next_fn = next_function<T>;

    struct iterator
    {
        using reference = typename iteration_result<T>::reference;
        using pointer = typename iteration_result<T>::pointer;
        using value_type = typename std::decay<reference>::type;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        next_fn next_;
        iteration_result<T> current_;
        difference_type index_;

        explicit iterator(next_fn next) : next_{ std::move(next) }, current_{ next_() }, index_{ 0 }
        {
        }

        iterator() : next_{}, current_{ stop_iteration }, index_{ std::numeric_limits<difference_type>::max() }
        {
        }

        iterator(const iterator&) = default;
        iterator(iterator&&) = default;

        iterator& operator=(iterator other)
        {
            std::swap(next_, other.next_);
            std::swap(current_, other.current_);
            std::swap(index_, other.index_);
            return *this;
        }

        reference operator*() const&
        {
            return current_.operator*();
        }

        reference operator*() &&
        {
            return std::move(current_).operator*();
        }

        pointer operator->() const&
        {
            return current_.operator->();
        }

        pointer operator->() &&
        {
            return std::move(current_).operator->();
        }

        iterator& operator++()
        {
            current_ = next_();
            ++index_;
            return *this;
        }

        iterator operator++(int)
        {
            iterator temp{ *this };
            ++(*this);
            return temp;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs)
        {
            return (!static_cast<bool>(lhs.current_) && !static_cast<bool>(rhs.current_))
                   || (static_cast<bool>(lhs.current_) && static_cast<bool>(rhs.current_) && lhs.index_ == rhs.index_);
        }

        friend bool operator!=(const iterator& lhs, const iterator& rhs)
        {
            return !(lhs == rhs);
        }
    };

    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using pointer = typename iterator::pointer;

    using const_reference = reference;
    using const_pointer = pointer;
    using const_iterator = iterator;
    using size_type = std::size_t;

    next_fn next_;

    explicit iterable(next_fn next) : next_{ std::move(next) }
    {
    }

    iterable()
        : iterable([]() -> iteration_result<T> { return stop_iteration; })
    {
    }

    template <class Container, class Iter = decltype(std::begin(std::declval<Container>()))>
    iterable(Container container) : iterable(iterable_impl<T, Container, Iter>{ std::make_shared<Container>(container) })
    {
    }

    next_fn get_next() const
    {
        return next_;
    }

    const_iterator begin() const
    {
        return const_iterator{ next_ };
    }

    const_iterator end() const
    {
        return const_iterator{};
    }

    bool empty() const
    {
        return begin() == end();
    }

    size_type size() const
    {
        return std::distance(begin(), end());
    }


    const_reference front() const
    {
        return *begin();
    }
};

}  // namespace ferrugo
