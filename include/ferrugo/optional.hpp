#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <ferrugo/either.hpp>
#include <type_traits>

namespace ferrugo
{

struct none_t
{
};

static constexpr none_t none;

template <class T>
class optional
{
public:
    using value_type = T;

    optional() = default;

    optional(none_t) : optional()
    {
    }

    template <class U, typename std::enable_if<std::is_constructible<value_type, U>::value, int>::type = 0>
    optional(U&& value) : storage_{ right(std::forward<U>(value)) }
    {
    }

    optional(const optional&) = default;
    optional(optional&&) = default;

    template <class U, typename std::enable_if<std::is_constructible<value_type, U>::value, int>::type = 0>
    optional(const optional<U>& other) : storage_{}
    {
        if (other)
        {
            storage_.emplace(detail::in_place_t<detail::either_side::right>{}, *other);
        }
    }

    template <class U, typename std::enable_if<std::is_constructible<value_type, U>::value, int>::type = 0>
    optional(optional<U>&& other) : storage_{}
    {
        if (other)
        {
            storage_.emplace(detail::in_place_t<detail::either_side::right>{}, *std::move(other));
        }
    }

    optional& operator=(optional other)
    {
        swap(other);
        return *this;
    }

    bool has_value() const
    {
        return storage_.has_right_value();
    }

    explicit operator bool() const
    {
        return has_value();
    }

    const value_type& operator*() const&
    {
        return storage_.right_value();
    }

    value_type& operator*() &
    {
        return storage_.right_value();
    }

    value_type&& operator*() &&
    {
        return std::move(storage_).right_value();
    }

    const value_type* operator->() const&
    {
        return &(*this);
    }

    value_type* operator->() &
    {
        return &(*this);
    }

    value_type* operator->() &&
    {
        return &(*this);
    }

    void reset()
    {
        storage_.reset();
    }

    void swap(optional& other)
    {
        std::swap(storage_, other.storage_);
    }

    either<none_t, T> storage_;
};

template <class T>
class optional<T&>
{
public:
    using value_type = T;

    optional() : _pointer{}
    {
    }

    optional(none_t) : optional()
    {
    }

    optional(T& value) : _pointer(&value)
    {
    }

    optional(const optional& other) = default;

    optional(optional&& other) = default;

    optional& operator=(optional other)
    {
        swap(other);
        return *this;
    }

    ~optional()
    {
    }

    bool has_value() const
    {
        return _pointer;
    }

    explicit operator bool() const
    {
        return has_value();
    }

    value_type& operator*() const
    {
        return *_pointer;
    }

    value_type* operator->() const
    {
        return _pointer;
    }

    void swap(optional& other)
    {
        std::swap(_pointer, other._pointer);
    }

    void reset()
    {
        _pointer = nullptr;
    }

private:
    T* _pointer;
};

template <class L, class R>
bool operator==(const optional<L>& lhs, const optional<R>& rhs)
{
    return (!lhs && !rhs) || (lhs && rhs && *lhs == *rhs);
}

template <class L, class R>
bool operator!=(const optional<L>& lhs, const optional<R>& rhs)
{
    return !(lhs == rhs);
}

template <class L, class R>
bool operator==(const optional<L>& lhs, const R& rhs)
{
    return lhs && (*lhs == rhs);
}

template <class L, class R>
bool operator!=(const optional<L>& lhs, const R& rhs)
{
    return !(lhs == rhs);
}

template <class L, class R>
bool operator==(const L& lhs, const optional<R>& rhs)
{
    return rhs == lhs;
}

template <class L, class R>
bool operator!=(const L& lhs, const optional<R>& rhs)
{
    return !(lhs == rhs);
}

template <class L>
bool operator==(const optional<L>& lhs, none_t)
{
    return !lhs.has_value();
}

template <class L>
bool operator!=(const optional<L>& lhs, none_t)
{
    return lhs.has_value();
}

template <class R>
bool operator==(none_t, const optional<R>& rhs)
{
    return !rhs.has_value();
}

template <class R>
bool operator!=(none_t, const optional<R>& rhs)
{
    return rhs.has_value();
}

}  // namespace ferrugo