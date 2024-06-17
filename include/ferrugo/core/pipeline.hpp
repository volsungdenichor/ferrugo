#pragma once

#include <ferrugo/core/type_traits.hpp>
#include <functional>
#include <tuple>

namespace ferrugo
{

namespace core
{

template <class... Pipes>
struct pipeline_t
{
    std::tuple<Pipes...> m_pipes;

    pipeline_t(std::tuple<Pipes...> pipes) : m_pipes{ std::move(pipes) }
    {
    }

    pipeline_t(Pipes... pipes) : pipeline_t{ std::tuple<Pipes...>{ std::move(pipes)... } }
    {
    }

private:
    template <std::size_t I, class... Args, require<(I + 1) == sizeof...(Pipes)> = 0>
    auto call(Args&&... args) const -> decltype(std::invoke(std::get<I>(m_pipes), std::forward<Args>(args)...))
    {
        return std::invoke(std::get<I>(m_pipes), std::forward<Args>(args)...);
    }

    template <std::size_t I, class... Args, require<(I + 1) < sizeof...(Pipes)> = 0>
    auto call(Args&&... args) const -> decltype(call<I + 1>(std::invoke(std::get<I>(m_pipes), std::forward<Args>(args)...)))
    {
        return call<I + 1>(std::invoke(std::get<I>(m_pipes), std::forward<Args>(args)...));
    }

public:
    template <class... Args>
    auto operator()(Args&&... args) const -> decltype(call<0>(std::forward<Args>(args)...))
    {
        return call<0>(std::forward<Args>(args)...);
    }
};

namespace detail
{

template <class T>
struct is_pipeline : std::false_type
{
};

template <class... Args>
struct is_pipeline<pipeline_t<Args...>> : std::true_type
{
};

struct make_pipeline_fn
{
private:
    template <class Pipe>
    auto to_tuple(Pipe pipe) const -> std::tuple<Pipe>
    {
        return std::tuple<Pipe>{ std::move(pipe) };
    }

    template <class... Pipes>
    auto to_tuple(pipeline_t<Pipes...> pipe) const -> std::tuple<Pipes...>
    {
        return pipe.m_pipes;
    }

    template <class... Pipes>
    auto from_tuple(std::tuple<Pipes...> tuple) const -> pipeline_t<Pipes...>
    {
        return pipeline_t<Pipes...>{ std::move(tuple) };
    }

public:
    template <class... Pipes>
    auto operator()(Pipes... pipes) const -> decltype(from_tuple(std::tuple_cat(to_tuple(std::move(pipes))...)))
    {
        return from_tuple(std::tuple_cat(to_tuple(std::move(pipes))...));
    }
};
}  // namespace detail

static constexpr inline auto pipe = detail::make_pipeline_fn{};
static constexpr inline auto fn = pipe;

template <class... L, class... R>
auto operator|=(pipeline_t<L...> lhs, pipeline_t<R...> rhs) -> decltype(pipe(std::move(lhs), std::move(rhs)))
{
    return pipe(std::move(lhs), std::move(rhs));
}

template <class T, class... Pipes, require<!detail::is_pipeline<std::decay_t<T>>{}> = 0>
auto operator|=(T&& item, const pipeline_t<Pipes...>& p) -> decltype(p(std::forward<T>(item)))
{
    return p(std::forward<T>(item));
}

// template <class T, class... Pipes, require<!detail::is_pipeline<std::decay_t<T>>{}> = 0>
// auto operator>>=(T&& item, const pipeline_t<Pipes...>& p) -> decltype(std::apply(p, std::forward<T>(item)))
// {
//     return std::apply(p, std::forward<T>(item));
// }

}  // namespace core

}  // namespace ferrugo
