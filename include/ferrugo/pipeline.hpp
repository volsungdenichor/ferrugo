#pragma once

#include <ferrugo/invoke.hpp>
#include <ferrugo/type_name.hpp>
#include <ferrugo/type_traits.hpp>
#include <iostream>
#include <tuple>

namespace ferrugo
{

namespace detail
{

template <std::size_t I, std::size_t N, require<(I < N)> = {}>
struct tag
{
};

}  // namespace detail

template <class... Pipes>
struct pipeline_t
{
    std::tuple<Pipes...> pipes_;

    pipeline_t(std::tuple<Pipes...> pipes) : pipes_(std::move(pipes))
    {
    }

    pipeline_t(Pipes... pipes) : pipes_({ std::move(pipes...) })
    {
    }

private:
    template <std::size_t I, class... Args>
    auto call(detail::tag<I, I + 1>, Args&&... args) const
        -> decltype(invoke(std::get<I>(pipes_), std::forward<Args>(args)...))
    {
        return invoke(std::get<I>(pipes_), std::forward<Args>(args)...);
    }

    template <std::size_t I, std::size_t N, class... Args>
    auto call(detail::tag<I, N>, Args&&... args) const
        -> decltype(call(detail::tag<I + 1, N>{}, invoke(std::get<I>(pipes_), std::forward<Args>(args)...)))
    {
        return call(detail::tag<I + 1, N>{}, invoke(std::get<I>(pipes_), std::forward<Args>(args)...));
    }

public:
    template <class... Args>
    auto operator()(Args&&... args) const -> decltype(call(detail::tag<0, sizeof...(Pipes)>{}, std::forward<Args>(args)...))
    {
        return call(detail::tag<0, sizeof...(Pipes)>{}, std::forward<Args>(args)...);
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
        return { std::move(pipe) };
    }

    template <class... Pipes>
    auto to_tuple(pipeline_t<Pipes...> pipe) const -> std::tuple<Pipes...>
    {
        return pipe.pipes_;
    }

    template <class... Pipes>
    auto from_tuple(std::tuple<Pipes...> tuple) const -> pipeline_t<Pipes...>
    {
        return { std::move(tuple) };
    }

public:
    template <class... Pipes>
    auto operator()(Pipes... pipes) const -> decltype(from_tuple(std::tuple_cat(to_tuple(std::move(pipes))...)))
    {
        return from_tuple(std::tuple_cat(to_tuple(std::move(pipes))...));
    }
};
}  // namespace detail

static constexpr auto make_pipeline = detail::make_pipeline_fn{};
static constexpr auto fn = make_pipeline;

template <class... L, class... R>
auto operator|(pipeline_t<L...> lhs, pipeline_t<R...> rhs) -> decltype(make_pipeline(std::move(lhs), std::move(rhs)))
{
    return make_pipeline(std::move(lhs), std::move(rhs));
}

template <class T, class... Pipes, require<!detail::is_pipeline<decay_t<T>>::value> = {}>
auto operator|(T&& item, const pipeline_t<Pipes...>& pipeline) -> decltype(pipeline(std::forward<T>(item)))
{
    return pipeline(std::forward<T>(item));
}

}  // namespace ferrugo
