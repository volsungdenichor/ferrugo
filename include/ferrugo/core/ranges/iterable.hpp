
#pragma once

#include <memory>

namespace ferrugo
{

namespace core
{

namespace detail
{

template <class It>
struct i_range
{
    virtual ~i_range() = default;
    virtual std::unique_ptr<It> begin() const = 0;
    virtual std::unique_ptr<It> end() const = 0;
};

template <class T>
struct i_cloneable
{
    virtual ~i_cloneable() = default;
    virtual std::unique_ptr<T> clone() const = 0;
};

}  // namespace detail
}  // namespace core
}  // namespace ferrugo
