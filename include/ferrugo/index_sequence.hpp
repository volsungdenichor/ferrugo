#pragma once

#include <cstddef>

namespace ferrugo
{

template <class T, T... Ints>
struct integer_sequence
{
    typedef T value_type;

    static constexpr std::size_t size()
    {
        return sizeof...(Ints);
    }
};

template <std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

template <class T, std::size_t N, T... Is>
struct make_integer_sequence : make_integer_sequence<T, N - 1, N - 1, Is...>
{
};

template <class T, T... Is>
struct make_integer_sequence<T, 0, Is...> : integer_sequence<T, Is...>
{
};

template <std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

template <typename... T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

namespace detail
{

template <std::size_t N, class Seq>
struct offset_sequence;

template <std::size_t N, std::size_t... Ints>
struct offset_sequence<N, index_sequence<Ints...>>
{
    using type = index_sequence<(Ints + N)...>;
};

// example = index_sequence<3, 4, 5, 6>
template <class Seq1, class Seq>
struct cat_sequence;

template <std::size_t... Ints1, std::size_t... Ints2>
struct cat_sequence<index_sequence<Ints1...>, index_sequence<Ints2...>>
{
    using type = index_sequence<Ints1..., Ints2...>;
};

}  // namespace detail

template <std::size_t N, class Seq>
using offset_sequence = typename detail::offset_sequence<N, Seq>::type;
template <class Seq1, class Seq2>
using cat_sequence = typename detail::cat_sequence<Seq1, Seq2>::type;

template <std::size_t B, std::size_t E>
using sequence = offset_sequence<B, make_index_sequence<E - B>>;

}  // namespace ferrugo
