#pragma once

#include <utility>

namespace ferrugo
{

namespace core
{

using std::integer_sequence;
using std::index_sequence;
using std::make_index_sequence;

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

}  // namespace core

}  // namespace ferrugo
