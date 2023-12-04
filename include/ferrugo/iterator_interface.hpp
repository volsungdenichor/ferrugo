#pragma once

#include <ferrugo/type_traits.hpp>

namespace ferrugo
{

namespace detail
{

template <class T>
using has_deref_impl = decltype(std::declval<T>().deref());

template <class T>
using has_inc_impl = decltype(std::declval<T>().inc());

template <class T>
using has_dec_impl = decltype(std::declval<T>().dec());

template <class T>
using has_advance_impl = decltype(std::declval<T>().advance(std::declval<convertible_to<std::is_integral>>()));

template <class T>
using has_is_equal_impl = decltype(std::declval<T>().is_equal(std::declval<T>()));

template <class T>
using has_is_less_impl = decltype(std::declval<T>().is_less(std::declval<T>()));

template <class T>
using has_distance_to_impl = decltype(std::declval<T>().distance_to(std::declval<T>()));

template <class T, class = void_t<>>
struct difference_type_impl
{
    using type = std::ptrdiff_t;
};

template <class T>
struct difference_type_impl<T, void_t<has_distance_to_impl<T>>>
{
    using type = decltype(std::declval<T>().distance_to(std::declval<T>()));
};

}  // namespace detail

template <class T>
using difference_type = typename detail::difference_type_impl<T>::type;

template <class T>
struct has_deref : is_detected<detail::has_deref_impl, T>
{
};

template <class T>
struct has_inc : is_detected<detail::has_inc_impl, T>
{
};

template <class T>
struct has_dec : is_detected<detail::has_dec_impl, T>
{
};

template <class T>
struct has_advance : is_detected<detail::has_advance_impl, T>
{
};

template <class T>
struct has_is_equal : is_detected<detail::has_is_equal_impl, T>
{
};

template <class T>
struct has_distance_to : is_detected<detail::has_distance_to_impl, T>
{
};

template <class T>
struct pointer_proxy
{
    T item;

    T* operator->()
    {
        return std::addressof(item);
    }
};

template <class Impl>
struct iterator_interface
{
    Impl impl_;

    template <class... Args, require<std::is_constructible<Args...>::value> = {}>
    iterator_interface(Args&&... args) : impl_{ std::forward<Args>(args)... }
    {
    }

    iterator_interface() = default;
    iterator_interface(const iterator_interface&) = default;
    iterator_interface(iterator_interface&&) = default;

    iterator_interface& operator=(iterator_interface other)
    {
        std::swap(impl_, other.impl_);
        return *this;
    }

    static_assert(has_deref<Impl>::value, ".deref required");

    using reference = decltype(impl_.deref());

private:
    template <class R = reference, require<std::is_reference<R>::value> = {}>
    auto get_pointer() const -> typename std::add_pointer<reference>::type
    {
        return std::addressof(**this);
    }

    template <class R = reference, require<!std::is_reference<R>::value> = {}>
    auto get_pointer() const -> pointer_proxy<reference>
    {
        return { **this };
    }

    template <class I = Impl, require<has_advance<I>::value && !has_inc<I>::value> = {}>
    void inc()
    {
        impl_.advance(+1);
    }

    template <class I = Impl, require<has_advance<I>::value && has_inc<I>::value> = {}>
    void inc()
    {
        impl_.inc();
    }

    template <class I = Impl, require<!has_advance<I>::value && has_inc<I>::value> = {}>
    void inc()
    {
        impl_.inc();
    }

    template <class I = Impl, require<has_advance<I>::value && !has_dec<I>::value> = {}>
    void dec()
    {
        impl_.advance(-1);
    }

    template <class I = Impl, require<has_advance<I>::value && has_dec<I>::value> = {}>
    void dec()
    {
        impl_.dec();
    }

    template <class I = Impl, require<!has_advance<I>::value && has_dec<I>::value> = {}>
    void dec()
    {
        impl_.dec();
    }

    template <class I = Impl, require<!has_is_equal<I>::value && has_distance_to<I>::value> = {}>
    bool is_equal(const Impl& other) const
    {
        return impl_.distance_to(other) == 0;
    }

    template <class I = Impl, require<has_is_equal<I>::value && has_distance_to<I>::value> = {}>
    bool is_equal(const Impl& other) const
    {
        return impl_.is_equal(other);
    }

    template <class I = Impl, require<has_is_equal<I>::value && !has_distance_to<I>::value> = {}>
    bool is_equal(const Impl& other) const
    {
        return impl_.is_equal(other);
    }

public:
    reference operator*() const
    {
        return impl_.deref();
    }

