#include <ferrugo/either.hpp>
#include <ferrugo/index_sequence.hpp>
#include <ferrugo/invoke.hpp>
#include <ferrugo/iterable.hpp>
#include <ferrugo/operators.hpp>
#include <ferrugo/optional.hpp>
#include <ferrugo/std_ostream.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

template <class T>
void print(const ferrugo::iterable<T>& range)
{
    for (const auto item : range)
    {
        std::cout << item << " ";
    }
    std::cout << std::endl << "---" << std::endl;
}

ferrugo::iterable<int> collatz(int n)
{
    using State = std::tuple<int, int>;
    return ferrugo::unfold<int, State>(
        State{ n, n },
        [](const State& state) -> ferrugo::iteration_result<std::tuple<int, State>>
        {
            const int prev = std::get<0>(state);
            const int current = std::get<1>(state);

            if (prev == 1)
            {
                return ferrugo::stop_iteration;
            }

            const int next = current % 2 == 0 ? current / 2 : 3 * current + 1;

            return std::tuple<int, State>{ current, State{ current, next } };
        });
}

ferrugo::iterable<int> fibonacci()
{
    using State = std::tuple<int, int>;
    return ferrugo::unfold<int, State>(
        State{ 0, 1 },
        [=](const State& state) -> ferrugo::iteration_result<std::tuple<int, State>>
        {
            const int prev = std::get<0>(state);
            const int current = std::get<1>(state);

            const int next = prev + current;

            return std::tuple<int, State>{ prev, State{ current, next } };
        });
}

struct source_location
{
    std::string file;
    int line;
    std::string function;

    friend std::ostream& operator<<(std::ostream& os, const source_location& item)
    {
        return os << item.file << ":" << item.line << " in " << item.function;
    }
};

struct size
{
    template <class T>
    std::size_t operator()(const T& item) const
    {
        return std::distance(std::begin(item), std::end(item));
    }
};

void assert_impl(bool condition, const std::string& expression, const source_location& loc)
{
    if (condition)
    {
        return;
    }
    std::stringstream ss;
    ss << "Assertion failure: " << expression << " (" << loc << ")";
    throw std::runtime_error{ ss.str() };
}

#define ASSERT(...) assert_impl(__VA_ARGS__, #__VA_ARGS__, source_location{ __FILE__, __LINE__, __PRETTY_FUNCTION__ })

using ferrugo::index_sequence;
using ferrugo::index_sequence_for;
using ferrugo::invoke;

template <class Proj, class Func>
struct proj_fn
{
    Proj proj;
    Func func;

    template <class... Args>
    auto operator()(Args&&... args) const -> RETURN(invoke(proj, invoke(func, std::forward<Args>(args))...));
};

template <class Proj, class Func>
auto proj(Proj&& proj, Func&& func) -> proj_fn<typename std::decay<Proj>::type, typename std::decay<Func>::type>
{
    return { std::forward<Proj>(proj), std::forward<Func>(func) };
}

template <class... Fields>
struct tie_fn
{
    std::tuple<Fields...> fields;

    template <class T, std::size_t... I>
    auto call(T& item, index_sequence<I...>) const -> RETURN(std::tie(invoke(std::get<I>(fields), item)...));

    template <class T>
    auto operator()(T& item) const -> RETURN(call(item, index_sequence_for<Fields...>{}));
};

template <class... Fields>
auto tie(Fields... fields) -> tie_fn<Fields...>
{
    return { { std::move(fields)... } };
}

struct Person
{
    std::string name;
    int age;
};

struct to_string_fn
{
    template <class... Args>
    std::string operator()(const Args&... args) const
    {
        std::stringstream ss;
        const int tab[] = { (ss << args, 0)..., 0 };
        (void)tab;
        return ss.str();
    }
};

constexpr auto str = to_string_fn{};

void run()
{
    const auto a = Person{ "Adam", 10 };
    const auto b = Person{ "Betaa", 10 };
    const auto c = Person{ "Celina", 10 };

    ASSERT(a.age > 18);

    // std::cout << invoke(to_string_fn{}, tie(&Person::age, &Person::name)), a, b) << std::endl;
    std::cout << proj(str, tie(&Person::age, &Person::name))(a, b, c) << std::endl;
}

int main(int argc, char const* argv[])
{
    try
    {
        run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Error: "
                  << "UNKNOWN ERROR" << std::endl;
    }
}
