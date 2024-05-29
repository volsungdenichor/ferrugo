#pragma once

#include <iterator>
#include <type_traits>

namespace ferrugo
{

namespace core
{

namespace detail
{

template <class AlwaysVoid, template <class...> class Op, class... Args>
struct detector_impl : std::false_type
{
};

template <template <class...> class Op, class... Args>
struct detector_impl<std::void_t<Op<Args...>>, Op, Args...> : std::true_type
{
};

}  // namespace detail

template <template <class...> class Op, class... Args>
struct is_detected : detail::detector_impl<std::void_t<>, Op, Args...>
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

template <template <class> class C>
struct convertible_to
{
    template <class T, class = std::enable_if_t<C<T>::value>>
    operator T() const;
};

template <class T>
using is_assignable_impl = decltype(std::declval<T>() = std::declval<convertible_to_any>());

}  // namespace detail

template <bool C>
using require = typename std::enable_if<C, int>::type;

template<class T>
struct remove_cvref
{
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template< class T >
using remove_cvref_t = remove_cvref<T>::type;

template <class...>
struct always_false : std::false_type
{
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

template <class T>
struct type_identity
{
    using type = T;
};

template <class T>
using type_identity_t = typename type_identity<T>::type;

template <class T, template <class...> class... Ops>
struct satisfies_all : std::conjunction<is_detected<Ops, T>...>{};

template <class T, template <class...> class... Ops>
struct satisfies_any : std::disjunction<is_detected<Ops, T>...>{};

template <class T, template <class...> class Op>
using satisfies = satisfies_all<T, Op>;

template <class T, template <class...> class Op, class = void>
struct each_satisfies;

template <class... Types, template <class...> class Op>
struct each_satisfies<std::tuple<Types...>, Op> : std::conjunction<is_detected<Op, Types>...>{};

template <class... Types, template <class...> class Op>
struct each_satisfies<std::pair<Types...>, Op> : std::conjunction<is_detected<Op, Types>...>{};

template <class L, class R = L>
using addition = decltype(std::declval<L>() + std::declval<R>());

template <class L, class R = L>
using addition_assignment = decltype(std::declval<L&>() += std::declval<R>());

template <class L, class R = L>
using subtraction = decltype(std::declval<L>() - std::declval<R>());

template <class L, class R = L>
using subtraction_assignment = decltype(std::declval<L&>() -= std::declval<R>());

template <class L, class R = L>
using multiplication = decltype(std::declval<L>() * std::declval<R>());

template <class L, class R = L>
using multiplication_assignment = decltype(std::declval<L&>() *= std::declval<R>());

template <class L, class R = L>
using division = decltype(std::declval<L>() / std::declval<R>());

template <class L, class R = L>
using division_assignment = decltype(std::declval<L&>() /= std::declval<R>());

template <class L, class R = L>
using remainder = decltype(std::declval<L>() % std::declval<R>());

template <class L, class R = L>
using remainder_assignment = decltype(std::declval<L&>() %= std::declval<R>());

template <class L, class R = L>
using bitwise_left_shift = decltype(std::declval<L>() << std::declval<R>());

template <class L, class R = L>
using bitwise_left_shift_assignment = decltype(std::declval<L&>() <<= std::declval<R>());

template <class L, class R = L>
using bitwise_right_shift = decltype(std::declval<L>() >> std::declval<R>());

template <class L, class R = L>
using bitwise_right_shift_assignment = decltype(std::declval<L&>() >>= std::declval<R>());

template <class T>
using pre_increment = decltype(++std::declval<T&>());

template <class T>
using post_increment = decltype(std::declval<T&>()++);

template <class T>
using pre_decrement = decltype(--std::declval<T&>());

template <class T>
using post_decrement = decltype(std::declval<T&>()--);

template <class L, class R = L>
using equal_to = decltype(std::declval<L>() == std::declval<R>());

template <class L, class R = L>
using not_equal_to = decltype(std::declval<L>() != std::declval<R>());

template <class L, class R = L>
using less_than = decltype(std::declval<L>() < std::declval<R>());

template <class L, class R = L>
using greater_than = decltype(std::declval<L>() > std::declval<R>());

template <class L, class R = L>
using less_than_or_equal_to = decltype(std::declval<L>() <= std::declval<R>());

template <class L, class R = L>
using greater_than_or_equal_to = decltype(std::declval<L>() >= std::declval<R>());

template <class T, class N>
using subscript = decltype(std::declval<T&>()[std::declval<N>()]);

template <class T>
using indirection = decltype(*std::declval<T&>());

template <class T>
using has_ostream_operator = bitwise_left_shift<std::ostream, T>;

}  // namespace core

}  // namespace ferrugo
