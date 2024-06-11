#pragma once

#include <ferrugo/core/iterator_interface.hpp>
#include <ferrugo/core/optional.hpp>
#include <ferrugo/core/pipeline.hpp>
#include <functional>
#include <limits>

namespace ferrugo
{
namespace core
{

namespace detail
{

template <class T>
using next_fn_t = std::function<optional<T>()>;

template <class T>
struct empty_sequence
{
    auto operator()() const -> optional<T>
    {
        return {};
    }
};

template <class T>
struct sequence_base
{
    using next_fn_type = next_fn_t<T>;

    struct iter
    {
        next_fn_type m_next;
        optional<T> m_current;
        std::ptrdiff_t m_index;

        iter(next_fn_type next) : m_next{ std::move(next) }, m_current{ m_next() }, m_index{ 0 }
        {
        }

        iter() : m_next{}, m_current{}, m_index(std::numeric_limits<std::ptrdiff_t>::max())
        {
        }

        T deref() const
        {
            return *m_current;
        }

        void inc()
        {
            m_current = m_next();
            ++m_index;
        }

        bool is_equal(const iter& other) const
        {
            return (!m_current && !other.m_current) || (m_current && other.m_current && m_index == other.m_index);
        }
    };

    using iterator = iterator_interface<iter>;

    sequence_base(next_fn_type next) : m_next(std::move(next))
    {
    }

    sequence_base() : sequence_base(empty_sequence<T>{})
    {
    }

    iterator begin() const
    {
        return iterator(m_next);
    }

    iterator end() const
    {
        return iterator();
    }

    const next_fn_type& get_next_fn() const
    {
        return m_next;
    }

    next_fn_type m_next;
};

template <class T>
struct sequence : range_interface<detail::sequence_base<T>>
{
    using base_type = range_interface<detail::sequence_base<T>>;
    using base_type::base_type;

    const detail::sequence_base<T>::next_fn_type& get_next_fn() const
    {
        return base_type::get_impl().get_next_fn();
    }
};

struct transform_maybe_fn
{
    template <class Func, class In, class Out>
    struct next_function
    {
        Func m_func;
        next_fn_t<In> m_next;

        auto operator()() const -> optional<Out>
        {
            while (true)
            {
                optional<In> res = m_next();
                if (!res)
                {
                    break;
                }

                optional<Out> r = std::invoke(m_func, *res);
                if (r)
                {
                    return r;
                }
            }
            return {};
        }
    };

    template <class Func>
    struct impl
    {
        Func m_func;

        template <class T, class Out = optional_underlying_type_t<std::invoke_result_t<Func, T>>>
        auto operator()(const sequence<T>& s) const -> sequence<Out>
        {
            return sequence<Out>{ next_function<Func, T, Out>{ m_func, s.get_next_fn() } };
        }
    };

    template <class Func>
    auto operator()(Func&& func) const -> impl<std::decay_t<Func>>
    {
        return impl<std::decay_t<Func>>{ std::forward<Func>(func) };
    }
};

struct transform_fn
{
    template <class Func, class In, class Out>
    struct next_function
    {
        Func m_func;
        next_fn_t<In> m_next;

        auto operator()() const -> optional<Out>
        {
            optional<In> res = m_next();
            if (!res)
            {
                return {};
            }
            return std::invoke(m_func, *res);
        }
    };

    template <class Func>
    struct impl
    {
        Func m_func;

        template <class T, class Out = std::invoke_result_t<Func, T>>
        auto operator()(const sequence<T>& s) const -> sequence<Out>
        {
            return sequence<Out>{ next_function<Func, T, Out>{ m_func, s.get_next_fn() } };
        }
    };

    template <class Func>
    auto operator()(Func&& func) const -> impl<std::decay_t<Func>>
    {
        return impl<std::decay_t<Func>>{ std::forward<Func>(func) };
    }
};

struct filter_fn
{
    template <class Pred, class In>
    struct next_function
    {
        Pred m_pred;
        next_fn_t<In> m_next;

        auto operator()() const -> optional<In>
        {
            while (true)
            {
                optional<In> res = m_next();
                if (!res)
                {
                    break;
                }

                if (std::invoke(m_pred, *res))
                {
                    return res;
                }
            }
            return {};
        }
    };

    template <class Pred>
    struct impl
    {
        Pred m_pred;

