#pragma

#include <ferrugo/core/type_traits.hpp>
#include <iostream>
#include <sstream>

namespace ferrugo
{
namespace core
{

template <class T, class = void>
struct formatter;

template <class... Args>
std::ostream& format_to(std::ostream& os, Args&&... args)
{
    (formatter<std::decay_t<Args>>{}(os, std::forward<Args>(args)), ...);
    return os;
}

struct print_fn
{
    std::ostream& m_os;

    template <class... Args>
    void operator()(Args&&... args) const
    {
        format_to(m_os, std::forward<Args>(args)...);
    }
};

struct format_fn
{
    template <class... Args>
    auto operator()(Args&&... args) const -> std::string
    {
        std::stringstream ss;
        format_to(ss, std::forward<Args>(args)...);
        return ss.str();
    }
};

static constexpr inline auto format = format_fn{};

static constexpr inline auto cout = print_fn{ std::cout };
static constexpr inline auto cerr = print_fn{ std::cerr };
static constexpr inline auto print = cout;

template <class T>
struct formatter<T>
{
    void operator()(std::ostream& os, const T& item) const
    {
        if constexpr (has_ostream_operator<T>{})
        {
            os << item;
        }
        else
        {
            os << "<<unknown>>";
        }
    }
};

}  // namespace core

}  // namespace ferrugo