    auto operator->() const -> decltype(get_pointer())
    {
        return get_pointer();
    }

    template <class I = Impl, require<has_advance<I>::value || has_inc<I>::value> = {}>
    iterator_interface& operator++()
    {
        inc();
        return *this;
    }

    template <class I = Impl, require<has_advance<I>::value || has_inc<I>::value> = {}>
    iterator_interface operator++(int)
    {
        iterator_interface tmp{ *this };
        ++(*this);
        return tmp;
    }

    template <class I = Impl, require<has_advance<I>::value || has_dec<I>::value> = {}>
    iterator_interface& operator--()
    {
        dec();
        return *this;
    }

    template <class I = Impl, require<has_advance<I>::value || has_dec<I>::value> = {}>
    iterator_interface operator--(int)
    {
        iterator_interface tmp{ *this };
        --(*this);
        return tmp;
    }

    template <class D, class I = Impl, require<has_advance<I>::value && std::is_integral<D>::value> = {}>
    friend iterator_interface& operator+=(iterator_interface& it, D offset)
    {
        static_assert(has_advance<Impl>::value, ".advance required");
        it.impl_.advance(offset);
        return it;
    }

    template <class D, class I = Impl, require<has_advance<I>::value && std::is_integral<D>::value> = {}>
    friend iterator_interface operator+(iterator_interface it, D offset)
    {
        return it += offset;
    }

    template <class D, class I = Impl, require<has_advance<I>::value && std::is_integral<D>::value> = {}>
    friend iterator_interface operator+(D offset, iterator_interface it)
    {
        return it + offset;
    }

    template <class D, class I = Impl, require<has_advance<I>::value && std::is_integral<D>::value> = {}>
    friend iterator_interface& operator-=(iterator_interface& it, D offset)
    {
        return it += -offset;
    }

    template <class D, class I = Impl, require<has_advance<I>::value && std::is_integral<D>::value> = {}>
    friend iterator_interface operator-(iterator_interface it, D offset)
    {
        return it -= offset;
    }

    template <class D, class I = Impl, require<has_advance<I>::value && std::is_integral<D>::value> = {}>
    reference operator[](D offset) const
    {
        return *(*this + offset);
    }

    template <class I = Impl, require<has_is_equal<I>::value || has_distance_to<I>::value> = {}>
    friend bool operator==(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        static_assert(has_is_equal<Impl>::value || has_distance_to<Impl>::value, ".is_equal or .distance_to required");
        return lhs.is_equal(rhs.impl_);
    }

    template <class I = Impl, require<has_is_equal<I>::value || has_distance_to<I>::value> = {}>
    friend bool operator!=(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return !(lhs == rhs);
    }

    template <class I = Impl, require<has_distance_to<I>::value> = {}>
    friend bool operator<(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return lhs.impl_.distance_to(rhs.impl_) > 0;
    }

    template <class I = Impl, require<has_distance_to<I>::value> = {}>
    friend bool operator>(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return rhs < lhs;
    }

    template <class I = Impl, require<has_distance_to<I>::value> = {}>
    friend bool operator<=(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return !(lhs > rhs);
    }

    template <class I = Impl, require<has_distance_to<I>::value> = {}>
    friend bool operator>=(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return !(lhs < rhs);
    }

    template <class I = Impl, require<has_distance_to<I>::value> = {}>
    friend auto operator-(const iterator_interface& lhs, const iterator_interface& rhs) -> difference_type<Impl>
    {
        return rhs.impl_.distance_to(lhs.impl_);
    }
};

template <class T, class = void_t<>>
struct iterator_category_impl
{
    using type = typename std::conditional<
        has_advance<T>::value && has_distance_to<T>::value,
        std::random_access_iterator_tag,
        typename std::conditional<
            has_dec<T>::value || has_advance<T>::value,
            std::bidirectional_iterator_tag,
            std::forward_iterator_tag>::type>::type;
};

template <class T>
struct iterator_category_impl<T, void_t<typename T::iterator_category>>
{
    using type = typename T::iterator_category;
};

}  // namespace ferrugo

namespace std
{

template <class Impl>
struct iterator_traits<::ferrugo::iterator_interface<Impl>>
{
    using it = ::ferrugo::iterator_interface<Impl>;
    using reference = decltype(std::declval<it>().operator*());
    using pointer = decltype(std::declval<it>().operator->());
    using value_type = typename std::decay<reference>::type;
    using difference_type = typename ::ferrugo::difference_type<Impl>;
    using iterator_category = typename ::ferrugo::iterator_category_impl<Impl>::type;
};

}  // namespace std
