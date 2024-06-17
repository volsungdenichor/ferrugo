#pragma once

#include <type_traits>

namespace ferrugo
{
namespace core
{

template <class T>
struct pass_by_value : std::bool_constant<(sizeof(T) < 2 * sizeof(void*)) && std::is_trivially_default_constructible<T>{}>
{
};

template <class T>
struct result_t : std::conditional_t<pass_by_value<T>{}, T, const T&>
{
};

template <class T>
struct in_t : std::conditional_t<pass_by_value<T>{}, const T, const T&>
{
};

}  // namespace core
}  // namespace ferrugo