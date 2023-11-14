#pragma once

#include <ferrugo/index_sequence.hpp>
#include <ostream>
#include <tuple>

namespace ferrugo
{

template <class Tuple, std::size_t N>
void print_tuple(std::ostream&, const Tuple&, index_sequence<N, N>)
{
}

template <class Tuple, std::size_t I, std::size_t N>
void print_tuple(std::ostream& os, const Tuple& tuple, index_sequence<I, N>)
{
    os << (I != 0 ? ", " : "") << std::get<I>(tuple);
    print_tuple(os, tuple, index_sequence<I + 1, N>{});
}

}  // namespace ferrugo

namespace std
{

template <class... Args>
ostream& operator<<(ostream& os, const tuple<Args...>& item)
{
    os << "(";
    ferrugo::print_tuple(os, item, ferrugo::index_sequence<0, sizeof...(Args)>{});
    os << ")";
    return os;
}

template <class F, class S>
ostream& operator<<(ostream& os, const pair<F, S>& item)
{
    os << "(";
    ferrugo::print_tuple(os, item, ferrugo::index_sequence<0, 2>{});
    os << ")";
    return os;
}

}  // namespace std