        template <class T>
        auto operator()(const sequence<T>& s) const -> sequence<T>
        {
            return sequence<T>{ next_function<Pred, T>{ m_pred, s.get_next_fn() } };
        }
    };

    template <class Pred>
    auto operator()(Pred&& pred) const -> impl<std::decay_t<Pred>>
    {
        return impl<std::decay_t<Pred>>{ std::forward<Pred>(pred) };
    }
};

struct take_fn
{
    template <class In>
    struct next_function
    {
        mutable std::ptrdiff_t m_count;
        next_fn_t<In> m_next;

        auto operator()() const -> optional<In>
        {
            if (m_count-- > 0)
            {
                return m_next();
            }
            return {};
        }
    };

    struct impl
    {
        std::ptrdiff_t m_count;

        template <class T>
        auto operator()(const sequence<T>& s) const -> sequence<T>
        {
            return sequence<T>{ next_function<T>{ m_count, s.get_next_fn() } };
        }
    };

    auto operator()(std::ptrdiff_t count) const -> impl
    {
        return impl{ count };
    }
};

struct drop_fn
{
    template <class In>
    struct next_function
    {
        mutable std::ptrdiff_t m_count;
        next_fn_t<In> m_next;

        auto operator()() const -> optional<In>
        {
            while (m_count-- > 0)
            {
                m_next();
            }
            return m_next();
        }
    };

    struct impl
    {
        std::ptrdiff_t m_count;

        template <class T>
        auto operator()(const sequence<T>& s) const -> sequence<T>
        {
            return sequence<T>{ next_function<T>{ m_count, s.get_next_fn() } };
        }
    };

    auto operator()(std::ptrdiff_t count) const -> impl
    {
        return impl{ count };
    }
};

struct step_fn
{
    template <class In>
    struct next_function
    {
        std::ptrdiff_t m_count;
        next_fn_t<In> m_next;
        mutable std::ptrdiff_t m_index = 0;

        auto operator()() const -> optional<In>
        {
            while (true)
            {
                optional<In> n = m_next();
                if (!n)
                {
                    break;
                }
                if (m_index++ % m_count == 0)
                {
                    return n;
                }
            }
            return {};
        }
    };

    struct impl
    {
        std::ptrdiff_t m_count;

        template <class T>
        auto operator()(const sequence<T>& s) const -> sequence<T>
        {
            return sequence<T>{ next_function<T>{ m_count, s.get_next_fn() } };
        }
    };

    auto operator()(std::ptrdiff_t count) const -> impl
    {
        return impl{ count };
    }
};

struct take_while_fn
{
    template <class Pred, class In>
    struct next_function
    {
        Pred m_pred;
        next_fn_t<In> m_next;

        auto operator()() const -> optional<In>
        {
            optional<In> res = m_next();
            if (!(res && std::invoke(m_pred, *res)))
            {
                return {};
            }
            return res;
        }
    };

    template <class Pred>
    struct impl
    {
        Pred m_pred;

        template <class T>
        auto operator()(const sequence<T>& s) const -> sequence<T>
        {
            return sequence<T>{ next_function<Pred, T>{ m_pred, s.get_next_fn() } };
        }
    };

    template <class Pred>
    auto operator()(Pred&& pred) const -> impl<std::decay_t<Pred>>
    {
        return impl<std::decay_t<Pred>>{ std::forward<Pred>(pred) };
    }
};

struct drop_while_fn
{
    template <class Pred, class In>
    struct next_function
    {
        Pred m_pred;
        next_fn_t<In> m_next;
        mutable bool m_init = true;

        auto operator()() const -> optional<In>
        {
            if (m_init)
            {
                while (true)
                {
                    optional<In> res = m_next();
                    if (!res)
                    {
                        return {};
                    }
                    if (!std::invoke(m_pred, *res))
                    {
                        break;
                    }
                }
                m_init = false;
            }
            return m_next();
        }
    };

    template <class Pred>
    struct impl
    {
        Pred m_pred;

        template <class T>
        auto operator()(const sequence<T>& s) const -> sequence<T>
        {
            return sequence<T>{ next_function<Pred, T>{ m_pred, s.get_next_fn() } };
        }
    };

    template <class Pred>
    auto operator()(Pred&& pred) const -> impl<std::decay_t<Pred>>
    {
        return impl<std::decay_t<Pred>>{ std::forward<Pred>(pred) };
    }
};

struct enumerate_fn
{
    template <class In>
    struct next_function
    {
        mutable std::ptrdiff_t m_index;
        next_fn_t<In> m_next;

