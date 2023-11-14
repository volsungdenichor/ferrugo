#pragma once

#include <utility>

namespace ferrugo
{

#define DEFINE_BINARY_OP(name, op)                                                                        \
    struct name                                                                                           \
    {                                                                                                     \
        template <class L, class R>                                                                       \
        auto operator()(L&& lhs, R&& rhs) const -> decltype(std::forward<L>(lhs) op std::forward<R>(rhs)) \
        {                                                                                                 \
            return std::forward<L>(lhs) op std::forward<R>(rhs);                                          \
        }                                                                                                 \
    };

DEFINE_BINARY_OP(equal_to, ==)
DEFINE_BINARY_OP(not_equal_to, !=)
DEFINE_BINARY_OP(less, <)
DEFINE_BINARY_OP(less_equal, <=)
DEFINE_BINARY_OP(greater, >)
DEFINE_BINARY_OP(greater_equal, >=)

DEFINE_BINARY_OP(plus, +)
DEFINE_BINARY_OP(minus, -)
DEFINE_BINARY_OP(multiplies, *)
DEFINE_BINARY_OP(divides, /)
DEFINE_BINARY_OP(modulus, %)

DEFINE_BINARY_OP(logical_and, &&)
DEFINE_BINARY_OP(logical_or, ||)

DEFINE_BINARY_OP(bit_and, &)
DEFINE_BINARY_OP(bit_or, |)

#undef DEFINE_BINARY_OP

}  // namespace ferrugo