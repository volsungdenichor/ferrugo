#pragma once

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
    constexpr either_base(in_place_t<either_side::left>, Args&&... args)
        : side_{ either_side::left }
        , left_{ std::forward<Args>(args)... }
    {
    }

    template <class... Args>
    constexpr either_base(in_place_t<either_side::right>, Args&&... args)
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
    constexpr either_base(in_place_t<either_side::left>, Args&&... args)
        : side_{ either_side::left }
        , left_{ std::forward<Args>(args)... }
    {
    }

    template <class... Args>
    constexpr either_base(in_place_t<either_side::right>, Args&&... args)
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

}  // namespace detail

using detail::in_place_t;

template <class T>
constexpr auto left(T&& value) -> detail::wrapper<detail::either_side::left, typename std::decay<T>::type>
{
    return { std::forward<T>(value) };
}

template <class T>
constexpr auto right(T&& value) -> detail::wrapper<detail::either_side::right, typename std::decay<T>::type>
{
    return { std::forward<T>(value) };
}

template <class Left, class Right>
struct either : detail::either_base<Left, Right, detail::are_trivially_destructible<Left, Right>::value>
{
    using base_type = detail::either_base<Left, Right, detail::are_trivially_destructible<Left, Right>::value>;

    void do_reset()
    {
        if (this->side_ == detail::either_side::left)
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

    void reset()
    {
        do_reset();
        do_init();
    }

    template <class... Args>
    void emplace(in_place_t<detail::either_side::left>, Args&&... args)
    {
        do_reset();
        this->side_ = detail::either_side::left;
        new (&this->left_) Left{ std::forward<Args>(args)... };
    }

    template <class... Args>
    void emplace(in_place_t<detail::either_side::right>, Args&&... args)
    {
        do_reset();
        this->side_ = detail::either_side::right;
        new (&this->right_) Right{ std::forward<Args>(args)... };
    }

    template <detail::either_side Side, class U>
    either(const detail::wrapper<Side, U>& w) : base_type{ in_place_t<Side>{}, w.value }
    {
    }

    template <detail::either_side Side, class U>
    either(detail::wrapper<Side, U>&& w) : base_type{ in_place_t<Side>{}, std::move(w.value) }
    {
    }

    either() : base_type{ in_place_t<detail::either_side::left>{}, Left{} }
    {
    }

    either(const either& other) : either{}
    {
        if (other.side_ == detail::either_side::left)
        {
            emplace(in_place_t<detail::either_side::left>{}, other.left_value());
        }
        else
        {
            emplace(in_place_t<detail::either_side::right>{}, other.right_value());
        }
    }

    either(either&& other) : either{}
    {
        if (other.side_ == detail::either_side::left)
        {
            emplace(in_place_t<detail::either_side::left>{}, std::move(other).left_value());
        }
        else
        {
            emplace(in_place_t<detail::either_side::right>{}, std::move(other).right_value());
        }
    }

    either& operator=(either other)
    {
        do_reset();
        this->side_ = other.side_;
        if (has_left_value())
        {
            emplace(in_place_t<detail::either_side::left>{}, std::move(other).left_value());
        }
        else
        {
            emplace(in_place_t<detail::either_side::right>{}, std::move(other).right_value());
        }
        return *this;
    }

    bool has_left_value() const
    {
        return this->side_ == detail::either_side::left;
    }

    bool has_right_value() const
    {
        return this->side_ == detail::either_side::right;
    }

    const Left& left_value() const&
    {
        return this->left_;
    }

    Left& left_value() &
    {
        return this->left_;
    }

    Left&& left_value() &&
    {
        return std::move(this->left_);
    }

    const Right& right_value() const&
    {
        return this->right_;
    }

    Right& right_value() &
    {
        return this->right_;
    }

    Right&& right_value() &&
    {
        return std::move(this->right_);
    }
};

}  // namespace ferrugo