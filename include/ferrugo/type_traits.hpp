#pragma once

#include <iterator>
#include <type_traits>

namespace ferrugo
{

template <bool B>
using bool_constant = std::integral_constant<bool, B>;

template <class...>
struct always_false : std::false_type{};

namespace detail
{

template <class... Args>
struct make_void
{
    using type = void;
};

template <class... Args>
using void_t = typename make_void<Args...>::type;

template <class AlwaysVoid, template <class...> class Op, class... Args>
struct detector_impl : std::false_type
{
};

template <template <class...> class Op, class... Args>
struct detector_impl<void_t<Op<Args...>>, Op, Args...> : std::true_type
{
};

}  // namespace detail

using detail::void_t;

template <template <class...> class Op, class... Args>
struct is_detected : detail::detector_impl<void_t<>, Op, Args...>
{
};

template <class T>
using iterator_t = decltype(std::begin(std::declval<T&>()));

template <class T>
using iter_category_t = typename std::iterator_traits<T>::iterator_category;

template <class T>
using iter_reference_t = typename std::iterator_traits<T>::reference;

template <class T>
using iter_value_t = typename std::iterator_traits<T>::value_type;

template <class T>
using iter_difference_t = typename std::iterator_traits<T>::difference_type;

template <class T>
using range_category_t = iter_category_t<iterator_t<T>>;

template <class T>
using range_reference_t = iter_reference_t<iterator_t<T>>;

template <class T>
using range_value_t = iter_value_t<iterator_t<T>>;

template <class T>
using range_difference_t = iter_difference_t<iterator_t<T>>;

namespace detail
{

template <class Category, class T>
struct iterator_of_category : std::is_base_of<Category, typename std::iterator_traits<T>::iterator_category>
{
};

struct convertible_to_any
{
    template <class T>
    operator T() const;
};

template <class T>
using is_assignable_impl = decltype(std::declval<T>() = std::declval<convertible_to_any>());

template <class T>
struct is_assignable : is_detected<is_assignable_impl, T>
{
};

}  // namespace detail

template <template <class> class C>
struct convertible_to
{
    template <class T, class = typename std::enable_if<C<T>::value>::type>
    operator T() const;
};

template <class T>
struct is_input_iterator : detail::iterator_of_category<std::input_iterator_tag, T>
{
};

template <class T>
struct is_forward_iterator : detail::iterator_of_category<std::forward_iterator_tag, T>
{
};

template <class T>
struct is_bidirectional_iterator : detail::iterator_of_category<std::bidirectional_iterator_tag, T>
{
};

template <class T>
struct is_random_access_iterator : detail::iterator_of_category<std::random_access_iterator_tag, T>
{
};

template <class T, class = void_t<>>
struct is_output_iterator : bool_constant<is_input_iterator<T>::value && detail::is_assignable<iter_reference_t<T>>::value>
{
};

template <class T>
struct is_input_range : is_input_iterator<iterator_t<T>>
{
};

template <class T>
struct is_forward_range : is_forward_iterator<iterator_t<T>>
{
};

template <class T>
struct is_bidirectional_range : is_bidirectional_iterator<iterator_t<T>>
{
};

template <class T>
struct is_random_access_range : is_random_access_iterator<iterator_t<T>>
{
};

}  // namespace ferrugo
