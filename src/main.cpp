#include <ferrugo/iterable.hpp>
#include <iostream>
#include <stdexcept>
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

template <class T>
void print(ferrugo::iterable<T> range)
{
    for (const auto& item : range)
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

void run()
{
    ferrugo::iterable<ferrugo::iterable<int>> f{ std::vector<ferrugo::iterable<int>>{ ferrugo::range(0, 3),
                                                                                      ferrugo::repeat(-1, 5),
                                                                                      ferrugo::empty<int>(),
                                                                                      ferrugo::range(1000, 1005),
                                                                                      collatz(12),
                                                                                      ferrugo::single(-1) } };

    print(fibonacci().take_while([](int x) { return x < 100; }).enumerate(0));
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