        auto operator()() const -> optional<std::tuple<std::ptrdiff_t, In>>
        {
            optional<In> n = m_next();
            if (!n)
            {
                return {};
            }
            return std::tuple<std::ptrdiff_t, In>{ m_index++, *n };
        }
    };

    struct impl
    {
        std::ptrdiff_t m_init;

        template <class T>
        auto operator()(const sequence<T>& s) const -> sequence<std::tuple<std::ptrdiff_t, T>>
        {
            return sequence<std::tuple<std::ptrdiff_t, T>>{ next_function<T>{ m_init, s.get_next_fn() } };
        }
    };

    auto operator()(std::ptrdiff_t init = 0) const -> impl
    {
        return impl{ init };
    }
};

struct zip_transform_fn
{
    template <class Func, class Out, class In0, class In1, class In2 = void, class In3 = void>
    struct next_function;

    template <class Func, class Out, class In0, class In1, class In2, class In3>
    struct next_function
    {
        Func m_func;
        next_fn_t<In0> m_next0;
        next_fn_t<In1> m_next1;
        next_fn_t<In2> m_next2;
        next_fn_t<In3> m_next3;

        auto operator()() const -> optional<Out>
        {
            optional<In0> n0 = m_next0();
            optional<In1> n1 = m_next1();
            optional<In2> n2 = m_next2();
            optional<In3> n3 = m_next3();
            if (n0 && n1 && n2 && n3)
            {
                return std::invoke(m_func, *n0, *n1, *n2, *n3);
            }
            return {};
        }
    };

    template <class Func, class Out, class In0, class In1, class In2>
    struct next_function<Func, Out, In0, In1, In2, void>
    {
        Func m_func;
        next_fn_t<In0> m_next0;
        next_fn_t<In1> m_next1;
        next_fn_t<In2> m_next2;

        auto operator()() const -> optional<Out>
        {
            optional<In0> n0 = m_next0();
            optional<In1> n1 = m_next1();
            optional<In2> n2 = m_next2();
            if (n0 && n1 && n2)
            {
                return std::invoke(m_func, *n0, *n1, *n2);
            }
            return {};
        }
    };

    template <class Func, class Out, class In0, class In1>
    struct next_function<Func, Out, In0, In1, void, void>
    {
        Func m_func;
        next_fn_t<In0> m_next0;
        next_fn_t<In1> m_next1;

        auto operator()() const -> optional<Out>
        {
            optional<In0> n0 = m_next0();
            optional<In1> n1 = m_next1();
            if (n0 && n1)
            {
                return std::invoke(m_func, *n0, *n1);
            }
            return {};
        }
    };

    template <class Func, class T0, class T1, class T2, class T3, class Out = std::invoke_result_t<Func, T0, T1, T2, T3>>
    auto operator()(
        Func func, const sequence<T0>& s0, const sequence<T1>& s1, const sequence<T2>& s2, const sequence<T3>& s3) const
        -> sequence<Out>
    {
        return sequence<Out>{ next_function<Func, Out, T0, T1, T2, T3>{
            func, s0.get_next_fn(), s1.get_next_fn(), s2.get_next_fn(), s3.get_next_fn() } };
    }

    template <class Func, class T0, class T1, class T2, class Out = std::invoke_result_t<Func, T0, T1, T2>>
    auto operator()(Func func, const sequence<T0>& s0, const sequence<T1>& s1, const sequence<T2>& s2) const -> sequence<Out>
    {
        return sequence<Out>{ next_function<Func, Out, T0, T1, T2>{
            func, s0.get_next_fn(), s1.get_next_fn(), s2.get_next_fn() } };
    }

    template <class Func, class T0, class T1, class Out = std::invoke_result_t<Func, T0, T1>>
    auto operator()(Func func, const sequence<T0>& s0, const sequence<T1>& s1) const -> sequence<Out>
    {
        return sequence<Out>{ next_function<Func, Out, T0, T1>{ func, s0.get_next_fn(), s1.get_next_fn() } };
    }
};

struct zip_fn
{
    template <class In0, class In1, class In2 = void, class In3 = void>
    struct next_function;

    template <class In0, class In1, class In2, class In3>
    struct next_function
    {
        next_fn_t<In0> m_next0;
        next_fn_t<In1> m_next1;
        next_fn_t<In2> m_next2;
        next_fn_t<In3> m_next3;

