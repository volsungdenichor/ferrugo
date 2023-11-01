#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
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

    optional() : _initialized{ false }, _buffer{}
    {
    }

    optional(none_t) : optional()
    {
    }

    template <class U, typename std::enable_if<std::is_constructible<value_type, U>::value, int>::type = 0>
    optional(U&& value) : _initialized{ true }
                        , _buffer{}
    {
        construct(std::forward<U>(value));
    }

    optional(const optional& other) : _initialized{ other.has_value() }, _buffer{}
    {
        if (_initialized)
        {
            construct(*other);
        }
    }

    optional(optional&& other) : _initialized{ other.has_value() }, _buffer{}
    {
        if (_initialized)
        {
            construct(*std::move(other));
        }
        other.reset();
    }

    template <class U, typename std::enable_if<std::is_constructible<value_type, U>::value, int>::type = 0>
    optional(const optional<U>& other) : _initialized{ other.has_value() }
                                       , _buffer{}
    {
        if (_initialized)
        {
            construct(*other);
        }
    }

    template <class U, typename std::enable_if<std::is_constructible<value_type, U>::value, int>::type = 0>
    optional(optional<U>&& other) : _initialized{ other.has_value() }
                                  , _buffer{}
    {
        if (_initialized)
        {
            construct(*std::move(other));
        }
        other.reset();
    }

    optional& operator=(optional other)
    {
        swap(other);
        return *this;
    }

    ~optional()
    {
        reset();
    }

    bool has_value() const
    {
        return _initialized;
    }

    explicit operator bool() const
    {
        return has_value();
    }

    const value_type& operator*() const&
    {
        return *get_address();
    }

    value_type& operator*() &
    {
        return *get_address();
    }

    value_type&& operator*() &&
    {
        return std::move(*get_address());
    }

    const value_type* operator->() const&
    {
        return get_address();
    }

    value_type* operator->() &
    {
        return get_address();
    }

    value_type* operator->() &&
    {
        return get_address();
    }

    void swap(optional& other)
    {
        std::swap(_initialized, other._initialized);
        std::swap(_buffer, other._buffer);
    }

    void reset()
    {
        if (_initialized)
        {
            get_address()->~T();
        }
        _initialized = false;
    }

private:
    template <class... Args>
    void construct(Args&&... args)
    {
        new (get_address()) value_type{ std::forward<Args>(args)... };
    }

    value_type* get_address()
    {
        return reinterpret_cast<value_type*>(&_buffer);
    }

    const value_type* get_address() const
    {
        return reinterpret_cast<const value_type*>(&_buffer);
    }

    bool _initialized;
    std::array<std::uint8_t, sizeof(value_type)> _buffer;
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