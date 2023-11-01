#include <ferrugo/iterable.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

template <class T>
void print(ferrugo::iterable<T> range)
{
    for (const auto& item : range)
    {
        std::cout << item << " ";
    }
    std::cout << std::endl << "---" << std::endl;
}

template <class T>
ferrugo::iterable<T> repeat(T value, int count)
{
    return ferrugo::unfold<T, int>(
        count,
        [=](int n) -> ferrugo::iteration_result<std::tuple<T, int>>
        {
            if (n == 0)
            {
                return ferrugo::stop_iteration;
            }
            return std::tuple<T, int>{ value, n - 1 };
        });
}

template <class T>
ferrugo::iterable<T> single(T value)
{
    return repeat(std::move(value), 1);
}

ferrugo::iterable<int> range(int lo, int up)
{
    return ferrugo::unfold<int, int>(
        lo,
        [=](int n) -> ferrugo::iteration_result<std::tuple<int, int>>
        {
            if (n == up)
            {
                return ferrugo::stop_iteration;
            }
            return std::tuple<int, int>{ n, n + 1 };
        });
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

ferrugo::iterable<int> fibonacci(int n)
{
    using State = std::tuple<int, int>;
    return ferrugo::unfold<int, State>(
        State{ 0, 1 },
        [=](const State& state) -> ferrugo::iteration_result<std::tuple<int, State>>
        {
            const int prev = std::get<0>(state);
            const int current = std::get<1>(state);

            if (prev > n)
            {
                return ferrugo::stop_iteration;
            }

            const int next = prev + current;

            return std::tuple<int, State>{ prev, State{ current, next } };
        });
}

void run()
{
    ferrugo::iterable<ferrugo::iterable<int>> f{ std::vector<ferrugo::iterable<int>>{
        range(0, 3), repeat(-1, 5), ferrugo::empty<int>(), range(1000, 1005), collatz(12), single(-1), fibonacci(100) } };

    print(f.join());
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