        auto operator()() const -> optional<std::tuple<In0, In1, In2, In3>>
        {
            optional<In0> n0 = m_next0();
            optional<In1> n1 = m_next1();
            optional<In2> n2 = m_next2();
            optional<In3> n3 = m_next3();
            if (n0 && n1 && n2 && n3)
            {
                return std::tuple<In0, In1, In2, In3>{ *n0, *n1, *n2, *n3 };
            }
            return {};
        }
    };

    template <class In0, class In1, class In2>
    struct next_function<In0, In1, In2, void>
    {
        next_fn_t<In0> m_next0;
        next_fn_t<In1> m_next1;
        next_fn_t<In2> m_next2;

        auto operator()() const -> optional<std::tuple<In0, In1, In2>>
        {
            optional<In0> n0 = m_next0();
            optional<In1> n1 = m_next1();
            optional<In2> n2 = m_next2();
            if (n0 && n1 && n2)
            {
                return std::tuple<In0, In1, In2>{ *n0, *n1, *n2 };
            }
            return {};
        }
    };

    template <class In0, class In1>
    struct next_function<In0, In1, void, void>
    {
        next_fn_t<In0> m_next0;
        next_fn_t<In1> m_next1;

        auto operator()() const -> optional<std::tuple<In0, In1>>
        {
            optional<In0> n0 = m_next0();
            optional<In1> n1 = m_next1();
            if (n0 && n1)
            {
                return std::tuple<In0, In1>{ *n0, *n1 };
            }
            return {};
        }
    };

    template <class T0, class T1, class T2, class T3, class Out = std::tuple<T0, T1, T2, T3>>
    auto operator()(const sequence<T0>& s0, const sequence<T1>& s1, const sequence<T2>& s2, const sequence<T3>& s3) const
        -> sequence<Out>
    {
        return sequence<Out>{ next_function<T0, T1, T2, T3>{
            s0.get_next_fn(), s1.get_next_fn(), s2.get_next_fn(), s3.get_next_fn() } };
    }

    template <class T0, class T1, class T2, class Out = std::tuple<T0, T1, T2>>
    auto operator()(const sequence<T0>& s0, const sequence<T1>& s1, const sequence<T2>& s2) const -> sequence<Out>
    {
        return sequence<Out>{ next_function<T0, T1, T2>{ s0.get_next_fn(), s1.get_next_fn(), s2.get_next_fn() } };
    }

    template <class T0, class T1, class Out = std::tuple<T0, T1>>
    auto operator()(const sequence<T0>& s0, const sequence<T1>& s1) const -> sequence<Out>
    {
        return sequence<Out>{ next_function<T0, T1>{ s0.get_next_fn(), s1.get_next_fn() } };
    }
};

struct chain_fn
{
    template <class In>
    struct next_function
    {
        next_fn_t<In> m_first;
        next_fn_t<In> m_second;
        mutable bool m_first_finished = false;

        auto operator()() const -> optional<In>
        {
            if (!m_first_finished)
            {
                optional<In> n = m_first();
                if (n)
                {
                    return n;
                }
                else
                {
                    m_first_finished = true;
                }
            }
            return m_second();
        }
    };

    template <class T>
    auto operator()(const sequence<T>& lhs, const sequence<T>& rhs) const -> sequence<T>
    {
        return sequence<T>{ next_function<T>{ lhs.get_next_fn(), rhs.get_next_fn() } };
    }
};

}  // namespace detail

using detail::sequence;

static constexpr inline auto zip_transform = detail::zip_transform_fn{};
static constexpr inline auto zip = detail::zip_fn{};

static constexpr inline auto chain = detail::chain_fn{};

static constexpr inline auto transform_maybe = detail::transform_maybe_fn{};
static constexpr inline auto transform = detail::transform_fn{};
static constexpr inline auto filter = detail::filter_fn{};
static constexpr inline auto take = detail::take_fn{};
static constexpr inline auto drop = detail::drop_fn{};
static constexpr inline auto take_while = detail::take_while_fn{};
static constexpr inline auto drop_while = detail::drop_while_fn{};
static constexpr inline auto step = detail::step_fn{};
static constexpr inline auto enumerate = detail::enumerate_fn{};

}  // namespace core
}  // namespace ferrugo
