#pragma once

#include <ferrugo/macros.hpp>
#include <type_traits>
#include <utility>

namespace ferrugo
{

namespace detail
{

template <class L, class R>
struct are_trivially_destructible
    : std::integral_constant<bool, std::is_trivially_destructible<L>::value && std::is_trivially_destructible<R>::value>
{
};

enum class either_side
{
    left,
    right
};

template <either_side Side, class T>
struct wrapper
{
    T value;
};

template <class Left, class Right, bool TriviallyDestructible>
struct either_base;

template <either_side Side>
struct in_place_t
{
};

using in_place_left_t = in_place_t<either_side::left>;
using in_place_right_t = in_place_t<either_side::right>;

constexpr auto in_place_left = in_place_left_t{};
constexpr auto in_place_right = in_place_right_t{};

template <class Left, class Right, bool>
struct either_base;

template <class Left, class Right>
struct either_base<Left, Right, true>
{
    either_side side_;
    union
    {
        Left left_;
        Right right_;
    };

    constexpr either_base() noexcept : side_{ either_side::left }, left_{ Left{} }
    {
    }

    template <class... Args>
    constexpr either_base(in_place_left_t, Args&&... args) : side_{ either_side::left }
                                                           , left_{ std::forward<Args>(args)... }
    {
    }

    template <class... Args>
    constexpr either_base(in_place_right_t, Args&&... args)
        : side_{ either_side::right }
        , right_{ std::forward<Args>(args)... }
    {
    }

    ~either_base() = default;
};

template <class Left, class Right>
struct either_base<Left, Right, false>
{
    either_side side_;
    union
    {
        Left left_;
        Right right_;
    };

    constexpr either_base() noexcept : side_{ either_side::left }, left_{ Left{} }
    {
    }

    template <class... Args>
    constexpr either_base(in_place_left_t, Args&&... args) : side_{ either_side::left }
                                                           , left_{ std::forward<Args>(args)... }
    {
    }

    template <class... Args>
    constexpr either_base(in_place_right_t, Args&&... args)
        : side_{ either_side::right }
        , right_{ std::forward<Args>(args)... }
    {
    }

    ~either_base()
    {
        if (side_ == either_side::left)
        {
            left_.~Left();
        }
        else
        {
            right_.~Right();
        }
    }
};

template <class T>
constexpr auto left(T&& value) -> wrapper<either_side::left, typename std::decay<T>::type>
{
    return { std::forward<T>(value) };
}

template <class T>
constexpr auto right(T&& value) -> wrapper<either_side::right, typename std::decay<T>::type>
{
    return { std::forward<T>(value) };
}

template <class Left, class Right>
class either : either_base<Left, Right, are_trivially_destructible<Left, Right>::value>
{
    using base_type = either_base<Left, Right, are_trivially_destructible<Left, Right>::value>;

    void do_reset()
    {
        if (this->side_ == either_side::left)
        {
            this->left_.~Left();
        }
        else
        {
            this->right_.~Right();
        }
    }

    void do_init()
    {
        emplace_left(Left{});
    }

public:
    using left_value_type = Left;
    using right_value_type = Right;

    template <either_side Side, class U>
    either(const wrapper<Side, U>& w) : base_type{ in_place_t<Side>{}, w.value }
    {
    }

    template <either_side Side, class U>
    either(wrapper<Side, U>&& w) : base_type{ in_place_t<Side>{}, std::move(w.value) }
    {
    }

    either() : base_type{ in_place_left, left_value_type{} }
    {
    }

    either(const either& other) : either{}
    {
        if (other.side_ == either_side::left)
        {
            emplace(in_place_left, other.left_value());
        }
        else
        {
            emplace(in_place_right, other.right_value());
        }
    }

    either(either&& other) : either{}
    {
        if (other.side_ == either_side::left)
        {
            emplace(in_place_left, std::move(other).left_value());
        }
        else
        {
            emplace(in_place_right, std::move(other).right_value());
        }
    }

    either& operator=(either other)
    {
        do_reset();
        this->side_ = other.side_;
        if (has_left_value())
        {
            emplace(in_place_left, std::move(other).left_value());
        }
        else
        {
            emplace(in_place_right, std::move(other).right_value());
        }
        return *this;
    }

    void reset()
    {
        do_reset();
        do_init();
    }

    template <class... Args>
    void emplace(in_place_left_t, Args&&... args)
    {
        do_reset();
        this->side_ = either_side::left;
        new (&this->left_) left_value_type{ std::forward<Args>(args)... };
    }

    template <class... Args>
    void emplace(in_place_right_t, Args&&... args)
    {
        do_reset();
        this->side_ = either_side::right;
        new (&this->right_) right_value_type{ std::forward<Args>(args)... };
    }

    bool has_left_value() const noexcept
    {
        return this->side_ == either_side::left;
    }

    bool has_right_value() const noexcept
    {
        return this->side_ == either_side::right;
    }

    const left_value_type& left_value() const&
    {
        return this->left_;
    }

    left_value_type& left_value() &
    {
        return this->left_;
    }

    left_value_type&& left_value() &&
    {
        return std::move(this->left_);
    }

    const right_value_type& right_value() const&
    {
        return this->right_;
    }

    right_value_type& right_value() &
    {
        return this->right_;
    }

    right_value_type&& right_value() &&
    {
        return std::move(this->right_);
    }

    // clang-format off

    template <class OnLeft, class OnRight>
    auto match(OnLeft&& on_left, OnRight&& on_right) const& -> RETURN(has_left_value() ? on_left(left_value()) : on_right(right_value()))

    template <class OnLeft, class OnRight>
    auto match(OnLeft&& on_left, OnRight&& on_right) && -> RETURN(has_left_value() ? on_left(std::move(*this).left_value()) : on_right(std::move(*this).right_value()))

    // clang-format on
};

}  // namespace detail

using detail::either;
using detail::left;
using detail::right;

}  // namespace ferrugo
