#include <ferrugo/either.hpp>
#include <ferrugo/iterable.hpp>
#include <ferrugo/optional.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

template <std::size_t I, std::size_t N>
struct tag_t
{
};

template <class Tuple, std::size_t N>
void print_tuple(std::ostream& os, const Tuple& tuple, tag_t<N, N>)
{
}

template <class Tuple, std::size_t I, std::size_t N>
void print_tuple(std::ostream& os, const Tuple& tuple, tag_t<I, N>)
{
    os << (I != 0 ? ", " : "") << std::get<I>(tuple);
    print_tuple(os, tuple, tag_t<I + 1, N>{});
}

namespace std
{

template <class... Args>
ostream& operator<<(ostream& os, const tuple<Args...>& item)
{
    os << "(";
    print_tuple(os, item, tag_t<0, sizeof...(Args)>{});
    os << ")";
    return os;
}

template <class F, class S>
ostream& operator<<(ostream& os, const pair<F, S>& item)
{
    os << "(";
    print_tuple(os, item, tag_t<0, 2>{});
    os << ")";
    return os;
}

}  // namespace std

#if 0
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
#endif

struct to_string
{
    template <class T>
    std::string operator()(const T& item) const
    {
        std::stringstream ss;
        ss << item;
        return ss.str();
    }
};

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

void run()
{
    ferrugo::either<int, std::string> e = ferrugo::right("AAA");
    e.match([](int x) { std::cout << x << std::endl; }, [](const std::string& x) { std::cout << x << std::endl; });
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
