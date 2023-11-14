#pragma once

#include <functional>

#include "macros.hpp"

namespace ferrugo
{

namespace detail
{

template <class Type, class Base, class Derived>
auto invoke(Type Base::*field, Derived&& arg) -> RETURN(std::mem_fn(field)(std::forward<Derived>(arg)));

template <class F, class... Args>
auto invoke(F f, Args&&... args) -> RETURN(std::ref(f)(std::forward<Args>(args)...));

}  // namespace detail

template <class F, class... Args>
auto invoke(F&& func, Args&&... args) -> RETURN(detail::invoke(std::forward<F>(func), std::forward<Args>(args)...));

}  // namespace ferrugo
